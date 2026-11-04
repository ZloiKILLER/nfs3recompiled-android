#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_hud_editor.py): defined in nfs3hp_main.cpp.
void hudLayoutDesigned(win32::WinApplication* app, x86::reg32 rect, x86::reg32 element);
void hudLayoutsDrawn(win32::WinApplication* app, x86::CPU& cpu, bool drawn);
x86::sreg32 hudDragSlack(win32::WinApplication* app, x86::reg32 object, x86::reg32 side);

/* sub_458b80: native, nfs3hp::port::fields::floatFieldKey */

/* sub_458ce0: native, nfs3hp::port::fields::layoutFloatField */

/* sub_458d90: native, nfs3hp::port::fields::floatFieldShown */

/* sub_458dd0: native, nfs3hp::port::fields::floatFieldHidden */

/* sub_458e00: native, nfs3hp::port::fields::bindFloatField */

/* sub_458e30: native, nfs3hp::port::fields::drawFixedField */

/* sub_459020: native, nfs3hp::port::fields::fixedRangeMessage */

/* sub_459090: native, nfs3hp::port::fields::fixedFieldKey */

/* sub_459210: native, nfs3hp::port::fields::layoutFixedField */

/* sub_4592b0: native, nfs3hp::port::fields::fixedFieldShown */

/* sub_4592f0: native, nfs3hp::port::fields::fixedFieldHidden */

/* sub_459320: native, nfs3hp::port::fields::bindFixedField */

/* sub_459350: native, nfs3hp::graphics::drawFlatRectangle */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_459410(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00459410  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00459411  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00459412  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00459413  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00459415  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00459416  8b4864                 -mov ecx, dword ptr [eax + 0x64]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */);
    // 00459419  8b5860                 -mov ebx, dword ptr [eax + 0x60]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(96) /* 0x60 */);
    // 0045941c  8b5006                 -mov edx, dword ptr [eax + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0045941f  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00459422  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00459425  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00459428  e823ffffff             -call 0x459350
    cpu.esp -= 4;
    nfs3hp::graphics::drawFlatRectangle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045942d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045942e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045942f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459430  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_459440: native, nfs3hp::graphics::hudEditorFrame */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_459670(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2;
  x86::reg16 fsw = 0;
    // 00459670  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00459671  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00459672  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00459673  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00459675  81ec00010000           -sub esp, 0x100
    (cpu.esp) -= x86::reg32(x86::sreg32(256 /*0x100*/));
    // 0045967b  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 00459681  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00459683  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 00459685  894d36                 -mov dword ptr [ebp + 0x36], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(54) /* 0x36 */) = cpu.ecx;
    // 00459688  8b0db0d36f00           -mov ecx, dword ptr [0x6fd3b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0045968e  83f901                 +cmp ecx, 1
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
    // 00459691  7504                   -jne 0x459697
    if (!cpu.flags.zf)
    {
        goto L_0x00459697;
    }
    // 00459693  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 00459695  eb02                   -jmp 0x459699
    goto L_0x00459699;
L_0x00459697:
    // 00459697  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x00459699:
    // 00459699  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0045969b  e83095fcff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004596a0  01d3                   -add ebx, edx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004596a2  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 004596a9  7407                   -je 0x4596b2
    if (cpu.flags.zf)
    {
        goto L_0x004596b2;
    }
    // 004596ab  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004596b0  eb02                   -jmp 0x4596b4
    goto L_0x004596b4;
L_0x004596b2:
    // 004596b2  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
L_0x004596b4:
    // 004596b4  d9ee                   +fldz 
    fp0 = 0.0;
    // 004596b6  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004596b9  d81e                   +fcomp dword ptr [esi]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.esi)));
    // 004596bb  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004596bd  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004596be  7741                   -ja 0x459701
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00459701;
    }
    // 004596c0  d9ee                   +fldz 
    fp0 = 0.0;
    // 004596c2  d85e08                 +fcomp dword ptr [esi + 8]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */)));
    // 004596c5  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004596c7  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004596c8  7737                   -ja 0x459701
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00459701;
    }
    // 004596ca  d9ee                   +fldz 
    fp0 = 0.0;
    // 004596cc  d85e04                 +fcomp dword ptr [esi + 4]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(4) /* 0x4 */)));
    // 004596cf  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004596d1  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004596d2  772d                   -ja 0x459701
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00459701;
    }
    // 004596d4  d9ee                   +fldz 
    fp0 = 0.0;
    // 004596d6  d85e0c                 +fcomp dword ptr [esi + 0xc]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(12) /* 0xc */)));
    // 004596d9  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004596db  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004596dc  7723                   -ja 0x459701
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00459701;
    }
    // 004596de  813e0000803f           +cmp dword ptr [esi], 0x3f800000
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1065353216 /*0x3f800000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004596e4  7f1b                   -jg 0x459701
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00459701;
    }
    // 004596e6  817e080000803f         +cmp dword ptr [esi + 8], 0x3f800000
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1065353216 /*0x3f800000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004596ed  7f12                   -jg 0x459701
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00459701;
    }
    // 004596ef  817e040000803f         +cmp dword ptr [esi + 4], 0x3f800000
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1065353216 /*0x3f800000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004596f6  7f09                   -jg 0x459701
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00459701;
    }
    // 004596f8  817e0c0000803f         +cmp dword ptr [esi + 0xc], 0x3f800000
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1065353216 /*0x3f800000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004596ff  7e07                   -jle 0x459708
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00459708;
    }
L_0x00459701:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00459701  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00459703  e993060000             -jmp 0x459d9b
    goto L_0x00459d9b;
L_0x00459708:
    // 00459708  8b15b0d36f00           -mov edx, dword ptr [0x6fd3b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0045970e  83fa01                 +cmp edx, 1
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
    // 00459711  7521                   -jne 0x459734
    if (!cpu.flags.zf)
    {
        goto L_0x00459734;
    }
    // 00459713  39d3                   +cmp ebx, edx
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459715  7510                   -jne 0x459727
    if (!cpu.flags.zf)
    {
        goto L_0x00459727;
    }
    // 00459717  817e080000003f         +cmp dword ptr [esi + 8], 0x3f000000
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1056964608 /*0x3f000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045971e  7e14                   -jle 0x459734
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00459734;
    }
    // 00459720  83ff08                 +cmp edi, 8
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459723  75dc                   -jne 0x459701
    if (!cpu.flags.zf)
    {
        goto L_0x00459701;
    }
    // 00459725  eb0d                   -jmp 0x459734
    goto L_0x00459734;
L_0x00459727:
    // 00459727  813e0000003f           +cmp dword ptr [esi], 0x3f000000
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1056964608 /*0x3f000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045972d  7d05                   -jge 0x459734
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00459734;
    }
    // 0045972f  83ff08                 +cmp edi, 8
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459732  75cd                   -jne 0x459701
    if (!cpu.flags.zf)
    {
        goto L_0x00459701;
    }
L_0x00459734:
    // 00459734  83ff11                 +cmp edi, 0x11
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(17 /*0x11*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459737  0f8459060000           -je 0x459d96
    if (cpu.flags.zf)
    {
        goto L_0x00459d96;
    }
    // 0045973d  83ff16                 +cmp edi, 0x16
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(22 /*0x16*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459740  0f8450060000           -je 0x459d96
    if (cpu.flags.zf)
    {
        goto L_0x00459d96;
    }
    // 00459746  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00459748  e93a060000             -jmp 0x459d87
    goto L_0x00459d87;
L_0x0045974d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0045974d  83fa16                 +cmp edx, 0x16
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(22 /*0x16*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459750  0f842b060000           -je 0x459d81
    if (cpu.flags.zf)
    {
        goto L_0x00459d81;
    }
    // 00459756  83ff08                 +cmp edi, 8
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459759  0f8422060000           -je 0x459d81
    if (cpu.flags.zf)
    {
        goto L_0x00459d81;
    }
    // 0045975f  83fa08                 +cmp edx, 8
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
    // 00459762  0f8419060000           -je 0x459d81
    if (cpu.flags.zf)
    {
        goto L_0x00459d81;
    }
    // 00459768  83fa09                 +cmp edx, 9
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(9 /*0x9*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045976b  750d                   -jne 0x45977a
    if (!cpu.flags.zf)
    {
        goto L_0x0045977a;
    }
    // 0045976d  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 00459774  0f8407060000           -je 0x459d81
    if (cpu.flags.zf)
    {
        goto L_0x00459d81;
    }
L_0x0045977a:
    // 0045977a  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0045977c  7509                   -jne 0x459787
    if (!cpu.flags.zf)
    {
        goto L_0x00459787;
    }
    // 0045977e  83fa01                 +cmp edx, 1
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
    // 00459781  0f84fa050000           -je 0x459d81
    if (cpu.flags.zf)
    {
        goto L_0x00459d81;
    }
L_0x00459787:
    // 00459787  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00459789  7509                   -jne 0x459794
    if (!cpu.flags.zf)
    {
        goto L_0x00459794;
    }
    // 0045978b  83ff01                 +cmp edi, 1
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045978e  0f84ed050000           -je 0x459d81
    if (cpu.flags.zf)
    {
        goto L_0x00459d81;
    }
L_0x00459794:
    // 00459794  83ff02                 +cmp edi, 2
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459797  7509                   -jne 0x4597a2
    if (!cpu.flags.zf)
    {
        goto L_0x004597a2;
    }
    // 00459799  83fa03                 +cmp edx, 3
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
    // 0045979c  0f84df050000           -je 0x459d81
    if (cpu.flags.zf)
    {
        goto L_0x00459d81;
    }
L_0x004597a2:
    // 004597a2  83fa02                 +cmp edx, 2
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
    // 004597a5  7509                   -jne 0x4597b0
    if (!cpu.flags.zf)
    {
        goto L_0x004597b0;
    }
    // 004597a7  83ff03                 +cmp edi, 3
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004597aa  0f84d1050000           -je 0x459d81
    if (cpu.flags.zf)
    {
        goto L_0x00459d81;
    }
L_0x004597b0:
    // 004597b0  83fa0a                 +cmp edx, 0xa
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(10 /*0xa*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004597b3  7c28                   -jl 0x4597dd
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004597dd;
    }
    // 004597b5  837d3600               +cmp dword ptr [ebp + 0x36], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(54) /* 0x36 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004597b9  740a                   -je 0x4597c5
    if (cpu.flags.zf)
    {
        goto L_0x004597c5;
    }
    // 004597bb  83fa0a                 +cmp edx, 0xa
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(10 /*0xa*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004597be  7c05                   -jl 0x4597c5
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004597c5;
    }
    // 004597c0  83fa12                 +cmp edx, 0x12
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18 /*0x12*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004597c3  7c18                   -jl 0x4597dd
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004597dd;
    }
L_0x004597c5:
    // 004597c5  837d3600               +cmp dword ptr [ebp + 0x36], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(54) /* 0x36 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004597c9  0f85b2050000           -jne 0x459d81
    if (!cpu.flags.zf)
    {
        goto L_0x00459d81;
    }
    // 004597cf  83fa0a                 +cmp edx, 0xa
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(10 /*0xa*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004597d2  7c09                   -jl 0x4597dd
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004597dd;
    }
    // 004597d4  83fa12                 +cmp edx, 0x12
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18 /*0x12*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004597d7  0f8ca4050000           -jl 0x459d81
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00459d81;
    }
L_0x004597dd:
    // 004597dd  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 004597e3  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004597e6  8945da                 -mov dword ptr [ebp - 0x26], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-38) /* -0x26 */) = cpu.eax;
    // 004597e9  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 004597ef  8b4dda                 -mov ecx, dword ptr [ebp - 0x26]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-38) /* -0x26 */);
    // 004597f2  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004597f4  894d8a                 -mov dword ptr [ebp - 0x76], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-118) /* -0x76 */) = cpu.ecx;
    // 004597f7  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004597f9  8b458a                 -mov eax, dword ptr [ebp - 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-118) /* -0x76 */);
    // 004597fc  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 004597ff  01c1                   +add ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00459801  d906                   +fld dword ptr [esi]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi));
    // 00459803  d89974bc6f00           +fcomp dword ptr [ecx + 0x6fbc74]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322740) /* 0x6fbc74 */)));
    // 00459809  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 0045980b  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0045980c  7604                   -jbe 0x459812
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00459812;
    }
    // 0045980e  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00459810  eb06                   -jmp 0x459818
    goto L_0x00459818;
L_0x00459812:
    // 00459812  8b8174bc6f00           -mov eax, dword ptr [ecx + 0x6fbc74]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322740) /* 0x6fbc74 */);
L_0x00459818:
    // 00459818  894512                 -mov dword ptr [ebp + 0x12], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(18) /* 0x12 */) = cpu.eax;
    // 0045981b  69cb48030000           -imul ecx, ebx, 0x348
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00459821  8b4512                 -mov eax, dword ptr [ebp + 0x12]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(18) /* 0x12 */);
    // 00459824  8945c6                 -mov dword ptr [ebp - 0x3a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-58) /* -0x3a */) = cpu.eax;
    // 00459827  69457ea4010000         -imul eax, dword ptr [ebp + 0x7e], 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */))) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0045982e  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00459830  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00459832  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 00459835  01c1                   +add ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00459837  d94608                 +fld dword ptr [esi + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */));
    // 0045983a  d8997cbc6f00           +fcomp dword ptr [ecx + 0x6fbc7c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */)));
    // 00459840  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459842  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459843  7305                   -jae 0x45984a
    if (!cpu.flags.cf)
    {
        goto L_0x0045984a;
    }
    // 00459845  8b4608                 -mov eax, dword ptr [esi + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 00459848  eb06                   -jmp 0x459850
    goto L_0x00459850;
L_0x0045984a:
    // 0045984a  8b817cbc6f00           -mov eax, dword ptr [ecx + 0x6fbc7c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */);
L_0x00459850:
    // 00459850  894516                 -mov dword ptr [ebp + 0x16], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(22) /* 0x16 */) = cpu.eax;
    // 00459853  8b4516                 -mov eax, dword ptr [ebp + 0x16]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(22) /* 0x16 */);
    // 00459856  8945ca                 -mov dword ptr [ebp - 0x36], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-54) /* -0x36 */) = cpu.eax;
    // 00459859  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 0045985f  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00459862  8945c2                 -mov dword ptr [ebp - 0x3e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-62) /* -0x3e */) = cpu.eax;
    // 00459865  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0045986b  8b4dc2                 -mov ecx, dword ptr [ebp - 0x3e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-62) /* -0x3e */);
    // 0045986e  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00459870  894da6                 -mov dword ptr [ebp - 0x5a], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-90) /* -0x5a */) = cpu.ecx;
    // 00459873  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00459875  8b45a6                 -mov eax, dword ptr [ebp - 0x5a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-90) /* -0x5a */);
    // 00459878  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 0045987b  01c1                   +add ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0045987d  d94604                 +fld dword ptr [esi + 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(4) /* 0x4 */));
    // 00459880  d89978bc6f00           +fcomp dword ptr [ecx + 0x6fbc78]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322744) /* 0x6fbc78 */)));
    // 00459886  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459888  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459889  7605                   -jbe 0x459890
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00459890;
    }
    // 0045988b  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0045988e  eb06                   -jmp 0x459896
    goto L_0x00459896;
L_0x00459890:
    // 00459890  8b8178bc6f00           -mov eax, dword ptr [ecx + 0x6fbc78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322744) /* 0x6fbc78 */);
L_0x00459896:
    // 00459896  8945e2                 -mov dword ptr [ebp - 0x1e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-30) /* -0x1e */) = cpu.eax;
    // 00459899  8b45e2                 -mov eax, dword ptr [ebp - 0x1e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-30) /* -0x1e */);
    // 0045989c  8945ba                 -mov dword ptr [ebp - 0x46], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-70) /* -0x46 */) = cpu.eax;
    // 0045989f  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 004598a5  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004598a8  894592                 -mov dword ptr [ebp - 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-110) /* -0x6e */) = cpu.eax;
    // 004598ab  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 004598b1  034592                 -add eax, dword ptr [ebp - 0x6e]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-110) /* -0x6e */)));
    // 004598b4  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004598b6  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 004598b9  01c1                   +add ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004598bb  d9460c                 +fld dword ptr [esi + 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(12) /* 0xc */));
    // 004598be  d89980bc6f00           +fcomp dword ptr [ecx + 0x6fbc80]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322752) /* 0x6fbc80 */)));
    // 004598c4  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004598c6  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004598c7  7305                   -jae 0x4598ce
    if (!cpu.flags.cf)
    {
        goto L_0x004598ce;
    }
    // 004598c9  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004598cc  eb06                   -jmp 0x4598d4
    goto L_0x004598d4;
L_0x004598ce:
    // 004598ce  8b8180bc6f00           -mov eax, dword ptr [ecx + 0x6fbc80]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322752) /* 0x6fbc80 */);
L_0x004598d4:
    // 004598d4  8945f6                 -mov dword ptr [ebp - 0xa], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-10) /* -0xa */) = cpu.eax;
    // 004598d7  8b45f6                 -mov eax, dword ptr [ebp - 0xa]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 004598da  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004598dd  8945d2                 -mov dword ptr [ebp - 0x2e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-46) /* -0x2e */) = cpu.eax;
    // 004598e0  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 004598e6  69cb48030000           -imul ecx, ebx, 0x348
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 004598ec  89459a                 -mov dword ptr [ebp - 0x66], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-102) /* -0x66 */) = cpu.eax;
    // 004598ef  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004598f1  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004598f4  8945be                 -mov dword ptr [ebp - 0x42], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-66) /* -0x42 */) = cpu.eax;
    // 004598f7  034d9a                 -add ecx, dword ptr [ebp - 0x66]
    (cpu.ecx) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-102) /* -0x66 */)));
    // 004598fa  8b45be                 -mov eax, dword ptr [ebp - 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-66) /* -0x42 */);
    // 004598fd  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004598ff  894562                 -mov dword ptr [ebp + 0x62], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */) = cpu.eax;
    // 00459902  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00459904  0f8532010000           -jne 0x459a3c
    if (!cpu.flags.zf)
    {
        goto L_0x00459a3c;
    }
    // 0045990a  8b8178bc6f00           -mov eax, dword ptr [ecx + 0x6fbc78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322744) /* 0x6fbc78 */);
    // 00459910  894586                 -mov dword ptr [ebp - 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-122) /* -0x7a */) = cpu.eax;
    // 00459913  8b8180bc6f00           -mov eax, dword ptr [ecx + 0x6fbc80]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322752) /* 0x6fbc80 */);
    // 00459919  89456a                 -mov dword ptr [ebp + 0x6a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */) = cpu.eax;
    // 0045991c  d9817cbc6f00           +fld dword ptr [ecx + 0x6fbc7c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */));
    // 00459922  d9456a                 +fld dword ptr [ebp + 0x6a]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */));
    // 00459925  d86586                 +fsub dword ptr [ebp - 0x7a]
    fp1 = fp::sub(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-122) /* -0x7a */)));
    // 00459928  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0045992a  d881ccbc6f00           +fadd dword ptr [ecx + 0x6fbccc]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322828) /* 0x6fbccc */)));
    // 00459930  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00459932  dc0dac9b5300           +fmul qword ptr [0x539bac]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5479340) /* 0x539bac */)));
    // 00459938  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0045993a  d8a1c4bc6f00           +fsub dword ptr [ecx + 0x6fbcc4]
    fp1 = fp::sub(fp1, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322820) /* 0x6fbcc4 */)));
    // 00459940  d9817cbc6f00           +fld dword ptr [ecx + 0x6fbc7c]
    fp2 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */));
    // 00459946  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00459948  d95d42                 +fstp dword ptr [ebp + 0x42]
    app->getMemory<float>(cpu.ebp + x86::reg32(66) /* 0x42 */) = float(fp2);
    // 0045994b  8b4562                 -mov eax, dword ptr [ebp + 0x62]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 0045994e  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00459950  d95db6                 +fstp dword ptr [ebp - 0x4a]
    app->getMemory<float>(cpu.ebp + x86::reg32(-74) /* -0x4a */) = float(fp1);
    // 00459953  d89874bc6f00           +fcomp dword ptr [eax + 0x6fbc74]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322740) /* 0x6fbc74 */)));
    // 00459959  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 0045995b  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0045995c  7608                   -jbe 0x459966
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00459966;
    }
    // 0045995e  8b817cbc6f00           -mov eax, dword ptr [ecx + 0x6fbc7c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */);
    // 00459964  eb09                   -jmp 0x45996f
    goto L_0x0045996f;
L_0x00459966:
    // 00459966  8b4562                 -mov eax, dword ptr [ebp + 0x62]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 00459969  8b8074bc6f00           -mov eax, dword ptr [eax + 0x6fbc74]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7322740) /* 0x6fbc74 */);
L_0x0045996f:
    // 0045996f  894546                 -mov dword ptr [ebp + 0x46], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */) = cpu.eax;
    // 00459972  694d7ea4010000         -imul ecx, dword ptr [ebp + 0x7e], 0x1a4
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */))) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00459979  8b4546                 -mov eax, dword ptr [ebp + 0x46]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 0045997c  89456e                 -mov dword ptr [ebp + 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.eax;
    // 0045997f  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00459985  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00459987  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00459989  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 0045998c  01c1                   +add ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0045998e  d94542                 +fld dword ptr [ebp + 0x42]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(66) /* 0x42 */));
    // 00459991  d8997cbc6f00           +fcomp dword ptr [ecx + 0x6fbc7c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */)));
    // 00459997  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459999  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0045999a  7305                   -jae 0x4599a1
    if (!cpu.flags.cf)
    {
        goto L_0x004599a1;
    }
    // 0045999c  8b4542                 -mov eax, dword ptr [ebp + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */);
    // 0045999f  eb06                   -jmp 0x4599a7
    goto L_0x004599a7;
L_0x004599a1:
    // 004599a1  8b817cbc6f00           -mov eax, dword ptr [ecx + 0x6fbc7c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */);
L_0x004599a7:
    // 004599a7  89453e                 -mov dword ptr [ebp + 0x3e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */) = cpu.eax;
    // 004599aa  8b453e                 -mov eax, dword ptr [ebp + 0x3e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */);
    // 004599ad  894576                 -mov dword ptr [ebp + 0x76], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.eax;
    // 004599b0  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 004599b6  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004599b9  8945b2                 -mov dword ptr [ebp - 0x4e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-78) /* -0x4e */) = cpu.eax;
    // 004599bc  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 004599c2  8b4db2                 -mov ecx, dword ptr [ebp - 0x4e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-78) /* -0x4e */);
    // 004599c5  d9456a                 -fld dword ptr [ebp + 0x6a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */));
    // 004599c8  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004599ca  d865b6                 -fsub dword ptr [ebp - 0x4a]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-74) /* -0x4a */)));
    // 004599cd  894daa                 -mov dword ptr [ebp - 0x56], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-86) /* -0x56 */) = cpu.ecx;
    // 004599d0  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004599d2  8b45aa                 -mov eax, dword ptr [ebp - 0x56]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-86) /* -0x56 */);
    // 004599d5  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 004599d8  d95d2e                 -fstp dword ptr [ebp + 0x2e]
    app->getMemory<float>(cpu.ebp + x86::reg32(46) /* 0x2e */) = float(fp0);
    // 004599db  01c1                   +add ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004599dd  d9452e                 +fld dword ptr [ebp + 0x2e]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(46) /* 0x2e */));
    // 004599e0  d89978bc6f00           +fcomp dword ptr [ecx + 0x6fbc78]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322744) /* 0x6fbc78 */)));
    // 004599e6  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004599e8  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004599e9  7605                   -jbe 0x4599f0
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004599f0;
    }
    // 004599eb  8b452e                 -mov eax, dword ptr [ebp + 0x2e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(46) /* 0x2e */);
    // 004599ee  eb06                   -jmp 0x4599f6
    goto L_0x004599f6;
L_0x004599f0:
    // 004599f0  8b8178bc6f00           -mov eax, dword ptr [ecx + 0x6fbc78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322744) /* 0x6fbc78 */);
L_0x004599f6:
    // 004599f6  89454a                 -mov dword ptr [ebp + 0x4a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */) = cpu.eax;
    // 004599f9  694d7ea4010000         -imul ecx, dword ptr [ebp + 0x7e], 0x1a4
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */))) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00459a00  8b454a                 -mov eax, dword ptr [ebp + 0x4a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 00459a03  894572                 -mov dword ptr [ebp + 0x72], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.eax;
    // 00459a06  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00459a0c  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00459a0e  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00459a10  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 00459a13  01c1                   +add ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00459a15  d9456a                 +fld dword ptr [ebp + 0x6a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */));
    // 00459a18  d89980bc6f00           +fcomp dword ptr [ecx + 0x6fbc80]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322752) /* 0x6fbc80 */)));
    // 00459a1e  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459a20  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459a21  7305                   -jae 0x459a28
    if (!cpu.flags.cf)
    {
        goto L_0x00459a28;
    }
    // 00459a23  8b456a                 -mov eax, dword ptr [ebp + 0x6a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 00459a26  eb06                   -jmp 0x459a2e
    goto L_0x00459a2e;
L_0x00459a28:
    // 00459a28  8b8180bc6f00           -mov eax, dword ptr [ecx + 0x6fbc80]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322752) /* 0x6fbc80 */);
L_0x00459a2e:
    // 00459a2e  894552                 -mov dword ptr [ebp + 0x52], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */) = cpu.eax;
    // 00459a31  8b4552                 -mov eax, dword ptr [ebp + 0x52]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00459a34  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 00459a37  e9b7020000             -jmp 0x459cf3
    goto L_0x00459cf3;
L_0x00459a3c:
    // 00459a3c  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00459a3e  0f85c4000000           -jne 0x459b08
    if (!cpu.flags.zf)
    {
        goto L_0x00459b08;
    }
    // 00459a44  8b8178bc6f00           -mov eax, dword ptr [ecx + 0x6fbc78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322744) /* 0x6fbc78 */);
    // 00459a4a  894582                 -mov dword ptr [ebp - 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-126) /* -0x7e */) = cpu.eax;
    // 00459a4d  8b8180bc6f00           -mov eax, dword ptr [ecx + 0x6fbc80]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322752) /* 0x6fbc80 */);
    // 00459a53  894566                 -mov dword ptr [ebp + 0x66], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */) = cpu.eax;
    // 00459a56  d9817cbc6f00           +fld dword ptr [ecx + 0x6fbc7c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */));
    // 00459a5c  d94566                 +fld dword ptr [ebp + 0x66]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(102) /* 0x66 */));
    // 00459a5f  d86582                 +fsub dword ptr [ebp - 0x7e]
    fp1 = fp::sub(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-126) /* -0x7e */)));
    // 00459a62  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00459a64  d881ccbc6f00           +fadd dword ptr [ecx + 0x6fbccc]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322828) /* 0x6fbccc */)));
    // 00459a6a  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00459a6c  dc0dac9b5300           +fmul qword ptr [0x539bac]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5479340) /* 0x539bac */)));
    // 00459a72  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00459a74  d8a1c4bc6f00           +fsub dword ptr [ecx + 0x6fbcc4]
    fp1 = fp::sub(fp1, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322820) /* 0x6fbcc4 */)));
    // 00459a7a  d906                   +fld dword ptr [esi]
    fp2 = x86::Float(app->getMemory<float>(cpu.esi));
    // 00459a7c  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00459a7e  d95d3a                 +fstp dword ptr [ebp + 0x3a]
    app->getMemory<float>(cpu.ebp + x86::reg32(58) /* 0x3a */) = float(fp2);
    // 00459a81  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00459a83  d95da2                 +fstp dword ptr [ebp - 0x5e]
    app->getMemory<float>(cpu.ebp + x86::reg32(-94) /* -0x5e */) = float(fp1);
    // 00459a86  d8997cbc6f00           +fcomp dword ptr [ecx + 0x6fbc7c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */)));
    // 00459a8c  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459a8e  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459a8f  7604                   -jbe 0x459a95
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00459a95;
    }
    // 00459a91  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00459a93  eb06                   -jmp 0x459a9b
    goto L_0x00459a9b;
L_0x00459a95:
    // 00459a95  8b817cbc6f00           -mov eax, dword ptr [ecx + 0x6fbc7c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */);
L_0x00459a9b:
    // 00459a9b  894532                 -mov dword ptr [ebp + 0x32], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(50) /* 0x32 */) = cpu.eax;
    // 00459a9e  8b4532                 -mov eax, dword ptr [ebp + 0x32]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(50) /* 0x32 */);
    // 00459aa1  d94608                 +fld dword ptr [esi + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */));
    // 00459aa4  89456e                 -mov dword ptr [ebp + 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.eax;
    // 00459aa7  d85d3a                 +fcomp dword ptr [ebp + 0x3a]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(58) /* 0x3a */)));
    // 00459aaa  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459aac  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459aad  7305                   -jae 0x459ab4
    if (!cpu.flags.cf)
    {
        goto L_0x00459ab4;
    }
    // 00459aaf  8b4608                 -mov eax, dword ptr [esi + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 00459ab2  eb03                   -jmp 0x459ab7
    goto L_0x00459ab7;
L_0x00459ab4:
    // 00459ab4  8b453a                 -mov eax, dword ptr [ebp + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(58) /* 0x3a */);
L_0x00459ab7:
    // 00459ab7  8945ee                 -mov dword ptr [ebp - 0x12], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-18) /* -0x12 */) = cpu.eax;
    // 00459aba  d94566                 +fld dword ptr [ebp + 0x66]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(102) /* 0x66 */));
    // 00459abd  d94604                 +fld dword ptr [esi + 4]
    fp1 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(4) /* 0x4 */));
    // 00459ac0  8b45ee                 -mov eax, dword ptr [ebp - 0x12]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-18) /* -0x12 */);
    // 00459ac3  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00459ac5  d865a2                 +fsub dword ptr [ebp - 0x5e]
    fp1 = fp::sub(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-94) /* -0x5e */)));
    // 00459ac8  894576                 -mov dword ptr [ebp + 0x76], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.eax;
    // 00459acb  d95dfe                 +fstp dword ptr [ebp - 2]
    app->getMemory<float>(cpu.ebp + x86::reg32(-2) /* -0x2 */) = float(fp1);
    // 00459ace  d85dfe                 +fcomp dword ptr [ebp - 2]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-2) /* -0x2 */)));
    // 00459ad1  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459ad3  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459ad4  7605                   -jbe 0x459adb
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00459adb;
    }
    // 00459ad6  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00459ad9  eb03                   -jmp 0x459ade
    goto L_0x00459ade;
L_0x00459adb:
    // 00459adb  8b45fe                 -mov eax, dword ptr [ebp - 2]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-2) /* -0x2 */);
L_0x00459ade:
    // 00459ade  89454e                 -mov dword ptr [ebp + 0x4e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */) = cpu.eax;
    // 00459ae1  8b454e                 -mov eax, dword ptr [ebp + 0x4e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */);
    // 00459ae4  d9460c                 +fld dword ptr [esi + 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(12) /* 0xc */));
    // 00459ae7  894572                 -mov dword ptr [ebp + 0x72], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.eax;
    // 00459aea  d85d66                 +fcomp dword ptr [ebp + 0x66]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(102) /* 0x66 */)));
    // 00459aed  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459aef  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459af0  7305                   -jae 0x459af7
    if (!cpu.flags.cf)
    {
        goto L_0x00459af7;
    }
    // 00459af2  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 00459af5  eb03                   -jmp 0x459afa
    goto L_0x00459afa;
L_0x00459af7:
    // 00459af7  8b4566                 -mov eax, dword ptr [ebp + 0x66]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */);
L_0x00459afa:
    // 00459afa  8945de                 -mov dword ptr [ebp - 0x22], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-34) /* -0x22 */) = cpu.eax;
    // 00459afd  8b45de                 -mov eax, dword ptr [ebp - 0x22]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-34) /* -0x22 */);
    // 00459b00  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 00459b03  e9eb010000             -jmp 0x459cf3
    goto L_0x00459cf3;
L_0x00459b08:
    // 00459b08  83ff01                 +cmp edi, 1
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459b0b  0f852e010000           -jne 0x459c3f
    if (!cpu.flags.zf)
    {
        goto L_0x00459c3f;
    }
    // 00459b11  8b8188bc6f00           -mov eax, dword ptr [ecx + 0x6fbc88]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322760) /* 0x6fbc88 */);
    // 00459b17  8945fa                 -mov dword ptr [ebp - 6], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.eax;
    // 00459b1a  8b8190bc6f00           -mov eax, dword ptr [ecx + 0x6fbc90]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322768) /* 0x6fbc90 */);
    // 00459b20  894556                 -mov dword ptr [ebp + 0x56], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */) = cpu.eax;
    // 00459b23  8b818cbc6f00           -mov eax, dword ptr [ecx + 0x6fbc8c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322764) /* 0x6fbc8c */);
    // 00459b29  89455e                 -mov dword ptr [ebp + 0x5e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */) = cpu.eax;
    // 00459b2c  d9455e                 +fld dword ptr [ebp + 0x5e]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(94) /* 0x5e */));
    // 00459b2f  d881ccbc6f00           +fadd dword ptr [ecx + 0x6fbccc]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322828) /* 0x6fbccc */)));
    // 00459b35  d9455e                 +fld dword ptr [ebp + 0x5e]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(94) /* 0x5e */));
    // 00459b38  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00459b3a  d8a1c4bc6f00           +fsub dword ptr [ecx + 0x6fbcc4]
    fp1 = fp::sub(fp1, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322820) /* 0x6fbcc4 */)));
    // 00459b40  8b4562                 -mov eax, dword ptr [ebp + 0x62]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 00459b43  d95d02                 +fstp dword ptr [ebp + 2]
    app->getMemory<float>(cpu.ebp + x86::reg32(2) /* 0x2 */) = float(fp1);
    // 00459b46  d89874bc6f00           +fcomp dword ptr [eax + 0x6fbc74]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322740) /* 0x6fbc74 */)));
    // 00459b4c  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459b4e  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459b4f  7605                   -jbe 0x459b56
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00459b56;
    }
    // 00459b51  8b455e                 -mov eax, dword ptr [ebp + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 00459b54  eb09                   -jmp 0x459b5f
    goto L_0x00459b5f;
L_0x00459b56:
    // 00459b56  8b4562                 -mov eax, dword ptr [ebp + 0x62]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 00459b59  8b8074bc6f00           -mov eax, dword ptr [eax + 0x6fbc74]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7322740) /* 0x6fbc74 */);
L_0x00459b5f:
    // 00459b5f  89451e                 -mov dword ptr [ebp + 0x1e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(30) /* 0x1e */) = cpu.eax;
    // 00459b62  8b451e                 -mov eax, dword ptr [ebp + 0x1e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(30) /* 0x1e */);
    // 00459b65  89456e                 -mov dword ptr [ebp + 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.eax;
    // 00459b68  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00459b6e  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00459b71  8945ce                 -mov dword ptr [ebp - 0x32], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-50) /* -0x32 */) = cpu.eax;
    // 00459b74  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00459b7a  8b4dce                 -mov ecx, dword ptr [ebp - 0x32]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-50) /* -0x32 */);
    // 00459b7d  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00459b7f  894d9e                 -mov dword ptr [ebp - 0x62], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-98) /* -0x62 */) = cpu.ecx;
    // 00459b82  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00459b84  8b459e                 -mov eax, dword ptr [ebp - 0x62]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-98) /* -0x62 */);
    // 00459b87  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 00459b8a  01c1                   +add ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00459b8c  d94502                 +fld dword ptr [ebp + 2]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(2) /* 0x2 */));
    // 00459b8f  d8997cbc6f00           +fcomp dword ptr [ecx + 0x6fbc7c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */)));
    // 00459b95  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459b97  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459b98  7305                   -jae 0x459b9f
    if (!cpu.flags.cf)
    {
        goto L_0x00459b9f;
    }
    // 00459b9a  8b4502                 -mov eax, dword ptr [ebp + 2]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(2) /* 0x2 */);
    // 00459b9d  eb06                   -jmp 0x459ba5
    goto L_0x00459ba5;
L_0x00459b9f:
    // 00459b9f  8b817cbc6f00           -mov eax, dword ptr [ecx + 0x6fbc7c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322748) /* 0x6fbc7c */);
L_0x00459ba5:
    // 00459ba5  894522                 -mov dword ptr [ebp + 0x22], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(34) /* 0x22 */) = cpu.eax;
    // 00459ba8  8b4522                 -mov eax, dword ptr [ebp + 0x22]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(34) /* 0x22 */);
    // 00459bab  894576                 -mov dword ptr [ebp + 0x76], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.eax;
    // 00459bae  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00459bb4  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00459bb7  8945ae                 -mov dword ptr [ebp - 0x52], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-82) /* -0x52 */) = cpu.eax;
    // 00459bba  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00459bc0  8b4dae                 -mov ecx, dword ptr [ebp - 0x52]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-82) /* -0x52 */);
    // 00459bc3  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00459bc5  894d96                 -mov dword ptr [ebp - 0x6a], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-106) /* -0x6a */) = cpu.ecx;
    // 00459bc8  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00459bca  8b4596                 -mov eax, dword ptr [ebp - 0x6a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-106) /* -0x6a */);
    // 00459bcd  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 00459bd0  01c1                   +add ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00459bd2  d945fa                 +fld dword ptr [ebp - 6]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-6) /* -0x6 */));
    // 00459bd5  d89978bc6f00           +fcomp dword ptr [ecx + 0x6fbc78]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322744) /* 0x6fbc78 */)));
    // 00459bdb  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459bdd  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459bde  7605                   -jbe 0x459be5
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00459be5;
    }
    // 00459be0  8b45fa                 -mov eax, dword ptr [ebp - 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 00459be3  eb06                   -jmp 0x459beb
    goto L_0x00459beb;
L_0x00459be5:
    // 00459be5  8b8178bc6f00           -mov eax, dword ptr [ecx + 0x6fbc78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322744) /* 0x6fbc78 */);
L_0x00459beb:
    // 00459beb  8945f2                 -mov dword ptr [ebp - 0xe], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-14) /* -0xe */) = cpu.eax;
    // 00459bee  8b45f2                 -mov eax, dword ptr [ebp - 0xe]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-14) /* -0xe */);
    // 00459bf1  894572                 -mov dword ptr [ebp + 0x72], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.eax;
    // 00459bf4  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00459bfa  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00459bfd  8945d6                 -mov dword ptr [ebp - 0x2a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-42) /* -0x2a */) = cpu.eax;
    // 00459c00  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00459c06  8b4dd6                 -mov ecx, dword ptr [ebp - 0x2a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-42) /* -0x2a */);
    // 00459c09  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00459c0b  894d8e                 -mov dword ptr [ebp - 0x72], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-114) /* -0x72 */) = cpu.ecx;
    // 00459c0e  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00459c10  8b458e                 -mov eax, dword ptr [ebp - 0x72]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-114) /* -0x72 */);
    // 00459c13  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 00459c16  01c1                   +add ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00459c18  d94556                 +fld dword ptr [ebp + 0x56]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */));
    // 00459c1b  d89980bc6f00           +fcomp dword ptr [ecx + 0x6fbc80]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322752) /* 0x6fbc80 */)));
    // 00459c21  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459c23  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459c24  7305                   -jae 0x459c2b
    if (!cpu.flags.cf)
    {
        goto L_0x00459c2b;
    }
    // 00459c26  8b4556                 -mov eax, dword ptr [ebp + 0x56]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 00459c29  eb06                   -jmp 0x459c31
    goto L_0x00459c31;
L_0x00459c2b:
    // 00459c2b  8b8180bc6f00           -mov eax, dword ptr [ecx + 0x6fbc80]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322752) /* 0x6fbc80 */);
L_0x00459c31:
    // 00459c31  8945ea                 -mov dword ptr [ebp - 0x16], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-22) /* -0x16 */) = cpu.eax;
    // 00459c34  8b45ea                 -mov eax, dword ptr [ebp - 0x16]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-22) /* -0x16 */);
    // 00459c37  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 00459c3a  e9b4000000             -jmp 0x459cf3
    goto L_0x00459cf3;
L_0x00459c3f:
    // 00459c3f  83fa01                 +cmp edx, 1
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
    // 00459c42  0f859f000000           -jne 0x459ce7
    if (!cpu.flags.zf)
    {
        goto L_0x00459ce7;
    }
    // 00459c48  8b8188bc6f00           -mov eax, dword ptr [ecx + 0x6fbc88]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322760) /* 0x6fbc88 */);
    // 00459c4e  89450e                 -mov dword ptr [ebp + 0xe], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(14) /* 0xe */) = cpu.eax;
    // 00459c51  8b8190bc6f00           -mov eax, dword ptr [ecx + 0x6fbc90]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322768) /* 0x6fbc90 */);
    // 00459c57  894526                 -mov dword ptr [ebp + 0x26], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(38) /* 0x26 */) = cpu.eax;
    // 00459c5a  8b818cbc6f00           -mov eax, dword ptr [ecx + 0x6fbc8c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322764) /* 0x6fbc8c */);
    // 00459c60  89455a                 -mov dword ptr [ebp + 0x5a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */) = cpu.eax;
    // 00459c63  d9455a                 +fld dword ptr [ebp + 0x5a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */));
    // 00459c66  d881ccbc6f00           +fadd dword ptr [ecx + 0x6fbccc]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322828) /* 0x6fbccc */)));
    // 00459c6c  d8a1c4bc6f00           +fsub dword ptr [ecx + 0x6fbcc4]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(7322820) /* 0x6fbcc4 */)));
    // 00459c72  d906                   +fld dword ptr [esi]
    fp1 = x86::Float(app->getMemory<float>(cpu.esi));
    // 00459c74  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00459c76  d95d2a                 +fstp dword ptr [ebp + 0x2a]
    app->getMemory<float>(cpu.ebp + x86::reg32(42) /* 0x2a */) = float(fp1);
    // 00459c79  d85d5a                 +fcomp dword ptr [ebp + 0x5a]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */)));
    // 00459c7c  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459c7e  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459c7f  7604                   -jbe 0x459c85
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00459c85;
    }
    // 00459c81  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00459c83  eb03                   -jmp 0x459c88
    goto L_0x00459c88;
L_0x00459c85:
    // 00459c85  8b455a                 -mov eax, dword ptr [ebp + 0x5a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */);
L_0x00459c88:
    // 00459c88  89451a                 -mov dword ptr [ebp + 0x1a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(26) /* 0x1a */) = cpu.eax;
    // 00459c8b  8b451a                 -mov eax, dword ptr [ebp + 0x1a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(26) /* 0x1a */);
    // 00459c8e  d94608                 +fld dword ptr [esi + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */));
    // 00459c91  89456e                 -mov dword ptr [ebp + 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.eax;
    // 00459c94  d85d2a                 +fcomp dword ptr [ebp + 0x2a]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(42) /* 0x2a */)));
    // 00459c97  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459c99  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459c9a  7305                   -jae 0x459ca1
    if (!cpu.flags.cf)
    {
        goto L_0x00459ca1;
    }
    // 00459c9c  8b4608                 -mov eax, dword ptr [esi + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 00459c9f  eb03                   -jmp 0x459ca4
    goto L_0x00459ca4;
L_0x00459ca1:
    // 00459ca1  8b452a                 -mov eax, dword ptr [ebp + 0x2a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(42) /* 0x2a */);
L_0x00459ca4:
    // 00459ca4  894506                 -mov dword ptr [ebp + 6], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(6) /* 0x6 */) = cpu.eax;
    // 00459ca7  8b4506                 -mov eax, dword ptr [ebp + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(6) /* 0x6 */);
    // 00459caa  d94604                 +fld dword ptr [esi + 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(4) /* 0x4 */));
    // 00459cad  894576                 -mov dword ptr [ebp + 0x76], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.eax;
    // 00459cb0  d85d0e                 +fcomp dword ptr [ebp + 0xe]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(14) /* 0xe */)));
    // 00459cb3  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459cb5  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459cb6  7605                   -jbe 0x459cbd
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00459cbd;
    }
    // 00459cb8  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00459cbb  eb03                   -jmp 0x459cc0
    goto L_0x00459cc0;
L_0x00459cbd:
    // 00459cbd  8b450e                 -mov eax, dword ptr [ebp + 0xe]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(14) /* 0xe */);
L_0x00459cc0:
    // 00459cc0  8945e6                 -mov dword ptr [ebp - 0x1a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-26) /* -0x1a */) = cpu.eax;
    // 00459cc3  8b45e6                 -mov eax, dword ptr [ebp - 0x1a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-26) /* -0x1a */);
    // 00459cc6  d9460c                 +fld dword ptr [esi + 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(12) /* 0xc */));
    // 00459cc9  894572                 -mov dword ptr [ebp + 0x72], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.eax;
    // 00459ccc  d85d26                 +fcomp dword ptr [ebp + 0x26]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(38) /* 0x26 */)));
    // 00459ccf  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459cd1  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459cd2  7305                   -jae 0x459cd9
    if (!cpu.flags.cf)
    {
        goto L_0x00459cd9;
    }
    // 00459cd4  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 00459cd7  eb03                   -jmp 0x459cdc
    goto L_0x00459cdc;
L_0x00459cd9:
    // 00459cd9  8b4526                 -mov eax, dword ptr [ebp + 0x26]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(38) /* 0x26 */);
L_0x00459cdc:
    // 00459cdc  89450a                 -mov dword ptr [ebp + 0xa], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(10) /* 0xa */) = cpu.eax;
    // 00459cdf  8b450a                 -mov eax, dword ptr [ebp + 0xa]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(10) /* 0xa */);
    // 00459ce2  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 00459ce5  eb0c                   -jmp 0x459cf3
    goto L_0x00459cf3;
L_0x00459ce7:
    // 00459ce7  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 00459cea  89456e                 -mov dword ptr [ebp + 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.eax;
    // 00459ced  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00459cf0  894572                 -mov dword ptr [ebp + 0x72], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.eax;
L_0x00459cf3:
    // 00459cf3  d945c6                 +fld dword ptr [ebp - 0x3a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-58) /* -0x3a */));
    // 00459cf6  d85dca                 +fcomp dword ptr [ebp - 0x36]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-54) /* -0x36 */)));
    // 00459cf9  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459cfb  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459cfc  730b                   -jae 0x459d09
    if (!cpu.flags.cf)
    {
        goto L_0x00459d09;
    }
    // 00459cfe  d945ba                 +fld dword ptr [ebp - 0x46]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-70) /* -0x46 */));
    // 00459d01  d85dd2                 +fcomp dword ptr [ebp - 0x2e]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-46) /* -0x2e */)));
    // 00459d04  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459d06  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459d07  721a                   -jb 0x459d23
    if (cpu.flags.cf)
    {
        goto L_0x00459d23;
    }
L_0x00459d09:
    // 00459d09  d9456e                 +fld dword ptr [ebp + 0x6e]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */));
    // 00459d0c  d85d76                 +fcomp dword ptr [ebp + 0x76]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(118) /* 0x76 */)));
    // 00459d0f  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459d11  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459d12  0f8369000000           -jae 0x459d81
    if (!cpu.flags.cf)
    {
        goto L_0x00459d81;
    }
    // 00459d18  d94572                 +fld dword ptr [ebp + 0x72]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(114) /* 0x72 */));
    // 00459d1b  d85d7a                 +fcomp dword ptr [ebp + 0x7a]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(122) /* 0x7a */)));
    // 00459d1e  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459d20  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459d21  735e                   -jae 0x459d81
    if (!cpu.flags.cf)
    {
        goto L_0x00459d81;
    }
L_0x00459d23:
    // 00459d23  e88811ffff             -call 0x44aeb0
    cpu.esp -= 4;
    nfs3hp::port::loop::currentMenu(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00459d28  83fb02                 +cmp ebx, 2
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
    // 00459d2b  7503                   -jne 0x459d30
    if (!cpu.flags.zf)
    {
        goto L_0x00459d30;
    }
    // 00459d2d  83c217                 -add edx, 0x17
    (cpu.edx) += x86::reg32(x86::sreg32(23 /*0x17*/));
L_0x00459d30:
    // 00459d30  8b5816                 -mov ebx, dword ptr [eax + 0x16]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(22) /* 0x16 */);
    // 00459d33  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
L_0x00459d36:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00459d36  8d0c9d00000000         -lea ecx, [ebx*4]
    cpu.ecx = x86::reg32(cpu.ebx * 4);
    // 00459d3d  29d9                   -sub ecx, ebx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00459d3f  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 00459d42  29d9                   -sub ecx, ebx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00459d44  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 00459d47  83b9e04e600000         +cmp dword ptr [ecx + 0x604ee0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6311648) /* 0x604ee0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459d4e  0f84adf9ffff           -je 0x459701
    if (cpu.flags.zf)
    {
        goto L_0x00459701;
    }
    // 00459d54  81c1e04e6000           +add ecx, 0x604ee0
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6311648 /*0x604ee0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00459d5a  7422                   -je 0x459d7e
    if (cpu.flags.zf)
    {
        goto L_0x00459d7e;
    }
    // 00459d5c  83391f                 +cmp dword ptr [ecx], 0x1f
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(31 /*0x1f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459d5f  751d                   -jne 0x459d7e
    if (!cpu.flags.zf)
    {
        goto L_0x00459d7e;
    }
    // 00459d61  3b513c                 +cmp edx, dword ptr [ecx + 0x3c]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459d64  7518                   -jne 0x459d7e
    if (!cpu.flags.zf)
    {
        goto L_0x00459d7e;
    }
    // 00459d66  8b8592000000           -mov eax, dword ptr [ebp + 0x92]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(146) /* 0x92 */);
    // 00459d6c  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00459d6e  8908                   -mov dword ptr [eax], ecx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.ecx;
    // 00459d70  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00459d72  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 00459d78  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459d79  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459d7a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459d7b  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00459d7e:
    // 00459d7e  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00459d7f  ebb5                   -jmp 0x459d36
    goto L_0x00459d36;
L_0x00459d81:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00459d81  42                     -inc edx
    (cpu.edx)++;
    // 00459d82  83fa17                 +cmp edx, 0x17
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(23 /*0x17*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459d85  7d0f                   -jge 0x459d96
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00459d96;
    }
L_0x00459d87:
    // 00459d87  39fa                   +cmp edx, edi
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459d89  74f6                   -je 0x459d81
    if (cpu.flags.zf)
    {
        goto L_0x00459d81;
    }
    // 00459d8b  83fa11                 +cmp edx, 0x11
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(17 /*0x11*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459d8e  0f85b9f9ffff           -jne 0x45974d
    if (!cpu.flags.zf)
    {
        goto L_0x0045974d;
    }
    // 00459d94  ebeb                   -jmp 0x459d81
    goto L_0x00459d81;
L_0x00459d96:
    // 00459d96  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00459d9b:
    // 00459d9b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00459d9d  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 00459da3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459da4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459da5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459da6  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_459db0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4;
  x86::reg16 fsw = 0;
    // 00459db0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00459db1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00459db2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00459db3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00459db4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00459db5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00459db6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00459db8  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00459dbb  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00459dbe  83783c17               +cmp dword ptr [eax + 0x3c], 0x17
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(23 /*0x17*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459dc2  7c07                   -jl 0x459dcb
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00459dcb;
    }
    // 00459dc4  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00459dc9  eb02                   -jmp 0x459dcd
    goto L_0x00459dcd;
L_0x00459dcb:
    // 00459dcb  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00459dcd:
    // 00459dcd  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00459dd0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00459dd2  740b                   -je 0x459ddf
    if (cpu.flags.zf)
    {
        goto L_0x00459ddf;
    }
    // 00459dd4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459dd7  8b583c                 -mov ebx, dword ptr [eax + 0x3c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 00459dda  83eb17                 +sub ebx, 0x17
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(23 /*0x17*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00459ddd  eb06                   -jmp 0x459de5
    goto L_0x00459de5;
L_0x00459ddf:
    // 00459ddf  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459de2  8b583c                 -mov ebx, dword ptr [eax + 0x3c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
L_0x00459de5:
    // 00459de5  8b0db0d36f00           -mov ecx, dword ptr [0x6fd3b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 00459deb  83f901                 +cmp ecx, 1
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
    // 00459dee  756a                   -jne 0x459e5a
    if (!cpu.flags.zf)
    {
        goto L_0x00459e5a;
    }
    // 00459df0  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459df3  df4006                 -fild word ptr [eax + 6]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */)));
    // 00459df6  db05c8da7c00           -fild dword ptr [0x7cdac8]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */)));
    // 00459dfc  d9e8                   -fld1 
    fp2 = 1.0;
    // 00459dfe  def1                   -fdivrp st(1)
    fp1 = fp::div(fp2, x86::Float(fp1));
    // 00459e00  df4008                 -fild word ptr [eax + 8]
    fp2 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 00459e03  db05ccda7c00           -fild dword ptr [0x7cdacc]
    fp3 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */)));
    // 00459e09  d9e8                   -fld1 
    fp4 = 1.0;
    // 00459e0b  def1                   -fdivrp st(1)
    fp3 = fp::div(fp4, x86::Float(fp3));
    // 00459e0d  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 00459e0f  d8ca                   -fmul st(2)
    fp3 = fp::mul(fp3, x86::Float(fp1));
    // 00459e11  db4060                 -fild dword ptr [eax + 0x60]
    fp4 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(96) /* 0x60 */)));
    // 00459e14  decb                   -fmulp st(3)
    fp1 = fp::mul(fp1, fp4);
    // 00459e16  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 00459e18  d8cb                   -fmul st(3)
    fp3 = fp::mul(fp3, x86::Float(fp0));
    // 00459e1a  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 00459e1c  d95844                 -fstp dword ptr [eax + 0x44]
    app->getMemory<float>(cpu.eax + x86::reg32(68) /* 0x44 */) = float(fp3);
    // 00459e1f  db4064                 -fild dword ptr [eax + 0x64]
    fp3 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */)));
    // 00459e22  decb                   -fmulp st(3)
    fp0 = fp::mul(fp0, fp3);
    // 00459e24  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00459e26  d9584c                 -fstp dword ptr [eax + 0x4c]
    app->getMemory<float>(cpu.eax + x86::reg32(76) /* 0x4c */) = float(fp2);
    // 00459e29  d95840                 -fstp dword ptr [eax + 0x40]
    app->getMemory<float>(cpu.eax + x86::reg32(64) /* 0x40 */) = float(fp1);
    // 00459e2c  d95848                 -fstp dword ptr [eax + 0x48]
    app->getMemory<float>(cpu.eax + x86::reg32(72) /* 0x48 */) = float(fp0);
    // 00459e2f  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00459e32  e8998dfcff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00459e37  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 00459e3e  7404                   -je 0x459e44
    if (cpu.flags.zf)
    {
        goto L_0x00459e44;
    }
    // 00459e40  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00459e42  eb02                   -jmp 0x459e46
    goto L_0x00459e46;
L_0x00459e44:
    // 00459e44  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00459e46:
    // 00459e46  69c0a4010000           -imul eax, eax, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00459e4c  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00459e4f  41                     -inc ecx
    (cpu.ecx)++;
    // 00459e50  69c948030000           -imul ecx, ecx, 0x348
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00459e56  01c8                   +add eax, ecx
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
    // 00459e58  eb5e                   -jmp 0x459eb8
    goto L_0x00459eb8;
L_0x00459e5a:
    // 00459e5a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459e5d  df4006                 -fild word ptr [eax + 6]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */)));
    // 00459e60  db05c8da7c00           -fild dword ptr [0x7cdac8]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */)));
    // 00459e66  d9e8                   -fld1 
    fp2 = 1.0;
    // 00459e68  def1                   -fdivrp st(1)
    fp1 = fp::div(fp2, x86::Float(fp1));
    // 00459e6a  df4008                 -fild word ptr [eax + 8]
    fp2 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 00459e6d  db05ccda7c00           -fild dword ptr [0x7cdacc]
    fp3 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */)));
    // 00459e73  d9e8                   -fld1 
    fp4 = 1.0;
    // 00459e75  def1                   -fdivrp st(1)
    fp3 = fp::div(fp4, x86::Float(fp3));
    // 00459e77  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 00459e79  d8ca                   -fmul st(2)
    fp3 = fp::mul(fp3, x86::Float(fp1));
    // 00459e7b  db4060                 -fild dword ptr [eax + 0x60]
    fp4 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(96) /* 0x60 */)));
    // 00459e7e  decb                   -fmulp st(3)
    fp1 = fp::mul(fp1, fp4);
    // 00459e80  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 00459e82  d8cb                   -fmul st(3)
    fp3 = fp::mul(fp3, x86::Float(fp0));
    // 00459e84  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 00459e86  d95844                 -fstp dword ptr [eax + 0x44]
    app->getMemory<float>(cpu.eax + x86::reg32(68) /* 0x44 */) = float(fp3);
    // 00459e89  db4064                 -fild dword ptr [eax + 0x64]
    fp3 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */)));
    // 00459e8c  decb                   -fmulp st(3)
    fp0 = fp::mul(fp0, fp3);
    // 00459e8e  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00459e90  d9584c                 -fstp dword ptr [eax + 0x4c]
    app->getMemory<float>(cpu.eax + x86::reg32(76) /* 0x4c */) = float(fp2);
    // 00459e93  d95840                 -fstp dword ptr [eax + 0x40]
    app->getMemory<float>(cpu.eax + x86::reg32(64) /* 0x40 */) = float(fp1);
    // 00459e96  d95848                 -fstp dword ptr [eax + 0x48]
    app->getMemory<float>(cpu.eax + x86::reg32(72) /* 0x48 */) = float(fp0);
    // 00459e99  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00459e9b  e8308dfcff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00459ea0  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 00459ea7  7407                   -je 0x459eb0
    if (cpu.flags.zf)
    {
        goto L_0x00459eb0;
    }
    // 00459ea9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00459eae  eb02                   -jmp 0x459eb2
    goto L_0x00459eb2;
L_0x00459eb0:
    // 00459eb0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00459eb2:
    // 00459eb2  69c0a4010000           -imul eax, eax, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
L_0x00459eb8:
    // 00459eb8  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 00459eba  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 00459ebd  8dbc0174bc6f00         -lea edi, [ecx + eax + 0x6fbc74]
    cpu.edi = x86::reg32(cpu.ecx + x86::reg32(7322740) /* 0x6fbc74 */ + cpu.eax * 1);
    // 00459ec4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459ec7  8d7040                 -lea esi, [eax + 0x40]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 00459eca  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00459ecb  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00459ecc  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00459ecd  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    hudLayoutDesigned(app, cpu.edi - 16, cpu.ebx); /* port: the layout as laid out, not as drawn */
    // 00459ece  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00459ed1  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00459ed3  e8e8b40200             -call 0x4853c0
    cpu.esp -= 4;
    nfs3hp::frontend::hudRebuildElement(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00459ed8  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00459edb  e8c0610200             -call 0x4800a0
    cpu.esp -= 4;
    nfs3hp::render::hudBand(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00459ee0  83fb11                 +cmp ebx, 0x11
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(17 /*0x11*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459ee3  7409                   -je 0x459eee
    if (cpu.flags.zf)
    {
        goto L_0x00459eee;
    }
    // 00459ee5  83fb16                 +cmp ebx, 0x16
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(22 /*0x16*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00459ee8  0f8575000000           -jne 0x459f63
    if (!cpu.flags.zf)
    {
        goto L_0x00459f63;
    }
L_0x00459eee:
    // 00459eee  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459ef1  d9404c                 +fld dword ptr [eax + 0x4c]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(76) /* 0x4c */));
    // 00459ef4  d86044                 +fsub dword ptr [eax + 0x44]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(68) /* 0x44 */)));
    // 00459ef7  d81db49b5300           +fcomp dword ptr [0x539bb4]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(5479348) /* 0x539bb4 */)));
    // 00459efd  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459eff  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459f00  7361                   -jae 0x459f63
    if (!cpu.flags.cf)
    {
        goto L_0x00459f63;
    }
    // 00459f02  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459f05  d94048                 +fld dword ptr [eax + 0x48]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(72) /* 0x48 */));
    // 00459f08  d86040                 +fsub dword ptr [eax + 0x40]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(64) /* 0x40 */)));
    // 00459f0b  d81db49b5300           +fcomp dword ptr [0x539bb4]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(5479348) /* 0x539bb4 */)));
    // 00459f11  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00459f13  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00459f14  734d                   -jae 0x459f63
    if (!cpu.flags.cf)
    {
        goto L_0x00459f63;
    }
    // 00459f16  8b35b0d36f00           -mov esi, dword ptr [0x6fd3b0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 00459f1c  83fe01                 +cmp esi, 1
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
    // 00459f1f  7504                   -jne 0x459f25
    if (!cpu.flags.zf)
    {
        goto L_0x00459f25;
    }
    // 00459f21  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00459f23  eb02                   -jmp 0x459f27
    goto L_0x00459f27;
L_0x00459f25:
    // 00459f25  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00459f27:
    // 00459f27  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00459f2a  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00459f2c  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00459f2f  e89c8cfcff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00459f34  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 00459f3b  7407                   -je 0x459f44
    if (cpu.flags.zf)
    {
        goto L_0x00459f44;
    }
    // 00459f3d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00459f42  eb02                   -jmp 0x459f46
    goto L_0x00459f46;
L_0x00459f44:
    // 00459f44  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00459f46:
    // 00459f46  69c0a4010000           -imul eax, eax, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00459f4c  69c948030000           -imul ecx, ecx, 0x348
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00459f52  8dbc01e4bd6f00         -lea edi, [ecx + eax + 0x6fbde4]
    cpu.edi = x86::reg32(cpu.ecx + x86::reg32(7323108) /* 0x6fbde4 */ + cpu.eax * 1);
    // 00459f59  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459f5c  8d7040                 -lea esi, [eax + 0x40]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 00459f5f  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00459f60  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00459f61  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00459f62  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x00459f63:
    // 00459f63  8d45f0                 -lea eax, [ebp - 0x10]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00459f66  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00459f67  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00459f6a  8b0c85f07d6700         -mov ecx, dword ptr [eax*4 + 0x677df0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(6782448) /* 0x677df0 */ + cpu.eax * 4);
    // 00459f71  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459f74  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00459f77  83c040                 -add eax, 0x40
    (cpu.eax) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 00459f7a  e8f1f6ffff             -call 0x459670
    cpu.esp -= 4;
    hudLayoutsDrawn(app, cpu.sync(), true); /* port: measured against what is drawn */
    sub_459670(app, cpu.sync());
    cpu.reload();
    hudLayoutsDrawn(app, cpu.sync(), false);
    if (cpu.terminate) return;
    // 00459f7f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00459f81  742c                   -je 0x459faf
    if (cpu.flags.zf)
    {
        goto L_0x00459faf;
    }
    // 00459f83  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459f86  8b7874                 -mov edi, dword ptr [eax + 0x74]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(116) /* 0x74 */);
    // 00459f89  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00459f8b  740e                   -je 0x459f9b
    if (cpu.flags.zf)
    {
        goto L_0x00459f9b;
    }
    // 00459f8d  c7477001000000         -mov dword ptr [edi + 0x70], 1
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(112) /* 0x70 */) = 1 /*0x1*/;
    // 00459f94  c7407400000000         -mov dword ptr [eax + 0x74], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(116) /* 0x74 */) = 0 /*0x0*/;
L_0x00459f9b:
    // 00459f9b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459f9e  8d7850                 -lea edi, [eax + 0x50]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(80) /* 0x50 */);
    // 00459fa1  8d7040                 -lea esi, [eax + 0x40]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 00459fa4  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00459fa9  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00459faa  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00459fab  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00459fac  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00459fad  eb23                   -jmp 0x459fd2
    goto L_0x00459fd2;
L_0x00459faf:
    // 00459faf  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459fb2  8b5074                 -mov edx, dword ptr [eax + 0x74]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(116) /* 0x74 */);
    // 00459fb5  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00459fb7  7407                   -je 0x459fc0
    if (cpu.flags.zf)
    {
        goto L_0x00459fc0;
    }
    // 00459fb9  c7427001000000         -mov dword ptr [edx + 0x70], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(112) /* 0x70 */) = 1 /*0x1*/;
L_0x00459fc0:
    // 00459fc0  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00459fc3  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00459fc6  894174                 -mov dword ptr [ecx + 0x74], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(116) /* 0x74 */) = cpu.eax;
    // 00459fc9  c7407000000000         -mov dword ptr [eax + 0x70], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(112) /* 0x70 */) = 0 /*0x0*/;
    // 00459fd0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00459fd2:
    // 00459fd2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00459fd4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459fd5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459fd6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459fd7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459fd8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459fd9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00459fda  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_459fe0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3;
    // 00459fe0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00459fe1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00459fe2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00459fe3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00459fe4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00459fe5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00459fe7  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00459fea  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00459fec  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 00459fee  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 00459ff5  0f84f8000000           -je 0x45a0f3
    if (cpu.flags.zf)
    {
        goto L_0x0045a0f3;
    }
    // 00459ffb  8b503c                 -mov edx, dword ptr [eax + 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 00459ffe  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0045a000  e8cb8bfcff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045a005  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0045a00c  7407                   -je 0x45a015
    if (cpu.flags.zf)
    {
        goto L_0x0045a015;
    }
    // 0045a00e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0045a013  eb02                   -jmp 0x45a017
    goto L_0x0045a017;
L_0x0045a015:
    // 0045a015  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0045a017:
    // 0045a017  69f0a4010000           -imul esi, eax, 0x1a4
    cpu.esi = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0045a01d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0045a01f  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0045a022  8dbc0674bc6f00         -lea edi, [esi + eax + 0x6fbc74]
    cpu.edi = x86::reg32(cpu.esi + x86::reg32(7322740) /* 0x6fbc74 */ + cpu.eax * 1);
    // 0045a029  8d7150                 -lea esi, [ecx + 0x50]
    cpu.esi = x86::reg32(cpu.ecx + x86::reg32(80) /* 0x50 */);
    // 0045a02c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0045a02e  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0045a02f  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0045a030  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0045a031  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    hudLayoutDesigned(app, cpu.edi - 16, cpu.edx); /* port: the layout as laid out, not as drawn */
    // 0045a032  e889b30200             -call 0x4853c0
    cpu.esp -= 4;
    nfs3hp::frontend::hudRebuildElement(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045a037  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0045a039  e862600200             -call 0x4800a0
    cpu.esp -= 4;
    nfs3hp::render::hudBand(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045a03e  668b4302               -mov ax, word ptr [ebx + 2]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 0045a042  6689411c               -mov word ptr [ecx + 0x1c], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(28) /* 0x1c */) = cpu.ax;
    // 0045a046  668b03                 -mov ax, word ptr [ebx]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ebx);
    // 0045a049  6689411a               -mov word ptr [ecx + 0x1a], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(26) /* 0x1a */) = cpu.ax;
    // 0045a04d  8b411a                 -mov eax, dword ptr [ecx + 0x1a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(26) /* 0x1a */);
    // 0045a050  8b1dc8da7c00           -mov ebx, dword ptr [0x7cdac8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a056  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a059  0fafc3                 -imul eax, ebx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 0045a05c  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a05f  dd05bc9b5300           -fld qword ptr [0x539bbc]
    fp0 = x86::Float(app->getMemory<double>(x86::reg32(5479356) /* 0x539bbc */));
    // 0045a065  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a068  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 0045a06a  e8e75c0800             -call 0x4dfd56
    cpu.fpu.push(fp1);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp1 = cpu.fpu.take();
    // 0045a06f  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp1));
    // 0045a072  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a075  66894106               -mov word ptr [ecx + 6], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */) = cpu.ax;
    // 0045a079  8b4118                 -mov eax, dword ptr [ecx + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */);
    // 0045a07c  8b35ccda7c00           -mov esi, dword ptr [0x7cdacc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045a082  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a085  0fafc6                 -imul eax, esi
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.esi)));
    // 0045a088  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a08b  dd05c49b5300           -fld qword ptr [0x539bc4]
    fp1 = x86::Float(app->getMemory<double>(x86::reg32(5479364) /* 0x539bc4 */));
    // 0045a091  db45fc                 -fild dword ptr [ebp - 4]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a094  d8c9                   -fmul st(1)
    fp2 = fp::mul(fp2, x86::Float(fp1));
    // 0045a096  e8bb5c0800             -call 0x4dfd56
    cpu.fpu.push(fp2);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp2 = cpu.fpu.take();
    // 0045a09b  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp2));
    // 0045a09e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a0a1  66894108               -mov word ptr [ecx + 8], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.ax;
    // 0045a0a5  8b411c                 -mov eax, dword ptr [ecx + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */);
    // 0045a0a8  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a0ab  0fafc3                 -imul eax, ebx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 0045a0ae  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a0b1  8b411e                 -mov eax, dword ptr [ecx + 0x1e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(30) /* 0x1e */);
    // 0045a0b4  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a0b7  0fafc6                 -imul eax, esi
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.esi)));
    // 0045a0ba  db45fc                 -fild dword ptr [ebp - 4]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a0bd  deca                   -fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 0045a0bf  df4106                 -fild word ptr [ecx + 6]
    fp2 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */)));
    // 0045a0c2  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a0c5  dec2                   -faddp st(2)
    fp0 = fp::add(fp0, fp2);
    // 0045a0c7  db45fc                 -fild dword ptr [ebp - 4]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a0ca  dec9                   -fmulp st(1)
    fp1 = fp::mul(fp1, fp2);
    // 0045a0cc  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0045a0ce  dd05cc9b5300           -fld qword ptr [0x539bcc]
    fp2 = x86::Float(app->getMemory<double>(x86::reg32(5479372) /* 0x539bcc */));
    // 0045a0d4  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 0045a0d6  d8c1                   -fadd st(1)
    fp2 = fp::add(fp2, x86::Float(fp1));
    // 0045a0d8  df4108                 -fild word ptr [ecx + 8]
    fp3 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */)));
    // 0045a0db  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 0045a0dd  e8745c0800             -call 0x4dfd56
    cpu.fpu.push(fp2);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp2 = cpu.fpu.take();
    // 0045a0e2  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 0045a0e4  dec1                   -faddp st(1)
    fp1 = fp::add(fp1, fp2);
    // 0045a0e6  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0045a0e8  db5960                 -fistp dword ptr [ecx + 0x60]
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(96) /* 0x60 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp1));
    // 0045a0eb  e8665c0800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a0f0  db5964                 -fistp dword ptr [ecx + 0x64]
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(100) /* 0x64 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
L_0x0045a0f3:
    // 0045a0f3  8b5974                 -mov ebx, dword ptr [ecx + 0x74]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(116) /* 0x74 */);
    // 0045a0f6  c7417001000000         -mov dword ptr [ecx + 0x70], 1
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(112) /* 0x70 */) = 1 /*0x1*/;
    // 0045a0fd  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0045a0ff  740e                   -je 0x45a10f
    if (cpu.flags.zf)
    {
        goto L_0x0045a10f;
    }
    // 0045a101  c7437001000000         -mov dword ptr [ebx + 0x70], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(112) /* 0x70 */) = 1 /*0x1*/;
    // 0045a108  c7417400000000         -mov dword ptr [ecx + 0x74], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(116) /* 0x74 */) = 0 /*0x0*/;
L_0x0045a10f:
    // 0045a10f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0045a111  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045a112  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045a113  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045a114  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045a115  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045a116  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_45a120(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3;
    // 0045a120  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0045a121  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0045a122  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0045a123  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0045a124  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0045a125  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0045a126  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0045a128  83ec64                 -sub esp, 0x64
    (cpu.esp) -= x86::reg32(x86::sreg32(100 /*0x64*/));
    // 0045a12b  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0045a12e  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 0045a131  8b15c8da7c00           -mov edx, dword ptr [0x7cdac8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a137  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a13a  0fafc2                 -imul eax, edx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 0045a13d  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0045a140  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a143  8b401e                 -mov eax, dword ptr [eax + 0x1e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(30) /* 0x1e */);
    // 0045a146  8b0dccda7c00           -mov ecx, dword ptr [0x7cdacc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045a14c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a14f  0fafc1                 -imul eax, ecx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ecx)));
    // 0045a152  db45f8                 -fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a155  dc0dd49b5300           -fmul qword ptr [0x539bd4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479380) /* 0x539bd4 */)));
    // 0045a15b  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0045a15e  db45f8                 -fild dword ptr [ebp - 8]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a161  dc0ddc9b5300           -fmul qword ptr [0x539bdc]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a167  8b1dfc7d6700           -mov ebx, dword ptr [0x677dfc]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(6782460) /* 0x677dfc */);
    // 0045a16d  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0045a16f  e8e25b0800             -call 0x4dfd56
    cpu.fpu.push(fp1);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp1 = cpu.fpu.take();
    // 0045a174  db5dec                 -fistp dword ptr [ebp - 0x14]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp1));
    // 0045a177  e8da5b0800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a17c  db5df0                 -fistp dword ptr [ebp - 0x10]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a17f  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0045a181  7405                   -je 0x45a188
    if (cpu.flags.zf)
    {
        goto L_0x0045a188;
    }
    // 0045a183  83fb01                 +cmp ebx, 1
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
    // 0045a186  752f                   -jne 0x45a1b7
    if (!cpu.flags.zf)
    {
        goto L_0x0045a1b7;
    }
L_0x0045a188:
    // 0045a188  8b15f87d6700           -mov edx, dword ptr [0x677df8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6782456) /* 0x677df8 */);
    // 0045a18e  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0045a195  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045a197  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0045a19a  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045a19c  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0045a19e  c1e204                 -shl edx, 4
    cpu.edx <<= 4 /*0x4*/ % 32;
    // 0045a1a1  8b82b4977400           -mov eax, dword ptr [edx + 0x7497b4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641012) /* 0x7497b4 */);
    // 0045a1a7  8bbaac977400           -mov edi, dword ptr [edx + 0x7497ac]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641004) /* 0x7497ac */);
    // 0045a1ad  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0045a1b0  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 0045a1b2  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0045a1b4  8955f0                 -mov dword ptr [ebp - 0x10], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edx;
L_0x0045a1b7:
    // 0045a1b7  e8c41e0400             -call 0x49c080
    cpu.esp -= 4;
    nfs3hp::frontend::mouseKey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045a1bc  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045a1be  0f84c8080000           -je 0x45aa8c
    if (cpu.flags.zf)
    {
        goto L_0x0045aa8c;
    }
    // 0045a1c4  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a1c7  6683787c00             +cmp word ptr [eax + 0x7c], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(124) /* 0x7c */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0045a1cc  0f8536030000           -jne 0x45a508
    if (!cpu.flags.zf)
    {
        goto L_0x0045a508;
    }
    // 0045a1d2  bf007e6700             -mov edi, 0x677e00
    cpu.edi = 6782464 /*0x677e00*/;
    // 0045a1d7  8d701a                 -lea esi, [eax + 0x1a]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(26) /* 0x1a */);
    // 0045a1da  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0045a1db  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0045a1dc  80480504               -or byte ptr [eax + 5], 4
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(4 /*0x4*/));
    // 0045a1e0  8b4078                 -mov eax, dword ptr [eax + 0x78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(120) /* 0x78 */);
    // 0045a1e3  3d0a050000             +cmp eax, 0x50a
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1290 /*0x50a*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a1e8  723c                   -jb 0x45a226
    if (cpu.flags.cf)
    {
        goto L_0x0045a226;
    }
    // 0045a1ea  0f8655000000           -jbe 0x45a245
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0045a245;
    }
    // 0045a1f0  3d09080000             +cmp eax, 0x809
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2057 /*0x809*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a1f5  7223                   -jb 0x45a21a
    if (cpu.flags.cf)
    {
        goto L_0x0045a21a;
    }
    // 0045a1f7  0f866c000000           -jbe 0x45a269
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0045a269;
    }
    // 0045a1fd  3d0b090000             +cmp eax, 0x90b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2315 /*0x90b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a202  0f828b000000           -jb 0x45a293
    if (cpu.flags.cf)
    {
        goto L_0x0045a293;
    }
    // 0045a208  0f865b000000           -jbe 0x45a269
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0045a269;
    }
    // 0045a20e  3d0a0a0000             +cmp eax, 0xa0a
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2570 /*0xa0a*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a213  7454                   -je 0x45a269
    if (cpu.flags.zf)
    {
        goto L_0x0045a269;
    }
    // 0045a215  e979000000             -jmp 0x45a293
    goto L_0x0045a293;
L_0x0045a21a:
    // 0045a21a  3d0b060000             +cmp eax, 0x60b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1547 /*0x60b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a21f  7424                   -je 0x45a245
    if (cpu.flags.zf)
    {
        goto L_0x0045a245;
    }
    // 0045a221  e96d000000             -jmp 0x45a293
    goto L_0x0045a293;
L_0x0045a226:
    // 0045a226  3d08010000             +cmp eax, 0x108
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(264 /*0x108*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a22b  7214                   -jb 0x45a241
    if (cpu.flags.cf)
    {
        goto L_0x0045a241;
    }
    // 0045a22d  7616                   -jbe 0x45a245
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0045a245;
    }
    // 0045a22f  3d08020000             +cmp eax, 0x208
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(520 /*0x208*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a234  725d                   -jb 0x45a293
    if (cpu.flags.cf)
    {
        goto L_0x0045a293;
    }
    // 0045a236  760d                   -jbe 0x45a245
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0045a245;
    }
    // 0045a238  3d09040000             +cmp eax, 0x409
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1033 /*0x409*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a23d  7406                   -je 0x45a245
    if (cpu.flags.zf)
    {
        goto L_0x0045a245;
    }
    // 0045a23f  eb52                   -jmp 0x45a293
    goto L_0x0045a293;
L_0x0045a241:
    // 0045a241  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045a243  754e                   -jne 0x45a293
    if (!cpu.flags.zf)
    {
        goto L_0x0045a293;
    }
L_0x0045a245:
    // 0045a245  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0045a24a  8b0dc8da7c00           -mov ecx, dword ptr [0x7cdac8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a250  0fafc1                 +imul eax, ecx
    {
        x86::sreg64 tmp = x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ecx));
        cpu.eax = static_cast<x86::reg32>(static_cast<x86::sreg32>(tmp));
        cpu.flags.of = cpu.flags.cf = (tmp != x86::sreg64(x86::sreg32(cpu.eax)));
    }
    // 0045a253  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0045a256  db45f8                 +fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a259  dc0dd49b5300           +fmul qword ptr [0x539bd4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479380) /* 0x539bd4 */)));
    // 0045a25f  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a262  df4006                 +fild word ptr [eax + 6]
    fp1 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */)));
    // 0045a265  dee9                   +fsubp st(1)
    fp0 = fp::sub(fp0, fp1);
    // 0045a267  eb22                   -jmp 0x45a28b
    goto L_0x0045a28b;
L_0x0045a269:
    // 0045a269  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0045a26e  8b1dc8da7c00           -mov ebx, dword ptr [0x7cdac8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a274  0fafc3                 -imul eax, ebx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 0045a277  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0045a27a  db45f8                 -fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a27d  dc0dd49b5300           -fmul qword ptr [0x539bd4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479380) /* 0x539bd4 */)));
    // 0045a283  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a286  db4060                 -fild dword ptr [eax + 0x60]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(96) /* 0x60 */)));
    // 0045a289  dee1                   -fsubrp st(1)
    fp0 = fp::sub(fp1, x86::Float(fp0));
L_0x0045a28b:
    // 0045a28b  e8c65a0800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a290  db5868                 -fistp dword ptr [eax + 0x68]
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(104) /* 0x68 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
L_0x0045a293:
    // 0045a293  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a296  8b4078                 -mov eax, dword ptr [eax + 0x78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(120) /* 0x78 */);
    // 0045a299  3d0a050000             +cmp eax, 0x50a
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1290 /*0x50a*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a29e  7234                   -jb 0x45a2d4
    if (cpu.flags.cf)
    {
        goto L_0x0045a2d4;
    }
    // 0045a2a0  0f864d000000           -jbe 0x45a2f3
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0045a2f3;
    }
    // 0045a2a6  3d09080000             +cmp eax, 0x809
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2057 /*0x809*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a2ab  721b                   -jb 0x45a2c8
    if (cpu.flags.cf)
    {
        goto L_0x0045a2c8;
    }
    // 0045a2ad  7644                   -jbe 0x45a2f3
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0045a2f3;
    }
    // 0045a2af  3d0b090000             +cmp eax, 0x90b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2315 /*0x90b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a2b4  0f8287000000           -jb 0x45a341
    if (cpu.flags.cf)
    {
        goto L_0x0045a341;
    }
    // 0045a2ba  7637                   -jbe 0x45a2f3
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0045a2f3;
    }
    // 0045a2bc  3d0a0a0000             +cmp eax, 0xa0a
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2570 /*0xa0a*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a2c1  7454                   -je 0x45a317
    if (cpu.flags.zf)
    {
        goto L_0x0045a317;
    }
    // 0045a2c3  e979000000             -jmp 0x45a341
    goto L_0x0045a341;
L_0x0045a2c8:
    // 0045a2c8  3d0b060000             +cmp eax, 0x60b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1547 /*0x60b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a2cd  7448                   -je 0x45a317
    if (cpu.flags.zf)
    {
        goto L_0x0045a317;
    }
    // 0045a2cf  e96d000000             -jmp 0x45a341
    goto L_0x0045a341;
L_0x0045a2d4:
    // 0045a2d4  3d08010000             +cmp eax, 0x108
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(264 /*0x108*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a2d9  7214                   -jb 0x45a2ef
    if (cpu.flags.cf)
    {
        goto L_0x0045a2ef;
    }
    // 0045a2db  7616                   -jbe 0x45a2f3
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0045a2f3;
    }
    // 0045a2dd  3d08020000             +cmp eax, 0x208
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(520 /*0x208*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a2e2  725d                   -jb 0x45a341
    if (cpu.flags.cf)
    {
        goto L_0x0045a341;
    }
    // 0045a2e4  7631                   -jbe 0x45a317
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0045a317;
    }
    // 0045a2e6  3d09040000             +cmp eax, 0x409
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1033 /*0x409*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a2eb  7406                   -je 0x45a2f3
    if (cpu.flags.zf)
    {
        goto L_0x0045a2f3;
    }
    // 0045a2ed  eb52                   -jmp 0x45a341
    goto L_0x0045a341;
L_0x0045a2ef:
    // 0045a2ef  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045a2f1  754e                   -jne 0x45a341
    if (!cpu.flags.zf)
    {
        goto L_0x0045a341;
    }
L_0x0045a2f3:
    // 0045a2f3  a1e8e55500             -mov eax, dword ptr [0x55e5e8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */);
    // 0045a2f8  8b35ccda7c00           -mov esi, dword ptr [0x7cdacc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045a2fe  0fafc6                 +imul eax, esi
    {
        x86::sreg64 tmp = x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.esi));
        cpu.eax = static_cast<x86::reg32>(static_cast<x86::sreg32>(tmp));
        cpu.flags.of = cpu.flags.cf = (tmp != x86::sreg64(x86::sreg32(cpu.eax)));
    }
    // 0045a301  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0045a304  db45f8                 +fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a307  dc0ddc9b5300           +fmul qword ptr [0x539bdc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a30d  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a310  df4008                 +fild word ptr [eax + 8]
    fp1 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 0045a313  dee9                   +fsubp st(1)
    fp0 = fp::sub(fp0, fp1);
    // 0045a315  eb22                   -jmp 0x45a339
    goto L_0x0045a339;
L_0x0045a317:
    // 0045a317  a1e8e55500             -mov eax, dword ptr [0x55e5e8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */);
    // 0045a31c  8b3dccda7c00           -mov edi, dword ptr [0x7cdacc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045a322  0fafc7                 -imul eax, edi
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edi)));
    // 0045a325  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0045a328  db45f8                 -fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a32b  dc0ddc9b5300           -fmul qword ptr [0x539bdc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a331  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a334  db4064                 -fild dword ptr [eax + 0x64]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */)));
    // 0045a337  dee1                   -fsubrp st(1)
    fp0 = fp::sub(fp1, x86::Float(fp0));
L_0x0045a339:
    // 0045a339  e8185a0800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a33e  db586c                 -fistp dword ptr [eax + 0x6c]
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(108) /* 0x6c */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
L_0x0045a341:
    // 0045a341  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a344  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 0045a347  8b15c8da7c00           -mov edx, dword ptr [0x7cdac8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a34d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a350  0fafc2                 -imul eax, edx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 0045a353  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0045a356  db45f8                 -fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a359  dc0dd49b5300           -fmul qword ptr [0x539bd4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479380) /* 0x539bd4 */)));
    // 0045a35f  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a362  df4006                 -fild word ptr [eax + 6]
    fp1 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */)));
    // 0045a365  dec1                   -faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 0045a367  dd05e49b5300           -fld qword ptr [0x539be4]
    fp1 = x86::Float(app->getMemory<double>(x86::reg32(5479396) /* 0x539be4 */));
    // 0045a36d  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0045a36f  d8c1                   -fadd st(1)
    fp1 = fp::add(fp1, x86::Float(fp0));
    // 0045a371  e8e0590800             -call 0x4dfd56
    cpu.fpu.push(fp1);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp1 = cpu.fpu.take();
    // 0045a376  db5860                 -fistp dword ptr [eax + 0x60]
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(96) /* 0x60 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp1));
    // 0045a379  8b401e                 -mov eax, dword ptr [eax + 0x1e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(30) /* 0x1e */);
    // 0045a37c  8b0dccda7c00           -mov ecx, dword ptr [0x7cdacc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045a382  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a385  0fafc1                 -imul eax, ecx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ecx)));
    // 0045a388  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0045a38b  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a38e  df4006                 -fild word ptr [eax + 6]
    fp1 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */)));
    // 0045a391  dc0dec9b5300           -fmul qword ptr [0x539bec]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5479404) /* 0x539bec */)));
    // 0045a397  db05c8da7c00           -fild dword ptr [0x7cdac8]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */)));
    // 0045a39d  def9                   -fdivp st(1)
    fp1 = fp::div(fp1, fp2);
    // 0045a39f  db45f8                 -fild dword ptr [ebp - 8]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a3a2  dc0ddc9b5300           -fmul qword ptr [0x539bdc]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a3a8  df4008                 -fild word ptr [eax + 8]
    fp3 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 0045a3ab  dec1                   -faddp st(1)
    fp2 = fp::add(fp2, fp3);
    // 0045a3ad  dec2                   -faddp st(2)
    fp0 = fp::add(fp0, fp2);
    // 0045a3af  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a3b2  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0045a3b4  e89d590800             -call 0x4dfd56
    cpu.fpu.push(fp1);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp1 = cpu.fpu.take();
    // 0045a3b9  db5864                 -fistp dword ptr [eax + 0x64]
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp1));
    // 0045a3bc  e895590800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a3c1  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a3c4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a3c7  6689421c               -mov word ptr [edx + 0x1c], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(28) /* 0x1c */) = cpu.ax;
    // 0045a3cb  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0045a3cd  df4008                 -fild word ptr [eax + 8]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 0045a3d0  dc0df49b5300           -fmul qword ptr [0x539bf4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479412) /* 0x539bf4 */)));
    // 0045a3d6  db05ccda7c00           -fild dword ptr [0x7cdacc]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */)));
    // 0045a3dc  dd5ddc                 -fstp qword ptr [ebp - 0x24]
    app->getMemory<double>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = double(fp1);
    // 0045a3df  dc75dc                 -fdiv qword ptr [ebp - 0x24]
    fp0 = fp::div(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-36) /* -0x24 */)));
    // 0045a3e2  e86f590800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a3e7  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a3ea  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a3ed  8b1db0d36f00           -mov ebx, dword ptr [0x6fd3b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0045a3f3  6689421a               -mov word ptr [edx + 0x1a], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(26) /* 0x1a */) = cpu.ax;
    // 0045a3f7  83fb01                 +cmp ebx, 1
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
    // 0045a3fa  0f85ce000000           -jne 0x45a4ce
    if (!cpu.flags.zf)
    {
        goto L_0x0045a4ce;
    }
    // 0045a400  837a3c08               +cmp dword ptr [edx + 0x3c], 8
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a404  0f84c4000000           -je 0x45a4ce
    if (cpu.flags.zf)
    {
        goto L_0x0045a4ce;
    }
    // 0045a40a  8b3dccda7c00           -mov edi, dword ptr [0x7cdacc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045a410  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0045a412  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0045a414  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0045a417  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045a419  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0045a41b  8b0de8e55500           -mov ecx, dword ptr [0x55e5e8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */);
    // 0045a421  0fafcf                 -imul ecx, edi
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(cpu.edi)));
    // 0045a424  813de8e55500f0000000   +cmp dword ptr [0x55e5e8], 0xf0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(240 /*0xf0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a42e  7d49                   -jge 0x45a479
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0045a479;
    }
    // 0045a430  894df8                 -mov dword ptr [ebp - 8], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ecx;
    // 0045a433  db45f8                 -fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a436  dc0ddc9b5300           -fmul qword ptr [0x539bdc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a43c  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a43f  8b1dc8da7c00           -mov ebx, dword ptr [0x7cdac8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a445  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0045a448  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0045a44a  db4264                 -fild dword ptr [edx + 0x64]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(100) /* 0x64 */)));
    // 0045a44d  df4008                 -fild word ptr [eax + 8]
    fp2 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 0045a450  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0045a452  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 0045a454  d8e2                   +fsub st(2)
    fp2 = fp::sub(fp2, x86::Float(fp0));
    // 0045a456  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 0045a458  deea                   +fsubp st(2)
    fp0 = fp::sub(fp0, fp2);
    // 0045a45a  db45f8                 +fild dword ptr [ebp - 8]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a45d  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 0045a45f  e8f2580800             -call 0x4dfd56
    cpu.fpu.push(fp2);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp2 = cpu.fpu.take();
    // 0045a464  db5df8                 +fistp dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp2));
    // 0045a467  dee9                   +fsubp st(1)
    fp0 = fp::sub(fp0, fp1);
    // 0045a469  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0045a46c  e8e5580800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a471  db5df8                 +fistp dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a474  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0045a477  eb50                   -jmp 0x45a4c9
    goto L_0x0045a4c9;
L_0x0045a479:
    // 0045a479  894df8                 -mov dword ptr [ebp - 8], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ecx;
    // 0045a47c  db45f8                 -fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a47f  dc0ddc9b5300           -fmul qword ptr [0x539bdc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a485  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a488  8b1dc8da7c00           -mov ebx, dword ptr [0x7cdac8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a48e  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0045a491  db4264                 -fild dword ptr [edx + 0x64]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(100) /* 0x64 */)));
    // 0045a494  df4208                 -fild word ptr [edx + 8]
    fp2 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 0045a497  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0045a499  d8ea                   -fsubr st(2)
    fp2 = fp::sub(x86::Float(fp0), fp2);
    // 0045a49b  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 0045a49d  dee2                   -fsubrp st(2)
    fp0 = fp::sub(fp2, x86::Float(fp0));
    // 0045a49f  db45f8                 -fild dword ptr [ebp - 8]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a4a2  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 0045a4a4  dc6ddc                 -fsubr qword ptr [ebp - 0x24]
    fp2 = fp::sub(x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-36) /* -0x24 */)), fp2);
    // 0045a4a7  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 0045a4a9  dec1                   -faddp st(1)
    fp1 = fp::add(fp1, fp2);
    // 0045a4ab  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0045a4ad  e8a4580800             -call 0x4dfd56
    cpu.fpu.push(fp1);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp1 = cpu.fpu.take();
    // 0045a4b2  d9e8                   -fld1 
    fp2 = 1.0;
    // 0045a4b4  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 0045a4b6  db5df8                 -fistp dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp2));
    // 0045a4b9  dec1                   -faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 0045a4bb  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0045a4be  e893580800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a4c3  db5df8                 -fistp dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a4c6  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
L_0x0045a4c9:
    // 0045a4c9  e802aefdff             -call 0x4352d0
    cpu.esp -= 4;
    nfs3hp::controls::setPointerBounds(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0045a4ce:
    // 0045a4ce  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a4d1  83783c17               +cmp dword ptr [eax + 0x3c], 0x17
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(23 /*0x17*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a4d5  7c07                   -jl 0x45a4de
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0045a4de;
    }
    // 0045a4d7  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0045a4dc  eb02                   -jmp 0x45a4e0
    goto L_0x0045a4e0;
L_0x0045a4de:
    // 0045a4de  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0045a4e0:
    // 0045a4e0  a3f87d6700             -mov dword ptr [0x677df8], eax
    app->getMemory<x86::reg32>(x86::reg32(6782456) /* 0x677df8 */) = cpu.eax;
    // 0045a4e5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045a4e7  740b                   -je 0x45a4f4
    if (cpu.flags.zf)
    {
        goto L_0x0045a4f4;
    }
    // 0045a4e9  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a4ec  8b403c                 -mov eax, dword ptr [eax + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 0045a4ef  83e817                 +sub eax, 0x17
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(23 /*0x17*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0045a4f2  eb06                   -jmp 0x45a4fa
    goto L_0x0045a4fa;
L_0x0045a4f4:
    // 0045a4f4  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a4f7  8b403c                 -mov eax, dword ptr [eax + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
L_0x0045a4fa:
    // 0045a4fa  a3fc7d6700             -mov dword ptr [0x677dfc], eax
    app->getMemory<x86::reg32>(x86::reg32(6782460) /* 0x677dfc */) = cpu.eax;
    // 0045a4ff  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a502  66c7407c0100           -mov word ptr [eax + 0x7c], 1
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(124) /* 0x7c */) = 1 /*0x1*/;
L_0x0045a508:
    // 0045a508  8b1dccda7c00           -mov ebx, dword ptr [0x7cdacc]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045a50e  8b0de8e55500           -mov ecx, dword ptr [0x55e5e8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */);
    // 0045a514  0fafcb                 -imul ecx, ebx
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 0045a517  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a51a  83787800               +cmp dword ptr [eax + 0x78], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(120) /* 0x78 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a51e  0f8579010000           -jne 0x45a69d
    if (!cpu.flags.zf)
    {
        goto L_0x0045a69d;
    }
    // 0045a524  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0045a529  8b3dc8da7c00           -mov edi, dword ptr [0x7cdac8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a52f  0fafc7                 -imul eax, edi
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edi)));
    // 0045a532  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0045a535  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 0045a538  db45f8                 -fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0045a53b  dc0dd49b5300           -fmul qword ptr [0x539bd4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479380) /* 0x539bd4 */)));
    // 0045a541  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a544  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a547  dc0ddc9b5300           -fmul qword ptr [0x539bdc]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a54d  db4068                 -fild dword ptr [eax + 0x68]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(104) /* 0x68 */)));
    // 0045a550  deea                   -fsubp st(2)
    fp0 = fp::sub(fp0, fp2);
    // 0045a552  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a555  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0045a557  e8fa570800             -call 0x4dfd56
    cpu.fpu.push(fp1);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp1 = cpu.fpu.take();
    // 0045a55c  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp1));
    // 0045a55f  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a562  66894206               -mov word ptr [edx + 6], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(6) /* 0x6 */) = cpu.ax;
    // 0045a566  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0045a568  db406c                 -fild dword ptr [eax + 0x6c]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(108) /* 0x6c */)));
    // 0045a56b  dee9                   -fsubp st(1)
    fp0 = fp::sub(fp0, fp1);
    // 0045a56d  e8e4570800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a572  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a575  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a578  66894208               -mov word ptr [edx + 8], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8) /* 0x8 */) = cpu.ax;
    // 0045a57c  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0045a57f  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0045a581  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    cpu.eax += x86::reg32(hudDragSlack(app, app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12)), 1)); /* port: what is drawn stays on the screen */
    // 0045a583  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a586  8b5204                 -mov edx, dword ptr [edx + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 0045a589  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0045a58c  39c2                   +cmp edx, eax
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
    // 0045a58e  7e12                   -jle 0x45a5a2
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0045a5a2;
    }
    // 0045a590  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0045a593  66a1c8da7c00           -mov ax, word ptr [0x7cdac8]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a599  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a59c  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    cpu.eax += x86::reg32(hudDragSlack(app, cpu.edx, 1)); /* port: what is drawn stays on the screen */
    // 0045a59e  66894206               -mov word ptr [edx + 6], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(6) /* 0x6 */) = cpu.ax;
L_0x0045a5a2:
    // 0045a5a2  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a5a5  6683780600             +cmp word ptr [eax + 6], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(-hudDragSlack(app, cpu.eax, 0))); /* port: was 0 */
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0045a5aa  7d06                   -jge 0x45a5b2
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0045a5b2;
    }
    // 0045a5ac  66c740060000           -mov word ptr [eax + 6], 0
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */) = x86::reg16(x86::sreg16(-hudDragSlack(app, cpu.eax, 0))); /* port: was 0 */
L_0x0045a5b2:
    // 0045a5b2  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a5b5  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0045a5b8  8b15ccda7c00           -mov edx, dword ptr [0x7cdacc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045a5be  8b4006                 -mov eax, dword ptr [eax + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0045a5c1  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    cpu.edx += x86::reg32(hudDragSlack(app, app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12)), 3)); /* port: what is drawn stays on the screen */
    // 0045a5c3  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a5c6  39d0                   +cmp eax, edx
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
    // 0045a5c8  7e12                   -jle 0x45a5dc
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0045a5dc;
    }
    // 0045a5ca  8b75f0                 -mov esi, dword ptr [ebp - 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0045a5cd  66a1ccda7c00           -mov ax, word ptr [0x7cdacc]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045a5d3  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a5d6  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    cpu.eax += x86::reg32(hudDragSlack(app, cpu.edx, 3)); /* port: what is drawn stays on the screen */
    // 0045a5d8  66894208               -mov word ptr [edx + 8], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8) /* 0x8 */) = cpu.ax;
L_0x0045a5dc:
    // 0045a5dc  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a5df  6683780800             +cmp word ptr [eax + 8], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(-hudDragSlack(app, cpu.eax, 2))); /* port: was 0 */
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0045a5e4  7d06                   -jge 0x45a5ec
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0045a5ec;
    }
    // 0045a5e6  66c740080000           -mov word ptr [eax + 8], 0
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */) = x86::reg16(x86::sreg16(-hudDragSlack(app, cpu.eax, 2))); /* port: was 0 */
L_0x0045a5ec:
    // 0045a5ec  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a5ef  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 0045a5f2  8b1dc8da7c00           -mov ebx, dword ptr [0x7cdac8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a5f8  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a5fb  0fafc3                 -imul eax, ebx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 0045a5fe  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a601  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a604  dc0dd49b5300           -fmul qword ptr [0x539bd4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479380) /* 0x539bd4 */)));
    // 0045a60a  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a60d  df4006                 -fild word ptr [eax + 6]
    fp1 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */)));
    // 0045a610  dec1                   -faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 0045a612  dd05e49b5300           -fld qword ptr [0x539be4]
    fp1 = x86::Float(app->getMemory<double>(x86::reg32(5479396) /* 0x539be4 */));
    // 0045a618  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0045a61a  d8c1                   -fadd st(1)
    fp1 = fp::add(fp1, x86::Float(fp0));
    // 0045a61c  e835570800             -call 0x4dfd56
    cpu.fpu.push(fp1);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp1 = cpu.fpu.take();
    // 0045a621  db5860                 -fistp dword ptr [eax + 0x60]
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(96) /* 0x60 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp1));
    // 0045a624  8b401e                 -mov eax, dword ptr [eax + 0x1e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(30) /* 0x1e */);
    // 0045a627  8b35ccda7c00           -mov esi, dword ptr [0x7cdacc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045a62d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a630  0fafc6                 +imul eax, esi
    {
        x86::sreg64 tmp = x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.esi));
        cpu.eax = static_cast<x86::reg32>(static_cast<x86::sreg32>(tmp));
        cpu.flags.of = cpu.flags.cf = (tmp != x86::sreg64(x86::sreg32(cpu.eax)));
    }
    // 0045a633  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a636  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a639  df4006                 +fild word ptr [eax + 6]
    fp1 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */)));
    // 0045a63c  dc0dec9b5300           +fmul qword ptr [0x539bec]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5479404) /* 0x539bec */)));
    // 0045a642  db05c8da7c00           +fild dword ptr [0x7cdac8]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */)));
    // 0045a648  def9                   +fdivp st(1)
    fp1 = fp::div(fp1, fp2);
    // 0045a64a  db45fc                 +fild dword ptr [ebp - 4]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a64d  dc0ddc9b5300           +fmul qword ptr [0x539bdc]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a653  df4008                 +fild word ptr [eax + 8]
    fp3 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 0045a656  dec1                   +faddp st(1)
    fp2 = fp::add(fp2, fp3);
    // 0045a658  dec2                   +faddp st(2)
    fp0 = fp::add(fp0, fp2);
    // 0045a65a  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a65d  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0045a65f  e8f2560800             -call 0x4dfd56
    cpu.fpu.push(fp1);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp1 = cpu.fpu.take();
    // 0045a664  db5864                 +fistp dword ptr [eax + 0x64]
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp1));
    // 0045a667  e8ea560800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a66c  db5dfc                 +fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a66f  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a672  6689421c               -mov word ptr [edx + 0x1c], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(28) /* 0x1c */) = cpu.ax;
    // 0045a676  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0045a678  df4008                 +fild word ptr [eax + 8]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 0045a67b  dc0df49b5300           +fmul qword ptr [0x539bf4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479412) /* 0x539bf4 */)));
    // 0045a681  db05ccda7c00           +fild dword ptr [0x7cdacc]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */)));
    // 0045a687  def9                   +fdivp st(1)
    fp0 = fp::div(fp0, fp1);
    // 0045a689  e8c8560800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a68e  db5dfc                 +fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a691  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a694  6689421a               -mov word ptr [edx + 0x1a], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(26) /* 0x1a */) = cpu.ax;
    // 0045a698  e9cf030000             -jmp 0x45aa6c
    goto L_0x0045aa6c;
L_0x0045a69d:
    // 0045a69d  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 0045a6a4  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 0045a6a6  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0045a6a9  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a6ac  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0045a6ae  8a5a79                 -mov bl, byte ptr [edx + 0x79]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(121) /* 0x79 */);
    // 0045a6b1  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 0045a6b3  f6c301                 +test bl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bl & 1 /*0x1*/));
    // 0045a6b6  0f84f4000000           -je 0x45a7b0
    if (cpu.flags.zf)
    {
        goto L_0x0045a7b0;
    }
    // 0045a6bc  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 0045a6bf  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a6c2  dc0ddc9b5300           -fmul qword ptr [0x539bdc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a6c8  db426c                 -fild dword ptr [edx + 0x6c]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(108) /* 0x6c */)));
    // 0045a6cb  dee9                   -fsubp st(1)
    fp0 = fp::sub(fp0, fp1);
    // 0045a6cd  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a6d0  e881560800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a6d5  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a6d8  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a6db  66895108               -mov word ptr [ecx + 8], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.dx;
    // 0045a6df  8b5106                 -mov edx, dword ptr [ecx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 0045a6e2  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0045a6e5  8b4964                 -mov ecx, dword ptr [ecx + 0x64]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(100) /* 0x64 */);
    // 0045a6e8  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045a6ea  8d5101                 -lea edx, [ecx + 1]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(1) /* 0x1 */);
    // 0045a6ed  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0045a6f0  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a6f3  dc0df49b5300           -fmul qword ptr [0x539bf4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479412) /* 0x539bf4 */)));
    // 0045a6f9  db05ccda7c00           -fild dword ptr [0x7cdacc]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */)));
    // 0045a6ff  dd5dbc                 -fstp qword ptr [ebp - 0x44]
    app->getMemory<double>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = double(fp1);
    // 0045a702  d9e8                   -fld1 
    fp1 = 1.0;
    // 0045a704  dc75bc                 -fdiv qword ptr [ebp - 0x44]
    fp1 = fp::div(fp1, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-68) /* -0x44 */)));
    // 0045a707  dd5dcc                 -fstp qword ptr [ebp - 0x34]
    app->getMemory<double>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = double(fp1);
    // 0045a70a  dc4dcc                 -fmul qword ptr [ebp - 0x34]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-52) /* -0x34 */)));
    // 0045a70d  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a710  e841560800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a715  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a718  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a71b  66895120               -mov word ptr [ecx + 0x20], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(32) /* 0x20 */) = cpu.dx;
    // 0045a71f  6683fa1a               +cmp dx, 0x1a
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(26 /*0x1a*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0045a723  7d2e                   -jge 0x45a753
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0045a753;
    }
    // 0045a725  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a728  db45fc                 +fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a72b  dc0ddc9b5300           +fmul qword ptr [0x539bdc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a731  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0045a733  66c741201a00           -mov word ptr [ecx + 0x20], 0x1a
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(32) /* 0x20 */) = 26 /*0x1a*/;
    // 0045a739  db4064                 +fild dword ptr [eax + 0x64]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */)));
    // 0045a73c  dee1                   +fsubrp st(1)
    fp0 = fp::sub(fp1, x86::Float(fp0));
    // 0045a73e  d9e8                   +fld1 
    fp1 = 1.0;
    // 0045a740  dec1                   +faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 0045a742  e80f560800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a747  db5dfc                 +fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a74a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a74d  66894108               -mov word ptr [ecx + 8], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.ax;
    // 0045a751  eb32                   -jmp 0x45a785
    goto L_0x0045a785;
L_0x0045a753:
    // 0045a753  6683790800             +cmp word ptr [ecx + 8], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0045a758  7d2b                   -jge 0x45a785
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0045a785;
    }
    // 0045a75a  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a75d  db4064                 +fild dword ptr [eax + 0x64]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */)));
    // 0045a760  dc0df49b5300           +fmul qword ptr [0x539bf4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479412) /* 0x539bf4 */)));
    // 0045a766  dc4dcc                 +fmul qword ptr [ebp - 0x34]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-52) /* -0x34 */)));
    // 0045a769  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a76c  d9e8                   +fld1 
    fp1 = 1.0;
    // 0045a76e  dec1                   +faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 0045a770  66c740080000           -mov word ptr [eax + 8], 0
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 0045a776  e8db550800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a77b  db5dfc                 +fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a77e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a781  66894220               -mov word ptr [edx + 0x20], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(32) /* 0x20 */) = cpu.ax;
L_0x0045a785:
    // 0045a785  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a788  df4008                 +fild word ptr [eax + 8]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 0045a78b  dc0df49b5300           +fmul qword ptr [0x539bf4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479412) /* 0x539bf4 */)));
    // 0045a791  db05ccda7c00           +fild dword ptr [0x7cdacc]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */)));
    // 0045a797  def9                   +fdivp st(1)
    fp0 = fp::div(fp0, fp1);
    // 0045a799  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a79c  e8b5550800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a7a1  db5dfc                 +fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a7a4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a7a7  6689421a               -mov word ptr [edx + 0x1a], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(26) /* 0x1a */) = cpu.ax;
    // 0045a7ab  e9cd000000             -jmp 0x45a87d
    goto L_0x0045a87d;
L_0x0045a7b0:
    // 0045a7b0  f6c302                 +test bl, 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bl & 2 /*0x2*/));
    // 0045a7b3  0f84c4000000           -je 0x45a87d
    if (cpu.flags.zf)
    {
        goto L_0x0045a87d;
    }
    // 0045a7b9  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 0045a7bc  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a7bf  dc0ddc9b5300           -fmul qword ptr [0x539bdc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a7c5  db426c                 -fild dword ptr [edx + 0x6c]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(108) /* 0x6c */)));
    // 0045a7c8  dec1                   -faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 0045a7ca  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a7cd  e884550800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a7d2  db5a64                 -fistp dword ptr [edx + 0x64]
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(100) /* 0x64 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a7d5  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0045a7d8  8b4964                 -mov ecx, dword ptr [ecx + 0x64]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(100) /* 0x64 */);
    // 0045a7db  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0045a7de  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045a7e0  8d5101                 -lea edx, [ecx + 1]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(1) /* 0x1 */);
    // 0045a7e3  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0045a7e6  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a7e9  dc0df49b5300           -fmul qword ptr [0x539bf4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479412) /* 0x539bf4 */)));
    // 0045a7ef  db05ccda7c00           -fild dword ptr [0x7cdacc]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */)));
    // 0045a7f5  dd5d9c                 -fstp qword ptr [ebp - 0x64]
    app->getMemory<double>(cpu.ebp + x86::reg32(-100) /* -0x64 */) = double(fp1);
    // 0045a7f8  d9e8                   -fld1 
    fp1 = 1.0;
    // 0045a7fa  dc759c                 -fdiv qword ptr [ebp - 0x64]
    fp1 = fp::div(fp1, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-100) /* -0x64 */)));
    // 0045a7fd  dd5dd4                 -fstp qword ptr [ebp - 0x2c]
    app->getMemory<double>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = double(fp1);
    // 0045a800  dc4dd4                 -fmul qword ptr [ebp - 0x2c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-44) /* -0x2c */)));
    // 0045a803  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a806  e84b550800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a80b  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a80e  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a811  66895120               -mov word ptr [ecx + 0x20], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(32) /* 0x20 */) = cpu.dx;
    // 0045a815  6683fa1a               +cmp dx, 0x1a
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(26 /*0x1a*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0045a819  7d29                   -jge 0x45a844
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0045a844;
    }
    // 0045a81b  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a81e  db45fc                 +fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a821  dc0ddc9b5300           +fmul qword ptr [0x539bdc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479388) /* 0x539bdc */)));
    // 0045a827  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0045a829  66c741201a00           -mov word ptr [ecx + 0x20], 0x1a
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(32) /* 0x20 */) = 26 /*0x1a*/;
    // 0045a82f  df4008                 +fild word ptr [eax + 8]
    fp1 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 0045a832  dec1                   +faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 0045a834  dc05e49b5300           +fadd qword ptr [0x539be4]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479396) /* 0x539be4 */)));
    // 0045a83a  e817550800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a83f  db5864                 +fistp dword ptr [eax + 0x64]
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a842  eb39                   -jmp 0x45a87d
    goto L_0x0045a87d;
L_0x0045a844:
    // 0045a844  8b3dccda7c00           -mov edi, dword ptr [0x7cdacc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045a84a  3b7964                 +cmp edi, dword ptr [ecx + 0x64]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(100) /* 0x64 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045a84d  7f2e                   -jg 0x45a87d
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0045a87d;
    }
    // 0045a84f  8d47ff                 -lea eax, [edi - 1]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */);
    // 0045a852  894164                 -mov dword ptr [ecx + 0x64], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(100) /* 0x64 */) = cpu.eax;
    // 0045a855  8b4106                 -mov eax, dword ptr [ecx + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 0045a858  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0045a85a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a85d  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0045a85f  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0045a862  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a865  dc0df49b5300           -fmul qword ptr [0x539bf4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479412) /* 0x539bf4 */)));
    // 0045a86b  dc4dd4                 -fmul qword ptr [ebp - 0x2c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-44) /* -0x2c */)));
    // 0045a86e  e8e3540800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a873  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a876  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a879  66894120               -mov word ptr [ecx + 0x20], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(32) /* 0x20 */) = cpu.ax;
L_0x0045a87d:
    // 0045a87d  8b15c8da7c00           -mov edx, dword ptr [0x7cdac8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a883  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0045a88a  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045a88c  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0045a88f  8b0dc8da7c00           -mov ecx, dword ptr [0x7cdac8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045a895  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0045a897  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0045a89c  0fafc1                 -imul eax, ecx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ecx)));
    // 0045a89f  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a8a2  8a5979                 -mov bl, byte ptr [ecx + 0x79]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(121) /* 0x79 */);
    // 0045a8a5  01d2                   -add edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0045a8a7  f6c304                 +test bl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bl & 4 /*0x4*/));
    // 0045a8aa  0f84f0000000           -je 0x45a9a0
    if (cpu.flags.zf)
    {
        goto L_0x0045a9a0;
    }
    // 0045a8b0  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a8b3  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a8b6  dc0dd49b5300           -fmul qword ptr [0x539bd4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479380) /* 0x539bd4 */)));
    // 0045a8bc  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0045a8be  db4068                 -fild dword ptr [eax + 0x68]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(104) /* 0x68 */)));
    // 0045a8c1  dee9                   -fsubp st(1)
    fp0 = fp::sub(fp0, fp1);
    // 0045a8c3  e88e540800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a8c8  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a8cb  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a8ce  66894106               -mov word ptr [ecx + 6], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */) = cpu.ax;
    // 0045a8d2  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0045a8d5  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a8d8  8b4960                 -mov ecx, dword ptr [ecx + 0x60]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(96) /* 0x60 */);
    // 0045a8db  29c1                   -sub ecx, eax
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0045a8dd  8d4101                 -lea eax, [ecx + 1]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(1) /* 0x1 */);
    // 0045a8e0  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a8e3  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a8e6  dc0dec9b5300           -fmul qword ptr [0x539bec]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479404) /* 0x539bec */)));
    // 0045a8ec  db05c8da7c00           -fild dword ptr [0x7cdac8]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */)));
    // 0045a8f2  dd5db4                 -fstp qword ptr [ebp - 0x4c]
    app->getMemory<double>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = double(fp1);
    // 0045a8f5  d9e8                   -fld1 
    fp1 = 1.0;
    // 0045a8f7  dc75b4                 -fdiv qword ptr [ebp - 0x4c]
    fp1 = fp::div(fp1, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-76) /* -0x4c */)));
    // 0045a8fa  dd5de4                 -fstp qword ptr [ebp - 0x1c]
    app->getMemory<double>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = double(fp1);
    // 0045a8fd  dc4de4                 -fmul qword ptr [ebp - 0x1c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-28) /* -0x1c */)));
    // 0045a900  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a903  e84e540800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a908  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a90b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a90e  6689411e               -mov word ptr [ecx + 0x1e], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(30) /* 0x1e */) = cpu.ax;
    // 0045a912  663d1a00               +cmp ax, 0x1a
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(26 /*0x1a*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0045a916  7d2e                   -jge 0x45a946
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0045a946;
    }
    // 0045a918  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0045a91b  db45fc                 +fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a91e  dc0dd49b5300           +fmul qword ptr [0x539bd4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479380) /* 0x539bd4 */)));
    // 0045a924  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0045a926  66c7411e1a00           -mov word ptr [ecx + 0x1e], 0x1a
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(30) /* 0x1e */) = 26 /*0x1a*/;
    // 0045a92c  db4060                 +fild dword ptr [eax + 0x60]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(96) /* 0x60 */)));
    // 0045a92f  dee1                   +fsubrp st(1)
    fp0 = fp::sub(fp1, x86::Float(fp0));
    // 0045a931  d9e8                   +fld1 
    fp1 = 1.0;
    // 0045a933  dec1                   +faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 0045a935  e81c540800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a93a  db5dfc                 +fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a93d  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a940  66894106               -mov word ptr [ecx + 6], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */) = cpu.ax;
    // 0045a944  eb2f                   -jmp 0x45a975
    goto L_0x0045a975;
L_0x0045a946:
    // 0045a946  6683790600             +cmp word ptr [ecx + 6], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0045a94b  7d28                   -jge 0x45a975
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0045a975;
    }
    // 0045a94d  8b4160                 -mov eax, dword ptr [ecx + 0x60]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(96) /* 0x60 */);
    // 0045a950  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0045a951  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a954  db45fc                 +fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a957  dc0dec9b5300           +fmul qword ptr [0x539bec]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479404) /* 0x539bec */)));
    // 0045a95d  dc4de4                 +fmul qword ptr [ebp - 0x1c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-28) /* -0x1c */)));
    // 0045a960  66c741060000           -mov word ptr [ecx + 6], 0
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */) = 0 /*0x0*/;
    // 0045a966  e8eb530800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a96b  db5dfc                 +fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a96e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a971  6689411e               -mov word ptr [ecx + 0x1e], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(30) /* 0x1e */) = cpu.ax;
L_0x0045a975:
    // 0045a975  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a978  df4006                 +fild word ptr [eax + 6]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */)));
    // 0045a97b  dc0dec9b5300           +fmul qword ptr [0x539bec]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479404) /* 0x539bec */)));
    // 0045a981  db05c8da7c00           +fild dword ptr [0x7cdac8]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */)));
    // 0045a987  def9                   +fdivp st(1)
    fp0 = fp::div(fp0, fp1);
    // 0045a989  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a98c  e8c5530800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a991  db5dfc                 +fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a994  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045a997  6689421c               -mov word ptr [edx + 0x1c], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(28) /* 0x1c */) = cpu.ax;
    // 0045a99b  e9cc000000             -jmp 0x45aa6c
    goto L_0x0045aa6c;
L_0x0045a9a0:
    // 0045a9a0  f6c308                 +test bl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bl & 8 /*0x8*/));
    // 0045a9a3  0f84c3000000           -je 0x45aa6c
    if (cpu.flags.zf)
    {
        goto L_0x0045aa6c;
    }
    // 0045a9a9  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a9ac  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a9af  dc0dd49b5300           -fmul qword ptr [0x539bd4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479380) /* 0x539bd4 */)));
    // 0045a9b5  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0045a9b7  db4068                 -fild dword ptr [eax + 0x68]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(104) /* 0x68 */)));
    // 0045a9ba  dec1                   -faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 0045a9bc  e895530800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a9c1  db5860                 -fistp dword ptr [eax + 0x60]
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(96) /* 0x60 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a9c4  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0045a9c7  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045a9ca  8b4960                 -mov ecx, dword ptr [ecx + 0x60]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(96) /* 0x60 */);
    // 0045a9cd  29c1                   -sub ecx, eax
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0045a9cf  8d4101                 -lea eax, [ecx + 1]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(1) /* 0x1 */);
    // 0045a9d2  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045a9d5  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045a9d8  dc0dec9b5300           -fmul qword ptr [0x539bec]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479404) /* 0x539bec */)));
    // 0045a9de  db05c8da7c00           -fild dword ptr [0x7cdac8]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */)));
    // 0045a9e4  dd5dac                 -fstp qword ptr [ebp - 0x54]
    app->getMemory<double>(cpu.ebp + x86::reg32(-84) /* -0x54 */) = double(fp1);
    // 0045a9e7  d9e8                   -fld1 
    fp1 = 1.0;
    // 0045a9e9  dc75ac                 -fdiv qword ptr [ebp - 0x54]
    fp1 = fp::div(fp1, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-84) /* -0x54 */)));
    // 0045a9ec  dd5dc4                 -fstp qword ptr [ebp - 0x3c]
    app->getMemory<double>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = double(fp1);
    // 0045a9ef  dc4dc4                 -fmul qword ptr [ebp - 0x3c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-60) /* -0x3c */)));
    // 0045a9f2  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045a9f5  e85c530800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045a9fa  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045a9fd  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045aa00  6689411e               -mov word ptr [ecx + 0x1e], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(30) /* 0x1e */) = cpu.ax;
    // 0045aa04  663d1a00               +cmp ax, 0x1a
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(26 /*0x1a*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0045aa08  7d29                   -jge 0x45aa33
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0045aa33;
    }
    // 0045aa0a  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0045aa0d  db45fc                 +fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045aa10  dc0dd49b5300           +fmul qword ptr [0x539bd4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479380) /* 0x539bd4 */)));
    // 0045aa16  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0045aa18  66c7411e1a00           -mov word ptr [ecx + 0x1e], 0x1a
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(30) /* 0x1e */) = 26 /*0x1a*/;
    // 0045aa1e  df4006                 +fild word ptr [eax + 6]
    fp1 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */)));
    // 0045aa21  dec1                   +faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 0045aa23  dc05e49b5300           +fadd qword ptr [0x539be4]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479396) /* 0x539be4 */)));
    // 0045aa29  e828530800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045aa2e  db5860                 +fistp dword ptr [eax + 0x60]
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(96) /* 0x60 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045aa31  eb39                   -jmp 0x45aa6c
    goto L_0x0045aa6c;
L_0x0045aa33:
    // 0045aa33  8b1dc8da7c00           -mov ebx, dword ptr [0x7cdac8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045aa39  3b5960                 +cmp ebx, dword ptr [ecx + 0x60]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(96) /* 0x60 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045aa3c  7f2e                   -jg 0x45aa6c
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0045aa6c;
    }
    // 0045aa3e  8d43ff                 -lea eax, [ebx - 1]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(-1) /* -0x1 */);
    // 0045aa41  894160                 -mov dword ptr [ecx + 0x60], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(96) /* 0x60 */) = cpu.eax;
    // 0045aa44  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0045aa47  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0045aa49  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045aa4c  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0045aa4e  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0045aa51  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0045aa54  dc0dec9b5300           -fmul qword ptr [0x539bec]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5479404) /* 0x539bec */)));
    // 0045aa5a  dc4dc4                 -fmul qword ptr [ebp - 0x3c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-60) /* -0x3c */)));
    // 0045aa5d  e8f4520800             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0045aa62  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0045aa65  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045aa68  6689411e               -mov word ptr [ecx + 0x1e], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(30) /* 0x1e */) = cpu.ax;
L_0x0045aa6c:
    // 0045aa6c  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045aa6f  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045aa72  e839f3ffff             -call 0x459db0
    cpu.esp -= 4;
    sub_459db0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045aa77  894270                 -mov dword ptr [edx + 0x70], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(112) /* 0x70 */) = cpu.eax;
    // 0045aa7a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045aa7c  7446                   -je 0x45aac4
    if (cpu.flags.zf)
    {
        goto L_0x0045aac4;
    }
    // 0045aa7e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0045aa80  bf007e6700             -mov edi, 0x677e00
    cpu.edi = 6782464 /*0x677e00*/;
    // 0045aa85  8d701a                 -lea esi, [eax + 0x1a]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(26) /* 0x1a */);
    // 0045aa88  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0045aa89  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0045aa8a  eb38                   -jmp 0x45aac4
    goto L_0x0045aac4;
L_0x0045aa8c:
    // 0045aa8c  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0045aa8f  6683787c00             +cmp word ptr [eax + 0x7c], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(124) /* 0x7c */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0045aa94  742e                   -je 0x45aac4
    if (cpu.flags.zf)
    {
        goto L_0x0045aac4;
    }
    // 0045aa96  8a4805                 -mov cl, byte ptr [eax + 5]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */);
    // 0045aa99  66c7407c0000           -mov word ptr [eax + 0x7c], 0
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(124) /* 0x7c */) = 0 /*0x0*/;
    // 0045aa9f  80e1fb                 -and cl, 0xfb
    cpu.cl &= x86::reg8(x86::sreg8(251 /*0xfb*/));
    // 0045aaa2  ba007e6700             -mov edx, 0x677e00
    cpu.edx = 6782464 /*0x677e00*/;
    // 0045aaa7  884805                 -mov byte ptr [eax + 5], cl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) = cpu.cl;
    // 0045aaaa  e831f5ffff             -call 0x459fe0
    cpu.esp -= 4;
    sub_459fe0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045aaaf  8b0dccda7c00           -mov ecx, dword ptr [0x7cdacc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0045aab5  8b1dc8da7c00           -mov ebx, dword ptr [0x7cdac8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0045aabb  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0045aabd  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0045aabf  e80ca8fdff             -call 0x4352d0
    cpu.esp -= 4;
    nfs3hp::controls::setPointerBounds(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0045aac4:
    // 0045aac4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0045aac6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045aac7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045aac8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045aac9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045aaca  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045aacb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045aacc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_45ab30: native, nfs3hp::frontend::hudDrawElement */

/* sub_45b940: native, nfs3hp::frontend::hudElementPickCursor */

/* sub_45bcb0: native, nfs3hp::frontend::hudElementDraw */

/* sub_45bd20: native, nfs3hp::frontend::hudElementKey */

/* sub_45c9d0: native, nfs3hp::frontend::hudElementToRecord */

/* sub_45ca90: native, nfs3hp::frontend::hudEditPlaceElement */

/* sub_45cb50: native, nfs3hp::frontend::hudElementHidden */

/* sub_45cdd0: native, nfs3hp::frontend::mainMenuPlayerNameAnswer */

/* sub_45ce60: native, nfs3hp::frontend::splitMenuPlayer2NameAnswer */

/* sub_45cef0: native, nfs3hp::frontend::mainMenuOnPlayerName */

/* sub_45cf70: native, nfs3hp::frontend::splitMenuOnPlayer2Name */

/* sub_45cff0: native, nfs3hp::frontend::mainMenuOnFrame */

/* sub_45d050: native, nfs3hp::frontend::mainMenuOnEnter */

/* sub_45d2f0: native, nfs3hp::frontend::mainMenuOnExit */

/* sub_45d300: native, nfs3hp::frontend::splitMenuOnFrame */

/* sub_45d360: native, nfs3hp::frontend::splitMenuOnEnter */

/* sub_45d620: native, nfs3hp::frontend::splitMenuOnExit */

/* sub_45d630: native, nfs3hp::frontend::levelOnEnter */

/* sub_45d770: native, nfs3hp::frontend::pauseCameraAnswered */

/* sub_45d840: native, nfs3hp::frontend::pauseCameraKey */

/* sub_45d920: native, nfs3hp::frontend::pauseCameraOnEnter */

/* sub_45d9a0: native, nfs3hp::frontend::pauseCameraEnsureOne */

/* sub_45d9e0: native, nfs3hp::frontend::pauseCameraOnExit */

/* sub_45dab0: native, nfs3hp::frontend::raceTypeOnExit */

/* sub_45dae0: native, nfs3hp::frontend::hudEditShowSpeedometer */

/* sub_45deb0: native, nfs3hp::frontend::hudEditShowTachometer */

/* sub_45e160: native, nfs3hp::frontend::hudEditGreyMirrors */

/* sub_45e210: native, nfs3hp::frontend::hudEditGreyOpponentNames */

/* sub_45e310: native, nfs3hp::frontend::hudEditPlaceImages */

/* sub_45e3d0: native, nfs3hp::frontend::hudEditShowScreenItems */

/* sub_45e960: native, nfs3hp::frontend::hudEditOnEnter */

/* sub_45f160: native, nfs3hp::frontend::hudEditOnExit */

/* sub_45f300: native, nfs3hp::frontend::hudEditSetScreenArt */

/* sub_45f4c0: native, nfs3hp::frontend::hudEditOnFrame */

/* sub_45fae0: native, nfs3hp::port::buttons::rectDraw */

/* sub_45fc90: native, nfs3hp::port::buttons::speechDraw */

/* sub_45fca0: native, nfs3hp::port::buttons::speechShown */

/* sub_45fcb0: native, nfs3hp::port::buttons::speechHidden */

/* sub_45fcd0: native, nfs3hp::port::buttons::textDraw */

/* sub_460070: native, nfs3hp::port::buttons::textUp */

/* sub_4600b0: native, nfs3hp::port::buttons::textShown */

/* sub_460100: native, nfs3hp::port::buttons::imageDraw */

/* sub_460410: native, nfs3hp::port::buttons::imageUp */

/* sub_460440: native, nfs3hp::port::buttons::imageShown */

/* sub_460700: native, nfs3hp::port::buttons::pulseOn */

/* sub_460760: native, nfs3hp::frontend::resetButtonPulse */

/* sub_460770: native, nfs3hp::port::buttons::pulseColour */

/* sub_4607c0: native, nfs3hp::port::buttons::buttonDraw */

/* sub_461270: native, nfs3hp::port::buttons::buttonKey */

/* sub_4613e0: native, nfs3hp::port::buttons::layoutMenuItem */

/* sub_461900: native, nfs3hp::port::buttons::buttonShown */

/* sub_461a70: native, nfs3hp::port::buttons::cascadeLayoutClosed */

/* sub_461aa0: native, nfs3hp::port::buttons::cascadeLineText */

/* sub_461d00: native, nfs3hp::port::buttons::cascadeDraw */

/* sub_462320: native, nfs3hp::port::buttons::cascadePass */

/* sub_4623a0: native, nfs3hp::port::buttons::cascadeShown */

/* sub_4624a0: native, nfs3hp::port::buttons::cascadeKey */

/* sub_4628d0: native, nfs3hp::port::buttons::cascadeUp */

/* sub_4629c0: native, nfs3hp::port::buttons::checkboxDraw */

/* sub_462ad0: native, nfs3hp::port::buttons::checkboxKey */

/* sub_462b20: native, nfs3hp::port::buttons::checkboxShown */

/* sub_462b30: native, nfs3hp::port::buttons::checkboxUp */

/* sub_462b90: native, nfs3hp::port::buttons::boxOverlayDraw */

/* sub_462c30: native, nfs3hp::port::buttons::selectListShown */

/* sub_462ce0: native, nfs3hp::port::buttons::selectListHidden */

/* sub_462d20: native, nfs3hp::port::buttons::selectListDraw */

/* sub_462f90: native, nfs3hp::port::buttons::selectListPass */

/* sub_462fa0: native, nfs3hp::port::buttons::selectListKey */

/* sub_4633d0: native, nfs3hp::port::buttons::selectListUp */

/* sub_4634d0: native, nfs3hp::port::buttons::dropListDraw */

/* sub_463de0: native, nfs3hp::port::buttons::dropListPass */

/* sub_463e60: native, nfs3hp::port::buttons::dropListShown */

/* sub_463ff0: native, nfs3hp::port::buttons::listKey */

/* sub_464560: native, nfs3hp::port::buttons::layoutList */

/* sub_4646e0: native, nfs3hp::port::view::storeIncremented */

/* sub_464700: native, nfs3hp::frontend::viewMakeTiles */

/* sub_4647a0: native, nfs3hp::frontend::viewFillTiles */

/* sub_464840: native, nfs3hp::port::view::gridDrawn */

/* sub_464eb0: native, nfs3hp::frontend::viewLoadInterior */

/* sub_464fd0: native, nfs3hp::port::view::viewFrame */

/* sub_4651e0: native, nfs3hp::port::view::panKey */

}
