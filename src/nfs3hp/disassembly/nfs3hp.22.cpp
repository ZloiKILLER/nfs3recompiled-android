#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_496550(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00496550  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00496551  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00496552  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00496553  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00496554  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00496556  8b1a                   -mov ebx, dword ptr [edx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx);
    // 00496558  8b08                   -mov ecx, dword ptr [eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax);
    // 0049655a  29d9                   -sub ecx, ebx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049655c  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0049655e  7f02                   -jg 0x496562
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00496562;
    }
    // 00496560  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
L_0x00496562:
    // 00496562  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 00496564  8b7204                 -mov esi, dword ptr [edx + 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 00496567  8b4804                 -mov ecx, dword ptr [eax + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0049656a  29f1                   -sub ecx, esi
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 0049656c  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0049656e  7f02                   -jg 0x496572
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00496572;
    }
    // 00496570  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
L_0x00496572:
    // 00496572  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00496575  2b4208                 -sub eax, dword ptr [edx + 8]
    (cpu.eax) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00496578  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049657a  7f02                   -jg 0x49657e
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0049657e;
    }
    // 0049657c  f7d8                   -neg eax
    cpu.eax = ~cpu.eax + 1;
L_0x0049657e:
    // 0049657e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00496580  39cb                   +cmp ebx, ecx
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00496582  7e09                   -jle 0x49658d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0049658d;
    }
    // 00496584  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00496586  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 00496589  01d8                   +add eax, ebx
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
    // 0049658b  eb07                   -jmp 0x496594
    goto L_0x00496594;
L_0x0049658d:
    // 0049658d  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0049658f  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 00496592  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
L_0x00496594:
    // 00496594  39d0                   +cmp eax, edx
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
    // 00496596  7e05                   -jle 0x49659d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0049659d;
    }
    // 00496598  c1fa02                 +sar edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 0049659b  eb03                   -jmp 0x4965a0
    goto L_0x004965a0;
L_0x0049659d:
    // 0049659d  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
L_0x004965a0:
    // 004965a0  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004965a2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004965a3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004965a4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004965a5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004965a6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4965b0: native, nfs3hp::frontend::simQuickDistance */

/* sub_496690: native, nfs3hp::dead::stub_0x496690 */

/* sub_4967e0: native, nfs3hp::frontend::simPlanarLength */

/* sub_4968a0: native, nfs3hp::vertices::groundHeight */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4969c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004969c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004969c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004969c2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004969c3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004969c4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004969c5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004969c6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004969c8  83ec3c                 -sub esp, 0x3c
    (cpu.esp) -= x86::reg32(x86::sreg32(60 /*0x3c*/));
    // 004969cb  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004969cd  8b451c                 -mov eax, dword ptr [ebp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004969d0  8b5528                 -mov edx, dword ptr [ebp + 0x28]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(40) /* 0x28 */);
    // 004969d3  8b5d2c                 -mov ebx, dword ptr [ebp + 0x2c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(44) /* 0x2c */);
    // 004969d6  8b7530                 -mov esi, dword ptr [ebp + 0x30]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(48) /* 0x30 */);
    // 004969d9  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004969db  8b4520                 -mov eax, dword ptr [ebp + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */);
    // 004969de  895528                 -mov dword ptr [ebp + 0x28], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(40) /* 0x28 */) = cpu.edx;
    // 004969e1  8d55c4                 -lea edx, [ebp - 0x3c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 004969e4  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004969e6  8b4524                 -mov eax, dword ptr [ebp + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(36) /* 0x24 */);
    // 004969e9  895d2c                 -mov dword ptr [ebp + 0x2c], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(44) /* 0x2c */) = cpu.ebx;
    // 004969ec  8d5de8                 -lea ebx, [ebp - 0x18]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004969ef  29c6                   -sub esi, eax
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004969f1  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004969f3  897530                 -mov dword ptr [ebp + 0x30], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(48) /* 0x30 */) = cpu.esi;
    // 004969f6  e8e5da0500             -call 0x4f44e0
    cpu.esp -= 4;
    sub_4f44e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004969fb  8d55c4                 -lea edx, [ebp - 0x3c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 004969fe  8d4528                 -lea eax, [ebp + 0x28]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(40) /* 0x28 */);
    // 00496a01  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00496a03  e83cdb0500             -call 0x4f4544
    cpu.esp -= 4;
    sub_4f4544(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00496a08  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00496a0b  8d45e8                 -lea eax, [ebp - 0x18]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00496a0e  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00496a10  897dec                 -mov dword ptr [ebp - 0x14], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edi;
    // 00496a13  e82cdb0500             -call 0x4f4544
    cpu.esp -= 4;
    sub_4f4544(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00496a18  8b451c                 -mov eax, dword ptr [ebp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 00496a1b  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00496a1e  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00496a21  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00496a23  8b4520                 -mov eax, dword ptr [ebp + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */);
    // 00496a26  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00496a29  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00496a2b  8b4524                 -mov eax, dword ptr [ebp + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(36) /* 0x24 */);
    // 00496a2e  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
    // 00496a31  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00496a33  894df8                 -mov dword ptr [ebp - 8], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ecx;
    // 00496a36  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 00496a39  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00496a3b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00496a3d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496a3e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496a3f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496a40  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496a41  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496a42  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496a43  c21800                 -ret 0x18
    cpu.esp += 4+24 /*0x18*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_496a50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
  x86::reg16 fsw = 0;
    // 00496a50  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00496a51  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00496a52  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00496a53  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00496a54  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00496a55  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00496a57  83ec28                 +sub esp, 0x28
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(40 /*0x28*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00496a5a  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00496a5d  d900                   +fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00496a5f  d8600c                 +fsub dword ptr [eax + 0xc]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(12) /* 0xc */)));
    // 00496a62  d95df0                 +fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp0);
    // 00496a65  d94004                 +fld dword ptr [eax + 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00496a68  d86010                 +fsub dword ptr [eax + 0x10]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(16) /* 0x10 */)));
    // 00496a6b  d95df4                 +fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp0);
    // 00496a6e  d94008                 +fld dword ptr [eax + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00496a71  d86014                 +fsub dword ptr [eax + 0x14]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(20) /* 0x14 */)));
    // 00496a74  d95df8                 +fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 00496a77  d94018                 +fld dword ptr [eax + 0x18]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(24) /* 0x18 */));
    // 00496a7a  d8600c                 +fsub dword ptr [eax + 0xc]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(12) /* 0xc */)));
    // 00496a7d  8d5dd8                 -lea ebx, [ebp - 0x28]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 00496a80  d95de4                 +fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp0);
    // 00496a83  d9401c                 +fld dword ptr [eax + 0x1c]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(28) /* 0x1c */));
    // 00496a86  d86010                 +fsub dword ptr [eax + 0x10]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(16) /* 0x10 */)));
    // 00496a89  8d55e4                 -lea edx, [ebp - 0x1c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00496a8c  d95de8                 +fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp0);
    // 00496a8f  d94020                 +fld dword ptr [eax + 0x20]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(32) /* 0x20 */));
    // 00496a92  d86014                 +fsub dword ptr [eax + 0x14]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(20) /* 0x14 */)));
    // 00496a95  8d45f0                 -lea eax, [ebp - 0x10]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00496a98  d95dec                 +fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fp0);
    // 00496a9b  e8c0960400             -call 0x4e0160
    cpu.esp -= 4;
    sub_4e0160(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00496aa0  8d45f0                 -lea eax, [ebp - 0x10]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00496aa3  e848080000             -call 0x4972f0
    cpu.esp -= 4;
    nfs3hp::vertices::normalise(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00496aa8  d9ee                   +fldz 
    fp1 = 0.0;
    // 00496aaa  ded9                   +fcompp 
    fsw = fp::compare(fp1, fp0);
    // 00496aac  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00496aae  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00496aaf  7442                   -je 0x496af3
    if (cpu.flags.zf)
    {
        goto L_0x00496af3;
    }
    // 00496ab1  8d45d8                 -lea eax, [ebp - 0x28]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 00496ab4  e837080000             -call 0x4972f0
    cpu.esp -= 4;
    nfs3hp::vertices::normalise(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00496ab9  d9ee                   +fldz 
    fp1 = 0.0;
    // 00496abb  ded9                   +fcompp 
    fsw = fp::compare(fp1, fp0);
    // 00496abd  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00496abf  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00496ac0  7431                   -je 0x496af3
    if (cpu.flags.zf)
    {
        goto L_0x00496af3;
    }
    // 00496ac2  8d45e4                 -lea eax, [ebp - 0x1c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00496ac5  e826080000             -call 0x4972f0
    cpu.esp -= 4;
    nfs3hp::vertices::normalise(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00496aca  d9ee                   +fldz 
    fp1 = 0.0;
    // 00496acc  ded9                   +fcompp 
    fsw = fp::compare(fp1, fp0);
    // 00496ace  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00496ad0  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00496ad1  7420                   -je 0x496af3
    if (cpu.flags.zf)
    {
        goto L_0x00496af3;
    }
    // 00496ad3  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00496ad6  8d75f0                 -lea esi, [ebp - 0x10]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00496ad9  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00496ada  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00496adb  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00496adc  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00496adf  8d75d8                 -lea esi, [ebp - 0x28]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 00496ae2  8d780c                 -lea edi, [eax + 0xc]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00496ae5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00496ae6  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00496ae7  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00496ae8  8d75e4                 -lea esi, [ebp - 0x1c]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00496aeb  8d7818                 -lea edi, [eax + 0x18]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 00496aee  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00496aef  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00496af0  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00496af1  eb0f                   -jmp 0x496b02
    goto L_0x00496b02;
L_0x00496af3:
    // 00496af3  b909000000             -mov ecx, 9
    cpu.ecx = 9 /*0x9*/;
    // 00496af8  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00496afb  be98445600             -mov esi, 0x564498
    cpu.esi = 5653656 /*0x564498*/;
    // 00496b00  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x00496b02:
    // 00496b02  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00496b04  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496b05  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496b06  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496b07  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496b08  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496b09  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_496b10: native, nfs3hp::dead::stub_0x496b10 */

/* sub_496b60: native, nfs3hp::dead::stub_0x496b60 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_496bf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4;
    // 00496bf0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00496bf1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00496bf3  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00496bf6  d94510                 -fld dword ptr [ebp + 0x10]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(16) /* 0x10 */));
    // 00496bf9  dc0d1cbf5300           -fmul qword ptr [0x53bf1c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5488412) /* 0x53bf1c */)));
    // 00496bff  d9c0                   -fld st(0)
    fp1 = x86::Float(fp0);
    // 00496c01  d9fe                   -fsin 
    fp1 = fp::sin(fp1);
    // 00496c03  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00496c05  d9ff                   -fcos 
    fp1 = fp::cos(fp1);
    // 00496c07  d94508                 -fld dword ptr [ebp + 8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(8) /* 0x8 */));
    // 00496c0a  d9c2                   -fld st(2)
    fp3 = x86::Float(fp0);
    // 00496c0c  d8c9                   -fmul st(1)
    fp3 = fp::mul(fp3, x86::Float(fp2));
    // 00496c0e  d9c2                   -fld st(2)
    fp4 = x86::Float(fp1);
    // 00496c10  deca                   -fmulp st(2)
    fp2 = fp::mul(fp2, fp4);
    // 00496c12  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp3;
        fp3 = fp1;
        fp1 = tmp;
    }
    // 00496c14  d9450c                 -fld dword ptr [ebp + 0xc]
    fp4 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(12) /* 0xc */));
    // 00496c17  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp4;
        fp4 = fp3;
        fp3 = tmp;
    }
    // 00496c19  d8c9                   -fmul st(1)
    fp4 = fp::mul(fp4, x86::Float(fp3));
    // 00496c1b  d9cc                   -fxch st(4)
    {
        x86::Float tmp = fp4;
        fp4 = fp0;
        fp0 = tmp;
    }
    // 00496c1d  dec9                   -fmulp st(1)
    fp3 = fp::mul(fp3, fp4);
    // 00496c1f  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00496c22  dee9                   -fsubp st(1)
    fp2 = fp::sub(fp2, fp3);
    // 00496c24  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00496c26  dec1                   -faddp st(1)
    fp1 = fp::add(fp1, fp2);
    // 00496c28  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00496c2a  d918                   -fstp dword ptr [eax]
    app->getMemory<float>(cpu.eax) = float(fp1);
    // 00496c2c  8b4518                 -mov eax, dword ptr [ebp + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 00496c2f  d918                   -fstp dword ptr [eax]
    app->getMemory<float>(cpu.eax) = float(fp0);
    // 00496c31  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00496c33  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496c34  c21400                 -ret 0x14
    cpu.esp += 4+20 /*0x14*/;
    return;
}

/* sub_496c40: native, nfs3hp::dead::stub_0x496c40 */

/* sub_496cb0: native, nfs3hp::dead::stub_0x496cb0 */

/* sub_496df0: native, nfs3hp::dead::stub_0x496df0 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_496f20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
  x86::reg16 fsw = 0;
    // 00496f20  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00496f21  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00496f22  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00496f24  83ec18                 +sub esp, 0x18
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(24 /*0x18*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00496f27  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00496f29  d901                   +fld dword ptr [ecx]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx));
    // 00496f2b  d822                   +fsub dword ptr [edx]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00496f2d  d9ee                   +fldz 
    fp1 = 0.0;
    // 00496f2f  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00496f31  d95df8                 +fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp1);
    // 00496f34  d85df8                 +fcomp dword ptr [ebp - 8]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00496f37  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00496f39  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00496f3a  760a                   -jbe 0x496f46
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00496f46;
    }
    // 00496f3c  d945f8                 +fld dword ptr [ebp - 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 00496f3f  d9e0                   +fchs 
    fp0 = -fp0;
    // 00496f41  d95de8                 +fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp0);
    // 00496f44  eb06                   -jmp 0x496f4c
    goto L_0x00496f4c;
L_0x00496f46:
    // 00496f46  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00496f49  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
L_0x00496f4c:
    // 00496f4c  d9ee                   +fldz 
    fp0 = 0.0;
    // 00496f4e  d94108                 +fld dword ptr [ecx + 8]
    fp1 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(8) /* 0x8 */));
    // 00496f51  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00496f54  d86208                 +fsub dword ptr [edx + 8]
    fp1 = fp::sub(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00496f57  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 00496f5a  d95df4                 +fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp1);
    // 00496f5d  d85df4                 +fcomp dword ptr [ebp - 0xc]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 00496f60  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00496f62  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00496f63  760a                   -jbe 0x496f6f
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00496f6f;
    }
    // 00496f65  d945f4                 +fld dword ptr [ebp - 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 00496f68  d9e0                   +fchs 
    fp0 = -fp0;
    // 00496f6a  d95dfc                 +fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 00496f6d  eb06                   -jmp 0x496f75
    goto L_0x00496f75;
L_0x00496f6f:
    // 00496f6f  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00496f72  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
L_0x00496f75:
    // 00496f75  d945f0                 +fld dword ptr [ebp - 0x10]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */));
    // 00496f78  d85dfc                 +fcomp dword ptr [ebp - 4]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00496f7b  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00496f7d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00496f7e  760e                   -jbe 0x496f8e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00496f8e;
    }
    // 00496f80  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00496f83  d80d24bf5300           +fmul dword ptr [0x53bf24]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5488420) /* 0x53bf24 */)));
    // 00496f89  d845f0                 +fadd dword ptr [ebp - 0x10]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 00496f8c  eb0c                   -jmp 0x496f9a
    goto L_0x00496f9a;
L_0x00496f8e:
    // 00496f8e  d945f0                 +fld dword ptr [ebp - 0x10]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */));
    // 00496f91  d80d24bf5300           +fmul dword ptr [0x53bf24]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5488420) /* 0x53bf24 */)));
    // 00496f97  d845fc                 +fadd dword ptr [ebp - 4]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
L_0x00496f9a:
    // 00496f9a  d95dec                 +fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fp0);
    // 00496f9d  d945ec                 +fld dword ptr [ebp - 0x14]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */));
    // 00496fa0  d85d0c                 +fcomp dword ptr [ebp + 0xc]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(12) /* 0xc */)));
    // 00496fa3  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00496fa5  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00496fa6  7307                   -jae 0x496faf
    if (!cpu.flags.cf)
    {
        goto L_0x00496faf;
    }
    // 00496fa8  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00496fad  eb02                   -jmp 0x496fb1
    goto L_0x00496fb1;
L_0x00496faf:
    // 00496faf  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00496fb1:
    // 00496fb1  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00496fb3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496fb4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496fb5  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* sub_496fc0: native, nfs3hp::dead::stub_0x496fc0 */

/* sub_497010: native, nfs3hp::dead::stub_0x497010 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4970c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004970c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004970c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004970c2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004970c3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004970c4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004970c5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004970c6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004970c8  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 004970cb  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004970ce  8b38                   -mov edi, dword ptr [eax]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax);
    // 004970d0  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004970d3  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004970d6  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004970d9  8b7008                 -mov esi, dword ptr [eax + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004970dc  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004970de  7e04                   -jle 0x4970e4
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004970e4;
    }
    // 004970e0  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004970e2  eb04                   -jmp 0x4970e8
    goto L_0x004970e8;
L_0x004970e4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004970e4  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004970e6  f7d8                   -neg eax
    cpu.eax = ~cpu.eax + 1;
L_0x004970e8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004970e8  3d00000001             +cmp eax, 0x1000000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16777216 /*0x1000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004970ed  7f29                   -jg 0x497118
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00497118;
    }
    // 004970ef  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004970f2  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004970f4  7e04                   -jle 0x4970fa
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004970fa;
    }
    // 004970f6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004970f8  eb04                   -jmp 0x4970fe
    goto L_0x004970fe;
L_0x004970fa:
    // 004970fa  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004970fc  f7d8                   -neg eax
    cpu.eax = ~cpu.eax + 1;
L_0x004970fe:
    // 004970fe  3d00000001             +cmp eax, 0x1000000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16777216 /*0x1000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497103  7f13                   -jg 0x497118
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00497118;
    }
    // 00497105  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00497107  7e04                   -jle 0x49710d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0049710d;
    }
    // 00497109  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0049710b  eb04                   -jmp 0x497111
    goto L_0x00497111;
L_0x0049710d:
    // 0049710d  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0049710f  f7d8                   -neg eax
    cpu.eax = ~cpu.eax + 1;
L_0x00497111:
    // 00497111  3d00000001             +cmp eax, 0x1000000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16777216 /*0x1000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497116  7e14                   -jle 0x49712c
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0049712c;
    }
L_0x00497118:
    // 00497118  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049711b  d1fe                   -sar esi, 1
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (1 /*0x1*/ % 32));
    // 0049711d  d1f9                   -sar ecx, 1
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (1 /*0x1*/ % 32));
    // 0049711f  d1ff                   -sar edi, 1
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (1 /*0x1*/ % 32));
    // 00497121  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 00497124  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00497126  7ebc                   -jle 0x4970e4
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004970e4;
    }
    // 00497128  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0049712a  ebbc                   -jmp 0x4970e8
    goto L_0x004970e8;
L_0x0049712c:
    // 0049712c  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0049712e  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00497130  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 00497132  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 00497135  c1e810                 +shr eax, 0x10
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
    // 00497138  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0049713a  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049713d  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0049713f  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 00497142  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x00497144:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00497144  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 00497146  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 00497149  c1e810                 +shr eax, 0x10
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
    // 0049714c  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0049714e  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00497150  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 00497153  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00497155  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00497157  c1eb02                 -shr ebx, 2
    cpu.ebx >>= 2 /*0x2*/ % 32;
    // 0049715a  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 0049715c  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 0049715f  c1e810                 +shr eax, 0x10
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
    // 00497162  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 00497164  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 00497167  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 0049716a  c1e802                 -shr eax, 2
    cpu.eax >>= 2 /*0x2*/ % 32;
    // 0049716d  01d9                   -add ecx, ebx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0049716f  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00497171  3d00000040             +cmp eax, 0x40000000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1073741824 /*0x40000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497176  7626                   -jbe 0x49719e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0049719e;
    }
    // 00497178  d1ff                   -sar edi, 1
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (1 /*0x1*/ % 32));
    // 0049717a  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049717d  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0049717f  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00497181  d1fb                   -sar ebx, 1
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (1 /*0x1*/ % 32));
    // 00497183  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 00497185  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 00497188  c1e810                 +shr eax, 0x10
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
    // 0049718b  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0049718d  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 00497190  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 00497193  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00497196  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00497198  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049719a  d1fe                   +sar esi, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.esi;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 0049719c  eba6                   -jmp 0x497144
    goto L_0x00497144;
L_0x0049719e:
    // 0049719e  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004971a1  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004971a4  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004971a7  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004971a9  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004971ab  0fbdc8                 +bsr ecx, eax
    if (cpu.eax == 0)
    {
        cpu.flags.zf = 1;
    }
    else
    {
        cpu.flags.zf = 0;
        cpu.ecx = x86::reg32(31 - __builtin_clz(cpu.eax));
    }
    // 004971ae  7412                   -je 0x4971c2
    if (cpu.flags.zf)
    {
        goto L_0x004971c2;
    }
    // 004971b0  f7248dc4615600         -mul dword ptr [ecx*4 + 0x5661c4]
    cpu.edx_eax = x86::reg64(cpu.eax) * x86::reg64(app->getMemory<x86::reg32>(x86::reg32(5661124) /* 0x5661c4 */ + cpu.ecx * 4));
    // 004971b7  0facd010               -shrd eax, edx, 0x10
    {
        x86::reg32& destination = cpu.eax;
        destination >>= (16 /*0x10*/ % 32);
        destination |= cpu.edx  << (32 - (16 /*0x10*/ % 32));
    }
    // 004971bb  03048d48625600         -add eax, dword ptr [ecx*4 + 0x566248]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5661256) /* 0x566248 */ + cpu.ecx * 4)));
L_0x004971c2:
    // 004971c2  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004971c5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004971c7  745f                   -je 0x497228
    if (cpu.flags.zf)
    {
        goto L_0x00497228;
    }
    // 004971c9  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004971cb  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004971cd  99                     -cdq 
    cpu.edx_eax = x86::reg64(static_cast<x86::sreg32>(cpu.eax));
    // 004971ce  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004971d0  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004971d2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004971d4  0facd010               -shrd eax, edx, 0x10
    {
        x86::reg32& destination = cpu.eax;
        destination >>= (16 /*0x10*/ % 32);
        destination |= cpu.edx  << (32 - (16 /*0x10*/ % 32));
    }
    // 004971d8  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004971db  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004971dd  c1e110                 -shl ecx, 0x10
    cpu.ecx <<= 16 /*0x10*/ % 32;
    // 004971e0  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004971e2  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004971e5  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004971e8  8901                   -mov dword ptr [ecx], eax
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.eax;
    // 004971ea  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004971ed  99                     -cdq 
    cpu.edx_eax = x86::reg64(static_cast<x86::sreg32>(cpu.eax));
    // 004971ee  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004971f0  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004971f2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004971f4  0facd010               -shrd eax, edx, 0x10
    {
        x86::reg32& destination = cpu.eax;
        destination >>= (16 /*0x10*/ % 32);
        destination |= cpu.edx  << (32 - (16 /*0x10*/ % 32));
    }
    // 004971f8  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004971fb  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004971fd  c1e110                 -shl ecx, 0x10
    cpu.ecx <<= 16 /*0x10*/ % 32;
    // 00497200  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00497202  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497205  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497208  894104                 -mov dword ptr [ecx + 4], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 0049720b  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0049720d  99                     -cdq 
    cpu.edx_eax = x86::reg64(static_cast<x86::sreg32>(cpu.eax));
    // 0049720e  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00497210  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00497212  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00497214  0facd010               -shrd eax, edx, 0x10
    {
        x86::reg32& destination = cpu.eax;
        destination >>= (16 /*0x10*/ % 32);
        destination |= cpu.edx  << (32 - (16 /*0x10*/ % 32));
    }
    // 00497218  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0049721b  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0049721d  c1e110                 -shl ecx, 0x10
    cpu.ecx <<= 16 /*0x10*/ % 32;
    // 00497220  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00497222  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497225  894108                 -mov dword ptr [ecx + 8], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.eax;
L_0x00497228:
    // 00497228  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049722a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049722b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049722c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049722d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049722e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049722f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497230  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_497240: native, nfs3hp::dead::stub_0x497240 */

/* sub_4972f0: native, nfs3hp::vertices::normalise */

/* sub_497360: native, nfs3hp::dead::stub_0x497360 */

/* sub_497370: native, nfs3hp::dead::stub_0x497370 */

/* sub_497380: native, nfs3hp::dead::stub_0x497380 */

/* sub_4973e0: native, nfs3hp::dead::stub_0x4973e0 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_497430(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    // 00497430  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00497431  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00497432  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00497433  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00497435  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00497438  b964f37900             -mov ecx, 0x79f364
    cpu.ecx = 7992164 /*0x79f364*/;
    // 0049743d  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0049743f  eb08                   -jmp 0x497449
    goto L_0x00497449;
L_0x00497441:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00497441  81fa80000000           +cmp edx, 0x80
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(128 /*0x80*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497447  7d2e                   -jge 0x497477
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00497477;
    }
L_0x00497449:
    // 00497449  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049744b  c1e011                 -shl eax, 0x11
    cpu.eax <<= 17 /*0x11*/ % 32;
    // 0049744e  0d0000003f             -or eax, 0x3f000000
    cpu.eax |= x86::reg32(x86::sreg32(1056964608 /*0x3f000000*/));
    // 00497453  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00497454  e8c7340500             -call 0x4ea920
    cpu.esp -= 4;
    nfs3hp::frontend::simSquareRoot(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00497459  d9e8                   -fld1 
    fp1 = 1.0;
    // 0049745b  def1                   -fdivrp st(1)
    fp0 = fp::div(fp1, x86::Float(fp0));
    // 0049745d  d95dfc                 -fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 00497460  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00497463  0500200000             -add eax, 0x2000
    (cpu.eax) += x86::reg32(x86::sreg32(8192 /*0x2000*/));
    // 00497468  c1e80f                 -shr eax, 0xf
    cpu.eax >>= 15 /*0xf*/ % 32;
    // 0049746b  41                     -inc ecx
    (cpu.ecx)++;
    // 0049746c  25ff000000             +and eax, 0xff
    cpu.clear_co();
    cpu.set_szp((cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/))));
    // 00497471  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00497472  8841ff                 -mov byte ptr [ecx - 1], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(-1) /* -0x1 */) = cpu.al;
    // 00497475  ebca                   -jmp 0x497441
    goto L_0x00497441;
L_0x00497477:
    // 00497477  c605a4f37900ff         -mov byte ptr [0x79f3a4], 0xff
    app->getMemory<x86::reg8>(x86::reg32(7992228) /* 0x79f3a4 */) = 255 /*0xff*/;
    // 0049747e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00497480  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497481  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497482  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497483  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_497490: native, nfs3hp::dead::stub_0x497490 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_497500(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    // 00497500  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00497501  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00497502  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00497503  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00497504  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00497506  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00497509  d94004                 -fld dword ptr [eax + 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 0049750c  d8c8                   -fmul st(0)
    fp0 = fp::mul(fp0, x86::Float(fp0));
    // 0049750e  d900                   -fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00497510  d8c8                   -fmul st(0)
    fp1 = fp::mul(fp1, x86::Float(fp1));
    // 00497512  dec1                   -faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 00497514  d94008                 -fld dword ptr [eax + 8]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00497517  d8c8                   -fmul st(0)
    fp1 = fp::mul(fp1, x86::Float(fp1));
    // 00497519  dec1                   -faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 0049751b  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp0);
    // 0049751e  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497521  c1e911                 -shr ecx, 0x11
    cpu.ecx >>= 17 /*0x11*/ % 32;
    // 00497524  83e17f                 -and ecx, 0x7f
    cpu.ecx &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 00497527  8a8964f37900           -mov cl, byte ptr [ecx + 0x79f364]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(7992164) /* 0x79f364 */);
    // 0049752d  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497530  c1eb17                 -shr ebx, 0x17
    cpu.ebx >>= 23 /*0x17*/ % 32;
    // 00497533  be7c010000             -mov esi, 0x17c
    cpu.esi = 380 /*0x17c*/;
    // 00497538  81e3ff000000           -and ebx, 0xff
    cpu.ebx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049753e  29de                   -sub esi, ebx
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00497540  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00497542  81e1ff000000           -and ecx, 0xff
    cpu.ecx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00497548  d1eb                   -shr ebx, 1
    cpu.ebx >>= 1 /*0x1*/ % 32;
    // 0049754a  c1e10f                 -shl ecx, 0xf
    cpu.ecx <<= 15 /*0xf*/ % 32;
    // 0049754d  c1e317                 -shl ebx, 0x17
    cpu.ebx <<= 23 /*0x17*/ % 32;
    // 00497550  09cb                   -or ebx, ecx
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.ecx));
    // 00497552  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 00497555  d945f8                 -fld dword ptr [ebp - 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 00497558  d8c8                   -fmul st(0)
    fp0 = fp::mul(fp0, x86::Float(fp0));
    // 0049755a  d84df4                 -fmul dword ptr [ebp - 0xc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 0049755d  dc2d3cbf5300           -fsubr qword ptr [0x53bf3c]
    fp0 = fp::sub(x86::Float(app->getMemory<double>(x86::reg32(5488444) /* 0x53bf3c */)), fp0);
    // 00497563  d84df8                 -fmul dword ptr [ebp - 8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00497566  dc0d44bf5300           -fmul qword ptr [0x53bf44]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5488452) /* 0x53bf44 */)));
    // 0049756c  d900                   -fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 0049756e  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 00497570  d91a                   -fstp dword ptr [edx]
    app->getMemory<float>(cpu.edx) = float(fp1);
    // 00497572  d94004                 -fld dword ptr [eax + 4]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00497575  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 00497577  d95a04                 -fstp dword ptr [edx + 4]
    app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 0049757a  d84808                 -fmul dword ptr [eax + 8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 0049757d  d95a08                 -fstp dword ptr [edx + 8]
    app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 00497580  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00497582  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497583  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497584  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497585  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497586  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_497590: native, nfs3hp::frontend::hudVectorLength */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
/* data blob: 61000000000000004cbf530050bf530058bf530060bf530064bf53006cbf530074bf53007cbf530080bf53000000000000000000010000000100000001000000000000000100000001000000010000002000000088bf53008cbf530090bf530088bf53008cbf530090bf53008d442000 */
void Application::sub_497620(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00497620  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00497621  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00497622  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00497623  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00497624  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00497625  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00497626  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00497628  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0049762b  a3a4f57900             -mov dword ptr [0x79f5a4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */) = cpu.eax;
    // 00497630  ff501c                 -call dword ptr [eax + 0x1c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497633  e8981d0000             -call 0x4993d0
    cpu.esp -= 4;
    sub_4993d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497638  8b15dcf57900           -mov edx, dword ptr [0x79f5dc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992796) /* 0x79f5dc */);
    // 0049763e  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00497640  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00497642  7408                   -je 0x49764c
    if (cpu.flags.zf)
    {
        goto L_0x0049764c;
    }
    // 00497644  8915c4f57900           -mov dword ptr [0x79f5c4], edx
    app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */) = cpu.edx;
    // 0049764a  eb28                   -jmp 0x497674
    goto L_0x00497674;
L_0x0049764c:
    // 0049764c  e86fcf0500             -call 0x4f45c0
    cpu.esp -= 4;
    nfs3hp::watcom::stackavail(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497651  3d00200000             +cmp eax, 0x2000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8192 /*0x2000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497656  7615                   -jbe 0x49766d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0049766d;
    }
    // 00497658  6800200000             -push 0x2000
    app->getMemory<x86::reg32>(cpu.esp-4) = 8192 /*0x2000*/;
    cpu.esp -= 4;
    // 0049765d  e8aecf0500             -call 0x4f4610
    cpu.esp -= 4;
    nfs3hp::watcom::GRO(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497662  b800200000             -mov eax, 0x2000
    cpu.eax = 8192 /*0x2000*/;
    // 00497667  29c4                   +sub esp, eax
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00497669  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0049766b  eb02                   -jmp 0x49766f
    goto L_0x0049766f;
L_0x0049766d:
    // 0049766d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0049766f:
    // 0049766f  a3c4f57900             -mov dword ptr [0x79f5c4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */) = cpu.eax;
L_0x00497674:
    // 00497674  833dc4f5790000         -cmp dword ptr [0x79f5c4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049767b  e820050000             -call 0x497ba0
    cpu.esp -= 4;
    sub_497ba0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497680  833da0259f0000         +cmp dword ptr [0x9f25a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10429856) /* 0x9f25a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497687  7529                   -jne 0x4976b2
    if (!cpu.flags.zf)
    {
        goto L_0x004976b2;
    }
    // 00497689  bab0954900             -mov edx, 0x4995b0
    cpu.edx = 4822448 /*0x4995b0*/;
    // 0049768e  b8f8bf5300             -mov eax, 0x53bff8
    cpu.eax = 5488632 /*0x53bff8*/;
    // 00497693  e8b8cf0500             -call 0x4f4650
    cpu.esp -= 4;
    sub_4f4650(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497698  baa09e4900             -mov edx, 0x499ea0
    cpu.edx = 4824736 /*0x499ea0*/;
    // 0049769d  b800c05300             -mov eax, 0x53c000
    cpu.eax = 5488640 /*0x53c000*/;
    // 004976a2  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004976a7  e8a4cf0500             -call 0x4f4650
    cpu.esp -= 4;
    sub_4f4650(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004976ac  893da0259f00           -mov dword ptr [0x9f25a0], edi
    app->getMemory<x86::reg32>(x86::reg32(10429856) /* 0x9f25a0 */) = cpu.edi;
L_0x004976b2:
    // 004976b2  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004976b4  e807060000             -call 0x497cc0
    cpu.esp -= 4;
    sub_497cc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004976b9  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 004976bb  e939000000             -jmp 0x4976f9
    goto L_0x004976f9;
L_0x004976c0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004976c0  3d004d0000             +cmp eax, 0x4d00
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(19712 /*0x4d00*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004976c5  0f8402030000           -je 0x4979cd
    if (cpu.flags.zf)
    {
        goto L_0x004979cd;
    }
L_0x004976cb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004976cb  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
L_0x004976cd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004976cd  833da8f5790001         +cmp dword ptr [0x79f5a8], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7992744) /* 0x79f5a8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004976d4  7505                   -jne 0x4976db
    if (!cpu.flags.zf)
    {
        goto L_0x004976db;
    }
L_0x004976d6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004976d6  e8b5090000             -call 0x498090
    cpu.esp -= 4;
    sub_498090(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004976db:
    // 004976db  837dfc01               +cmp dword ptr [ebp - 4], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004976df  7409                   -je 0x4976ea
    if (cpu.flags.zf)
    {
        goto L_0x004976ea;
    }
    // 004976e1  833db4f5790001         +cmp dword ptr [0x79f5b4], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004976e8  7507                   -jne 0x4976f1
    if (!cpu.flags.zf)
    {
        goto L_0x004976f1;
    }
L_0x004976ea:
    // 004976ea  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004976ec  e87f0c0000             -call 0x498370
    cpu.esp -= 4;
    sub_498370(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004976f1:
    // 004976f1  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004976f3  0f858f040000           -jne 0x497b88
    if (!cpu.flags.zf)
    {
        goto L_0x00497b88;
    }
L_0x004976f9:
    // 004976f9  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004976fe  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00497700  ff5018                 -call dword ptr [eax + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497703  890da0e55500           -mov dword ptr [0x55e5a0], ecx
    app->getMemory<x86::reg32>(x86::reg32(5629344) /* 0x55e5a0 */) = cpu.ecx;
    // 00497709  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 0049770c  890db4f57900           -mov dword ptr [0x79f5b4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.ecx;
    // 00497712  890da8f57900           -mov dword ptr [0x79f5a8], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992744) /* 0x79f5a8 */) = cpu.ecx;
    // 00497718  83f81b                 +cmp eax, 0x1b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(27 /*0x1b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049771b  7505                   -jne 0x497722
    if (!cpu.flags.zf)
    {
        goto L_0x00497722;
    }
    // 0049771d  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
L_0x00497722:
    // 00497722  83f853                 +cmp eax, 0x53
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(83 /*0x53*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497725  7276                   -jb 0x49779d
    if (cpu.flags.cf)
    {
        goto L_0x0049779d;
    }
    // 00497727  0f8623030000           -jbe 0x497a50
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00497a50;
    }
    // 0049772d  8b0dccf57900           -mov ecx, dword ptr [0x79f5cc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992780) /* 0x79f5cc */);
    // 00497733  49                     -dec ecx
    (cpu.ecx)--;
    // 00497734  3d004b0000             +cmp eax, 0x4b00
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(19200 /*0x4b00*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497739  7241                   -jb 0x49777c
    if (cpu.flags.cf)
    {
        goto L_0x0049777c;
    }
    // 0049773b  8b15b0e55500           -mov edx, dword ptr [0x55e5b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629360) /* 0x55e5b0 */);
    // 00497741  80f201                 -xor dl, 1
    cpu.dl ^= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 00497744  3d004b0000             +cmp eax, 0x4b00
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(19200 /*0x4b00*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497749  0f86bf020000           -jbe 0x497a0e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00497a0e;
    }
    // 0049774f  3d00500000             +cmp eax, 0x5000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20480 /*0x5000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497754  0f8266ffffff           -jb 0x4976c0
    if (cpu.flags.cf)
    {
        goto L_0x004976c0;
    }
    // 0049775a  8b15d0f57900           -mov edx, dword ptr [0x79f5d0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992784) /* 0x79f5d0 */);
    // 00497760  4a                     -dec edx
    (cpu.edx)--;
    // 00497761  3d00500000             +cmp eax, 0x5000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20480 /*0x5000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497766  0f8686000000           -jbe 0x4977f2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004977f2;
    }
    // 0049776c  3d00510000             +cmp eax, 0x5100
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20736 /*0x5100*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497771  0f8434010000           -je 0x4978ab
    if (cpu.flags.zf)
    {
        goto L_0x004978ab;
    }
    // 00497777  e94fffffff             -jmp 0x4976cb
    goto L_0x004976cb;
L_0x0049777c:
    // 0049777c  3d00480000             +cmp eax, 0x4800
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18432 /*0x4800*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497781  0f8244ffffff           -jb 0x4976cb
    if (cpu.flags.cf)
    {
        goto L_0x004976cb;
    }
    // 00497787  0f86c1000000           -jbe 0x49784e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0049784e;
    }
    // 0049778d  3d00490000             +cmp eax, 0x4900
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18688 /*0x4900*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497792  0f84ac010000           -je 0x497944
    if (cpu.flags.zf)
    {
        goto L_0x00497944;
    }
    // 00497798  e92effffff             -jmp 0x4976cb
    goto L_0x004976cb;
L_0x0049779d:
    // 0049779d  83f820                 +cmp eax, 0x20
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(32 /*0x20*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004977a0  722d                   -jb 0x4977cf
    if (cpu.flags.cf)
    {
        goto L_0x004977cf;
    }
    // 004977a2  0f865e030000           -jbe 0x497b06
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00497b06;
    }
    // 004977a8  83f848                 +cmp eax, 0x48
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(72 /*0x48*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004977ab  7214                   -jb 0x4977c1
    if (cpu.flags.cf)
    {
        goto L_0x004977c1;
    }
    // 004977ad  0f86a9020000           -jbe 0x497a5c
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00497a5c;
    }
    // 004977b3  83f850                 +cmp eax, 0x50
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(80 /*0x50*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004977b6  0f84c2030000           -je 0x497b7e
    if (cpu.flags.zf)
    {
        goto L_0x00497b7e;
    }
    // 004977bc  e90affffff             -jmp 0x4976cb
    goto L_0x004976cb;
L_0x004977c1:
    // 004977c1  83f843                 +cmp eax, 0x43
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(67 /*0x43*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004977c4  0f84d2020000           -je 0x497a9c
    if (cpu.flags.zf)
    {
        goto L_0x00497a9c;
    }
    // 004977ca  e9fcfeffff             -jmp 0x4976cb
    goto L_0x004976cb;
L_0x004977cf:
    // 004977cf  83f80d                 +cmp eax, 0xd
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(13 /*0xd*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004977d2  0f82f3feffff           -jb 0x4976cb
    if (cpu.flags.cf)
    {
        goto L_0x004976cb;
    }
    // 004977d8  0f8665030000           -jbe 0x497b43
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00497b43;
    }
    // 004977de  83f81b                 +cmp eax, 0x1b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(27 /*0x1b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004977e1  7405                   -je 0x4977e8
    if (cpu.flags.zf)
    {
        goto L_0x004977e8;
    }
    // 004977e3  e9e3feffff             -jmp 0x4976cb
    goto L_0x004976cb;
L_0x004977e8:
    // 004977e8  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004977ed  e9dbfeffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x004977f2:
    // 004977f2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004977f4  a3b0e55500             -mov dword ptr [0x55e5b0], eax
    app->getMemory<x86::reg32>(x86::reg32(5629360) /* 0x55e5b0 */) = cpu.eax;
    // 004977f9  a158e55500             -mov eax, dword ptr [0x55e558]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */);
    // 004977fe  a3d8f57900             -mov dword ptr [0x79f5d8], eax
    app->getMemory<x86::reg32>(x86::reg32(7992792) /* 0x79f5d8 */) = cpu.eax;
    // 00497803  39c2                   +cmp edx, eax
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
    // 00497805  7e0f                   -jle 0x497816
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00497816;
    }
    // 00497807  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0049780c  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0049780e  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 00497811  a358e55500             -mov dword ptr [0x55e558], eax
    app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */) = cpu.eax;
L_0x00497816:
    // 00497816  8b15d4f57900           -mov edx, dword ptr [0x79f5d4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */);
    // 0049781c  3b1558e55500           +cmp edx, dword ptr [0x55e558]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00497822  0f8da5feffff           -jge 0x4976cd
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004976cd;
    }
    // 00497828  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0049782d  8b1dc8f57900           -mov ebx, dword ptr [0x79f5c8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */);
    // 00497833  890db4f57900           -mov dword ptr [0x79f5b4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.ecx;
    // 00497839  01cb                   +add ebx, ecx
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0049783b  8d040a                 -lea eax, [edx + ecx]
    cpu.eax = x86::reg32(cpu.edx + cpu.ecx * 1);
    // 0049783e  891dc8f57900           -mov dword ptr [0x79f5c8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */) = cpu.ebx;
    // 00497844  a3d4f57900             -mov dword ptr [0x79f5d4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */) = cpu.eax;
    // 00497849  e97ffeffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x0049784e:
    // 0049784e  a158e55500             -mov eax, dword ptr [0x55e558]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */);
    // 00497853  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00497855  a3d8f57900             -mov dword ptr [0x79f5d8], eax
    app->getMemory<x86::reg32>(x86::reg32(7992792) /* 0x79f5d8 */) = cpu.eax;
    // 0049785a  8915b0e55500           -mov dword ptr [0x55e5b0], edx
    app->getMemory<x86::reg32>(x86::reg32(5629360) /* 0x55e5b0 */) = cpu.edx;
    // 00497860  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00497862  7e0f                   -jle 0x497873
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00497873;
    }
    // 00497864  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00497869  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049786b  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 0049786e  a358e55500             -mov dword ptr [0x55e558], eax
    app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */) = cpu.eax;
L_0x00497873:
    // 00497873  8b15c8f57900           -mov edx, dword ptr [0x79f5c8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */);
    // 00497879  3b1558e55500           +cmp edx, dword ptr [0x55e558]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049787f  0f8e48feffff           -jle 0x4976cd
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004976cd;
    }
    // 00497885  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0049788a  a1d4f57900             -mov eax, dword ptr [0x79f5d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */);
    // 0049788f  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 00497891  890db4f57900           -mov dword ptr [0x79f5b4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.ecx;
    // 00497897  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00497899  29c8                   +sub eax, ecx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0049789b  891dc8f57900           -mov dword ptr [0x79f5c8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */) = cpu.ebx;
    // 004978a1  a3d4f57900             -mov dword ptr [0x79f5d4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */) = cpu.eax;
    // 004978a6  e922feffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x004978ab:
    // 004978ab  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004978ad  a158e55500             -mov eax, dword ptr [0x55e558]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */);
    // 004978b2  891db0e55500           -mov dword ptr [0x55e5b0], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629360) /* 0x55e5b0 */) = cpu.ebx;
    // 004978b8  8b1dd4f57900           -mov ebx, dword ptr [0x79f5d4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */);
    // 004978be  a3d8f57900             -mov dword ptr [0x79f5d8], eax
    app->getMemory<x86::reg32>(x86::reg32(7992792) /* 0x79f5d8 */) = cpu.eax;
    // 004978c3  39d8                   +cmp eax, ebx
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
    // 004978c5  7d15                   -jge 0x4978dc
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004978dc;
    }
    // 004978c7  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004978cc  891d58e55500           -mov dword ptr [0x55e558], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */) = cpu.ebx;
    // 004978d2  a3b4f57900             -mov dword ptr [0x79f5b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.eax;
    // 004978d7  e9f1fdffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x004978dc:
    // 004978dc  39da                   +cmp edx, ebx
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
    // 004978de  0f8ee9fdffff           -jle 0x4976cd
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004976cd;
    }
    // 004978e4  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004978e6  0305ccf57900           -add eax, dword ptr [0x79f5cc]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992780) /* 0x79f5cc */)));
    // 004978ec  3b05d0f57900           +cmp eax, dword ptr [0x79f5d0]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992784) /* 0x79f5d0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004978f2  7e2d                   -jle 0x497921
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00497921;
    }
    // 004978f4  8b0dd4f57900           -mov ecx, dword ptr [0x79f5d4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */);
    // 004978fa  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004978fc  8b1dc8f57900           -mov ebx, dword ptr [0x79f5c8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */);
    // 00497902  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00497904  8915d4f57900           -mov dword ptr [0x79f5d4], edx
    app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */) = cpu.edx;
    // 0049790a  01c3                   +add ebx, eax
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
    // 0049790c  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00497911  891dc8f57900           -mov dword ptr [0x79f5c8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */) = cpu.ebx;
    // 00497917  a3b4f57900             -mov dword ptr [0x79f5b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.eax;
    // 0049791c  e9acfdffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x00497921:
    // 00497921  a1d4f57900             -mov eax, dword ptr [0x79f5d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */);
    // 00497926  a3c8f57900             -mov dword ptr [0x79f5c8], eax
    app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */) = cpu.eax;
    // 0049792b  8d1408                 -lea edx, [eax + ecx]
    cpu.edx = x86::reg32(cpu.eax + cpu.ecx * 1);
    // 0049792e  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00497933  8915d4f57900           -mov dword ptr [0x79f5d4], edx
    app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */) = cpu.edx;
    // 00497939  890db4f57900           -mov dword ptr [0x79f5b4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.ecx;
    // 0049793f  e989fdffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x00497944:
    // 00497944  a158e55500             -mov eax, dword ptr [0x55e558]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */);
    // 00497949  8b15c8f57900           -mov edx, dword ptr [0x79f5c8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */);
    // 0049794f  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00497951  a3d8f57900             -mov dword ptr [0x79f5d8], eax
    app->getMemory<x86::reg32>(x86::reg32(7992792) /* 0x79f5d8 */) = cpu.eax;
    // 00497956  891db0e55500           -mov dword ptr [0x55e5b0], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629360) /* 0x55e5b0 */) = cpu.ebx;
    // 0049795c  39d0                   +cmp eax, edx
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
    // 0049795e  7e0d                   -jle 0x49796d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0049796d;
    }
    // 00497960  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00497965  891558e55500           -mov dword ptr [0x55e558], edx
    app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */) = cpu.edx;
    // 0049796b  eb55                   -jmp 0x4979c2
    goto L_0x004979c2;
L_0x0049796d:
    // 0049796d  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0049796f  0f8e58fdffff           -jle 0x4976cd
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004976cd;
    }
    // 00497975  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00497977  2b05ccf57900           -sub eax, dword ptr [0x79f5cc]
    (cpu.eax) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992780) /* 0x79f5cc */)));
    // 0049797d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049797f  7d28                   -jge 0x4979a9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004979a9;
    }
    // 00497981  a1c8f57900             -mov eax, dword ptr [0x79f5c8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */);
    // 00497986  8b0dd4f57900           -mov ecx, dword ptr [0x79f5d4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */);
    // 0049798c  891dc8f57900           -mov dword ptr [0x79f5c8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */) = cpu.ebx;
    // 00497992  29c1                   +sub ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00497994  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00497999  890dd4f57900           -mov dword ptr [0x79f5d4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */) = cpu.ecx;
    // 0049799f  a3b4f57900             -mov dword ptr [0x79f5b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.eax;
    // 004979a4  e924fdffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x004979a9:
    // 004979a9  a1c8f57900             -mov eax, dword ptr [0x79f5c8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */);
    // 004979ae  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004979b0  a3d4f57900             -mov dword ptr [0x79f5d4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */) = cpu.eax;
    // 004979b5  29ca                   +sub edx, ecx
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004979b7  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004979bc  8915c8f57900           -mov dword ptr [0x79f5c8], edx
    app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */) = cpu.edx;
L_0x004979c2:
    // 004979c2  890db4f57900           -mov dword ptr [0x79f5b4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.ecx;
    // 004979c8  e900fdffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x004979cd:
    // 004979cd  8b1db0f57900           -mov ebx, dword ptr [0x79f5b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */);
    // 004979d3  8915b0e55500           -mov dword ptr [0x55e5b0], edx
    app->getMemory<x86::reg32>(x86::reg32(5629360) /* 0x55e5b0 */) = cpu.edx;
    // 004979d9  83fb08                 +cmp ebx, 8
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004979dc  7d21                   -jge 0x4979ff
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004979ff;
    }
    // 004979de  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004979e3  8b1568e55500           -mov edx, dword ptr [0x55e568]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */);
    // 004979e9  a3a8f57900             -mov dword ptr [0x79f5a8], eax
    app->getMemory<x86::reg32>(x86::reg32(7992744) /* 0x79f5a8 */) = cpu.eax;
    // 004979ee  01c2                   +add edx, eax
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
    // 004979f0  8d0c03                 -lea ecx, [ebx + eax]
    cpu.ecx = x86::reg32(cpu.ebx + cpu.eax * 1);
    // 004979f3  891568e55500           -mov dword ptr [0x55e568], edx
    app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */) = cpu.edx;
    // 004979f9  890db0f57900           -mov dword ptr [0x79f5b0], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */) = cpu.ecx;
L_0x004979ff:
    // 004979ff  c705b4f5790001000000   -mov dword ptr [0x79f5b4], 1
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = 1 /*0x1*/;
    // 00497a09  e9bffcffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x00497a0e:
    // 00497a0e  a168e55500             -mov eax, dword ptr [0x55e568]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */);
    // 00497a13  8915b0e55500           -mov dword ptr [0x55e5b0], edx
    app->getMemory<x86::reg32>(x86::reg32(5629360) /* 0x55e5b0 */) = cpu.edx;
    // 00497a19  83f802                 +cmp eax, 2
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
    // 00497a1c  7e23                   -jle 0x497a41
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00497a41;
    }
    // 00497a1e  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00497a23  8b1db0f57900           -mov ebx, dword ptr [0x79f5b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */);
    // 00497a29  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00497a2b  8915a8f57900           -mov dword ptr [0x79f5a8], edx
    app->getMemory<x86::reg32>(x86::reg32(7992744) /* 0x79f5a8 */) = cpu.edx;
    // 00497a31  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00497a33  29d3                   +sub ebx, edx
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00497a35  890d68e55500           -mov dword ptr [0x55e568], ecx
    app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */) = cpu.ecx;
    // 00497a3b  891db0f57900           -mov dword ptr [0x79f5b0], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */) = cpu.ebx;
L_0x00497a41:
    // 00497a41  c705b4f5790001000000   -mov dword ptr [0x79f5b4], 1
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = 1 /*0x1*/;
    // 00497a4b  e97dfcffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x00497a50:
    // 00497a50  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00497a52  e8a9140000             -call 0x498f00
    cpu.esp -= 4;
    sub_498f00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497a57  e971fcffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x00497a5c:
    // 00497a5c  8d4df8                 -lea ecx, [ebp - 8]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497a5f  8a25ace55500           -mov ah, byte ptr [0x55e5ac]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(5629356) /* 0x55e5ac */);
    // 00497a65  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497a68  80f401                 +xor ah, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ah ^= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 00497a6b  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00497a6e  8825ace55500           -mov byte ptr [0x55e5ac], ah
    app->getMemory<x86::reg8>(x86::reg32(5629356) /* 0x55e5ac */) = cpu.ah;
    // 00497a74  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00497a77  e8b4180000             -call 0x499330
    cpu.esp -= 4;
    sub_499330(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497a7c  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497a7f  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497a82  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00497a85  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00497a88  e893020000             -call 0x497d20
    cpu.esp -= 4;
    sub_497d20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497a8d  c705b4f5790001000000   -mov dword ptr [0x79f5b4], 1
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = 1 /*0x1*/;
    // 00497a97  e931fcffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x00497a9c:
    // 00497a9c  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00497aa1  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00497aa3  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00497aa6  890db0e55500           -mov dword ptr [0x55e5b0], ecx
    app->getMemory<x86::reg32>(x86::reg32(5629360) /* 0x55e5b0 */) = cpu.ecx;
    // 00497aac  e85f130000             -call 0x498e10
    cpu.esp -= 4;
    sub_498e10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497ab1  8d4df8                 -lea ecx, [ebp - 8]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497ab4  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00497ab7  891d94e55500           -mov dword ptr [0x55e594], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629332) /* 0x55e594 */) = cpu.ebx;
    // 00497abd  891d90e55500           -mov dword ptr [0x55e590], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629328) /* 0x55e590 */) = cpu.ebx;
    // 00497ac3  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497ac6  e865180000             -call 0x499330
    cpu.esp -= 4;
    sub_499330(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497acb  e800190000             -call 0x4993d0
    cpu.esp -= 4;
    sub_4993d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497ad0  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00497ad2  a3acf57900             -mov dword ptr [0x79f5ac], eax
    app->getMemory<x86::reg32>(x86::reg32(7992748) /* 0x79f5ac */) = cpu.eax;
    // 00497ad7  e804050000             -call 0x497fe0
    cpu.esp -= 4;
    sub_497fe0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497adc  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497adf  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497ae2  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00497ae5  a3d0f57900             -mov dword ptr [0x79f5d0], eax
    app->getMemory<x86::reg32>(x86::reg32(7992784) /* 0x79f5d0 */) = cpu.eax;
    // 00497aea  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00497aed  e82e020000             -call 0x497d20
    cpu.esp -= 4;
    sub_497d20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497af2  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00497af7  a3a8f57900             -mov dword ptr [0x79f5a8], eax
    app->getMemory<x86::reg32>(x86::reg32(7992744) /* 0x79f5a8 */) = cpu.eax;
    // 00497afc  a3b4f57900             -mov dword ptr [0x79f5b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.eax;
    // 00497b01  e9d0fbffff             -jmp 0x4976d6
    goto L_0x004976d6;
L_0x00497b06:
    // 00497b06  e8a51a0000             -call 0x4995b0
    cpu.esp -= 4;
    sub_4995b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497b0b  8d4df8                 -lea ecx, [ebp - 8]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497b0e  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497b11  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00497b14  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00497b17  e814180000             -call 0x499330
    cpu.esp -= 4;
    sub_499330(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497b1c  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497b1f  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497b22  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00497b25  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00497b28  e8f3010000             -call 0x497d20
    cpu.esp -= 4;
    sub_497d20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497b2d  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00497b32  890da8f57900           -mov dword ptr [0x79f5a8], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992744) /* 0x79f5a8 */) = cpu.ecx;
    // 00497b38  890db4f57900           -mov dword ptr [0x79f5b4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.ecx;
    // 00497b3e  e993fbffff             -jmp 0x4976d6
    goto L_0x004976d6;
L_0x00497b43:
    // 00497b43  e8a8100000             -call 0x498bf0
    cpu.esp -= 4;
    sub_498bf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497b48  8d4df8                 -lea ecx, [ebp - 8]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497b4b  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497b4e  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00497b51  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00497b54  e8d7170000             -call 0x499330
    cpu.esp -= 4;
    sub_499330(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497b59  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497b5c  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497b5f  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00497b62  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00497b65  e8b6010000             -call 0x497d20
    cpu.esp -= 4;
    sub_497d20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497b6a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00497b6f  a3a8f57900             -mov dword ptr [0x79f5a8], eax
    app->getMemory<x86::reg32>(x86::reg32(7992744) /* 0x79f5a8 */) = cpu.eax;
    // 00497b74  a3b4f57900             -mov dword ptr [0x79f5b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.eax;
    // 00497b79  e958fbffff             -jmp 0x4976d6
    goto L_0x004976d6;
L_0x00497b7e:
    // 00497b7e  e89d250000             -call 0x49a120
    cpu.esp -= 4;
    sub_49a120(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497b83  e945fbffff             -jmp 0x4976cd
    goto L_0x004976cd;
L_0x00497b88:
    // 00497b88  e823010000             -call 0x497cb0
    cpu.esp -= 4;
    sub_497cb0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497b8d  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00497b8f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497b90  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497b91  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497b92  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497b93  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497b94  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497b95  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_497ba0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00497ba0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00497ba1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00497ba2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00497ba3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00497ba4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00497ba6  b8fcf37900             -mov eax, 0x79f3fc
    cpu.eax = 7992316 /*0x79f3fc*/;
    // 00497bab  e820cb0500             -call 0x4f46d0
    cpu.esp -= 4;
    sub_4f46d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497bb0  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00497bb2  b86cf47900             -mov eax, 0x79f46c
    cpu.eax = 7992428 /*0x79f46c*/;
    // 00497bb7  891558e55500           -mov dword ptr [0x55e558], edx
    app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */) = cpu.edx;
    // 00497bbd  e87ecb0500             -call 0x4f4740
    cpu.esp -= 4;
    sub_4f4740(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497bc2  8b15a4f57900           -mov edx, dword ptr [0x79f5a4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497bc8  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 00497bcd  ff5208                 -call dword ptr [edx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497bd0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00497bd2  740a                   -je 0x497bde
    if (cpu.flags.zf)
    {
        goto L_0x00497bde;
    }
    // 00497bd4  b8e4625600             -mov eax, 0x5662e4
    cpu.eax = 5661412 /*0x5662e4*/;
    // 00497bd9  e842790500             -call 0x4ef520
    cpu.esp -= 4;
    sub_4ef520(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00497bde:
    // 00497bde  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497be3  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00497be8  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497bea  8b15a4f57900           -mov edx, dword ptr [0x79f5a4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497bf0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00497bf2  890db4f57900           -mov dword ptr [0x79f5b4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.ecx;
    // 00497bf8  ff5208                 -call dword ptr [edx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497bfb  8b15a4f57900           -mov edx, dword ptr [0x79f5a4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497c01  a3c0f57900             -mov dword ptr [0x79f5c0], eax
    app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */) = cpu.eax;
    // 00497c06  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00497c08  ff5208                 -call dword ptr [edx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497c0b  a39cf57900             -mov dword ptr [0x79f59c], eax
    app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */) = cpu.eax;
    // 00497c10  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497c15  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00497c17  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497c1a  891da0f57900           -mov dword ptr [0x79f5a0], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992736) /* 0x79f5a0 */) = cpu.ebx;
    // 00497c20  891dd0f57900           -mov dword ptr [0x79f5d0], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992784) /* 0x79f5d0 */) = cpu.ebx;
    // 00497c26  891db8f57900           -mov dword ptr [0x79f5b8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992760) /* 0x79f5b8 */) = cpu.ebx;
    // 00497c2c  891d60e55500           -mov dword ptr [0x55e560], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629280) /* 0x55e560 */) = cpu.ebx;
    // 00497c32  891dbcf57900           -mov dword ptr [0x79f5bc], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */) = cpu.ebx;
    // 00497c38  891dd8f57900           -mov dword ptr [0x79f5d8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992792) /* 0x79f5d8 */) = cpu.ebx;
    // 00497c3e  891dc8f57900           -mov dword ptr [0x79f5c8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */) = cpu.ebx;
    // 00497c44  891dd4f57900           -mov dword ptr [0x79f5d4], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */) = cpu.ebx;
    // 00497c4a  891dccf57900           -mov dword ptr [0x79f5cc], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992780) /* 0x79f5cc */) = cpu.ebx;
    // 00497c50  891db0f57900           -mov dword ptr [0x79f5b0], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */) = cpu.ebx;
    // 00497c56  891d64e55500           -mov dword ptr [0x55e564], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */) = cpu.ebx;
    // 00497c5c  890db4f57900           -mov dword ptr [0x79f5b4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.ecx;
    // 00497c62  890da8f57900           -mov dword ptr [0x79f5a8], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992744) /* 0x79f5a8 */) = cpu.ecx;
    // 00497c68  890d90e55500           -mov dword ptr [0x55e590], ecx
    app->getMemory<x86::reg32>(x86::reg32(5629328) /* 0x55e590 */) = cpu.ecx;
    // 00497c6e  891d94e55500           -mov dword ptr [0x55e594], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629332) /* 0x55e594 */) = cpu.ebx;
    // 00497c74  891d98e55500           -mov dword ptr [0x55e598], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629336) /* 0x55e598 */) = cpu.ebx;
    // 00497c7a  891d9ce55500           -mov dword ptr [0x55e59c], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629340) /* 0x55e59c */) = cpu.ebx;
    // 00497c80  891da0e55500           -mov dword ptr [0x55e5a0], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629344) /* 0x55e5a0 */) = cpu.ebx;
    // 00497c86  891da4e55500           -mov dword ptr [0x55e5a4], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */) = cpu.ebx;
    // 00497c8c  890da8e55500           -mov dword ptr [0x55e5a8], ecx
    app->getMemory<x86::reg32>(x86::reg32(5629352) /* 0x55e5a8 */) = cpu.ecx;
    // 00497c92  891dace55500           -mov dword ptr [0x55e5ac], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629356) /* 0x55e5ac */) = cpu.ebx;
    // 00497c98  891dacf57900           -mov dword ptr [0x79f5ac], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992748) /* 0x79f5ac */) = cpu.ebx;
    // 00497c9e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497c9f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497ca0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497ca1  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497ca2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_497cb0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00497cb0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00497cb1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00497cb3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497cb4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_497cc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00497cc0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00497cc1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00497cc2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00497cc3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00497cc4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00497cc5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00497cc7  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00497cca  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00497ccc  8d4dfc                 -lea ecx, [ebp - 4]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00497ccf  8d5df8                 -lea ebx, [ebp - 8]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497cd2  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497cd5  a3acf57900             -mov dword ptr [0x79f5ac], eax
    app->getMemory<x86::reg32>(x86::reg32(7992748) /* 0x79f5ac */) = cpu.eax;
    // 00497cda  8d45f0                 -lea eax, [ebp - 0x10]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00497cdd  e84e160000             -call 0x499330
    cpu.esp -= 4;
    sub_499330(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497ce2  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00497ce5  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00497ce8  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00497ceb  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00497cee  e82d000000             -call 0x497d20
    cpu.esp -= 4;
    sub_497d20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497cf3  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00497cf5  e8e6020000             -call 0x497fe0
    cpu.esp -= 4;
    sub_497fe0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497cfa  a3d0f57900             -mov dword ptr [0x79f5d0], eax
    app->getMemory<x86::reg32>(x86::reg32(7992784) /* 0x79f5d0 */) = cpu.eax;
    // 00497cff  e88c030000             -call 0x498090
    cpu.esp -= 4;
    sub_498090(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497d04  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00497d06  e865060000             -call 0x498370
    cpu.esp -= 4;
    sub_498370(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497d0b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00497d0d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497d0e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497d0f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497d10  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497d11  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497d12  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_497d20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00497d20  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00497d21  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00497d22  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00497d23  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00497d25  81ecf8000000           -sub esp, 0xf8
    (cpu.esp) -= x86::reg32(x86::sreg32(248 /*0xf8*/));
    // 00497d2b  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 00497d31  894576                 -mov dword ptr [ebp + 0x76], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.eax;
    // 00497d34  895572                 -mov dword ptr [ebp + 0x72], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.edx;
    // 00497d37  895d6e                 -mov dword ptr [ebp + 0x6e], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.ebx;
    // 00497d3a  8d7d66                 -lea edi, [ebp + 0x66]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 00497d3d  beb0754900             -mov esi, 0x4975b0
    cpu.esi = 4814256 /*0x4975b0*/;
    // 00497d42  a15ce55500             -mov eax, dword ptr [0x55e55c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629276) /* 0x55e55c */);
    // 00497d47  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00497d49  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00497d4b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00497d4c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00497d4d  89557e                 -mov dword ptr [ebp + 0x7e], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.edx;
    // 00497d50  8d7d66                 -lea edi, [ebp + 0x66]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 00497d53  8b3485c0f59e00         -mov esi, dword ptr [eax*4 + 0x9ef5c0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10417600) /* 0x9ef5c0 */ + cpu.eax * 4);
    // 00497d5a  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00497d5f  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x00497d60:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00497d60  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 00497d62  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 00497d64  3c00                   +cmp al, 0
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
    // 00497d66  7410                   -je 0x497d78
    if (cpu.flags.zf)
    {
        goto L_0x00497d78;
    }
    // 00497d68  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00497d6b  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00497d6e  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00497d71  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00497d74  3c00                   +cmp al, 0
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
    // 00497d76  75e8                   -jne 0x497d60
    if (!cpu.flags.zf)
    {
        goto L_0x00497d60;
    }
L_0x00497d78:
    // 00497d78  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497d79  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497d7f  b80c000000             -mov eax, 0xc
    cpu.eax = 12 /*0xc*/;
    // 00497d84  bf28c05300             -mov edi, 0x53c028
    cpu.edi = 5488680 /*0x53c028*/;
    // 00497d89  ff5624                 -call dword ptr [esi + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497d8c  be0cc05300             -mov esi, 0x53c00c
    cpu.esi = 5488652 /*0x53c00c*/;
    // 00497d91  897d2a                 -mov dword ptr [ebp + 0x2a], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(42) /* 0x2a */) = cpu.edi;
    // 00497d94  894d1e                 -mov dword ptr [ebp + 0x1e], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(30) /* 0x1e */) = cpu.ecx;
    // 00497d97  30e4                   -xor ah, ah
    cpu.ah ^= x86::reg8(x86::sreg8(cpu.ah));
    // 00497d99  30d2                   -xor dl, dl
    cpu.dl ^= x86::reg8(x86::sreg8(cpu.dl));
    // 00497d9b  bf5cc05300             -mov edi, 0x53c05c
    cpu.edi = 5488732 /*0x53c05c*/;
    // 00497da0  88658a                 -mov byte ptr [ebp - 0x76], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-118) /* -0x76 */) = cpu.ah;
    // 00497da3  885552                 -mov byte ptr [ebp + 0x52], dl
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(82) /* 0x52 */) = cpu.dl;
    // 00497da6  897522                 -mov dword ptr [ebp + 0x22], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(34) /* 0x22 */) = cpu.esi;
    // 00497da9  89753a                 -mov dword ptr [ebp + 0x3a], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(58) /* 0x3a */) = cpu.esi;
    // 00497dac  ba44c05300             -mov edx, 0x53c044
    cpu.edx = 5488708 /*0x53c044*/;
    // 00497db1  a1acf57900             -mov eax, dword ptr [0x79f5ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992748) /* 0x79f5ac */);
    // 00497db6  897d32                 -mov dword ptr [ebp + 0x32], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(50) /* 0x32 */) = cpu.edi;
    // 00497db9  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00497dbb  89450a                 -mov dword ptr [ebp + 0xa], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(10) /* 0xa */) = cpu.eax;
    // 00497dbe  89750e                 -mov dword ptr [ebp + 0xe], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(14) /* 0xe */) = cpu.esi;
    // 00497dc1  89552e                 -mov dword ptr [ebp + 0x2e], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(46) /* 0x2e */) = cpu.edx;
    // 00497dc4  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00497dc6  b81cc05300             -mov eax, 0x53c01c
    cpu.eax = 5488668 /*0x53c01c*/;
    // 00497dcb  be50c05300             -mov esi, 0x53c050
    cpu.esi = 5488720 /*0x53c050*/;
    // 00497dd0  ba74c05300             -mov edx, 0x53c074
    cpu.edx = 5488756 /*0x53c074*/;
    // 00497dd5  894526                 -mov dword ptr [ebp + 0x26], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(38) /* 0x26 */) = cpu.eax;
    // 00497dd8  89453e                 -mov dword ptr [ebp + 0x3e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */) = cpu.eax;
    // 00497ddb  b834c05300             -mov eax, 0x53c034
    cpu.eax = 5488692 /*0x53c034*/;
    // 00497de0  897d7a                 -mov dword ptr [ebp + 0x7a], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.edi;
    // 00497de3  894542                 -mov dword ptr [ebp + 0x42], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */) = cpu.eax;
    // 00497de6  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 00497de9  897546                 -mov dword ptr [ebp + 0x46], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */) = cpu.esi;
    // 00497dec  894512                 -mov dword ptr [ebp + 0x12], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(18) /* 0x12 */) = cpu.eax;
    // 00497def  8b4572                 -mov eax, dword ptr [ebp + 0x72]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 00497df2  895536                 -mov dword ptr [ebp + 0x36], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(54) /* 0x36 */) = cpu.edx;
    // 00497df5  894516                 -mov dword ptr [ebp + 0x16], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(22) /* 0x16 */) = cpu.eax;
    // 00497df8  b868c05300             -mov eax, 0x53c068
    cpu.eax = 5488744 /*0x53c068*/;
    // 00497dfd  be84c05300             -mov esi, 0x53c084
    cpu.esi = 5488772 /*0x53c084*/;
    // 00497e02  89454a                 -mov dword ptr [ebp + 0x4a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */) = cpu.eax;
    // 00497e05  8b456e                 -mov eax, dword ptr [ebp + 0x6e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */);
    // 00497e08  89754e                 -mov dword ptr [ebp + 0x4e], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */) = cpu.esi;
    // 00497e0b  89451a                 -mov dword ptr [ebp + 0x1a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(26) /* 0x1a */) = cpu.eax;
L_0x00497e0e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00497e0e  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00497e11  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00497e14  8b74280a               -mov esi, dword ptr [eax + ebp + 0xa]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10) /* 0xa */ + cpu.ebp * 1);
    // 00497e18  8b3dace55500           -mov edi, dword ptr [0x55e5ac]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5629356) /* 0x55e5ac */);
    // 00497e1e  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00497e1f  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00497e21  7406                   -je 0x497e29
    if (cpu.flags.zf)
    {
        goto L_0x00497e29;
    }
    // 00497e23  8b442822               -mov eax, dword ptr [eax + ebp + 0x22]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(34) /* 0x22 */ + cpu.ebp * 1);
    // 00497e27  eb04                   -jmp 0x497e2d
    goto L_0x00497e2d;
L_0x00497e29:
    // 00497e29  8b44283a               -mov eax, dword ptr [eax + ebp + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */ + cpu.ebp * 1);
L_0x00497e2d:
    // 00497e2d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
L_0x00497e2e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00497e2e  8d4552                 -lea eax, [ebp + 0x52]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00497e31  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00497e32  e859780400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497e37  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00497e3a  8d7d52                 -lea edi, [ebp + 0x52]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00497e3d  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00497e3f  49                     -dec ecx
    (cpu.ecx)--;
    // 00497e40  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00497e42  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00497e44  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 00497e46  49                     -dec ecx
    (cpu.ecx)--;
    // 00497e47  01d9                   -add ecx, ebx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00497e49  a1c0f57900             -mov eax, dword ptr [0x79f5c0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00497e4e  41                     -inc ecx
    (cpu.ecx)++;
    // 00497e4f  39c1                   +cmp ecx, eax
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
    // 00497e51  770c                   -ja 0x497e5f
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00497e5f;
    }
    // 00497e53  8b557a                 -mov edx, dword ptr [ebp + 0x7a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00497e56  83fa02                 +cmp edx, 2
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
    // 00497e59  0f8586000000           -jne 0x497ee5
    if (!cpu.flags.zf)
    {
        goto L_0x00497ee5;
    }
L_0x00497e5f:
    // 00497e5f  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497e64  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00497e69  8b557e                 -mov edx, dword ptr [ebp + 0x7e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00497e6c  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497e6e  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 00497e70  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497e76  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00497e7c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00497e7e  ff5620                 -call dword ptr [esi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497e81  8d458a                 -lea eax, [ebp - 0x76]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-118) /* -0x76 */);
    // 00497e84  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00497e85  8b5d7e                 -mov ebx, dword ptr [ebp + 0x7e]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00497e88  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00497e89  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497e8f  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00497e91  ff510c                 -call dword ptr [ecx + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497e94  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497e99  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00497e9c  8d7301                 -lea esi, [ebx + 1]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 00497e9f  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497ea2  a1bcf57900             -mov eax, dword ptr [0x79f5bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */);
    // 00497ea7  89757e                 -mov dword ptr [ebp + 0x7e], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.esi;
    // 00497eaa  39f0                   +cmp eax, esi
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
    // 00497eac  7d06                   -jge 0x497eb4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00497eb4;
    }
    // 00497eae  8935bcf57900           -mov dword ptr [0x79f5bc], esi
    app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */) = cpu.esi;
L_0x00497eb4:
    // 00497eb4  8d7552                 -lea esi, [ebp + 0x52]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00497eb7  8d7d8a                 -lea edi, [ebp - 0x76]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-118) /* -0x76 */);
    // 00497eba  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x00497ebb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00497ebb  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 00497ebd  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 00497ebf  3c00                   +cmp al, 0
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
    // 00497ec1  7410                   -je 0x497ed3
    if (cpu.flags.zf)
    {
        goto L_0x00497ed3;
    }
    // 00497ec3  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00497ec6  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00497ec9  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00497ecc  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00497ecf  3c00                   +cmp al, 0
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
    // 00497ed1  75e8                   -jne 0x497ebb
    if (!cpu.flags.zf)
    {
        goto L_0x00497ebb;
    }
L_0x00497ed3:
    // 00497ed3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497ed4  8d7d8a                 -lea edi, [ebp - 0x76]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-118) /* -0x76 */);
    // 00497ed7  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00497ed9  49                     -dec ecx
    (cpu.ecx)--;
    // 00497eda  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00497edc  f2ae                   +repne scasb al, byte ptr es:[edi]
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
    // 00497ede  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 00497ee0  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 00497ee1  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 00497ee3  eb66                   -jmp 0x497f4b
    goto L_0x00497f4b;
L_0x00497ee5:
    // 00497ee5  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00497ee7  742a                   -je 0x497f13
    if (cpu.flags.zf)
    {
        goto L_0x00497f13;
    }
    // 00497ee9  be94c05300             -mov esi, 0x53c094
    cpu.esi = 5488788 /*0x53c094*/;
    // 00497eee  8d7d8a                 -lea edi, [ebp - 0x76]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-118) /* -0x76 */);
    // 00497ef1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00497ef2  2bc9                   +sub ecx, ecx
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
    // 00497ef4  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 00497ef5  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 00497ef7  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00497ef9  4f                     -dec edi
    (cpu.edi)--;
L_0x00497efa:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00497efa  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 00497efc  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 00497efe  3c00                   +cmp al, 0
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
    // 00497f00  7410                   -je 0x497f12
    if (cpu.flags.zf)
    {
        goto L_0x00497f12;
    }
    // 00497f02  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00497f05  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00497f08  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00497f0b  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00497f0e  3c00                   +cmp al, 0
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
    // 00497f10  75e8                   -jne 0x497efa
    if (!cpu.flags.zf)
    {
        goto L_0x00497efa;
    }
L_0x00497f12:
    // 00497f12  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x00497f13:
    // 00497f13  8d7552                 -lea esi, [ebp + 0x52]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00497f16  8d7d8a                 -lea edi, [ebp - 0x76]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-118) /* -0x76 */);
    // 00497f19  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00497f1a  2bc9                   +sub ecx, ecx
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
    // 00497f1c  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 00497f1d  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 00497f1f  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00497f21  4f                     -dec edi
    (cpu.edi)--;
L_0x00497f22:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00497f22  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 00497f24  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 00497f26  3c00                   +cmp al, 0
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
    // 00497f28  7410                   -je 0x497f3a
    if (cpu.flags.zf)
    {
        goto L_0x00497f3a;
    }
    // 00497f2a  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00497f2d  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00497f30  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00497f33  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00497f36  3c00                   +cmp al, 0
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
    // 00497f38  75e8                   -jne 0x497f22
    if (!cpu.flags.zf)
    {
        goto L_0x00497f22;
    }
L_0x00497f3a:
    // 00497f3a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497f3b  8d7d52                 -lea edi, [ebp + 0x52]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00497f3e  43                     -inc ebx
    (cpu.ebx)++;
    // 00497f3f  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00497f41  49                     -dec ecx
    (cpu.ecx)--;
    // 00497f42  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00497f44  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00497f46  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 00497f48  49                     -dec ecx
    (cpu.ecx)--;
    // 00497f49  01cb                   -add ebx, ecx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ecx));
L_0x00497f4b:
    // 00497f4b  ff457a                 -inc dword ptr [ebp + 0x7a]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */))++;
    // 00497f4e  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00497f51  83f806                 +cmp eax, 6
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
    // 00497f54  7d1a                   -jge 0x497f70
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00497f70;
    }
    // 00497f56  83f801                 +cmp eax, 1
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
    // 00497f59  0f85affeffff           -jne 0x497e0e
    if (!cpu.flags.zf)
    {
        goto L_0x00497e0e;
    }
    // 00497f5f  8d4566                 -lea eax, [ebp + 0x66]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 00497f62  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00497f63  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00497f66  8b4c8522               -mov ecx, dword ptr [ebp + eax*4 + 0x22]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(34) /* 0x22 */ + cpu.eax * 4);
    // 00497f6a  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00497f6b  e9befeffff             -jmp 0x497e2e
    goto L_0x00497e2e;
L_0x00497f70:
    // 00497f70  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497f75  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00497f7a  8b557e                 -mov edx, dword ptr [ebp + 0x7e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00497f7d  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497f7f  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 00497f81  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497f87  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00497f8d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00497f8f  ff5620                 -call dword ptr [esi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497f92  8d458a                 -lea eax, [ebp - 0x76]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-118) /* -0x76 */);
    // 00497f95  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00497f96  8b5d7e                 -mov ebx, dword ptr [ebp + 0x7e]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00497f99  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00497f9a  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497fa0  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00497fa2  ff510c                 -call dword ptr [ecx + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497fa5  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00497faa  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00497fad  8d7301                 -lea esi, [ebx + 1]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 00497fb0  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00497fb3  a1bcf57900             -mov eax, dword ptr [0x79f5bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */);
    // 00497fb8  89757e                 -mov dword ptr [ebp + 0x7e], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.esi;
    // 00497fbb  39f0                   +cmp eax, esi
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
    // 00497fbd  7d0c                   -jge 0x497fcb
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00497fcb;
    }
    // 00497fbf  8935b8f57900           -mov dword ptr [0x79f5b8], esi
    app->getMemory<x86::reg32>(x86::reg32(7992760) /* 0x79f5b8 */) = cpu.esi;
    // 00497fc5  8935bcf57900           -mov dword ptr [0x79f5bc], esi
    app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */) = cpu.esi;
L_0x00497fcb:
    // 00497fcb  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 00497fd1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497fd2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497fd3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00497fd4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_497fe0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00497fe0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00497fe1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00497fe2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00497fe3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00497fe4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00497fe5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00497fe6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00497fe8  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 00497feb  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00497fee  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00497ff0  8a255ce55500           -mov ah, byte ptr [0x55e55c]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(5629276) /* 0x55e55c */);
    // 00497ff6  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 00497ff9  f6c401                 +test ah, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 1 /*0x1*/));
    // 00497ffc  7407                   -je 0x498005
    if (cpu.flags.zf)
    {
        goto L_0x00498005;
    }
    // 00497ffe  c745fc01000000         -mov dword ptr [ebp - 4], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = 1 /*0x1*/;
L_0x00498005:
    // 00498005  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x00498007:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498007  3b75f8                 +cmp esi, dword ptr [ebp - 8]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049800a  0f8d66000000           -jge 0x498076
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00498076;
    }
    // 00498010  8d45f4                 -lea eax, [ebp - 0xc]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00498013  8d4df0                 -lea ecx, [ebp - 0x10]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00498016  8d5dec                 -lea ebx, [ebp - 0x14]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00498019  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049801a  8d55e8                 -lea edx, [ebp - 0x18]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0049801d  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0049801f  e8dc130000             -call 0x499400
    cpu.esp -= 4;
    sub_499400(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498024  a1a0f57900             -mov eax, dword ptr [0x79f5a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992736) /* 0x79f5a0 */);
    // 00498029  6681384642             +cmp word ptr [eax], 0x4246
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(16966 /*0x4246*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049802e  7506                   -jne 0x498036
    if (!cpu.flags.zf)
    {
        goto L_0x00498036;
    }
    // 00498030  837dfc01               +cmp dword ptr [ebp - 4], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498034  743d                   -je 0x498073
    if (cpu.flags.zf)
    {
        goto L_0x00498073;
    }
L_0x00498036:
    // 00498036  8b15c4f57900           -mov edx, dword ptr [0x79f5c4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049803c  8d04f500000000         -lea eax, [esi*8]
    cpu.eax = x86::reg32(cpu.esi * 8);
    // 00498043  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00498045  a1a0f57900             -mov eax, dword ptr [0x79f5a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992736) /* 0x79f5a0 */);
    // 0049804a  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
    // 0049804c  668b18                 -mov bx, word ptr [eax]
    cpu.bx = app->getMemory<x86::reg16>(cpu.eax);
    // 0049804f  47                     -inc edi
    (cpu.edi)++;
    // 00498050  6681fb4642             +cmp bx, 0x4246
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(16966 /*0x4246*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00498055  7410                   -je 0x498067
    if (cpu.flags.zf)
    {
        goto L_0x00498067;
    }
    // 00498057  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0049805a  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049805d  83c010                 +add eax, 0x10
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00498060  e81bc70500             -call 0x4f4780
    cpu.esp -= 4;
    sub_4f4780(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498065  eb02                   -jmp 0x498069
    goto L_0x00498069;
L_0x00498067:
    // 00498067  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
L_0x00498069:
    // 00498069  8b0dc4f57900           -mov ecx, dword ptr [0x79f5c4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049806f  8944f104               -mov dword ptr [ecx + esi*8 + 4], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */ + cpu.esi * 8) = cpu.eax;
L_0x00498073:
    // 00498073  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00498074  eb91                   -jmp 0x498007
    goto L_0x00498007;
L_0x00498076:
    // 00498076  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00498078  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049807a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049807b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049807c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049807d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049807e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049807f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498080  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_498090(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00498090  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00498091  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00498092  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00498093  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498094  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00498095  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00498096  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00498098  83ec5c                 -sub esp, 0x5c
    (cpu.esp) -= x86::reg32(x86::sreg32(92 /*0x5c*/));
    // 0049809b  b909000000             -mov ecx, 9
    cpu.ecx = 9 /*0x9*/;
    // 004980a0  8d7da4                 -lea edi, [ebp - 0x5c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 004980a3  beb8754900             -mov esi, 0x4975b8
    cpu.esi = 4814264 /*0x4975b8*/;
    // 004980a8  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004980aa  b909000000             -mov ecx, 9
    cpu.ecx = 9 /*0x9*/;
    // 004980af  8d7dc8                 -lea edi, [ebp - 0x38]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 004980b2  bedc754900             -mov esi, 0x4975dc
    cpu.esi = 4814300 /*0x4975dc*/;
    // 004980b7  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004980b9  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004980bb  8b0d60e55500           -mov ecx, dword ptr [0x55e560]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629280) /* 0x55e560 */);
    // 004980c1  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004980c4  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004980c6  7511                   -jne 0x4980d9
    if (!cpu.flags.zf)
    {
        goto L_0x004980d9;
    }
    // 004980c8  a1b8f57900             -mov eax, dword ptr [0x79f5b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992760) /* 0x79f5b8 */);
    // 004980cd  a3bcf57900             -mov dword ptr [0x79f5bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */) = cpu.eax;
    // 004980d2  a360e55500             -mov dword ptr [0x55e560], eax
    app->getMemory<x86::reg32>(x86::reg32(5629280) /* 0x55e560 */) = cpu.eax;
    // 004980d7  eb06                   -jmp 0x4980df
    goto L_0x004980df;
L_0x004980d9:
    // 004980d9  890dbcf57900           -mov dword ptr [0x79f5bc], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */) = cpu.ecx;
L_0x004980df:
    // 004980df  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004980e4  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004980e9  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004980eb  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 004980ed  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004980f3  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 004980f9  8b15bcf57900           -mov edx, dword ptr [0x79f5bc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */);
    // 004980ff  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00498101  ff5620                 -call dword ptr [esi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498104  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00498106  e977000000             -jmp 0x498182
    goto L_0x00498182;
L_0x0049810b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049810b  8b1da4f57900           -mov ebx, dword ptr [0x79f5a4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498111  ba05000000             -mov edx, 5
    cpu.edx = 5 /*0x5*/;
    // 00498116  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 0049811b  eb10                   -jmp 0x49812d
    goto L_0x0049812d;
L_0x0049811d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049811d  8b1da4f57900           -mov ebx, dword ptr [0x79f5a4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498123  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00498128  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
L_0x0049812d:
    // 0049812d  ff5324                 -call dword ptr [ebx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498130  8d7dec                 -lea edi, [ebp - 0x14]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00498133  8b748da4               -mov esi, dword ptr [ebp + ecx*4 - 0x5c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */ + cpu.ecx * 4);
    // 00498137  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x00498138:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498138  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049813a  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049813c  3c00                   +cmp al, 0
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
    // 0049813e  7410                   -je 0x498150
    if (cpu.flags.zf)
    {
        goto L_0x00498150;
    }
    // 00498140  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00498143  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00498146  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00498149  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049814c  3c00                   +cmp al, 0
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
    // 0049814e  75e8                   -jne 0x498138
    if (!cpu.flags.zf)
    {
        goto L_0x00498138;
    }
L_0x00498150:
    // 00498150  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498151  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00498154  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498155  8b3dbcf57900           -mov edi, dword ptr [0x79f5bc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */);
    // 0049815b  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049815c  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049815f  8b1da4f57900           -mov ebx, dword ptr [0x79f5a4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498165  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498166  ff530c                 -call dword ptr [ebx + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498169  8b048d6ce55500         -mov eax, dword ptr [ecx*4 + 0x55e56c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629292) /* 0x55e56c */ + cpu.ecx * 4);
    // 00498170  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00498173  40                     -inc eax
    (cpu.eax)++;
    // 00498174  41                     -inc ecx
    (cpu.ecx)++;
    // 00498175  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00498177  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0049817a  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0049817d  83f902                 +cmp ecx, 2
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498180  7d17                   -jge 0x498199
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00498199;
    }
L_0x00498182:
    // 00498182  3b0da4e55500           +cmp ecx, dword ptr [0x55e5a4]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498188  7593                   -jne 0x49811d
    if (!cpu.flags.zf)
    {
        goto L_0x0049811d;
    }
    // 0049818a  833da0e5550001         +cmp dword ptr [0x55e5a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629344) /* 0x55e5a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498191  0f8474ffffff           -je 0x49810b
    if (cpu.flags.zf)
    {
        goto L_0x0049810b;
    }
    // 00498197  eb84                   -jmp 0x49811d
    goto L_0x0049811d;
L_0x00498199:
    // 00498199  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049819c  49                     -dec ecx
    (cpu.ecx)--;
    // 0049819d  8b1d64e55500           -mov ebx, dword ptr [0x55e564]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 004981a3  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 004981a6  39d9                   +cmp ecx, ebx
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
    // 004981a8  7e06                   -jle 0x4981b0
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004981b0;
    }
    // 004981aa  890d64e55500           -mov dword ptr [0x55e564], ecx
    app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */) = cpu.ecx;
L_0x004981b0:
    // 004981b0  a168e55500             -mov eax, dword ptr [0x55e568]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */);
    // 004981b5  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
L_0x004981b8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004981b8  8b75f8                 -mov esi, dword ptr [ebp - 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004981bb  83fe09                 +cmp esi, 9
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(9 /*0x9*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004981be  0f8d10010000           -jge 0x4982d4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004982d4;
    }
    // 004981c4  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004981cb  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004981ce  8bb86ce55500           -mov edi, dword ptr [eax + 0x55e56c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629292) /* 0x55e56c */);
    // 004981d4  01fa                   -add edx, edi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004981d6  42                     -inc edx
    (cpu.edx)++;
    // 004981d7  8b0dc0f57900           -mov ecx, dword ptr [0x79f5c0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 004981dd  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
    // 004981e0  39ca                   +cmp edx, ecx
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004981e2  0f8dec000000           -jge 0x4982d4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004982d4;
    }
    // 004981e8  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004981eb  a164e55500             -mov eax, dword ptr [0x55e564]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 004981f0  8d5f01                 -lea ebx, [edi + 1]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 004981f3  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004981f5  39d8                   +cmp eax, ebx
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
    // 004981f7  7e02                   -jle 0x4981fb
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004981fb;
    }
    // 004981f9  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
L_0x004981fb:
    // 004981fb  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 004981fd  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00498202  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498208  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049820b  8b15bcf57900           -mov edx, dword ptr [0x79f5bc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */);
    // 00498211  ff5620                 -call dword ptr [esi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498214  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00498217  8d7dec                 -lea edi, [ebp - 0x14]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049821a  8b7485a4               -mov esi, dword ptr [ebp + eax*4 - 0x5c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */ + cpu.eax * 4);
    // 0049821e  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0049821f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049821f  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 00498221  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 00498223  3c00                   +cmp al, 0
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
    // 00498225  7410                   -je 0x498237
    if (cpu.flags.zf)
    {
        goto L_0x00498237;
    }
    // 00498227  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049822a  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049822d  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00498230  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00498233  3c00                   +cmp al, 0
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
    // 00498235  75e8                   -jne 0x49821f
    if (!cpu.flags.zf)
    {
        goto L_0x0049821f;
    }
L_0x00498237:
    // 00498237  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498238  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049823b  3b05a4e55500           +cmp eax, dword ptr [0x55e5a4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498241  751b                   -jne 0x49825e
    if (!cpu.flags.zf)
    {
        goto L_0x0049825e;
    }
    // 00498243  833da0e5550001         +cmp dword ptr [0x55e5a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629344) /* 0x55e5a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049824a  7512                   -jne 0x49825e
    if (!cpu.flags.zf)
    {
        goto L_0x0049825e;
    }
    // 0049824c  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498252  ba05000000             -mov edx, 5
    cpu.edx = 5 /*0x5*/;
    // 00498257  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 0049825c  eb10                   -jmp 0x49826e
    goto L_0x0049826e;
L_0x0049825e:
    // 0049825e  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498264  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 00498269  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
L_0x0049826e:
    // 0049826e  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498271  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00498274  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00498277  837c28c801             +cmp dword ptr [eax + ebp - 0x38], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-56) /* -0x38 */ + cpu.ebp * 1);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049827c  7515                   -jne 0x498293
    if (!cpu.flags.zf)
    {
        goto L_0x00498293;
    }
    // 0049827e  8b7c28a4               -mov edi, dword ptr [eax + ebp - 0x5c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-92) /* -0x5c */ + cpu.ebp * 1);
    // 00498282  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00498284  49                     -dec ecx
    (cpu.ecx)--;
    // 00498285  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00498287  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00498289  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 0049828b  49                     -dec ecx
    (cpu.ecx)--;
    // 0049828c  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049828f  29c8                   +sub eax, ecx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00498291  eb04                   -jmp 0x498297
    goto L_0x00498297;
L_0x00498293:
    // 00498293  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00498296  40                     -inc eax
    (cpu.eax)++;
L_0x00498297:
    // 00498297  8d55ec                 -lea edx, [ebp - 0x14]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049829a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049829b  8b1dbcf57900           -mov ebx, dword ptr [0x79f5bc]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */);
    // 004982a1  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004982a2  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004982a8  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004982a9  ff510c                 -call dword ptr [ecx + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004982ac  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004982af  8b3564e55500           -mov esi, dword ptr [0x55e564]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 004982b5  a3b0f57900             -mov dword ptr [0x79f5b0], eax
    app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */) = cpu.eax;
    // 004982ba  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004982bd  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004982c0  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004982c3  39f0                   +cmp eax, esi
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
    // 004982c5  7e05                   -jle 0x4982cc
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004982cc;
    }
    // 004982c7  a364e55500             -mov dword ptr [0x55e564], eax
    app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */) = cpu.eax;
L_0x004982cc:
    // 004982cc  ff45f8                 +inc dword ptr [ebp - 8]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004982cf  e9e4feffff             -jmp 0x4981b8
    goto L_0x004981b8;
L_0x004982d4:
    // 004982d4  8b15bcf57900           -mov edx, dword ptr [0x79f5bc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */);
    // 004982da  42                     -inc edx
    (cpu.edx)++;
    // 004982db  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004982e0  8915bcf57900           -mov dword ptr [0x79f5bc], edx
    app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */) = cpu.edx;
    // 004982e6  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004982e9  a1bcf57900             -mov eax, dword ptr [0x79f5bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */);
    // 004982ee  8b0d90e55500           -mov ecx, dword ptr [0x55e590]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629328) /* 0x55e590 */);
    // 004982f4  a3b8f57900             -mov dword ptr [0x79f5b8], eax
    app->getMemory<x86::reg32>(x86::reg32(7992760) /* 0x79f5b8 */) = cpu.eax;
    // 004982f9  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004982fb  744e                   -je 0x49834b
    if (cpu.flags.zf)
    {
        goto L_0x0049834b;
    }
    // 004982fd  a19cf57900             -mov eax, dword ptr [0x79f59c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00498302  8b3db8f57900           -mov edi, dword ptr [0x79f5b8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992760) /* 0x79f5b8 */);
    // 00498308  8b15d0f57900           -mov edx, dword ptr [0x79f5d0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992784) /* 0x79f5d0 */);
    // 0049830e  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00498310  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00498312  891dc8f57900           -mov dword ptr [0x79f5c8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */) = cpu.ebx;
    // 00498318  48                     -dec eax
    (cpu.eax)--;
    // 00498319  891d58e55500           -mov dword ptr [0x55e558], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */) = cpu.ebx;
    // 0049831f  a3d4f57900             -mov dword ptr [0x79f5d4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */) = cpu.eax;
    // 00498324  39d0                   +cmp eax, edx
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
    // 00498326  7c08                   -jl 0x498330
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00498330;
    }
    // 00498328  8d42ff                 -lea eax, [edx - 1]
    cpu.eax = x86::reg32(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 0049832b  a3d4f57900             -mov dword ptr [0x79f5d4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */) = cpu.eax;
L_0x00498330:
    // 00498330  8b0dc8f57900           -mov ecx, dword ptr [0x79f5c8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */);
    // 00498336  a1d4f57900             -mov eax, dword ptr [0x79f5d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */);
    // 0049833b  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049833d  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049833f  48                     -dec eax
    (cpu.eax)--;
    // 00498340  891d90e55500           -mov dword ptr [0x55e590], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629328) /* 0x55e590 */) = cpu.ebx;
    // 00498346  a3ccf57900             -mov dword ptr [0x79f5cc], eax
    app->getMemory<x86::reg32>(x86::reg32(7992780) /* 0x79f5cc */) = cpu.eax;
L_0x0049834b:
    // 0049834b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049834d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049834e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049834f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498350  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498351  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498352  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498353  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_498370(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00498370  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00498371  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00498372  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00498373  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498374  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00498375  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00498376  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00498378  83ec5c                 -sub esp, 0x5c
    (cpu.esp) -= x86::reg32(x86::sreg32(92 /*0x5c*/));
    // 0049837b  66a100764900           -mov ax, word ptr [0x497600]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(4814336) /* 0x497600 */);
    // 00498381  8d7dc4                 -lea edi, [ebp - 0x3c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00498384  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00498387  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0049838a  be04764900             -mov esi, 0x497604
    cpu.esi = 4814340 /*0x497604*/;
    // 0049838f  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
    // 00498392  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498397  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00498398  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00498399  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049839a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049839c  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049839e  8b0d94e55500           -mov ecx, dword ptr [0x55e594]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629332) /* 0x55e594 */);
    // 004983a4  8955e4                 -mov dword ptr [ebp - 0x1c], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.edx;
    // 004983a7  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004983a9  7427                   -je 0x4983d2
    if (cpu.flags.zf)
    {
        goto L_0x004983d2;
    }
    // 004983ab  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 004983ad  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004983b3  8b0d9ce55500           -mov ecx, dword ptr [0x55e59c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629340) /* 0x55e59c */);
    // 004983b9  8b1d98e55500           -mov ebx, dword ptr [0x55e598]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629336) /* 0x55e598 */);
    // 004983bf  8b15bcf57900           -mov edx, dword ptr [0x79f5bc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */);
    // 004983c5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004983c7  ff5620                 -call dword ptr [esi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004983ca  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004983cc  891d94e55500           -mov dword ptr [0x55e594], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629332) /* 0x55e594 */) = cpu.ebx;
L_0x004983d2:
    // 004983d2  833db4f5790000         +cmp dword ptr [0x79f5b4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004983d9  743c                   -je 0x498417
    if (cpu.flags.zf)
    {
        goto L_0x00498417;
    }
    // 004983db  8b3dbcf57900           -mov edi, dword ptr [0x79f5bc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */);
    // 004983e1  a19cf57900             -mov eax, dword ptr [0x79f59c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 004983e6  8b15d0f57900           -mov edx, dword ptr [0x79f5d0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992784) /* 0x79f5d0 */);
    // 004983ec  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004983ee  39d0                   +cmp eax, edx
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
    // 004983f0  7d04                   -jge 0x4983f6
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004983f6;
    }
    // 004983f2  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004983f4  eb02                   -jmp 0x4983f8
    goto L_0x004983f8;
L_0x004983f6:
    // 004983f6  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
L_0x004983f8:
    // 004983f8  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 004983fa  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498400  8b1d64e55500           -mov ebx, dword ptr [0x55e564]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 00498406  8b15bcf57900           -mov edx, dword ptr [0x79f5bc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992764) /* 0x79f5bc */);
    // 0049840c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049840e  890d9ce55500           -mov dword ptr [0x55e59c], ecx
    app->getMemory<x86::reg32>(x86::reg32(5629340) /* 0x55e59c */) = cpu.ecx;
    // 00498414  ff5620                 -call dword ptr [esi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00498417:
    // 00498417  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 0049841d  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00498422  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00498424  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498427  a1b8f57900             -mov eax, dword ptr [0x79f5b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992760) /* 0x79f5b8 */);
    // 0049842c  8b0dc8f57900           -mov ecx, dword ptr [0x79f5c8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */);
    // 00498432  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00498435  a1d4f57900             -mov eax, dword ptr [0x79f5d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */);
    // 0049843a  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049843c  40                     -inc eax
    (cpu.eax)++;
    // 0049843d  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 0049843f  a3ccf57900             -mov dword ptr [0x79f5cc], eax
    app->getMemory<x86::reg32>(x86::reg32(7992780) /* 0x79f5cc */) = cpu.eax;
L_0x00498444:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498444  a1d4f57900             -mov eax, dword ptr [0x79f5d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */);
    // 00498449  40                     -inc eax
    (cpu.eax)++;
    // 0049844a  39c6                   +cmp esi, eax
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
    // 0049844c  0f8d9c010000           -jge 0x4985ee
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004985ee;
    }
    // 00498452  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00498457  8b3db4f57900           -mov edi, dword ptr [0x79f5b4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */);
    // 0049845d  895df0                 -mov dword ptr [ebp - 0x10], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ebx;
    // 00498460  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00498462  7523                   -jne 0x498487
    if (!cpu.flags.zf)
    {
        goto L_0x00498487;
    }
    // 00498464  3b3558e55500           +cmp esi, dword ptr [0x55e558]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049846a  7414                   -je 0x498480
    if (cpu.flags.zf)
    {
        goto L_0x00498480;
    }
    // 0049846c  3b35d8f57900           +cmp esi, dword ptr [0x79f5d8]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992792) /* 0x79f5d8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498472  740c                   -je 0x498480
    if (cpu.flags.zf)
    {
        goto L_0x00498480;
    }
    // 00498474  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00498477  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00498478  897df0                 -mov dword ptr [ebp - 0x10], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edi;
    // 0049847b  895df4                 -mov dword ptr [ebp - 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;
    // 0049847e  eb07                   -jmp 0x498487
    goto L_0x00498487;
L_0x00498480:
    // 00498480  c745f001000000         -mov dword ptr [ebp - 0x10], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = 1 /*0x1*/;
L_0x00498487:
    // 00498487  837df001               +cmp dword ptr [ebp - 0x10], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049848b  0f8557010000           -jne 0x4985e8
    if (!cpu.flags.zf)
    {
        goto L_0x004985e8;
    }
    // 00498491  8b15c4f57900           -mov edx, dword ptr [0x79f5c4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 00498497  8b04f2                 -mov eax, dword ptr [edx + esi*8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.esi * 8);
    // 0049849a  6681384642             +cmp word ptr [eax], 0x4246
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(16966 /*0x4246*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049849f  0f8582000000           -jne 0x498527
    if (!cpu.flags.zf)
    {
        goto L_0x00498527;
    }
    // 004984a5  8b1558e55500           -mov edx, dword ptr [0x55e558]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */);
    // 004984ab  39d6                   +cmp esi, edx
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
    // 004984ad  7534                   -jne 0x4984e3
    if (!cpu.flags.zf)
    {
        goto L_0x004984e3;
    }
    // 004984af  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004984b1  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004984b6  e8e5040000             -call 0x4989a0
    cpu.esp -= 4;
    sub_4989a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004984bb  6a0b                   -push 0xb
    app->getMemory<x86::reg32>(cpu.esp-4) = 11 /*0xb*/;
    cpu.esp -= 4;
    // 004984bd  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004984c0  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004984c6  8b1d64e55500           -mov ebx, dword ptr [0x55e564]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 004984cc  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004984ce  ff5720                 -call dword ptr [edi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004984d1  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004984d7  ba0b000000             -mov edx, 0xb
    cpu.edx = 11 /*0xb*/;
    // 004984dc  b80a000000             -mov eax, 0xa
    cpu.eax = 10 /*0xa*/;
    // 004984e1  eb2b                   -jmp 0x49850e
    goto L_0x0049850e;
L_0x004984e3:
    // 004984e3  6a09                   -push 9
    app->getMemory<x86::reg32>(cpu.esp-4) = 9 /*0x9*/;
    cpu.esp -= 4;
    // 004984e5  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004984ea  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004984ed  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004984f3  8b1d64e55500           -mov ebx, dword ptr [0x55e564]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 004984f9  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004984fb  ff5720                 -call dword ptr [edi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004984fe  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498504  ba09000000             -mov edx, 9
    cpu.edx = 9 /*0x9*/;
    // 00498509  b808000000             -mov eax, 8
    cpu.eax = 8 /*0x8*/;
L_0x0049850e:
    // 0049850e  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498511  6898c05300             -push 0x53c098
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488792 /*0x53c098*/;
    cpu.esp -= 4;
    // 00498516  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 00498519  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049851a  e871710400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049851f  83c408                 +add esp, 8
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
    // 00498522  e9a5000000             -jmp 0x4985cc
    goto L_0x004985cc;
L_0x00498527:
    // 00498527  8b0d58e55500           -mov ecx, dword ptr [0x55e558]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */);
    // 0049852d  39ce                   +cmp esi, ecx
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
    // 0049852f  7534                   -jne 0x498565
    if (!cpu.flags.zf)
    {
        goto L_0x00498565;
    }
    // 00498531  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00498533  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00498536  e865040000             -call 0x4989a0
    cpu.esp -= 4;
    sub_4989a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049853b  6a07                   -push 7
    app->getMemory<x86::reg32>(cpu.esp-4) = 7 /*0x7*/;
    cpu.esp -= 4;
    // 0049853d  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00498542  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498548  8b1d64e55500           -mov ebx, dword ptr [0x55e564]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 0049854e  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00498550  ff5720                 -call dword ptr [edi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498553  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498559  ba07000000             -mov edx, 7
    cpu.edx = 7 /*0x7*/;
    // 0049855e  b806000000             -mov eax, 6
    cpu.eax = 6 /*0x6*/;
    // 00498563  eb28                   -jmp 0x49858d
    goto L_0x0049858d;
L_0x00498565:
    // 00498565  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 00498567  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0049856c  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049856f  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498575  8b1d64e55500           -mov ebx, dword ptr [0x55e564]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 0049857b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049857d  ff5720                 -call dword ptr [edi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498580  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498586  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0049858b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0049858d:
    // 0049858d  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498590  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 00498595  8b04f0                 -mov eax, dword ptr [eax + esi*8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + cpu.esi * 8);
    // 00498598  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0049859b  e840900400             -call 0x4e15e0
    cpu.esp -= 4;
    nfs3hp::ea::memBlockName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004985a0  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004985a2  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004985a4  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004985a6  49                     -dec ecx
    (cpu.ecx)--;
    // 004985a7  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004985a9  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004985ab  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004985ad  49                     -dec ecx
    (cpu.ecx)--;
    // 004985ae  83f90c                 +cmp ecx, 0xc
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(12 /*0xc*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004985b1  7e05                   -jle 0x4985b8
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004985b8;
    }
    // 004985b3  8d41f4                 -lea eax, [ecx - 0xc]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(-12) /* -0xc */);
    // 004985b6  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
L_0x004985b8:
    // 004985b8  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004985b9  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004985bb  68a4c05300             -push 0x53c0a4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488804 /*0x53c0a4*/;
    cpu.esp -= 4;
    // 004985c0  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 004985c3  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004985c4  e8c7700400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004985c9  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
L_0x004985cc:
    // 004985cc  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 004985cf  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004985d0  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004985d3  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004985d4  8b15a4f57900           -mov edx, dword ptr [0x79f5a4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004985da  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004985dc  8d7b01                 -lea edi, [ebx + 1]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 004985df  ff520c                 -call dword ptr [edx + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004985e2  897df4                 -mov dword ptr [ebp - 0xc], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edi;
    // 004985e5  83c40c                 +add esp, 0xc
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
L_0x004985e8:
    // 004985e8  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004985e9  e956feffff             -jmp 0x498444
    goto L_0x00498444;
L_0x004985ee:
    // 004985ee  8b1570e55500           -mov edx, dword ptr [0x55e570]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629296) /* 0x55e570 */);
    // 004985f4  a16ce55500             -mov eax, dword ptr [0x55e56c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629292) /* 0x55e56c */);
    // 004985f9  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004985fb  40                     -inc eax
    (cpu.eax)++;
    // 004985fc  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 004985ff  a168e55500             -mov eax, dword ptr [0x55e568]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */);
    // 00498604  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
L_0x00498607:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498607  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049860a  3b05b0f57900           +cmp eax, dword ptr [0x79f5b0]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498610  0f8f79030000           -jg 0x49898f
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0049898f;
    }
    // 00498616  a1b8f57900             -mov eax, dword ptr [0x79f5b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992760) /* 0x79f5b8 */);
    // 0049861b  8b1dc8f57900           -mov ebx, dword ptr [0x79f5c8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992776) /* 0x79f5c8 */);
    // 00498621  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
L_0x00498624:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498624  a1d4f57900             -mov eax, dword ptr [0x79f5d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992788) /* 0x79f5d4 */);
    // 00498629  40                     -inc eax
    (cpu.eax)++;
    // 0049862a  39c3                   +cmp ebx, eax
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
    // 0049862c  0f8d24030000           -jge 0x498956
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00498956;
    }
    // 00498632  8b35b4f57900           -mov esi, dword ptr [0x79f5b4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */);
    // 00498638  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0049863a  7523                   -jne 0x49865f
    if (!cpu.flags.zf)
    {
        goto L_0x0049865f;
    }
    // 0049863c  3b1d58e55500           +cmp ebx, dword ptr [0x55e558]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498642  7414                   -je 0x498658
    if (cpu.flags.zf)
    {
        goto L_0x00498658;
    }
    // 00498644  3b1dd8f57900           +cmp ebx, dword ptr [0x79f5d8]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992792) /* 0x79f5d8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049864a  740c                   -je 0x498658
    if (cpu.flags.zf)
    {
        goto L_0x00498658;
    }
    // 0049864c  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049864f  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00498650  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
    // 00498653  894dec                 -mov dword ptr [ebp - 0x14], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.ecx;
    // 00498656  eb07                   -jmp 0x49865f
    goto L_0x0049865f;
L_0x00498658:
    // 00498658  c745f001000000         -mov dword ptr [ebp - 0x10], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = 1 /*0x1*/;
L_0x0049865f:
    // 0049865f  8b7df0                 -mov edi, dword ptr [ebp - 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00498662  83ff01                 +cmp edi, 1
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
    // 00498665  0f85bf020000           -jne 0x49892a
    if (!cpu.flags.zf)
    {
        goto L_0x0049892a;
    }
    // 0049866b  8b1558e55500           -mov edx, dword ptr [0x55e558]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */);
    // 00498671  8d04dd00000000         -lea eax, [ebx*8]
    cpu.eax = x86::reg32(cpu.ebx * 8);
    // 00498678  39d3                   +cmp ebx, edx
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
    // 0049867a  7539                   -jne 0x4986b5
    if (!cpu.flags.zf)
    {
        goto L_0x004986b5;
    }
    // 0049867c  8b15c4f57900           -mov edx, dword ptr [0x79f5c4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 00498682  8b0402                 -mov eax, dword ptr [edx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 1);
    // 00498685  e8f60e0000             -call 0x499580
    cpu.esp -= 4;
    sub_499580(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049868a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049868c  7412                   -je 0x4986a0
    if (cpu.flags.zf)
    {
        goto L_0x004986a0;
    }
    // 0049868e  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498694  ba0b000000             -mov edx, 0xb
    cpu.edx = 11 /*0xb*/;
    // 00498699  b80a000000             -mov eax, 0xa
    cpu.eax = 10 /*0xa*/;
    // 0049869e  eb10                   -jmp 0x4986b0
    goto L_0x004986b0;
L_0x004986a0:
    // 004986a0  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004986a6  ba07000000             -mov edx, 7
    cpu.edx = 7 /*0x7*/;
    // 004986ab  b806000000             -mov eax, 6
    cpu.eax = 6 /*0x6*/;
L_0x004986b0:
    // 004986b0  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004986b3  eb2f                   -jmp 0x4986e4
    goto L_0x004986e4;
L_0x004986b5:
    // 004986b5  8b15c4f57900           -mov edx, dword ptr [0x79f5c4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 004986bb  8b0402                 -mov eax, dword ptr [edx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 1);
    // 004986be  e8bd0e0000             -call 0x499580
    cpu.esp -= 4;
    sub_499580(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004986c3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004986c5  7412                   -je 0x4986d9
    if (cpu.flags.zf)
    {
        goto L_0x004986d9;
    }
    // 004986c7  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004986cd  ba09000000             -mov edx, 9
    cpu.edx = 9 /*0x9*/;
    // 004986d2  b808000000             -mov eax, 8
    cpu.eax = 8 /*0x8*/;
    // 004986d7  eb08                   -jmp 0x4986e1
    goto L_0x004986e1;
L_0x004986d9:
    // 004986d9  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004986df  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
L_0x004986e1:
    // 004986e1  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004986e4:
    // 004986e4  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004986e7  40                     -inc eax
    (cpu.eax)++;
    // 004986e8  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
    // 004986eb  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004986ee  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004986f1  83f806                 +cmp eax, 6
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
    // 004986f4  0f870b020000           -ja 0x498905
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00498905;
    }
    // 004986fa  ff248554834900         -jmp dword ptr [eax*4 + 0x498354]
    cpu.ip = app->getMemory<x86::reg32>(4817748 + cpu.eax * 4); goto dynamic_jump;
  case 0x00498701:
    // 00498701  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 00498706  8b04d8                 -mov eax, dword ptr [eax + ebx*8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + cpu.ebx * 8);
    // 00498709  e8520d0000             -call 0x499460
    cpu.esp -= 4;
    sub_499460(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049870e  8b0dace55500           -mov ecx, dword ptr [0x55e5ac]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629356) /* 0x55e5ac */);
    // 00498714  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498715  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00498717  7407                   -je 0x498720
    if (cpu.flags.zf)
    {
        goto L_0x00498720;
    }
    // 00498719  b8b4c05300             -mov eax, 0x53c0b4
    cpu.eax = 5488820 /*0x53c0b4*/;
    // 0049871e  eb05                   -jmp 0x498725
    goto L_0x00498725;
L_0x00498720:
    // 00498720  b8bcc05300             -mov eax, 0x53c0bc
    cpu.eax = 5488828 /*0x53c0bc*/;
L_0x00498725:
    // 00498725  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498726  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 00498729  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049872a  e8616f0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049872f  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00498732  8b35b0f57900           -mov esi, dword ptr [0x79f5b0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */);
    // 00498738  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0049873b  39f0                   +cmp eax, esi
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
    // 0049873d  0f84c8010000           -je 0x49890b
    if (cpu.flags.zf)
    {
        goto L_0x0049890b;
    }
    // 00498743  8d75fc                 -lea esi, [ebp - 4]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00498746  8b7ddc                 -mov edi, dword ptr [ebp - 0x24]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00498749  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049874a  2bc9                   +sub ecx, ecx
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
    // 0049874c  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0049874d  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0049874f  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00498751  4f                     -dec edi
    (cpu.edi)--;
L_0x00498752:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498752  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 00498754  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 00498756  3c00                   +cmp al, 0
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
    // 00498758  7410                   -je 0x49876a
    if (cpu.flags.zf)
    {
        goto L_0x0049876a;
    }
    // 0049875a  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049875d  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00498760  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00498763  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00498766  3c00                   +cmp al, 0
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
    // 00498768  75e8                   -jne 0x498752
    if (!cpu.flags.zf)
    {
        goto L_0x00498752;
    }
L_0x0049876a:
    // 0049876a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049876b  e99b010000             -jmp 0x49890b
    goto L_0x0049890b;
  case 0x00498770:
    // 00498770  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 00498775  8b04d8                 -mov eax, dword ptr [eax + ebx*8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + cpu.ebx * 8);
    // 00498778  e8030d0000             -call 0x499480
    cpu.esp -= 4;
    sub_499480(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049877d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049877e  68c4c05300             -push 0x53c0c4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488836 /*0x53c0c4*/;
    cpu.esp -= 4;
    // 00498783  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 00498786  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498787  e8046f0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049878c  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049878f  8b3db0f57900           -mov edi, dword ptr [0x79f5b0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */);
    // 00498795  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00498798  39f8                   +cmp eax, edi
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
    // 0049879a  0f846b010000           -je 0x49890b
    if (cpu.flags.zf)
    {
        goto L_0x0049890b;
    }
    // 004987a0  8d75fc                 -lea esi, [ebp - 4]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004987a3  8d7da4                 -lea edi, [ebp - 0x5c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 004987a6  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004987a7  2bc9                   +sub ecx, ecx
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
    // 004987a9  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004987aa  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 004987ac  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004987ae  4f                     -dec edi
    (cpu.edi)--;
L_0x004987af:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004987af  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004987b1  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004987b3  3c00                   +cmp al, 0
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
    // 004987b5  7410                   -je 0x4987c7
    if (cpu.flags.zf)
    {
        goto L_0x004987c7;
    }
    // 004987b7  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004987ba  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004987bd  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004987c0  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004987c3  3c00                   +cmp al, 0
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
    // 004987c5  75e8                   -jne 0x4987af
    if (!cpu.flags.zf)
    {
        goto L_0x004987af;
    }
L_0x004987c7:
    // 004987c7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004987c8  e93e010000             -jmp 0x49890b
    goto L_0x0049890b;
  case 0x004987cd:
    // 004987cd  8b15c4f57900           -mov edx, dword ptr [0x79f5c4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 004987d3  8b04da                 -mov eax, dword ptr [edx + ebx*8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.ebx * 8);
    // 004987d6  8d55d0                 -lea edx, [ebp - 0x30]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 004987d9  e8b20c0000             -call 0x499490
    cpu.esp -= 4;
    sub_499490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004987de  e85d060000             -call 0x498e40
    cpu.esp -= 4;
    sub_498e40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004987e3  8d45d0                 -lea eax, [ebp - 0x30]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 004987e6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004987e7  68c8c05300             -push 0x53c0c8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488840 /*0x53c0c8*/;
    cpu.esp -= 4;
    // 004987ec  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 004987ef  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004987f0  e89b6e0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004987f5  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004987f8  8b15b0f57900           -mov edx, dword ptr [0x79f5b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */);
    // 004987fe  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00498801  39d0                   +cmp eax, edx
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
    // 00498803  0f8402010000           -je 0x49890b
    if (cpu.flags.zf)
    {
        goto L_0x0049890b;
    }
    // 00498809  8d75fc                 -lea esi, [ebp - 4]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049880c  8d7da4                 -lea edi, [ebp - 0x5c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0049880f  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00498810  2bc9                   +sub ecx, ecx
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
    // 00498812  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 00498813  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 00498815  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00498817  4f                     -dec edi
    (cpu.edi)--;
L_0x00498818:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498818  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049881a  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049881c  3c00                   +cmp al, 0
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
    // 0049881e  7410                   -je 0x498830
    if (cpu.flags.zf)
    {
        goto L_0x00498830;
    }
    // 00498820  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00498823  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00498826  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00498829  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049882c  3c00                   +cmp al, 0
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
    // 0049882e  75e8                   -jne 0x498818
    if (!cpu.flags.zf)
    {
        goto L_0x00498818;
    }
L_0x00498830:
    // 00498830  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498831  e9d5000000             -jmp 0x49890b
    goto L_0x0049890b;
  case 0x00498836:
    // 00498836  68d0c05300             -push 0x53c0d0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488848 /*0x53c0d0*/;
    cpu.esp -= 4;
    // 0049883b  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0049883e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049883f  e84c6e0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498844  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00498847  8b0db0f57900           -mov ecx, dword ptr [0x79f5b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */);
    // 0049884d  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00498850  39c8                   +cmp eax, ecx
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
    // 00498852  0f84b3000000           -je 0x49890b
    if (cpu.flags.zf)
    {
        goto L_0x0049890b;
    }
    // 00498858  8d75fc                 -lea esi, [ebp - 4]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049885b  8d7da4                 -lea edi, [ebp - 0x5c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0049885e  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049885f  2bc9                   +sub ecx, ecx
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
    // 00498861  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 00498862  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 00498864  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00498866  4f                     -dec edi
    (cpu.edi)--;
L_0x00498867:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498867  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 00498869  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049886b  3c00                   +cmp al, 0
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
    // 0049886d  7410                   -je 0x49887f
    if (cpu.flags.zf)
    {
        goto L_0x0049887f;
    }
    // 0049886f  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00498872  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00498875  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00498878  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049887b  3c00                   +cmp al, 0
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
    // 0049887d  75e8                   -jne 0x498867
    if (!cpu.flags.zf)
    {
        goto L_0x00498867;
    }
L_0x0049887f:
    // 0049887f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498880  e986000000             -jmp 0x49890b
    goto L_0x0049890b;
  case 0x00498885:
    // 00498885  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049888a  8d0cdd00000000         -lea ecx, [ebx*8]
    cpu.ecx = x86::reg32(cpu.ebx * 8);
    // 00498891  8b0401                 -mov eax, dword ptr [ecx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1);
    // 00498894  e8f70b0000             -call 0x499490
    cpu.esp -= 4;
    sub_499490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498899  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049889e  8b0401                 -mov eax, dword ptr [ecx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1);
    // 004988a1  e8da0c0000             -call 0x499580
    cpu.esp -= 4;
    sub_499580(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004988a6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004988a8  0f8557000000           -jne 0x498905
    if (!cpu.flags.zf)
    {
        goto L_0x00498905;
    }
    // 004988ae  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 004988b3  8b740104               -mov esi, dword ptr [ecx + eax + 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */ + cpu.eax * 1);
    // 004988b7  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004988b8  68d4c05300             -push 0x53c0d4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488852 /*0x53c0d4*/;
    cpu.esp -= 4;
    // 004988bd  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 004988c0  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004988c1  e8ca6d0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004988c6  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004988c9  8b3db0f57900           -mov edi, dword ptr [0x79f5b0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */);
    // 004988cf  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004988d2  39f8                   +cmp eax, edi
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
    // 004988d4  7428                   -je 0x4988fe
    if (cpu.flags.zf)
    {
        goto L_0x004988fe;
    }
    // 004988d6  8d75fc                 -lea esi, [ebp - 4]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004988d9  8d7da4                 -lea edi, [ebp - 0x5c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 004988dc  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004988dd  2bc9                   +sub ecx, ecx
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
    // 004988df  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004988e0  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 004988e2  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004988e4  4f                     -dec edi
    (cpu.edi)--;
L_0x004988e5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004988e5  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004988e7  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004988e9  3c00                   +cmp al, 0
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
    // 004988eb  7410                   -je 0x4988fd
    if (cpu.flags.zf)
    {
        goto L_0x004988fd;
    }
    // 004988ed  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004988f0  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004988f3  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004988f6  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004988f9  3c00                   +cmp al, 0
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
    // 004988fb  75e8                   -jne 0x4988e5
    if (!cpu.flags.zf)
    {
        goto L_0x004988e5;
    }
L_0x004988fd:
    // 004988fd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x004988fe:
    // 004988fe  c745e401000000         -mov dword ptr [ebp - 0x1c], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = 1 /*0x1*/;
  [[fallthrough]];
  case 0x00498905:
L_0x00498905:
    // 00498905  837de401               +cmp dword ptr [ebp - 0x1c], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498909  751c                   -jne 0x498927
    if (!cpu.flags.zf)
    {
        goto L_0x00498927;
    }
L_0x0049890b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049890b  8b75dc                 -mov esi, dword ptr [ebp - 0x24]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0049890e  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049890f  8b7dec                 -mov edi, dword ptr [ebp - 0x14]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00498912  8b55e0                 -mov edx, dword ptr [ebp - 0x20]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00498915  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00498916  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 0049891b  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049891d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049891e  894de4                 -mov dword ptr [ebp - 0x1c], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.ecx;
    // 00498921  ff500c                 -call dword ptr [eax + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498924  83c40c                 +add esp, 0xc
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
L_0x00498927:
    // 00498927  ff45ec                 +inc dword ptr [ebp - 0x14]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
L_0x0049892a:
    // 0049892a  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049892b  e9f4fcffff             -jmp 0x498624
    goto L_0x00498624;
  case 0x00498930:
    // 00498930  8b15c4f57900           -mov edx, dword ptr [0x79f5c4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 00498936  8b04da                 -mov eax, dword ptr [edx + ebx*8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.ebx * 8);
    // 00498939  e882be0500             -call 0x4f47c0
    cpu.esp -= 4;
    sub_4f47c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049893e  8b5485c4               -mov edx, dword ptr [ebp + eax*4 - 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */ + cpu.eax * 4);
    // 00498942  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00498943  68dcc05300             -push 0x53c0dc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488860 /*0x53c0dc*/;
    cpu.esp -= 4;
    // 00498948  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0049894b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049894c  e83f6d0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498951  83c40c                 +add esp, 0xc
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
    // 00498954  ebb5                   -jmp 0x49890b
    goto L_0x0049890b;
L_0x00498956:
    // 00498956  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00498959  8b04856ce55500         -mov eax, dword ptr [eax*4 + 0x55e56c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629292) /* 0x55e56c */ + cpu.eax * 4);
    // 00498960  8b7de8                 -mov edi, dword ptr [ebp - 0x18]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00498963  40                     -inc eax
    (cpu.eax)++;
    // 00498964  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 00498966  8b1564e55500           -mov edx, dword ptr [0x55e564]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 0049896c  897de8                 -mov dword ptr [ebp - 0x18], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.edi;
    // 0049896f  39d7                   +cmp edi, edx
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498971  7e06                   -jle 0x498979
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00498979;
    }
    // 00498973  893d64e55500           -mov dword ptr [0x55e564], edi
    app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */) = cpu.edi;
L_0x00498979:
    // 00498979  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049897c  a164e55500             -mov eax, dword ptr [0x55e564]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 00498981  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00498982  a398e55500             -mov dword ptr [0x55e598], eax
    app->getMemory<x86::reg32>(x86::reg32(5629336) /* 0x55e598 */) = cpu.eax;
    // 00498987  894df8                 -mov dword ptr [ebp - 8], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ecx;
    // 0049898a  e978fcffff             -jmp 0x498607
    goto L_0x00498607;
L_0x0049898f:
    // 0049898f  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498994  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498997  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00498999  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049899a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049899b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049899c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049899d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049899e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049899f  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip  */
void Application::sub_4989a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004989a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004989a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004989a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004989a3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004989a4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004989a5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004989a7  83ec50                 -sub esp, 0x50
    (cpu.esp) -= x86::reg32(x86::sreg32(80 /*0x50*/));
    // 004989aa  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004989ac  e8ef0a0000             -call 0x4994a0
    cpu.esp -= 4;
    sub_4994a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004989b1  8d5db0                 -lea ebx, [ebp - 0x50]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 004989b4  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004989b6  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004989b8  e883000000             -call 0x498a40
    cpu.esp -= 4;
    sub_498a40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004989bd  6a0e                   -push 0xe
    app->getMemory<x86::reg32>(cpu.esp-4) = 14 /*0xe*/;
    cpu.esp -= 4;
    // 004989bf  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004989c4  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004989ca  8b1d64e55500           -mov ebx, dword ptr [0x55e564]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 004989d0  8b159cf57900           -mov edx, dword ptr [0x79f59c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 004989d6  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004989d8  ff5620                 -call dword ptr [esi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004989db  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004989e1  ba0e000000             -mov edx, 0xe
    cpu.edx = 14 /*0xe*/;
    // 004989e6  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 004989eb  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004989ee  8d55b0                 -lea edx, [ebp - 0x50]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 004989f1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004989f2  8b159cf57900           -mov edx, dword ptr [0x79f59c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 004989f8  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004989f9  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004989fe  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00498a00  ff500c                 -call dword ptr [eax + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498a03  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498a09  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00498a0c  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00498a11  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00498a13  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498a16  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00498a18  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a19  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a1a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a1b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a1c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a1d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_498a40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00498a40  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00498a41  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498a42  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00498a43  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00498a44  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00498a46  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00498a49  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 00498a4b  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00498a4d  8b1db0e55500           -mov ebx, dword ptr [0x55e5b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629360) /* 0x55e5b0 */);
    // 00498a53  894df4                 -mov dword ptr [ebp - 0xc], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ecx;
    // 00498a56  894df8                 -mov dword ptr [ebp - 8], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ecx;
    // 00498a59  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 00498a5c  894df0                 -mov dword ptr [ebp - 0x10], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ecx;
    // 00498a5f  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00498a61  0f85d5000000           -jne 0x498b3c
    if (!cpu.flags.zf)
    {
        goto L_0x00498b3c;
    }
    // 00498a67  83fa05                 +cmp edx, 5
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498a6a  0f87b7000000           -ja 0x498b27
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00498b27;
    }
    // 00498a70  ff2495208a4900         -jmp dword ptr [edx*4 + 0x498a20]
    cpu.ip = app->getMemory<x86::reg32>(4819488 + cpu.edx * 4); goto dynamic_jump;
  case 0x00498a77:
    // 00498a77  6848c15300             -push 0x53c148
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488968 /*0x53c148*/;
    cpu.esp -= 4;
    // 00498a7c  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498a7d  e80e6c0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498a82  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00498a85  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00498a87  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a88  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a89  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a8a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a8b  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00498a8c:
    // 00498a8c  6854c15300             -push 0x53c154
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488980 /*0x53c154*/;
    cpu.esp -= 4;
    // 00498a91  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498a92  e8f96b0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498a97  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00498a9a  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00498a9c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a9d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a9e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498a9f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498aa0  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00498aa1:
    // 00498aa1  8d14c500000000         -lea edx, [eax*8]
    cpu.edx = x86::reg32(cpu.eax * 8);
    // 00498aa8  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 00498aad  8d4df8                 -lea ecx, [ebp - 8]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00498ab0  8b0402                 -mov eax, dword ptr [edx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 1);
    // 00498ab3  8d55fc                 -lea edx, [ebp - 4]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00498ab6  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00498ab9  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00498aba  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00498abd  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00498ac0  e8ab000000             -call 0x498b70
    cpu.esp -= 4;
    sub_498b70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498ac5  8b7df4                 -mov edi, dword ptr [ebp - 0xc]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00498ac8  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00498ac9  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00498acc  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498acd  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00498ad0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00498ad1  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00498ad4  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00498ada  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00498adb  83fb46                 +cmp ebx, 0x46
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(70 /*0x46*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498ade  7d07                   -jge 0x498ae7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00498ae7;
    }
    // 00498ae0  b8e4c05300             -mov eax, 0x53c0e4
    cpu.eax = 5488868 /*0x53c0e4*/;
    // 00498ae5  eb05                   -jmp 0x498aec
    goto L_0x00498aec;
L_0x00498ae7:
    // 00498ae7  b80cc15300             -mov eax, 0x53c10c
    cpu.eax = 5488908 /*0x53c10c*/;
L_0x00498aec:
    // 00498aec  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498aed  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498aee  e89d6b0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498af3  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 00498af6  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00498af8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498af9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498afa  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498afb  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498afc  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00498afd:
    // 00498afd  686cc15300             -push 0x53c16c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489004 /*0x53c16c*/;
    cpu.esp -= 4;
    // 00498b02  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498b03  e8886b0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498b08  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00498b0b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00498b0d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b0e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b0f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b10  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b11  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00498b12:
    // 00498b12  688cc15300             -push 0x53c18c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489036 /*0x53c18c*/;
    cpu.esp -= 4;
    // 00498b17  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498b18  e8736b0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498b1d  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00498b20  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00498b22  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b23  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b24  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b25  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b26  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00498b27:
L_0x00498b27:
    // 00498b27  6864c15300             -push 0x53c164
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488996 /*0x53c164*/;
    cpu.esp -= 4;
    // 00498b2c  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498b2d  e85e6b0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498b32  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00498b35  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00498b37  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b38  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b39  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b3a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b3b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00498b3c:
    // 00498b3c  8d14c500000000         -lea edx, [eax*8]
    cpu.edx = x86::reg32(cpu.eax * 8);
    // 00498b43  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 00498b48  8b0402                 -mov eax, dword ptr [edx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 1);
    // 00498b4b  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00498b4e  e88d8a0400             -call 0x4e15e0
    cpu.esp -= 4;
    nfs3hp::ea::memBlockName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498b53  68e0c05300             -push 0x53c0e0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488864 /*0x53c0e0*/;
    cpu.esp -= 4;
    // 00498b58  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498b59  68acc15300             -push 0x53c1ac
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489068 /*0x53c1ac*/;
    cpu.esp -= 4;
    // 00498b5e  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498b5f  e82c6b0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498b64  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00498b67  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00498b69  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b6a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b6b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b6c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498b6d  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8b 0xc0 */
void Application::sub_498b70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00498b70  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498b71  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00498b72  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00498b73  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00498b75  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00498b78  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00498b7a  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00498b7d  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 00498b7f  894df8                 -mov dword ptr [ebp - 8], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ecx;
    // 00498b82  e8cd1a0500             -call 0x4ea654
    cpu.esp -= 4;
    nfs3hp::frontend::simPackCount(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498b87  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00498b89  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
L_0x00498b8b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498b8b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00498b8e  3b08                   +cmp ecx, dword ptr [eax]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498b90  7d54                   -jge 0x498be6
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00498be6;
    }
    // 00498b92  8d5df0                 -lea ebx, [ebp - 0x10]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00498b95  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00498b97  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00498b99  e8c71a0500             -call 0x4ea665
    cpu.esp -= 4;
    sub_4ea665(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498b9e  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00498ba1  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00498ba3  e838660500             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498ba8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00498baa  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00498bac  83e07f                 -and eax, 0x7f
    cpu.eax &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 00498baf  83f87c                 +cmp eax, 0x7c
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(124 /*0x7c*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498bb2  740a                   -je 0x498bbe
    if (cpu.flags.zf)
    {
        goto L_0x00498bbe;
    }
    // 00498bb4  83f86e                 +cmp eax, 0x6e
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(110 /*0x6e*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498bb7  7405                   -je 0x498bbe
    if (cpu.flags.zf)
    {
        goto L_0x00498bbe;
    }
    // 00498bb9  83f86f                 +cmp eax, 0x6f
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(111 /*0x6f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498bbc  7504                   -jne 0x498bc2
    if (!cpu.flags.zf)
    {
        goto L_0x00498bc2;
    }
L_0x00498bbe:
    // 00498bbe  ff07                   +inc dword ptr [edi]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.edi);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00498bc0  eb21                   -jmp 0x498be3
    goto L_0x00498be3;
L_0x00498bc2:
    // 00498bc2  83e070                 -and eax, 0x70
    cpu.eax &= x86::reg32(x86::sreg32(112 /*0x70*/));
    // 00498bc5  83f820                 +cmp eax, 0x20
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(32 /*0x20*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498bc8  7504                   -jne 0x498bce
    if (!cpu.flags.zf)
    {
        goto L_0x00498bce;
    }
    // 00498bca  ff07                   +inc dword ptr [edi]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.edi);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00498bcc  eb15                   -jmp 0x498be3
    goto L_0x00498be3;
L_0x00498bce:
    // 00498bce  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 00498bd0  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00498bd5  a880                   +test al, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 128 /*0x80*/));
    // 00498bd7  7405                   -je 0x498bde
    if (cpu.flags.zf)
    {
        goto L_0x00498bde;
    }
    // 00498bd9  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00498bdc  eb03                   -jmp 0x498be1
    goto L_0x00498be1;
L_0x00498bde:
    // 00498bde  8b4510                 -mov eax, dword ptr [ebp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
L_0x00498be1:
    // 00498be1  ff00                   +inc dword ptr [eax]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.eax);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
L_0x00498be3:
    // 00498be3  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00498be4  eba5                   -jmp 0x498b8b
    goto L_0x00498b8b;
L_0x00498be6:
    // 00498be6  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00498be8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498be9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498bea  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498beb  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_498bf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00498bf0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00498bf1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00498bf2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00498bf3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498bf4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00498bf5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00498bf6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00498bf8  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 00498bfb  a1e0625600             -mov eax, dword ptr [0x5662e0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5661408) /* 0x5662e0 */);
    // 00498c00  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00498c02  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00498c05  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498c0a  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00498c0f  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00498c12  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498c14  6a0f                   -push 0xf
    app->getMemory<x86::reg32>(cpu.esp-4) = 15 /*0xf*/;
    cpu.esp -= 4;
    // 00498c16  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498c1c  8b1d64e55500           -mov ebx, dword ptr [0x55e564]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629284) /* 0x55e564 */);
    // 00498c22  8b159cf57900           -mov edx, dword ptr [0x79f59c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00498c28  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00498c2a  ff5720                 -call dword ptr [edi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498c2d  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498c32  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00498c34  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00498c36:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498c36  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498c39  3b4df8                 +cmp ecx, dword ptr [ebp - 8]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498c3c  0f8d7f000000           -jge 0x498cc1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00498cc1;
    }
    // 00498c42  39f1                   +cmp ecx, esi
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
    // 00498c44  7512                   -jne 0x498c58
    if (!cpu.flags.zf)
    {
        goto L_0x00498c58;
    }
    // 00498c46  8b1da4f57900           -mov ebx, dword ptr [0x79f5a4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498c4c  ba11000000             -mov edx, 0x11
    cpu.edx = 17 /*0x11*/;
    // 00498c51  b812000000             -mov eax, 0x12
    cpu.eax = 18 /*0x12*/;
    // 00498c56  eb10                   -jmp 0x498c68
    goto L_0x00498c68;
L_0x00498c58:
    // 00498c58  8b1da4f57900           -mov ebx, dword ptr [0x79f5a4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498c5e  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 00498c63  b810000000             -mov eax, 0x10
    cpu.eax = 16 /*0x10*/;
L_0x00498c68:
    // 00498c68  ff5324                 -call dword ptr [ebx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498c6b  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 00498c72  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00498c74  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00498c77  0524f57900             -add eax, 0x79f524
    (cpu.eax) += x86::reg32(x86::sreg32(7992612 /*0x79f524*/));
    // 00498c7c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498c7d  68c0c15300             -push 0x53c1c0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489088 /*0x53c1c0*/;
    cpu.esp -= 4;
    // 00498c82  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00498c85  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498c86  e8056a0400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498c8b  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498c90  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00498c93  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498c95  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00498c98  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498c99  8b3d9cf57900           -mov edi, dword ptr [0x79f59c]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00498c9f  8d04cd00000000         -lea eax, [ecx*8]
    cpu.eax = x86::reg32(cpu.ecx * 8);
    // 00498ca6  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00498ca7  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00498ca9  8b1da4f57900           -mov ebx, dword ptr [0x79f5a4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498caf  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498cb0  ff530c                 -call dword ptr [ebx + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498cb3  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498cb8  83c40c                 +add esp, 0xc
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
    // 00498cbb  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00498cbc  e975ffffff             -jmp 0x498c36
    goto L_0x00498c36;
L_0x00498cc1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498cc1  837dfc00               +cmp dword ptr [ebp - 4], 0
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
    // 00498cc5  0f8528010000           -jne 0x498df3
    if (!cpu.flags.zf)
    {
        goto L_0x00498df3;
    }
    // 00498ccb  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498cd0  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00498cd2  ff5018                 -call dword ptr [eax + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498cd5  3d00480000             +cmp eax, 0x4800
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18432 /*0x4800*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498cda  721d                   -jb 0x498cf9
    if (cpu.flags.cf)
    {
        goto L_0x00498cf9;
    }
    // 00498cdc  7627                   -jbe 0x498d05
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00498d05;
    }
    // 00498cde  3d004d0000             +cmp eax, 0x4d00
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(19712 /*0x4d00*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498ce3  720b                   -jb 0x498cf0
    if (cpu.flags.cf)
    {
        goto L_0x00498cf0;
    }
    // 00498ce5  7627                   -jbe 0x498d0e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00498d0e;
    }
    // 00498ce7  3d00500000             +cmp eax, 0x5000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20480 /*0x5000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498cec  7417                   -je 0x498d05
    if (cpu.flags.zf)
    {
        goto L_0x00498d05;
    }
    // 00498cee  eb54                   -jmp 0x498d44
    goto L_0x00498d44;
L_0x00498cf0:
    // 00498cf0  3d004b0000             +cmp eax, 0x4b00
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(19200 /*0x4b00*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498cf5  7428                   -je 0x498d1f
    if (cpu.flags.zf)
    {
        goto L_0x00498d1f;
    }
    // 00498cf7  eb4b                   -jmp 0x498d44
    goto L_0x00498d44;
L_0x00498cf9:
    // 00498cf9  83f80d                 +cmp eax, 0xd
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(13 /*0xd*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498cfc  7246                   -jb 0x498d44
    if (cpu.flags.cf)
    {
        goto L_0x00498d44;
    }
    // 00498cfe  762c                   -jbe 0x498d2c
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00498d2c;
    }
    // 00498d00  83f81b                 +cmp eax, 0x1b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(27 /*0x1b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498d03  753f                   -jne 0x498d44
    if (!cpu.flags.zf)
    {
        goto L_0x00498d44;
    }
L_0x00498d05:
    // 00498d05  c745fc01000000         -mov dword ptr [ebp - 4], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = 1 /*0x1*/;
    // 00498d0c  eb36                   -jmp 0x498d44
    goto L_0x00498d44;
L_0x00498d0e:
    // 00498d0e  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00498d11  48                     -dec eax
    (cpu.eax)--;
    // 00498d12  39c6                   +cmp esi, eax
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
    // 00498d14  7d2e                   -jge 0x498d44
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00498d44;
    }
    // 00498d16  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00498d1b  01ce                   +add esi, ecx
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00498d1d  eb25                   -jmp 0x498d44
    goto L_0x00498d44;
L_0x00498d1f:
    // 00498d1f  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00498d21  7e21                   -jle 0x498d44
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00498d44;
    }
    // 00498d23  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00498d28  29ce                   +sub esi, ecx
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00498d2a  eb18                   -jmp 0x498d44
    goto L_0x00498d44;
L_0x00498d2c:
    // 00498d2c  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 00498d33  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00498d38  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 00498d3a  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 00498d3d  ff14852cf57900         -call dword ptr [eax*4 + 0x79f52c]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(7992620) /* 0x79f52c */ + cpu.eax * 4);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00498d44:
    // 00498d44  83f901                 +cmp ecx, 1
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
    // 00498d47  0f8574ffffff           -jne 0x498cc1
    if (!cpu.flags.zf)
    {
        goto L_0x00498cc1;
    }
    // 00498d4d  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00498d4f  7504                   -jne 0x498d55
    if (!cpu.flags.zf)
    {
        goto L_0x00498d55;
    }
    // 00498d51  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00498d53  eb03                   -jmp 0x498d58
    goto L_0x00498d58;
L_0x00498d55:
    // 00498d55  8d4eff                 -lea ecx, [esi - 1]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(-1) /* -0x1 */);
L_0x00498d58:
    // 00498d58  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00498d5b  48                     -dec eax
    (cpu.eax)--;
    // 00498d5c  39c6                   +cmp esi, eax
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
    // 00498d5e  7504                   -jne 0x498d64
    if (!cpu.flags.zf)
    {
        goto L_0x00498d64;
    }
    // 00498d60  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00498d62  eb03                   -jmp 0x498d67
    goto L_0x00498d67;
L_0x00498d64:
    // 00498d64  8d5e01                 -lea ebx, [esi + 1]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
L_0x00498d67:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498d67  8d4301                 -lea eax, [ebx + 1]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 00498d6a  39c1                   +cmp ecx, eax
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
    // 00498d6c  0f8d4fffffff           -jge 0x498cc1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00498cc1;
    }
    // 00498d72  39f1                   +cmp ecx, esi
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
    // 00498d74  7512                   -jne 0x498d88
    if (!cpu.flags.zf)
    {
        goto L_0x00498d88;
    }
    // 00498d76  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498d7c  ba11000000             -mov edx, 0x11
    cpu.edx = 17 /*0x11*/;
    // 00498d81  b812000000             -mov eax, 0x12
    cpu.eax = 18 /*0x12*/;
    // 00498d86  eb10                   -jmp 0x498d98
    goto L_0x00498d98;
L_0x00498d88:
    // 00498d88  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498d8e  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 00498d93  b810000000             -mov eax, 0x10
    cpu.eax = 16 /*0x10*/;
L_0x00498d98:
    // 00498d98  ff5724                 -call dword ptr [edi + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498d9b  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 00498da2  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00498da4  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00498da7  0524f57900             -add eax, 0x79f524
    (cpu.eax) += x86::reg32(x86::sreg32(7992612 /*0x79f524*/));
    // 00498dac  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498dad  68c0c15300             -push 0x53c1c0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489088 /*0x53c1c0*/;
    cpu.esp -= 4;
    // 00498db2  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00498db5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498db6  e8d5680400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498dbb  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498dc0  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00498dc3  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498dc5  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00498dc8  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498dc9  a19cf57900             -mov eax, dword ptr [0x79f59c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00498dce  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498dcf  8d04cd00000000         -lea eax, [ecx*8]
    cpu.eax = x86::reg32(cpu.ecx * 8);
    // 00498dd6  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00498dd8  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498dde  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00498ddf  ff570c                 -call dword ptr [edi + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498de2  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498de7  83c40c                 +add esp, 0xc
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
    // 00498dea  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00498deb  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498dee  e974ffffff             -jmp 0x498d67
    goto L_0x00498d67;
L_0x00498df3:
    // 00498df3  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00498df9  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00498dfe  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00498e00  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498e03  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00498e05  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498e06  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498e07  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498e08  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498e09  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498e0a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498e0b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_498e10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00498e10  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00498e11  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00498e12  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00498e14  8b155ce55500           -mov edx, dword ptr [0x55e55c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629276) /* 0x55e55c */);
L_0x00498e1a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498e1a  42                     -inc edx
    (cpu.edx)++;
    // 00498e1b  83e20f                 -and edx, 0xf
    cpu.edx &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 00498e1e  833c95c0f59e0000       +cmp dword ptr [edx*4 + 0x9ef5c0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10417600) /* 0x9ef5c0 */ + cpu.edx * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498e26  74f2                   -je 0x498e1a
    if (cpu.flags.zf)
    {
        goto L_0x00498e1a;
    }
    // 00498e28  89155ce55500           -mov dword ptr [0x55e55c], edx
    app->getMemory<x86::reg32>(x86::reg32(5629276) /* 0x55e55c */) = cpu.edx;
    // 00498e2e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498e2f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498e30  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_498e40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00498e40  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00498e41  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00498e42  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00498e44  f6c402                 +test ah, 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 2 /*0x2*/));
    // 00498e47  7407                   -je 0x498e50
    if (cpu.flags.zf)
    {
        goto L_0x00498e50;
    }
    // 00498e49  b965000000             -mov ecx, 0x65
    cpu.ecx = 101 /*0x65*/;
    // 00498e4e  eb05                   -jmp 0x498e55
    goto L_0x00498e55;
L_0x00498e50:
    // 00498e50  b920000000             -mov ecx, 0x20
    cpu.ecx = 32 /*0x20*/;
L_0x00498e55:
    // 00498e55  880a                   -mov byte ptr [edx], cl
    app->getMemory<x86::reg8>(cpu.edx) = cpu.cl;
    // 00498e57  f6c420                 +test ah, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 32 /*0x20*/));
    // 00498e5a  7407                   -je 0x498e63
    if (cpu.flags.zf)
    {
        goto L_0x00498e63;
    }
    // 00498e5c  b962000000             -mov ecx, 0x62
    cpu.ecx = 98 /*0x62*/;
    // 00498e61  eb05                   -jmp 0x498e68
    goto L_0x00498e68;
L_0x00498e63:
    // 00498e63  b920000000             -mov ecx, 0x20
    cpu.ecx = 32 /*0x20*/;
L_0x00498e68:
    // 00498e68  884a01                 -mov byte ptr [edx + 1], cl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */) = cpu.cl;
    // 00498e6b  a810                   +test al, 0x10
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 16 /*0x10*/));
    // 00498e6d  7407                   -je 0x498e76
    if (cpu.flags.zf)
    {
        goto L_0x00498e76;
    }
    // 00498e6f  b968000000             -mov ecx, 0x68
    cpu.ecx = 104 /*0x68*/;
    // 00498e74  eb05                   -jmp 0x498e7b
    goto L_0x00498e7b;
L_0x00498e76:
    // 00498e76  b920000000             -mov ecx, 0x20
    cpu.ecx = 32 /*0x20*/;
L_0x00498e7b:
    // 00498e7b  c6420320               -mov byte ptr [edx + 3], 0x20
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(3) /* 0x3 */) = 32 /*0x20*/;
    // 00498e7f  c6420420               -mov byte ptr [edx + 4], 0x20
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */) = 32 /*0x20*/;
    // 00498e83  884a02                 -mov byte ptr [edx + 2], cl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(2) /* 0x2 */) = cpu.cl;
    // 00498e86  a840                   +test al, 0x40
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 64 /*0x40*/));
    // 00498e88  7407                   -je 0x498e91
    if (cpu.flags.zf)
    {
        goto L_0x00498e91;
    }
    // 00498e8a  b961000000             -mov ecx, 0x61
    cpu.ecx = 97 /*0x61*/;
    // 00498e8f  eb05                   -jmp 0x498e96
    goto L_0x00498e96;
L_0x00498e91:
    // 00498e91  b920000000             -mov ecx, 0x20
    cpu.ecx = 32 /*0x20*/;
L_0x00498e96:
    // 00498e96  884a05                 -mov byte ptr [edx + 5], cl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(5) /* 0x5 */) = cpu.cl;
    // 00498e99  f6c480                 +test ah, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 128 /*0x80*/));
    // 00498e9c  7407                   -je 0x498ea5
    if (cpu.flags.zf)
    {
        goto L_0x00498ea5;
    }
    // 00498e9e  b873000000             -mov eax, 0x73
    cpu.eax = 115 /*0x73*/;
    // 00498ea3  eb05                   -jmp 0x498eaa
    goto L_0x00498eaa;
L_0x00498ea5:
    // 00498ea5  b820000000             -mov eax, 0x20
    cpu.eax = 32 /*0x20*/;
L_0x00498eaa:
    // 00498eaa  c6420720               -mov byte ptr [edx + 7], 0x20
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(7) /* 0x7 */) = 32 /*0x20*/;
    // 00498eae  c6420820               -mov byte ptr [edx + 8], 0x20
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(8) /* 0x8 */) = 32 /*0x20*/;
    // 00498eb2  c6420920               -mov byte ptr [edx + 9], 0x20
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(9) /* 0x9 */) = 32 /*0x20*/;
    // 00498eb6  884206                 -mov byte ptr [edx + 6], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(6) /* 0x6 */) = cpu.al;
    // 00498eb9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498eba  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00498ebb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_498f00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00498f00  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00498f01  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00498f02  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00498f03  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00498f04  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00498f05  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00498f06  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00498f08  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00498f0b  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00498f0e  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00498f13  be08000000             -mov esi, 8
    cpu.esi = 8 /*0x8*/;
    // 00498f18  8b3dd0f57900           -mov edi, dword ptr [0x79f5d0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992784) /* 0x79f5d0 */);
    // 00498f1e  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00498f20  8915a0e55500           -mov dword ptr [0x55e5a0], edx
    app->getMemory<x86::reg32>(x86::reg32(5629344) /* 0x55e5a0 */) = cpu.edx;
    // 00498f26  890da4e55500           -mov dword ptr [0x55e5a4], ecx
    app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */) = cpu.ecx;
    // 00498f2c  8915a8e55500           -mov dword ptr [0x55e5a8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629352) /* 0x55e5a8 */) = cpu.edx;
    // 00498f32  e859f1ffff             -call 0x498090
    cpu.esp -= 4;
    sub_498090(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00498f37  e9ab010000             -jmp 0x4990e7
    goto L_0x004990e7;
L_0x00498f3c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498f3c  83f81b                 +cmp eax, 0x1b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(27 /*0x1b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498f3f  0f8283010000           -jb 0x4990c8
    if (cpu.flags.cf)
    {
        goto L_0x004990c8;
    }
    // 00498f45  760c                   -jbe 0x498f53
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00498f53;
    }
    // 00498f47  3d00480000             +cmp eax, 0x4800
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18432 /*0x4800*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00498f4c  7415                   -je 0x498f63
    if (cpu.flags.zf)
    {
        goto L_0x00498f63;
    }
    // 00498f4e  e975010000             -jmp 0x4990c8
    goto L_0x004990c8;
L_0x00498f53:
    // 00498f53  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00498f58  8915a0e55500           -mov dword ptr [0x55e5a0], edx
    app->getMemory<x86::reg32>(x86::reg32(5629344) /* 0x55e5a0 */) = cpu.edx;
    // 00498f5e  e96a010000             -jmp 0x4990cd
    goto L_0x004990cd;
L_0x00498f63:
    // 00498f63  a1a4e55500             -mov eax, dword ptr [0x55e5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */);
    // 00498f68  8915a8e55500           -mov dword ptr [0x55e5a8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629352) /* 0x55e5a8 */) = cpu.edx;
    // 00498f6e  83f806                 +cmp eax, 6
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
    // 00498f71  771a                   -ja 0x498f8d
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00498f8d;
    }
    // 00498f73  ff2485bc8e4900         -jmp dword ptr [eax*4 + 0x498ebc]
    cpu.ip = app->getMemory<x86::reg32>(4820668 + cpu.eax * 4); goto dynamic_jump;
  case 0x00498f7a:
    // 00498f7a  b950914900             -mov ecx, 0x499150
    cpu.ecx = 4821328 /*0x499150*/;
    // 00498f7f  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 00498f84  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00498f86  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 00498f88  e86b550500             -call 0x4ee4f8
    cpu.esp -= 4;
    nfs3hp::watcom::qsort(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
  [[fallthrough]];
  case 0x00498f8d:
L_0x00498f8d:
    // 00498f8d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00498f92  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00498f94  a3b4f57900             -mov dword ptr [0x79f5b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.eax;
    // 00498f99  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00498f9b  891da0e55500           -mov dword ptr [0x55e5a0], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629344) /* 0x55e5a0 */) = cpu.ebx;
    // 00498fa1  e927010000             -jmp 0x4990cd
    goto L_0x004990cd;
L_0x00498fa6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498fa6  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00498fab  a1a4e55500             -mov eax, dword ptr [0x55e5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */);
    // 00498fb0  8915a8e55500           -mov dword ptr [0x55e5a8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629352) /* 0x55e5a8 */) = cpu.edx;
    // 00498fb6  83f806                 +cmp eax, 6
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
    // 00498fb9  771a                   -ja 0x498fd5
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00498fd5;
    }
    // 00498fbb  ff2485d88e4900         -jmp dword ptr [eax*4 + 0x498ed8]
    cpu.ip = app->getMemory<x86::reg32>(4820696 + cpu.eax * 4); goto dynamic_jump;
  case 0x00498fc2:
    // 00498fc2  b950914900             -mov ecx, 0x499150
    cpu.ecx = 4821328 /*0x499150*/;
    // 00498fc7  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 00498fcc  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00498fce  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 00498fd0  e823550500             -call 0x4ee4f8
    cpu.esp -= 4;
    nfs3hp::watcom::qsort(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
  [[fallthrough]];
  case 0x00498fd5:
L_0x00498fd5:
    // 00498fd5  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00498fd7  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00498fdc  890da0e55500           -mov dword ptr [0x55e5a0], ecx
    app->getMemory<x86::reg32>(x86::reg32(5629344) /* 0x55e5a0 */) = cpu.ecx;
    // 00498fe2  891db4f57900           -mov dword ptr [0x79f5b4], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.ebx;
    // 00498fe8  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 00498fea  e9de000000             -jmp 0x4990cd
    goto L_0x004990cd;
L_0x00498fef:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00498fef  a1a4e55500             -mov eax, dword ptr [0x55e5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */);
    // 00498ff4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00498ff6  7e0f                   -jle 0x499007
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00499007;
    }
    // 00498ff8  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00498ffd  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00498fff  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00499001  891da4e55500           -mov dword ptr [0x55e5a4], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */) = cpu.ebx;
L_0x00499007:
    // 00499007  833d68e5550002         +cmp dword ptr [0x55e568], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049900e  0f8eb4000000           -jle 0x4990c8
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004990c8;
    }
    // 00499014  8b1d68e55500           -mov ebx, dword ptr [0x55e568]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */);
    // 0049901a  a1a4e55500             -mov eax, dword ptr [0x55e5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */);
    // 0049901f  39d8                   +cmp eax, ebx
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
    // 00499021  0f8da1000000           -jge 0x4990c8
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004990c8;
    }
    // 00499027  83f801                 +cmp eax, 1
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
    // 0049902a  0f8e98000000           -jle 0x4990c8
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004990c8;
    }
    // 00499030  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00499035  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00499037  8b1db0f57900           -mov ebx, dword ptr [0x79f5b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */);
    // 0049903d  8915b4f57900           -mov dword ptr [0x79f5b4], edx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.edx;
    // 00499043  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00499045  29d3                   +sub ebx, edx
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00499047  a368e55500             -mov dword ptr [0x55e568], eax
    app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */) = cpu.eax;
    // 0049904c  891db0f57900           -mov dword ptr [0x79f5b0], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */) = cpu.ebx;
    // 00499052  e971000000             -jmp 0x4990c8
    goto L_0x004990c8;
L_0x00499057:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00499057  8b1db0f57900           -mov ebx, dword ptr [0x79f5b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */);
    // 0049905d  3b1da4e55500           +cmp ebx, dword ptr [0x55e5a4]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499063  7530                   -jne 0x499095
    if (!cpu.flags.zf)
    {
        goto L_0x00499095;
    }
    // 00499065  83fb08                 +cmp ebx, 8
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499068  7d2b                   -jge 0x499095
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00499095;
    }
    // 0049906a  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0049906f  a168e55500             -mov eax, dword ptr [0x55e568]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */);
    // 00499074  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00499076  8915b4f57900           -mov dword ptr [0x79f5b4], edx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.edx;
    // 0049907c  a368e55500             -mov dword ptr [0x55e568], eax
    app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */) = cpu.eax;
    // 00499081  a1a4e55500             -mov eax, dword ptr [0x55e5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */);
    // 00499086  01d3                   -add ebx, edx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edx));
    // 00499088  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0049908a  891db0f57900           -mov dword ptr [0x79f5b0], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */) = cpu.ebx;
    // 00499090  a3a4e55500             -mov dword ptr [0x55e5a4], eax
    app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */) = cpu.eax;
L_0x00499095:
    // 00499095  8b1db0f57900           -mov ebx, dword ptr [0x79f5b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992752) /* 0x79f5b0 */);
    // 0049909b  3b1da4e55500           +cmp ebx, dword ptr [0x55e5a4]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004990a1  7e25                   -jle 0x4990c8
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004990c8;
    }
    // 004990a3  a168e55500             -mov eax, dword ptr [0x55e568]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629288) /* 0x55e568 */);
    // 004990a8  83f802                 +cmp eax, 2
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
    // 004990ab  7e10                   -jle 0x4990bd
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004990bd;
    }
    // 004990ad  833da4e5550001         +cmp dword ptr [0x55e5a4], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004990b4  7507                   -jne 0x4990bd
    if (!cpu.flags.zf)
    {
        goto L_0x004990bd;
    }
    // 004990b6  a3a4e55500             -mov dword ptr [0x55e5a4], eax
    app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */) = cpu.eax;
    // 004990bb  eb06                   -jmp 0x4990c3
    goto L_0x004990c3;
L_0x004990bd:
    // 004990bd  ff05a4e55500           -inc dword ptr [0x55e5a4]
    (app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */))++;
L_0x004990c3:
    // 004990c3  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x004990c8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004990c8  83fa01                 +cmp edx, 1
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
    // 004990cb  7505                   -jne 0x4990d2
    if (!cpu.flags.zf)
    {
        goto L_0x004990d2;
    }
L_0x004990cd:
    // 004990cd  e8beefffff             -call 0x498090
    cpu.esp -= 4;
    sub_498090(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004990d2:
    // 004990d2  833db4f5790001         +cmp dword ptr [0x79f5b4], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004990d9  7508                   -jne 0x4990e3
    if (!cpu.flags.zf)
    {
        goto L_0x004990e3;
    }
    // 004990db  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004990de  e88df2ffff             -call 0x498370
    cpu.esp -= 4;
    sub_498370(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004990e3:
    // 004990e3  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004990e5  753b                   -jne 0x499122
    if (!cpu.flags.zf)
    {
        goto L_0x00499122;
    }
L_0x004990e7:
    // 004990e7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004990e9  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004990ee  8915b4f57900           -mov dword ptr [0x79f5b4], edx
    app->getMemory<x86::reg32>(x86::reg32(7992756) /* 0x79f5b4 */) = cpu.edx;
    // 004990f4  ff5018                 -call dword ptr [eax + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004990f7  3d004b0000             +cmp eax, 0x4b00
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(19200 /*0x4b00*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004990fc  0f823afeffff           -jb 0x498f3c
    if (cpu.flags.cf)
    {
        goto L_0x00498f3c;
    }
    // 00499102  0f86e7feffff           -jbe 0x498fef
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00498fef;
    }
    // 00499108  3d004d0000             +cmp eax, 0x4d00
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(19712 /*0x4d00*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049910d  72b9                   -jb 0x4990c8
    if (cpu.flags.cf)
    {
        goto L_0x004990c8;
    }
    // 0049910f  0f8642ffffff           -jbe 0x499057
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00499057;
    }
    // 00499115  3d00500000             +cmp eax, 0x5000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20480 /*0x5000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049911a  0f8486feffff           -je 0x498fa6
    if (cpu.flags.zf)
    {
        goto L_0x00498fa6;
    }
    // 00499120  eba6                   -jmp 0x4990c8
    goto L_0x004990c8;
L_0x00499122:
    // 00499122  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00499124  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499125  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499126  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499127  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499128  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499129  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049912a  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_499150(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00499150  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00499151  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00499152  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00499153  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00499154  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499155  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499157  83ec1c                 -sub esp, 0x1c
    (cpu.esp) -= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 0049915a  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0049915c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049915e  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00499161  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00499164  a1a4e55500             -mov eax, dword ptr [0x55e5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629348) /* 0x55e5a4 */);
    // 00499169  83f806                 +cmp eax, 6
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
    // 0049916c  0f8723010000           -ja 0x499295
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00499295;
    }
    // 00499172  c1e002                 +shl eax, 2
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
    // 00499175  ffa02c914900           -jmp dword ptr [eax + 0x49912c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + 4821292); goto dynamic_jump;
  case 0x0049917b:
    // 0049917b  c745f401000000         -mov dword ptr [ebp - 0xc], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = 1 /*0x1*/;
    // 00499182  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 00499184  8d4810                 -lea ecx, [eax + 0x10]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 00499187  e804030000             -call 0x499490
    cpu.esp -= 4;
    sub_499490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049918c  f6c440                 +test ah, 0x40
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 64 /*0x40*/));
    // 0049918f  7407                   -je 0x499198
    if (cpu.flags.zf)
    {
        goto L_0x00499198;
    }
    // 00499191  b9c8c15300             -mov ecx, 0x53c1c8
    cpu.ecx = 5489096 /*0x53c1c8*/;
    // 00499196  eb09                   -jmp 0x4991a1
    goto L_0x004991a1;
L_0x00499198:
    // 00499198  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049919a  e841840400             -call 0x4e15e0
    cpu.esp -= 4;
    nfs3hp::ea::memBlockName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049919f  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x004991a1:
    // 004991a1  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004991a3  8d5010                 -lea edx, [eax + 0x10]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004991a6  e8e5020000             -call 0x499490
    cpu.esp -= 4;
    sub_499490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004991ab  f6c440                 +test ah, 0x40
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 64 /*0x40*/));
    // 004991ae  7407                   -je 0x4991b7
    if (cpu.flags.zf)
    {
        goto L_0x004991b7;
    }
    // 004991b0  b8c8c15300             -mov eax, 0x53c1c8
    cpu.eax = 5489096 /*0x53c1c8*/;
    // 004991b5  eb07                   -jmp 0x4991be
    goto L_0x004991be;
L_0x004991b7:
    // 004991b7  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004991b9  e822840400             -call 0x4e15e0
    cpu.esp -= 4;
    nfs3hp::ea::memBlockName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004991be:
    // 004991be  833da8e5550000         +cmp dword ptr [0x55e5a8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629352) /* 0x55e5a8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004991c5  7404                   -je 0x4991cb
    if (cpu.flags.zf)
    {
        goto L_0x004991cb;
    }
    // 004991c7  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004991c9  eb02                   -jmp 0x4991cd
    goto L_0x004991cd;
L_0x004991cb:
    // 004991cb  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
L_0x004991cd:
    // 004991cd  8955e8                 -mov dword ptr [ebp - 0x18], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.edx;
    // 004991d0  833da8e5550000         +cmp dword ptr [0x55e5a8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629352) /* 0x55e5a8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004991d7  7502                   -jne 0x4991db
    if (!cpu.flags.zf)
    {
        goto L_0x004991db;
    }
    // 004991d9  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
L_0x004991db:
    // 004991db  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
    // 004991de  e9b6000000             -jmp 0x499299
    goto L_0x00499299;
  case 0x004991e3:
    // 004991e3  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 004991e5  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004991ea  e8a1020000             -call 0x499490
    cpu.esp -= 4;
    sub_499490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004991ef  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 004991f1  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 004991f4  e867020000             -call 0x499460
    cpu.esp -= 4;
    sub_499460(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004991f9  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004991fb  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004991fd  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 004991ff  e87c020000             -call 0x499480
    cpu.esp -= 4;
    sub_499480(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499204  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00499207  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 00499209  e882020000             -call 0x499490
    cpu.esp -= 4;
    sub_499490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049920e  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 00499210  e84b020000             -call 0x499460
    cpu.esp -= 4;
    sub_499460(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499215  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00499217  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0049921a  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 0049921c  e85f020000             -call 0x499480
    cpu.esp -= 4;
    sub_499480(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499221  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00499223  833da8e5550000         +cmp dword ptr [0x55e5a8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629352) /* 0x55e5a8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049922a  7502                   -jne 0x49922e
    if (!cpu.flags.zf)
    {
        goto L_0x0049922e;
    }
    // 0049922c  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
L_0x0049922e:
    // 0049922e  833da8e5550000         +cmp dword ptr [0x55e5a8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629352) /* 0x55e5a8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499235  7462                   -je 0x499299
    if (cpu.flags.zf)
    {
        goto L_0x00499299;
    }
    // 00499237  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049923a  eb5d                   -jmp 0x499299
    goto L_0x00499299;
  case 0x0049923c:
    // 0049923c  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 0049923e  e84d020000             -call 0x499490
    cpu.esp -= 4;
    sub_499490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499243  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 00499245  e836020000             -call 0x499480
    cpu.esp -= 4;
    sub_499480(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049924a  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049924c  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 0049924f  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 00499251  e80a020000             -call 0x499460
    cpu.esp -= 4;
    sub_499460(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499256  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00499259  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 0049925b  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00499260  e82b020000             -call 0x499490
    cpu.esp -= 4;
    sub_499490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499265  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 00499267  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 0049926a  e811020000             -call 0x499480
    cpu.esp -= 4;
    sub_499480(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049926f  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00499271  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00499273  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 00499275  e8e6010000             -call 0x499460
    cpu.esp -= 4;
    sub_499460(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049927a  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0049927c  833da8e5550000         +cmp dword ptr [0x55e5a8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629352) /* 0x55e5a8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499283  7502                   -jne 0x499287
    if (!cpu.flags.zf)
    {
        goto L_0x00499287;
    }
    // 00499285  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
L_0x00499287:
    // 00499287  833da8e5550000         +cmp dword ptr [0x55e5a8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629352) /* 0x55e5a8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049928e  7509                   -jne 0x499299
    if (!cpu.flags.zf)
    {
        goto L_0x00499299;
    }
    // 00499290  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00499293  eb04                   -jmp 0x499299
    goto L_0x00499299;
  case 0x00499295:
L_0x00499295:
    // 00499295  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00499297  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
  [[fallthrough]];
  case 0x00499299:
L_0x00499299:
    // 00499299  837df400               +cmp dword ptr [ebp - 0xc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049929d  7532                   -jne 0x4992d1
    if (!cpu.flags.zf)
    {
        goto L_0x004992d1;
    }
    // 0049929f  39de                   +cmp esi, ebx
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
    // 004992a1  7e07                   -jle 0x4992aa
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004992aa;
    }
    // 004992a3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004992a8  eb32                   -jmp 0x4992dc
    goto L_0x004992dc;
L_0x004992aa:
    // 004992aa  751e                   -jne 0x4992ca
    if (!cpu.flags.zf)
    {
        goto L_0x004992ca;
    }
    // 004992ac  837dfc00               +cmp dword ptr [ebp - 4], 0
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
    // 004992b0  7504                   -jne 0x4992b6
    if (!cpu.flags.zf)
    {
        goto L_0x004992b6;
    }
    // 004992b2  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004992b4  eb26                   -jmp 0x4992dc
    goto L_0x004992dc;
L_0x004992b6:
    // 004992b6  8b75f8                 -mov esi, dword ptr [ebp - 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004992b9  39f7                   +cmp edi, esi
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
    // 004992bb  7d07                   -jge 0x4992c4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004992c4;
    }
    // 004992bd  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004992c2  eb18                   -jmp 0x4992dc
    goto L_0x004992dc;
L_0x004992c4:
    // 004992c4  7504                   -jne 0x4992ca
    if (!cpu.flags.zf)
    {
        goto L_0x004992ca;
    }
    // 004992c6  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004992c8  eb12                   -jmp 0x4992dc
    goto L_0x004992dc;
L_0x004992ca:
    // 004992ca  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 004992cf  eb0b                   -jmp 0x4992dc
    goto L_0x004992dc;
L_0x004992d1:
    // 004992d1  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004992d4  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004992d7  e834500500             -call 0x4ee310
    cpu.esp -= 4;
    nfs3hp::watcom::strcmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004992dc:
    // 004992dc  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004992de  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004992df  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004992e0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004992e1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004992e2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004992e3  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* sub_4992f0: native, nfs3hp::dead::stub_0x4992f0 */

/* sub_499300: native, nfs3hp::dead::stub_0x499300 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_499330(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00499330  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00499331  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00499332  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499333  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499335  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00499338  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0049933a  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0049933d  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 0049933f  894df8                 -mov dword ptr [ebp - 8], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ecx;
    // 00499342  e869000000             -call 0x4993b0
    cpu.esp -= 4;
    sub_4993b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499347  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0049934a  a15ce55500             -mov eax, dword ptr [0x55e55c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629276) /* 0x55e55c */);
    // 0049934f  8b0485c0f59e00         -mov eax, dword ptr [eax*4 + 0x9ef5c0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10417600) /* 0x9ef5c0 */ + cpu.eax * 4);
    // 00499356  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00499358  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 0049935b  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049935d  2b45f4                 -sub eax, dword ptr [ebp - 0xc]
    (cpu.eax) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 00499360  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00499362  8907                   -mov dword ptr [edi], eax
    app->getMemory<x86::reg32>(cpu.edi) = cpu.eax;
    // 00499364  e847000000             -call 0x4993b0
    cpu.esp -= 4;
    sub_4993b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00499369:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00499369  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049936b  7427                   -je 0x499394
    if (cpu.flags.zf)
    {
        goto L_0x00499394;
    }
    // 0049936d  f6400340               +test byte ptr [eax + 3], 0x40
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(3) /* 0x3 */) & 64 /*0x40*/));
    // 00499371  740d                   -je 0x499380
    if (cpu.flags.zf)
    {
        goto L_0x00499380;
    }
    // 00499373  8b7804                 -mov edi, dword ptr [eax + 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00499376  01f9                   -add ecx, edi
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edi));
    // 00499378  39fa                   +cmp edx, edi
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
    // 0049937a  7d13                   -jge 0x49938f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049938f;
    }
    // 0049937c  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0049937e  eb0f                   -jmp 0x49938f
    goto L_0x0049938f;
L_0x00499380:
    // 00499380  f6400340               +test byte ptr [eax + 3], 0x40
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(3) /* 0x3 */) & 64 /*0x40*/));
    // 00499384  7509                   -jne 0x49938f
    if (!cpu.flags.zf)
    {
        goto L_0x0049938f;
    }
    // 00499386  f6400380               +test byte ptr [eax + 3], 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(3) /* 0x3 */) & 128 /*0x80*/));
    // 0049938a  7503                   -jne 0x49938f
    if (!cpu.flags.zf)
    {
        goto L_0x0049938f;
    }
    // 0049938c  035804                 +add ebx, dword ptr [eax + 4]
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */)));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
L_0x0049938f:
    // 0049938f  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00499392  ebd5                   -jmp 0x499369
    goto L_0x00499369;
L_0x00499394:
    // 00499394  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00499397  8918                   -mov dword ptr [eax], ebx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.ebx;
    // 00499399  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049939c  890e                   -mov dword ptr [esi], ecx
    app->getMemory<x86::reg32>(cpu.esi) = cpu.ecx;
    // 0049939e  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 004993a0  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004993a2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004993a3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004993a4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004993a5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4993b0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004993b0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004993b1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004993b3  a15ce55500             -mov eax, dword ptr [0x55e55c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629276) /* 0x55e55c */);
    // 004993b8  8b0485c0f59e00         -mov eax, dword ptr [eax*4 + 0x9ef5c0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10417600) /* 0x9ef5c0 */ + cpu.eax * 4);
    // 004993bf  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004993c2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004993c3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4993d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004993d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004993d1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004993d2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004993d3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004993d5  e8d6ffffff             -call 0x4993b0
    cpu.esp -= 4;
    sub_4993b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004993da  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004993dc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004993dc  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004993de  7417                   -je 0x4993f7
    if (cpu.flags.zf)
    {
        goto L_0x004993f7;
    }
    // 004993e0  668b18                 -mov bx, word ptr [eax]
    cpu.bx = app->getMemory<x86::reg16>(cpu.eax);
    // 004993e3  6681fb4d42             +cmp bx, 0x424d
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(16973 /*0x424d*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004993e8  7407                   -je 0x4993f1
    if (cpu.flags.zf)
    {
        goto L_0x004993f1;
    }
    // 004993ea  6681fb4642             +cmp bx, 0x4246
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(16966 /*0x4246*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004993ef  7501                   -jne 0x4993f2
    if (!cpu.flags.zf)
    {
        goto L_0x004993f2;
    }
L_0x004993f1:
    // 004993f1  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
L_0x004993f2:
    // 004993f2  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004993f5  ebe5                   -jmp 0x4993dc
    goto L_0x004993dc;
L_0x004993f7:
    // 004993f7  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004993f9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004993fa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004993fb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004993fc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_499400(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00499400  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00499401  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499402  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499404  8b35a0f57900           -mov esi, dword ptr [0x79f5a0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992736) /* 0x79f5a0 */);
    // 0049940a  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0049940c  7409                   -je 0x499417
    if (cpu.flags.zf)
    {
        goto L_0x00499417;
    }
    // 0049940e  833d94e5550000         +cmp dword ptr [0x55e594], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629332) /* 0x55e594 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499415  7416                   -je 0x49942d
    if (cpu.flags.zf)
    {
        goto L_0x0049942d;
    }
L_0x00499417:
    // 00499417  8935a0f57900           -mov dword ptr [0x79f5a0], esi
    app->getMemory<x86::reg32>(x86::reg32(7992736) /* 0x79f5a0 */) = cpu.esi;
    // 0049941d  e88effffff             -call 0x4993b0
    cpu.esp -= 4;
    sub_4993b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499422  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00499424  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00499426  a394e55500             -mov dword ptr [0x55e594], eax
    app->getMemory<x86::reg32>(x86::reg32(5629332) /* 0x55e594 */) = cpu.eax;
    // 0049942b  eb03                   -jmp 0x499430
    goto L_0x00499430;
L_0x0049942d:
    // 0049942d  8b7608                 -mov esi, dword ptr [esi + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
L_0x00499430:
    // 00499430  8932                   -mov dword ptr [edx], esi
    app->getMemory<x86::reg32>(cpu.edx) = cpu.esi;
    // 00499432  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00499434  668b5602               -mov dx, word ptr [esi + 2]
    cpu.dx = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 00499438  8911                   -mov dword ptr [ecx], edx
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.edx;
    // 0049943a  8b550c                 -mov edx, dword ptr [ebp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 0049943d  c70200000000           -mov dword ptr [edx], 0
    app->getMemory<x86::reg32>(cpu.edx) = 0 /*0x0*/;
    // 00499443  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00499446  8903                   -mov dword ptr [ebx], eax
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.eax;
    // 00499448  8935a0f57900           -mov dword ptr [0x79f5a0], esi
    app->getMemory<x86::reg32>(x86::reg32(7992736) /* 0x79f5a0 */) = cpu.esi;
    // 0049944e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049944f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499450  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_499460(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00499460  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00499461  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499462  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499464  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00499466  750e                   -jne 0x499476
    if (!cpu.flags.zf)
    {
        goto L_0x00499476;
    }
    // 00499468  8b15c4f57900           -mov edx, dword ptr [0x79f5c4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049946e  a158e55500             -mov eax, dword ptr [0x55e558]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629272) /* 0x55e558 */);
    // 00499473  8b04c2                 -mov eax, dword ptr [edx + eax*8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 8);
L_0x00499476:
    // 00499476  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00499479  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049947a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049947b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_499480(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00499480  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499481  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499483  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00499486  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499487  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_499490(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00499490  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499491  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499493  668b4002               -mov ax, word ptr [eax + 2]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(2) /* 0x2 */);
    // 00499497  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 0049949c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049949d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4994a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004994a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004994a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004994a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004994a3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004994a4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004994a5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004994a7  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004994aa  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004994ac  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 004994b1  8b04c8                 -mov eax, dword ptr [eax + ecx*8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + cpu.ecx * 8);
    // 004994b4  e8d7ffffff             -call 0x499490
    cpu.esp -= 4;
    sub_499490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004994b9  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004994bb  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004994be  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 004994c3  8b04c8                 -mov eax, dword ptr [eax + ecx*8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + cpu.ecx * 8);
    // 004994c6  e895ffffff             -call 0x499460
    cpu.esp -= 4;
    sub_499460(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004994cb  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004994cd  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 004994d2  8b04c8                 -mov eax, dword ptr [eax + ecx*8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + cpu.ecx * 8);
    // 004994d5  e8a6000000             -call 0x499580
    cpu.esp -= 4;
    sub_499580(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004994da  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004994dc  7407                   -je 0x4994e5
    if (cpu.flags.zf)
    {
        goto L_0x004994e5;
    }
    // 004994de  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004994e0  e992000000             -jmp 0x499577
    goto L_0x00499577;
L_0x004994e5:
    // 004994e5  f6c780                 +test bh, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bh & 128 /*0x80*/));
    // 004994e8  740d                   -je 0x4994f7
    if (cpu.flags.zf)
    {
        goto L_0x004994f7;
    }
    // 004994ea  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004994ef  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004994f1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004994f2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004994f3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004994f4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004994f5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004994f6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004994f7:
    // 004994f7  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004994f9  750d                   -jne 0x499508
    if (!cpu.flags.zf)
    {
        goto L_0x00499508;
    }
    // 004994fb  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00499500  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00499502  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499503  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499504  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499505  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499506  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499507  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00499508:
    // 00499508  30e4                   -xor ah, ah
    cpu.ah ^= x86::reg8(x86::sreg8(cpu.ah));
    // 0049950a  8865f8                 -mov byte ptr [ebp - 8], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ah;
    // 0049950d  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0049950f  eb05                   -jmp 0x499516
    goto L_0x00499516;
L_0x00499511:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00499511  83f804                 +cmp eax, 4
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
    // 00499514  7d17                   -jge 0x49952d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049952d;
    }
L_0x00499516:
    // 00499516  8b15c4f57900           -mov edx, dword ptr [0x79f5c4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049951c  8b14ca                 -mov edx, dword ptr [edx + ecx*8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + cpu.ecx * 8);
    // 0049951f  8a540210               -mov dl, byte ptr [edx + eax + 0x10]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(16) /* 0x10 */ + cpu.eax * 1);
    // 00499523  40                     -inc eax
    (cpu.eax)++;
    // 00499524  80e27f                 +and dl, 0x7f
    cpu.clear_co();
    cpu.set_szp((cpu.dl &= x86::reg8(x86::sreg8(127 /*0x7f*/))));
    // 00499527  885428f3               -mov byte ptr [eax + ebp - 0xd], dl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-13) /* -0xd */ + cpu.ebp * 1) = cpu.dl;
    // 0049952b  ebe4                   -jmp 0x499511
    goto L_0x00499511;
L_0x0049952d:
    // 0049952d  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 00499532  8d45f4                 -lea eax, [ebp - 0xc]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00499535  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00499537  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00499539  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0049953b  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00499542  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00499544  3d49504853             +cmp eax, 0x53485049
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1397248073 /*0x53485049*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499549  7427                   -je 0x499572
    if (cpu.flags.zf)
    {
        goto L_0x00499572;
    }
    // 0049954b  3d46504853             +cmp eax, 0x53485046
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1397248070 /*0x53485046*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499550  7420                   -je 0x499572
    if (cpu.flags.zf)
    {
        goto L_0x00499572;
    }
    // 00499552  f645fc10               +test byte ptr [ebp - 4], 0x10
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-4) /* -0x4 */) & 16 /*0x10*/));
    // 00499556  740d                   -je 0x499565
    if (cpu.flags.zf)
    {
        goto L_0x00499565;
    }
    // 00499558  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 0049955d  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049955f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499560  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499561  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499562  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499563  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499564  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00499565:
    // 00499565  b805000000             -mov eax, 5
    cpu.eax = 5 /*0x5*/;
    // 0049956a  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049956c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049956d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049956e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049956f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499570  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499571  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00499572:
    // 00499572  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
L_0x00499577:
    // 00499577  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00499579  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049957a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049957b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049957c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049957d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049957e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_499580(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00499580  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499581  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499583  6681384642             +cmp word ptr [eax], 0x4246
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(16966 /*0x4246*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00499588  7404                   -je 0x49958e
    if (cpu.flags.zf)
    {
        goto L_0x0049958e;
    }
    // 0049958a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049958c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049958d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049958e:
    // 0049958e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00499593  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499594  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4995b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 004995b0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004995b1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004995b2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004995b3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004995b4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004995b5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004995b6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004995b8  83ec4c                 -sub esp, 0x4c
    (cpu.esp) -= x86::reg32(x86::sreg32(76 /*0x4c*/));
    // 004995bb  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004995bd  8955c4                 -mov dword ptr [ebp - 0x3c], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.edx;
    // 004995c0  8955c8                 -mov dword ptr [ebp - 0x38], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.edx;
    // 004995c3  8955cc                 -mov dword ptr [ebp - 0x34], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.edx;
    // 004995c6  8955d0                 -mov dword ptr [ebp - 0x30], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.edx;
    // 004995c9  8955f0                 -mov dword ptr [ebp - 0x10], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edx;
    // 004995cc  e8dfb00500             -call 0x4f46b0
    cpu.esp -= 4;
    sub_4f46b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004995d1  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004995d4  30e4                   -xor ah, ah
    cpu.ah ^= x86::reg8(x86::sreg8(cpu.ah));
    // 004995d6  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004995d9  8865d8                 -mov byte ptr [ebp - 0x28], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.ah;
    // 004995dc  8865c0                 -mov byte ptr [ebp - 0x40], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.ah;
    // 004995df  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004995e1  eb05                   -jmp 0x4995e8
    goto L_0x004995e8;
L_0x004995e3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004995e3  83f804                 +cmp eax, 4
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
    // 004995e6  7d18                   -jge 0x499600
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00499600;
    }
L_0x004995e8:
    // 004995e8  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004995eb  8a1401                 -mov dl, byte ptr [ecx + eax]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ecx + cpu.eax * 1);
    // 004995ee  40                     -inc eax
    (cpu.eax)++;
    // 004995ef  80e27f                 +and dl, 0x7f
    cpu.clear_co();
    cpu.set_szp((cpu.dl &= x86::reg8(x86::sreg8(127 /*0x7f*/))));
    // 004995f2  885428d3               -mov byte ptr [eax + ebp - 0x2d], dl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-45) /* -0x2d */ + cpu.ebp * 1) = cpu.dl;
    // 004995f6  8a5401ff               -mov dl, byte ptr [ecx + eax - 1]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(-1) /* -0x1 */ + cpu.eax * 1);
    // 004995fa  885428bb               -mov byte ptr [eax + ebp - 0x45], dl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-69) /* -0x45 */ + cpu.ebp * 1) = cpu.dl;
    // 004995fe  ebe3                   -jmp 0x4995e3
    goto L_0x004995e3;
L_0x00499600:
    // 00499600  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 00499605  8d45d4                 -lea eax, [ebp - 0x2c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 00499608  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 0049960a  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 0049960c  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0049960e  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00499615  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00499617  3d49504853             +cmp eax, 0x53485049
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1397248073 /*0x53485049*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049961c  740b                   -je 0x499629
    if (cpu.flags.zf)
    {
        goto L_0x00499629;
    }
    // 0049961e  3d46504853             +cmp eax, 0x53485046
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1397248070 /*0x53485046*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499623  0f85c4040000           -jne 0x499aed
    if (!cpu.flags.zf)
    {
        goto L_0x00499aed;
    }
L_0x00499629:
    // 00499629  8d45d0                 -lea eax, [ebp - 0x30]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0049962c  8d4dcc                 -lea ecx, [ebp - 0x34]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 0049962f  8d5dc8                 -lea ebx, [ebp - 0x38]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 00499632  8d55c4                 -lea edx, [ebp - 0x3c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00499635  be60c25300             -mov esi, 0x53c260
    cpu.esi = 5489248 /*0x53c260*/;
    // 0049963a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049963b  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049963e  8975d8                 -mov dword ptr [ebp - 0x28], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.esi;
    // 00499641  e82af5ffff             -call 0x498b70
    cpu.esp -= 4;
    sub_498b70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499646  bad4c15300             -mov edx, 0x53c1d4
    cpu.edx = 5489108 /*0x53c1d4*/;
    // 0049964b  b900c25300             -mov ecx, 0x53c200
    cpu.ecx = 5489152 /*0x53c200*/;
    // 00499650  bb38c25300             -mov ebx, 0x53c238
    cpu.ebx = 5489208 /*0x53c238*/;
    // 00499655  8955e8                 -mov dword ptr [ebp - 0x18], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.edx;
    // 00499658  894de4                 -mov dword ptr [ebp - 0x1c], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.ecx;
    // 0049965b  895ddc                 -mov dword ptr [ebp - 0x24], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.ebx;
L_0x0049965e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049965e  837df000               +cmp dword ptr [ebp - 0x10], 0
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
    // 00499662  0f859b040000           -jne 0x499b03
    if (!cpu.flags.zf)
    {
        goto L_0x00499b03;
    }
    // 00499668  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049966b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0049966d  8b4dc4                 -mov ecx, dword ptr [ebp - 0x3c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00499670  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00499673  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00499675  8d5db4                 -lea ebx, [ebp - 0x4c]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 00499678  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049967b  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0049967e  e8e20f0500             -call 0x4ea665
    cpu.esp -= 4;
    sub_4ea665(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499683  8d55b4                 -lea edx, [ebp - 0x4c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 00499686  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00499689  e8525b0500             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049968e  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 00499690  83e27f                 -and edx, 0x7f
    cpu.edx &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 00499693  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00499695  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 00499698  83fa7c                 +cmp edx, 0x7c
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(124 /*0x7c*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049969b  740a                   -je 0x4996a7
    if (cpu.flags.zf)
    {
        goto L_0x004996a7;
    }
    // 0049969d  83fa6e                 +cmp edx, 0x6e
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(110 /*0x6e*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004996a0  7405                   -je 0x4996a7
    if (cpu.flags.zf)
    {
        goto L_0x004996a7;
    }
    // 004996a2  83fa6f                 +cmp edx, 0x6f
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(111 /*0x6f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004996a5  7507                   -jne 0x4996ae
    if (!cpu.flags.zf)
    {
        goto L_0x004996ae;
    }
L_0x004996a7:
    // 004996a7  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 004996ac  eb23                   -jmp 0x4996d1
    goto L_0x004996d1;
L_0x004996ae:
    // 004996ae  83e270                 -and edx, 0x70
    cpu.edx &= x86::reg32(x86::sreg32(112 /*0x70*/));
    // 004996b1  83fa20                 +cmp edx, 0x20
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(32 /*0x20*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004996b4  7507                   -jne 0x4996bd
    if (!cpu.flags.zf)
    {
        goto L_0x004996bd;
    }
    // 004996b6  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 004996bb  eb14                   -jmp 0x4996d1
    goto L_0x004996d1;
L_0x004996bd:
    // 004996bd  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004996bf  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004996c4  a880                   +test al, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 128 /*0x80*/));
    // 004996c6  7407                   -je 0x4996cf
    if (cpu.flags.zf)
    {
        goto L_0x004996cf;
    }
    // 004996c8  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004996cd  eb02                   -jmp 0x4996d1
    goto L_0x004996d1;
L_0x004996cf:
    // 004996cf  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004996d1:
    // 004996d1  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004996d3  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004996d5  83e07f                 -and eax, 0x7f
    cpu.eax &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 004996d8  8a80546d5600           -mov al, byte ptr [eax + 0x566d54]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5664084) /* 0x566d54 */);
    // 004996de  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 004996e1  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
    // 004996e4  83fa03                 +cmp edx, 3
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
    // 004996e7  0f8794030000           -ja 0x499a81
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00499a81;
    }
    // 004996ed  ff249598954900         -jmp dword ptr [edx*4 + 0x499598]
    cpu.ip = app->getMemory<x86::reg32>(4822424 + cpu.edx * 4); goto dynamic_jump;
  case 0x004996f4:
    // 004996f4  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004996f9  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004996fb  8b15a4f57900           -mov edx, dword ptr [0x79f5a4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499701  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00499706  ff5208                 -call dword ptr [edx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499709  8b5602                 -mov edx, dword ptr [esi + 2]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 0049970c  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0049970e  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00499711  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 00499716  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00499718  8b15a4f57900           -mov edx, dword ptr [0x79f5a4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 0049971e  d1fb                   -sar ebx, 1
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (1 /*0x1*/ % 32));
    // 00499720  ff5208                 -call dword ptr [edx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499723  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499729  8b5604                 -mov edx, dword ptr [esi + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0049972c  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0049972e  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00499731  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00499733  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00499735  8b55e0                 -mov edx, dword ptr [ebp - 0x20]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00499738  d1f9                   -sar ecx, 1
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (1 /*0x1*/ % 32));
    // 0049973a  ff5714                 -call dword ptr [edi + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049973d  6a0e                   -push 0xe
    app->getMemory<x86::reg32>(cpu.esp-4) = 14 /*0xe*/;
    cpu.esp -= 4;
    // 0049973f  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00499744  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 0049974a  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499750  8b159cf57900           -mov edx, dword ptr [0x79f59c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499756  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00499758  ff5720                 -call dword ptr [edi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049975b  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499761  ba0e000000             -mov edx, 0xe
    cpu.edx = 14 /*0xe*/;
    // 00499766  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 0049976b  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049976e  8b4608                 -mov eax, dword ptr [esi + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 00499771  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00499774  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499775  8b4606                 -mov eax, dword ptr [esi + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 00499778  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049977b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049977c  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 0049977f  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 00499782  c1f814                 -sar eax, 0x14
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (20 /*0x14*/ % 32));
    // 00499785  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499786  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 00499789  c1e014                 -shl eax, 0x14
    cpu.eax <<= 20 /*0x14*/ % 32;
    // 0049978c  c1f814                 -sar eax, 0x14
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (20 /*0x14*/ % 32));
    // 0049978f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499790  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00499792  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00499797  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499798  8b5de0                 -mov ebx, dword ptr [ebp - 0x20]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 0049979b  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0049979e  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049979f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004997a2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004997a3  8b4602                 -mov eax, dword ptr [esi + 2]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 004997a6  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004997a9  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004997af  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004997b0  8d45b4                 -lea eax, [ebp - 0x4c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 004997b3  8b35c0f57900           -mov esi, dword ptr [0x79f5c0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 004997b9  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004997ba  83fe46                 +cmp esi, 0x46
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(70 /*0x46*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004997bd  7d05                   -jge 0x4997c4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004997c4;
    }
    // 004997bf  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004997c2  eb03                   -jmp 0x4997c7
    goto L_0x004997c7;
L_0x004997c4:
    // 004997c4  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
L_0x004997c7:
    // 004997c7  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004997c8  8b3d9cf57900           -mov edi, dword ptr [0x79f59c]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 004997ce  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004997cf  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004997d1  ff510c                 -call dword ptr [ecx + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004997d4  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004997d9  83c430                 +add esp, 0x30
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(48 /*0x30*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004997dc  e99d020000             -jmp 0x499a7e
    goto L_0x00499a7e;
  case 0x004997e1:
    // 004997e1  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004997e6  8b3dc0f57900           -mov edi, dword ptr [0x79f5c0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 004997ec  8b1d9cf57900           -mov ebx, dword ptr [0x79f59c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 004997f2  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004997f4  b8000000ff             -mov eax, 0xff000000
    cpu.eax = 4278190080 /*0xff000000*/;
    // 004997f9  e8625f0500             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004997fe  e8ad590500             -call 0x4ef1b0
    cpu.esp -= 4;
    sub_4ef1b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499803  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499809  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0049980e  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00499813  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499816  6894c25300             -push 0x53c294
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489300 /*0x53c294*/;
    cpu.esp -= 4;
    // 0049981b  d1fb                   -sar ebx, 1
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (1 /*0x1*/ % 32));
    // 0049981d  83ef1c                 -sub edi, 0x1c
    (cpu.edi) -= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 00499820  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00499821  d1ff                   -sar edi, 1
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (1 /*0x1*/ % 32));
    // 00499823  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499828  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00499829  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0049982e  ff500c                 -call dword ptr [eax + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499831  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00499834  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 0049983a  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499840  6a0e                   -push 0xe
    app->getMemory<x86::reg32>(cpu.esp-4) = 14 /*0xe*/;
    cpu.esp -= 4;
    // 00499842  8b159cf57900           -mov edx, dword ptr [0x79f59c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499848  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049984a  ff5720                 -call dword ptr [edi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049984d  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499853  ba0e000000             -mov edx, 0xe
    cpu.edx = 14 /*0xe*/;
    // 00499858  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 0049985d  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499860  8b4608                 -mov eax, dword ptr [esi + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 00499863  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00499866  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499867  8b4606                 -mov eax, dword ptr [esi + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 0049986a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049986d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049986e  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 00499871  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 00499874  c1f814                 -sar eax, 0x14
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (20 /*0x14*/ % 32));
    // 00499877  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499878  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 0049987b  c1e014                 -shl eax, 0x14
    cpu.eax <<= 20 /*0x14*/ % 32;
    // 0049987e  c1f814                 -sar eax, 0x14
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (20 /*0x14*/ % 32));
    // 00499881  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499882  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00499884  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00499889  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049988a  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049988d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049988e  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00499891  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00499894  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499895  8b4602                 -mov eax, dword ptr [esi + 2]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 00499898  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049989b  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004998a1  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004998a2  8d45b4                 -lea eax, [ebp - 0x4c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 004998a5  8b15c0f57900           -mov edx, dword ptr [0x79f5c0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 004998ab  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004998ac  83fa46                 +cmp edx, 0x46
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(70 /*0x46*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004998af  7d05                   -jge 0x4998b6
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004998b6;
    }
    // 004998b1  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004998b4  eb03                   -jmp 0x4998b9
    goto L_0x004998b9;
L_0x004998b6:
    // 004998b6  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
L_0x004998b9:
    // 004998b9  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004998ba  8b1d9cf57900           -mov ebx, dword ptr [0x79f59c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 004998c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004998c1  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004998c3  ff510c                 -call dword ptr [ecx + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004998c6  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004998cb  83c430                 +add esp, 0x30
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(48 /*0x30*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004998ce  e9ab010000             -jmp 0x499a7e
    goto L_0x00499a7e;
  case 0x004998d3:
    // 004998d3  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004998d8  8b3dc0f57900           -mov edi, dword ptr [0x79f5c0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 004998de  8b0d9cf57900           -mov ecx, dword ptr [0x79f59c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 004998e4  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004998e6  b8000000ff             -mov eax, 0xff000000
    cpu.eax = 4278190080 /*0xff000000*/;
    // 004998eb  e8705e0500             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004998f0  e8bb580500             -call 0x4ef1b0
    cpu.esp -= 4;
    sub_4ef1b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004998f5  8b1da4f57900           -mov ebx, dword ptr [0x79f5a4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004998fb  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00499900  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00499905  ff5324                 -call dword ptr [ebx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499908  68b4c25300             -push 0x53c2b4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489332 /*0x53c2b4*/;
    cpu.esp -= 4;
    // 0049990d  d1f9                   -sar ecx, 1
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (1 /*0x1*/ % 32));
    // 0049990f  83ef1d                 -sub edi, 0x1d
    (cpu.edi) -= x86::reg32(x86::sreg32(29 /*0x1d*/));
    // 00499912  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00499913  d1ff                   -sar edi, 1
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (1 /*0x1*/ % 32));
    // 00499915  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 0049991a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049991b  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00499920  ff500c                 -call dword ptr [eax + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499923  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00499926  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 0049992c  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499932  6a0e                   -push 0xe
    app->getMemory<x86::reg32>(cpu.esp-4) = 14 /*0xe*/;
    cpu.esp -= 4;
    // 00499934  8b159cf57900           -mov edx, dword ptr [0x79f59c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 0049993a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049993c  ff5720                 -call dword ptr [edi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049993f  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499945  ba0e000000             -mov edx, 0xe
    cpu.edx = 14 /*0xe*/;
    // 0049994a  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 0049994f  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499952  8b4608                 -mov eax, dword ptr [esi + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 00499955  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00499958  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499959  8b4606                 -mov eax, dword ptr [esi + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 0049995c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049995f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499960  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00499962  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00499967  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499968  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0049996b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049996e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049996f  8b4602                 -mov eax, dword ptr [esi + 2]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 00499972  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00499975  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 0049997b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049997c  8d45b4                 -lea eax, [ebp - 0x4c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 0049997f  8b35c0f57900           -mov esi, dword ptr [0x79f5c0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499985  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499986  83fe46                 +cmp esi, 0x46
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(70 /*0x46*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499989  7d05                   -jge 0x499990
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00499990;
    }
    // 0049998b  8b55dc                 -mov edx, dword ptr [ebp - 0x24]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0049998e  eb03                   -jmp 0x499993
    goto L_0x00499993;
L_0x00499990:
    // 00499990  8b55d8                 -mov edx, dword ptr [ebp - 0x28]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
L_0x00499993:
    // 00499993  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00499994  8b3d9cf57900           -mov edi, dword ptr [0x79f59c]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 0049999a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049999b  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0049999d  ff510c                 -call dword ptr [ecx + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004999a0  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004999a5  83c424                 +add esp, 0x24
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(36 /*0x24*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004999a8  e9d1000000             -jmp 0x499a7e
    goto L_0x00499a7e;
  case 0x004999ad:
    // 004999ad  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004999b2  8b0dc0f57900           -mov ecx, dword ptr [0x79f5c0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 004999b8  8b1d9cf57900           -mov ebx, dword ptr [0x79f59c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 004999be  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004999c0  b8000000ff             -mov eax, 0xff000000
    cpu.eax = 4278190080 /*0xff000000*/;
    // 004999c5  e8965d0500             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004999ca  e8e1570500             -call 0x4ef1b0
    cpu.esp -= 4;
    sub_4ef1b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004999cf  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004999d5  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004999da  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 004999df  ff5624                 -call dword ptr [esi + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004999e2  68d4c25300             -push 0x53c2d4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489364 /*0x53c2d4*/;
    cpu.esp -= 4;
    // 004999e7  d1fb                   -sar ebx, 1
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (1 /*0x1*/ % 32));
    // 004999e9  83e91f                 -sub ecx, 0x1f
    (cpu.ecx) -= x86::reg32(x86::sreg32(31 /*0x1f*/));
    // 004999ec  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004999ed  d1f9                   -sar ecx, 1
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (1 /*0x1*/ % 32));
    // 004999ef  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 004999f4  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004999f5  ff500c                 -call dword ptr [eax + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004999f8  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004999fd  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00499a00  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499a06  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499a0c  6a0e                   -push 0xe
    app->getMemory<x86::reg32>(cpu.esp-4) = 14 /*0xe*/;
    cpu.esp -= 4;
    // 00499a0e  8b159cf57900           -mov edx, dword ptr [0x79f59c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499a14  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00499a16  ff5620                 -call dword ptr [esi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499a19  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499a1f  ba0e000000             -mov edx, 0xe
    cpu.edx = 14 /*0xe*/;
    // 00499a24  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 00499a29  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499a2c  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00499a2f  83f86f                 +cmp eax, 0x6f
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(111 /*0x6f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499a32  7209                   -jb 0x499a3d
    if (cpu.flags.cf)
    {
        goto L_0x00499a3d;
    }
    // 00499a34  762a                   -jbe 0x499a60
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00499a60;
    }
    // 00499a36  83f87c                 +cmp eax, 0x7c
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(124 /*0x7c*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499a39  7409                   -je 0x499a44
    if (cpu.flags.zf)
    {
        goto L_0x00499a44;
    }
    // 00499a3b  eb3c                   -jmp 0x499a79
    goto L_0x00499a79;
L_0x00499a3d:
    // 00499a3d  83f86e                 +cmp eax, 0x6e
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(110 /*0x6e*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499a40  7410                   -je 0x499a52
    if (cpu.flags.zf)
    {
        goto L_0x00499a52;
    }
    // 00499a42  eb35                   -jmp 0x499a79
    goto L_0x00499a79;
L_0x00499a44:
    // 00499a44  68f4c25300             -push 0x53c2f4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489396 /*0x53c2f4*/;
    cpu.esp -= 4;
    // 00499a49  8b359cf57900           -mov esi, dword ptr [0x79f59c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499a4f  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00499a50  eb1a                   -jmp 0x499a6c
    goto L_0x00499a6c;
L_0x00499a52:
    // 00499a52  6800c35300             -push 0x53c300
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489408 /*0x53c300*/;
    cpu.esp -= 4;
    // 00499a57  8b3d9cf57900           -mov edi, dword ptr [0x79f59c]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499a5d  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00499a5e  eb0c                   -jmp 0x499a6c
    goto L_0x00499a6c;
L_0x00499a60:
    // 00499a60  6814c35300             -push 0x53c314
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489428 /*0x53c314*/;
    cpu.esp -= 4;
    // 00499a65  8b159cf57900           -mov edx, dword ptr [0x79f59c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499a6b  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
L_0x00499a6c:
    // 00499a6c  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499a71  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00499a73  ff500c                 -call dword ptr [eax + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499a76  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
L_0x00499a79:
    // 00499a79  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
L_0x00499a7e:
    // 00499a7e  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00499a81:
    // 00499a81  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499a86  ff5018                 -call dword ptr [eax + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499a89  3d004b0000             +cmp eax, 0x4b00
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(19200 /*0x4b00*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499a8e  720b                   -jb 0x499a9b
    if (cpu.flags.cf)
    {
        goto L_0x00499a9b;
    }
    // 00499a90  761a                   -jbe 0x499aac
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00499aac;
    }
    // 00499a92  3d004d0000             +cmp eax, 0x4d00
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(19712 /*0x4d00*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499a97  742e                   -je 0x499ac7
    if (cpu.flags.zf)
    {
        goto L_0x00499ac7;
    }
    // 00499a99  eb48                   -jmp 0x499ae3
    goto L_0x00499ae3;
L_0x00499a9b:
    // 00499a9b  83f81b                 +cmp eax, 0x1b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(27 /*0x1b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499a9e  7543                   -jne 0x499ae3
    if (!cpu.flags.zf)
    {
        goto L_0x00499ae3;
    }
    // 00499aa0  c745f001000000         -mov dword ptr [ebp - 0x10], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = 1 /*0x1*/;
    // 00499aa7  e9b2fbffff             -jmp 0x49965e
    goto L_0x0049965e;
L_0x00499aac:
    // 00499aac  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00499aaf  4b                     -dec ebx
    (cpu.ebx)--;
    // 00499ab0  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 00499ab3  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00499ab5  7d06                   -jge 0x499abd
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00499abd;
    }
    // 00499ab7  8b45c4                 -mov eax, dword ptr [ebp - 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00499aba  48                     +dec eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 00499abb  eb02                   -jmp 0x499abf
    goto L_0x00499abf;
L_0x00499abd:
    // 00499abd  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
L_0x00499abf:
    // 00499abf  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00499ac2  e997fbffff             -jmp 0x49965e
    goto L_0x0049965e;
L_0x00499ac7:
    // 00499ac7  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00499aca  47                     -inc edi
    (cpu.edi)++;
    // 00499acb  8b55c4                 -mov edx, dword ptr [ebp - 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00499ace  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 00499ad1  39d7                   +cmp edi, edx
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499ad3  7c04                   -jl 0x499ad9
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00499ad9;
    }
    // 00499ad5  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00499ad7  eb02                   -jmp 0x499adb
    goto L_0x00499adb;
L_0x00499ad9:
    // 00499ad9  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
L_0x00499adb:
    // 00499adb  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00499ade  e97bfbffff             -jmp 0x49965e
    goto L_0x0049965e;
L_0x00499ae3:
    // 00499ae3  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00499ae5  894df0                 -mov dword ptr [ebp - 0x10], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ecx;
    // 00499ae8  e971fbffff             -jmp 0x49965e
    goto L_0x0049965e;
L_0x00499aed:
    // 00499aed  e83e020000             -call 0x499d30
    cpu.esp -= 4;
    sub_499d30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499af2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00499af4  7505                   -jne 0x499afb
    if (!cpu.flags.zf)
    {
        goto L_0x00499afb;
    }
    // 00499af6  e835000000             -call 0x499b30
    cpu.esp -= 4;
    sub_499b30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00499afb:
    // 00499afb  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499b00  ff5018                 -call dword ptr [eax + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00499b03:
    // 00499b03  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499b08  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499b0a  b8000000ff             -mov eax, 0xff000000
    cpu.eax = 4278190080 /*0xff000000*/;
    // 00499b0f  e84c5c0500             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499b14  e897560500             -call 0x4ef1b0
    cpu.esp -= 4;
    sub_4ef1b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499b19  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499b1e  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499b21  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00499b23  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499b24  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499b25  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499b26  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499b27  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499b28  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499b29  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_499b30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00499b30  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00499b31  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00499b32  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00499b33  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00499b34  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00499b35  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499b36  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499b38  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00499b3b  e870ab0500             -call 0x4f46b0
    cpu.esp -= 4;
    sub_4f46b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499b40  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 00499b42  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00499b44  81fa444e4957           +cmp edx, 0x57494e44
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1464421956 /*0x57494e44*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499b4a  7c09                   -jl 0x499b55
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00499b55;
    }
    // 00499b4c  7f07                   -jg 0x499b55
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00499b55;
    }
    // 00499b4e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00499b53  eb02                   -jmp 0x499b57
    goto L_0x00499b57;
L_0x00499b55:
    // 00499b55  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00499b57:
    // 00499b57  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00499b59  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00499b5b  7409                   -je 0x499b66
    if (cpu.flags.zf)
    {
        goto L_0x00499b66;
    }
    // 00499b5d  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00499b5f  e81caf0500             -call 0x4f4a80
    cpu.esp -= 4;
    sub_4f4a80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499b64  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
L_0x00499b66:
    // 00499b66  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00499b68  7566                   -jne 0x499bd0
    if (!cpu.flags.zf)
    {
        goto L_0x00499bd0;
    }
    // 00499b6a  bf1cc35300             -mov edi, 0x53c31c
    cpu.edi = 5489436 /*0x53c31c*/;
    // 00499b6f  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00499b71  49                     -dec ecx
    (cpu.ecx)--;
    // 00499b72  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00499b74  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00499b76  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 00499b78  49                     -dec ecx
    (cpu.ecx)--;
    // 00499b79  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499b7e  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499b84  8b359cf57900           -mov esi, dword ptr [0x79f59c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499b8a  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499b8c  b8000000ff             -mov eax, 0xff000000
    cpu.eax = 4278190080 /*0xff000000*/;
    // 00499b91  e8ca5b0500             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499b96  e815560500             -call 0x4ef1b0
    cpu.esp -= 4;
    sub_4ef1b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499b9b  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00499b9d  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499ba3  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00499ba8  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00499bad  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499bb0  681cc35300             -push 0x53c31c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489436 /*0x53c31c*/;
    cpu.esp -= 4;
    // 00499bb5  d1fe                   -sar esi, 1
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (1 /*0x1*/ % 32));
    // 00499bb7  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00499bb8  d1eb                   -shr ebx, 1
    cpu.ebx >>= 1 /*0x1*/ % 32;
    // 00499bba  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499bbf  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00499bc0  ff500c                 -call dword ptr [eax + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499bc3  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499bc8  83c40c                 +add esp, 0xc
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
    // 00499bcb  e94c010000             -jmp 0x499d1c
    goto L_0x00499d1c;
L_0x00499bd0:
    // 00499bd0  8b7e20                 -mov edi, dword ptr [esi + 0x20]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    // 00499bd3  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00499bd5  e8b6020000             -call 0x499e90
    cpu.esp -= 4;
    sub_499e90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499bda  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00499bdc  756d                   -jne 0x499c4b
    if (!cpu.flags.zf)
    {
        goto L_0x00499c4b;
    }
    // 00499bde  bf50c35300             -mov edi, 0x53c350
    cpu.edi = 5489488 /*0x53c350*/;
    // 00499be3  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00499be5  49                     -dec ecx
    (cpu.ecx)--;
    // 00499be6  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00499be8  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00499bea  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 00499bec  49                     -dec ecx
    (cpu.ecx)--;
    // 00499bed  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499bf3  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499bf8  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00499bfa  8b0d9cf57900           -mov ecx, dword ptr [0x79f59c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499c00  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499c02  b8000000ff             -mov eax, 0xff000000
    cpu.eax = 4278190080 /*0xff000000*/;
    // 00499c07  e8545b0500             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499c0c  e89f550500             -call 0x4ef1b0
    cpu.esp -= 4;
    sub_4ef1b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499c11  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499c17  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00499c1c  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00499c21  ff5624                 -call dword ptr [esi + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499c24  6850c35300             -push 0x53c350
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489488 /*0x53c350*/;
    cpu.esp -= 4;
    // 00499c29  d1f9                   -sar ecx, 1
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (1 /*0x1*/ % 32));
    // 00499c2b  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00499c2c  d1eb                   -shr ebx, 1
    cpu.ebx >>= 1 /*0x1*/ % 32;
    // 00499c2e  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499c33  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00499c34  ff500c                 -call dword ptr [eax + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499c37  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499c3c  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00499c3f  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499c42  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00499c44  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499c45  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499c46  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499c47  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499c48  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499c49  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499c4a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00499c4b:
    // 00499c4b  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00499c4d  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00499c4f  83e07f                 -and eax, 0x7f
    cpu.eax &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 00499c52  8a80546d5600           -mov al, byte ptr [eax + 0x566d54]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5664084) /* 0x566d54 */);
    // 00499c58  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00499c5a  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499c5f  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499c61  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499c67  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00499c6c  ff5108                 -call dword ptr [ecx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499c6f  8b4f02                 -mov ecx, dword ptr [edi + 2]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(2) /* 0x2 */);
    // 00499c72  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00499c74  c1f910                 -sar ecx, 0x10
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (16 /*0x10*/ % 32));
    // 00499c77  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00499c79  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499c7f  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 00499c84  ff5108                 -call dword ptr [ecx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499c87  8b4f04                 -mov ecx, dword ptr [edi + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 00499c8a  c1f910                 -sar ecx, 0x10
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (16 /*0x10*/ % 32));
    // 00499c8d  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00499c8f  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00499c91  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499c96  d1fb                   -sar ebx, 1
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (1 /*0x1*/ % 32));
    // 00499c98  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00499c9b  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00499c9d  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00499ca0  d1f9                   -sar ecx, 1
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (1 /*0x1*/ % 32));
    // 00499ca2  ff5714                 -call dword ptr [edi + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499ca5  6a0e                   -push 0xe
    app->getMemory<x86::reg32>(cpu.esp-4) = 14 /*0xe*/;
    cpu.esp -= 4;
    // 00499ca7  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00499cac  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499cb2  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499cb8  8b159cf57900           -mov edx, dword ptr [0x79f59c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499cbe  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00499cc0  ff5720                 -call dword ptr [edi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499cc3  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499cc9  ba0e000000             -mov edx, 0xe
    cpu.edx = 14 /*0xe*/;
    // 00499cce  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 00499cd3  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499cd6  8a661e                 -mov ah, byte ptr [esi + 0x1e]
    cpu.ah = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(30) /* 0x1e */);
    // 00499cd9  8b15a4f57900           -mov edx, dword ptr [0x79f5a4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499cdf  84e4                   +test ah, ah
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & cpu.ah));
    // 00499ce1  7507                   -jne 0x499cea
    if (!cpu.flags.zf)
    {
        goto L_0x00499cea;
    }
    // 00499ce3  b878c35300             -mov eax, 0x53c378
    cpu.eax = 5489528 /*0x53c378*/;
    // 00499ce8  eb05                   -jmp 0x499cef
    goto L_0x00499cef;
L_0x00499cea:
    // 00499cea  b880c35300             -mov eax, 0x53c380
    cpu.eax = 5489536 /*0x53c380*/;
L_0x00499cef:
    // 00499cef  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499cf0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00499cf2  8a461d                 -mov al, byte ptr [esi + 0x1d]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(29) /* 0x1d */);
    // 00499cf5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499cf6  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00499cf8  8a461c                 -mov al, byte ptr [esi + 0x1c]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(28) /* 0x1c */);
    // 00499cfb  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499cfc  8b5e08                 -mov ebx, dword ptr [esi + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 00499cff  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00499d00  8b7e04                 -mov edi, dword ptr [esi + 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00499d03  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00499d04  6888c35300             -push 0x53c388
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489544 /*0x53c388*/;
    cpu.esp -= 4;
    // 00499d09  a19cf57900             -mov eax, dword ptr [0x79f59c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499d0e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499d0f  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00499d11  ff520c                 -call dword ptr [edx + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499d14  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499d19  83c420                 -add esp, 0x20
    (cpu.esp) += x86::reg32(x86::sreg32(32 /*0x20*/));
L_0x00499d1c:
    // 00499d1c  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499d1f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00499d21  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499d22  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499d23  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499d24  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499d25  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499d26  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499d27  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_499d30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00499d30  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00499d31  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00499d32  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00499d33  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00499d34  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00499d35  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499d36  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499d38  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00499d3b  e870a90500             -call 0x4f46b0
    cpu.esp -= 4;
    sub_4f46b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499d40  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00499d42  e899780400             -call 0x4e15e0
    cpu.esp -= 4;
    nfs3hp::ea::memBlockName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499d47  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00499d4a  8d46f0                 -lea eax, [esi - 0x10]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(-16) /* -0x10 */);
    // 00499d4d  e80ef7ffff             -call 0x499460
    cpu.esp -= 4;
    sub_499460(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499d52  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00499d54  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00499d56  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00499d58  83e07f                 -and eax, 0x7f
    cpu.eax &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 00499d5b  8a80546d5600           -mov al, byte ptr [eax + 0x566d54]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5664084) /* 0x566d54 */);
    // 00499d61  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00499d64  83f808                 +cmp eax, 8
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499d67  7c07                   -jl 0x499d70
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00499d70;
    }
    // 00499d69  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00499d6b  c1fb03                 +sar ebx, 3
    {
        x86::reg8 tmp = 3 /*0x3*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 00499d6e  eb0a                   -jmp 0x499d7a
    goto L_0x00499d7a;
L_0x00499d70:
    // 00499d70  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00499d72  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00499d75  e900010000             -jmp 0x499e7a
    goto L_0x00499e7a;
L_0x00499d7a:
    // 00499d7a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00499d7c  0f84f8000000           -je 0x499e7a
    if (cpu.flags.zf)
    {
        goto L_0x00499e7a;
    }
    // 00499d82  8b4e04                 -mov ecx, dword ptr [esi + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00499d85  8b4602                 -mov eax, dword ptr [esi + 2]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 00499d88  c1f910                 -sar ecx, 0x10
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (16 /*0x10*/ % 32));
    // 00499d8b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00499d8e  0fafc1                 -imul eax, ecx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ecx)));
    // 00499d91  0fafc3                 -imul eax, ebx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 00499d94  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00499d97  39c2                   +cmp edx, eax
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
    // 00499d99  740c                   -je 0x499da7
    if (cpu.flags.zf)
    {
        goto L_0x00499da7;
    }
    // 00499d9b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00499d9e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00499da0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499da1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499da2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499da3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499da4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499da5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499da6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00499da7:
    // 00499da7  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00499da9  e8e2000000             -call 0x499e90
    cpu.esp -= 4;
    sub_499e90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499dae  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00499db0  0f84c4000000           -je 0x499e7a
    if (cpu.flags.zf)
    {
        goto L_0x00499e7a;
    }
    // 00499db6  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00499db8  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00499dba  83e07f                 -and eax, 0x7f
    cpu.eax &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 00499dbd  8a80546d5600           -mov al, byte ptr [eax + 0x566d54]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5664084) /* 0x566d54 */);
    // 00499dc3  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00499dc6  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00499dc9  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499dce  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499dd0  8b15a4f57900           -mov edx, dword ptr [0x79f5a4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499dd6  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00499ddb  ff5208                 -call dword ptr [edx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499dde  8b5602                 -mov edx, dword ptr [esi + 2]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 00499de1  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00499de3  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00499de6  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 00499deb  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00499ded  8b15a4f57900           -mov edx, dword ptr [0x79f5a4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499df3  d1fb                   -sar ebx, 1
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (1 /*0x1*/ % 32));
    // 00499df5  ff5208                 -call dword ptr [edx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499df8  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499dfe  8b5604                 -mov edx, dword ptr [esi + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00499e01  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00499e03  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00499e06  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00499e08  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00499e0a  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00499e0d  d1f9                   -sar ecx, 1
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (1 /*0x1*/ % 32));
    // 00499e0f  ff5714                 -call dword ptr [edi + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499e12  6a0e                   -push 0xe
    app->getMemory<x86::reg32>(cpu.esp-4) = 14 /*0xe*/;
    cpu.esp -= 4;
    // 00499e14  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00499e19  8b3da4f57900           -mov edi, dword ptr [0x79f5a4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499e1f  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499e25  8b159cf57900           -mov edx, dword ptr [0x79f59c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499e2b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00499e2d  ff5720                 -call dword ptr [edi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499e30  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499e36  ba0e000000             -mov edx, 0xe
    cpu.edx = 14 /*0xe*/;
    // 00499e3b  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 00499e40  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499e43  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00499e46  8b5604                 -mov edx, dword ptr [esi + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00499e49  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00499e4a  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00499e4d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00499e4e  8b5602                 -mov edx, dword ptr [esi + 2]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 00499e51  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00499e54  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00499e55  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00499e58  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00499e59  68b4c35300             -push 0x53c3b4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489588 /*0x53c3b4*/;
    cpu.esp -= 4;
    // 00499e5e  8b359cf57900           -mov esi, dword ptr [0x79f59c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499e64  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00499e65  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499e6a  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00499e6c  ff500c                 -call dword ptr [eax + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499e6f  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499e74  83c41c                 -add esp, 0x1c
    (cpu.esp) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 00499e77  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00499e7a:
    // 00499e7a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00499e7d  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00499e7f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499e80  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499e81  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499e82  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499e83  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499e84  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499e85  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_499e90(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00499e90  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499e91  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499e93  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00499e95  83e07f                 -and eax, 0x7f
    cpu.eax &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 00499e98  8a80546d5600           -mov al, byte ptr [eax + 0x566d54]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5664084) /* 0x566d54 */);
    // 00499e9e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499e9f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_499ea0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00499ea0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00499ea1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00499ea2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00499ea3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00499ea4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499ea5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499ea7  e804a80500             -call 0x4f46b0
    cpu.esp -= 4;
    sub_4f46b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499eac  8b15c0f57900           -mov edx, dword ptr [0x79f5c0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499eb2  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00499eb4  83fa50                 +cmp edx, 0x50
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(80 /*0x50*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499eb7  7d07                   -jge 0x499ec0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00499ec0;
    }
    // 00499eb9  bb08000000             -mov ebx, 8
    cpu.ebx = 8 /*0x8*/;
    // 00499ebe  eb05                   -jmp 0x499ec5
    goto L_0x00499ec5;
L_0x00499ec0:
    // 00499ec0  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x00499ec5:
    // 00499ec5  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00499ec7  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00499ec9  e8a2000000             -call 0x499f70
    cpu.esp -= 4;
    sub_499f70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499ece  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 00499ed0  eb32                   -jmp 0x499f04
    goto L_0x00499f04;
L_0x00499ed2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00499ed2  83f81b                 +cmp eax, 0x1b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(27 /*0x1b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499ed5  721b                   -jb 0x499ef2
    if (cpu.flags.cf)
    {
        goto L_0x00499ef2;
    }
    // 00499ed7  7619                   -jbe 0x499ef2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00499ef2;
    }
    // 00499ed9  3d00480000             +cmp eax, 0x4800
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18432 /*0x4800*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499ede  7406                   -je 0x499ee6
    if (cpu.flags.zf)
    {
        goto L_0x00499ee6;
    }
    // 00499ee0  eb10                   -jmp 0x499ef2
    goto L_0x00499ef2;
L_0x00499ee2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00499ee2  01d9                   +add ecx, ebx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00499ee4  eb11                   -jmp 0x499ef7
    goto L_0x00499ef7;
L_0x00499ee6:
    // 00499ee6  29d9                   +sub ecx, ebx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00499ee8  eb0d                   -jmp 0x499ef7
    goto L_0x00499ef7;
L_0x00499eea:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00499eea  01d1                   +add ecx, edx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00499eec  eb09                   -jmp 0x499ef7
    goto L_0x00499ef7;
L_0x00499eee:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00499eee  29d1                   +sub ecx, edx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00499ef0  eb05                   -jmp 0x499ef7
    goto L_0x00499ef7;
L_0x00499ef2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00499ef2  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
L_0x00499ef7:
    // 00499ef7  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00499ef9  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00499efb  e870000000             -call 0x499f70
    cpu.esp -= 4;
    sub_499f70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499f00  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00499f02  7531                   -jne 0x499f35
    if (!cpu.flags.zf)
    {
        goto L_0x00499f35;
    }
L_0x00499f04:
    // 00499f04  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499f09  ff5018                 -call dword ptr [eax + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499f0c  3d00490000             +cmp eax, 0x4900
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18688 /*0x4900*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499f11  72bf                   -jb 0x499ed2
    if (cpu.flags.cf)
    {
        goto L_0x00499ed2;
    }
    // 00499f13  8b159cf57900           -mov edx, dword ptr [0x79f59c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499f19  0fafd3                 -imul edx, ebx
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 00499f1c  3d00490000             +cmp eax, 0x4900
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18688 /*0x4900*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499f21  76cb                   -jbe 0x499eee
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00499eee;
    }
    // 00499f23  3d00500000             +cmp eax, 0x5000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20480 /*0x5000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499f28  72c8                   -jb 0x499ef2
    if (cpu.flags.cf)
    {
        goto L_0x00499ef2;
    }
    // 00499f2a  76b6                   -jbe 0x499ee2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00499ee2;
    }
    // 00499f2c  3d00510000             +cmp eax, 0x5100
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20736 /*0x5100*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499f31  74b7                   -je 0x499eea
    if (cpu.flags.zf)
    {
        goto L_0x00499eea;
    }
    // 00499f33  ebbd                   -jmp 0x499ef2
    goto L_0x00499ef2;
L_0x00499f35:
    // 00499f35  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499f3a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00499f3c  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499f3e  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 00499f40  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499f46  8b0d9cf57900           -mov ecx, dword ptr [0x79f59c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499f4c  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499f52  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00499f54  41                     -inc ecx
    (cpu.ecx)++;
    // 00499f55  ff5620                 -call dword ptr [esi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499f58  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499f5d  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499f60  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499f61  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499f62  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499f63  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499f64  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00499f65  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_499f70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00499f70  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00499f71  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00499f72  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00499f73  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00499f74  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00499f75  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00499f77  81ec0c010000           -sub esp, 0x10c
    (cpu.esp) -= x86::reg32(x86::sreg32(268 /*0x10c*/));
    // 00499f7d  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 00499f83  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 00499f86  89557a                 -mov dword ptr [ebp + 0x7a], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.edx;
    // 00499f89  894576                 -mov dword ptr [ebp + 0x76], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.eax;
    // 00499f8c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00499f8e  0f8480010000           -je 0x49a114
    if (cpu.flags.zf)
    {
        goto L_0x0049a114;
    }
    // 00499f94  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499f99  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00499f9b  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499f9d  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 00499f9f  8b35a4f57900           -mov esi, dword ptr [0x79f5a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499fa5  8b0d9cf57900           -mov ecx, dword ptr [0x79f59c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */);
    // 00499fab  8b1dc0f57900           -mov ebx, dword ptr [0x79f5c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992768) /* 0x79f5c0 */);
    // 00499fb1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00499fb3  41                     -inc ecx
    (cpu.ecx)++;
    // 00499fb4  ff5620                 -call dword ptr [esi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499fb7  8b0da4f57900           -mov ecx, dword ptr [0x79f5a4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 00499fbd  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00499fc2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00499fc4  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00499fc7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00499fc9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00499fc9  3b159cf57900           +cmp edx, dword ptr [0x79f59c]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992732) /* 0x79f59c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00499fcf  0f8d37010000           -jge 0x49a10c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049a10c;
    }
    // 00499fd5  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00499fd8  0fafc2                 -imul eax, edx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 00499fdb  034576                 -add eax, dword ptr [ebp + 0x76]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */)));
    // 00499fde  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499fdf  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00499fe2  83e808                 +sub eax, 8
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00499fe5  7407                   -je 0x499fee
    if (cpu.flags.zf)
    {
        goto L_0x00499fee;
    }
    // 00499fe7  b8d4c35300             -mov eax, 0x53c3d4
    cpu.eax = 5489620 /*0x53c3d4*/;
    // 00499fec  eb05                   -jmp 0x499ff3
    goto L_0x00499ff3;
L_0x00499fee:
    // 00499fee  b8dcc35300             -mov eax, 0x53c3dc
    cpu.eax = 5489628 /*0x53c3dc*/;
L_0x00499ff3:
    // 00499ff3  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499ff4  8d8576ffffff           -lea eax, [ebp - 0x8a]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-138) /* -0x8a */);
    // 00499ffa  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00499ffb  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00499ffd  e88e560400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a002  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
L_0x0049a005:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049a005  3b5d7a                 +cmp ebx, dword ptr [ebp + 0x7a]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049a008  7d4d                   -jge 0x49a057
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049a057;
    }
    // 0049a00a  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0049a00d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a00f  8a01                   -mov al, byte ptr [ecx]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx);
    // 0049a011  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a012  68e4c35300             -push 0x53c3e4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489636 /*0x53c3e4*/;
    cpu.esp -= 4;
    // 0049a017  8d45f6                 -lea eax, [ebp - 0xa]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 0049a01a  8d7901                 -lea edi, [ecx + 1]
    cpu.edi = x86::reg32(cpu.ecx + x86::reg32(1) /* 0x1 */);
    // 0049a01d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a01e  8d75f6                 -lea esi, [ebp - 0xa]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 0049a021  897d7e                 -mov dword ptr [ebp + 0x7e], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.edi;
    // 0049a024  e867560400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a029  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0049a02c  8dbd76ffffff           -lea edi, [ebp - 0x8a]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-138) /* -0x8a */);
    // 0049a032  43                     -inc ebx
    (cpu.ebx)++;
    // 0049a033  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049a034  2bc9                   +sub ecx, ecx
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
    // 0049a036  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0049a037  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0049a039  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0049a03b  4f                     -dec edi
    (cpu.edi)--;
L_0x0049a03c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049a03c  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049a03e  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049a040  3c00                   +cmp al, 0
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
    // 0049a042  7410                   -je 0x49a054
    if (cpu.flags.zf)
    {
        goto L_0x0049a054;
    }
    // 0049a044  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049a047  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a04a  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049a04d  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a050  3c00                   +cmp al, 0
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
    // 0049a052  75e8                   -jne 0x49a03c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a03c;
    }
L_0x0049a054:
    // 0049a054  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a055  ebae                   -jmp 0x49a005
    goto L_0x0049a005;
L_0x0049a057:
    // 0049a057  be94c05300             -mov esi, 0x53c094
    cpu.esi = 5488788 /*0x53c094*/;
    // 0049a05c  8dbd76ffffff           -lea edi, [ebp - 0x8a]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-138) /* -0x8a */);
    // 0049a062  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049a063  2bc9                   +sub ecx, ecx
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
    // 0049a065  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0049a066  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0049a068  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0049a06a  4f                     -dec edi
    (cpu.edi)--;
L_0x0049a06b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049a06b  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049a06d  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049a06f  3c00                   +cmp al, 0
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
    // 0049a071  7410                   -je 0x49a083
    if (cpu.flags.zf)
    {
        goto L_0x0049a083;
    }
    // 0049a073  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049a076  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a079  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049a07c  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a07f  3c00                   +cmp al, 0
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
    // 0049a081  75e8                   -jne 0x49a06b
    if (!cpu.flags.zf)
    {
        goto L_0x0049a06b;
    }
L_0x0049a083:
    // 0049a083  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a084  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 0049a087  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0049a08a  29c1                   -sub ecx, eax
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a08c  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049a08e  894d7e                 -mov dword ptr [ebp + 0x7e], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.ecx;
L_0x0049a091:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049a091  3b5d7a                 +cmp ebx, dword ptr [ebp + 0x7a]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049a094  7d5b                   -jge 0x49a0f1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049a0f1;
    }
    // 0049a096  8b7d7e                 -mov edi, dword ptr [ebp + 0x7e]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0049a099  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0049a09c  47                     -inc edi
    (cpu.edi)++;
    // 0049a09d  8a00                   -mov al, byte ptr [eax]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax);
    // 0049a09f  897d7e                 -mov dword ptr [ebp + 0x7e], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.edi;
    // 0049a0a2  3c20                   +cmp al, 0x20
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(32 /*0x20*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0049a0a4  7204                   -jb 0x49a0aa
    if (cpu.flags.cf)
    {
        goto L_0x0049a0aa;
    }
    // 0049a0a6  3c7e                   +cmp al, 0x7e
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(126 /*0x7e*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0049a0a8  7602                   -jbe 0x49a0ac
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0049a0ac;
    }
L_0x0049a0aa:
    // 0049a0aa  b02e                   -mov al, 0x2e
    cpu.al = 46 /*0x2e*/;
L_0x0049a0ac:
    // 0049a0ac  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049a0b1  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a0b2  68ecc35300             -push 0x53c3ec
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489644 /*0x53c3ec*/;
    cpu.esp -= 4;
    // 0049a0b7  8d45f6                 -lea eax, [ebp - 0xa]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 0049a0ba  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a0bb  8d75f6                 -lea esi, [ebp - 0xa]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 0049a0be  e8cd550400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a0c3  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0049a0c6  8dbd76ffffff           -lea edi, [ebp - 0x8a]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-138) /* -0x8a */);
    // 0049a0cc  43                     -inc ebx
    (cpu.ebx)++;
    // 0049a0cd  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049a0ce  2bc9                   +sub ecx, ecx
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
    // 0049a0d0  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0049a0d1  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0049a0d3  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0049a0d5  4f                     -dec edi
    (cpu.edi)--;
L_0x0049a0d6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049a0d6  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049a0d8  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049a0da  3c00                   +cmp al, 0
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
    // 0049a0dc  7410                   -je 0x49a0ee
    if (cpu.flags.zf)
    {
        goto L_0x0049a0ee;
    }
    // 0049a0de  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049a0e1  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a0e4  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049a0e7  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a0ea  3c00                   +cmp al, 0
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
    // 0049a0ec  75e8                   -jne 0x49a0d6
    if (!cpu.flags.zf)
    {
        goto L_0x0049a0d6;
    }
L_0x0049a0ee:
    // 0049a0ee  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a0ef  eba0                   -jmp 0x49a091
    goto L_0x0049a091;
L_0x0049a0f1:
    // 0049a0f1  8d8d76ffffff           -lea ecx, [ebp - 0x8a]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-138) /* -0x8a */);
    // 0049a0f7  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049a0f8  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049a0f9  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 0049a0fe  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0049a100  42                     -inc edx
    (cpu.edx)++;
    // 0049a101  ff500c                 -call dword ptr [eax + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a104  83c40c                 +add esp, 0xc
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
    // 0049a107  e9bdfeffff             -jmp 0x499fc9
    goto L_0x00499fc9;
L_0x0049a10c:
    // 0049a10c  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 0049a111  ff5004                 -call dword ptr [eax + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049a114:
    // 0049a114  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 0049a11a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a11b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a11c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a11d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a11e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a11f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_49a120(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049a120  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049a121  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049a122  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049a123  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049a124  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049a125  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049a126  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049a128  81ec64030000           -sub esp, 0x364
    (cpu.esp) -= x86::reg32(x86::sreg32(868 /*0x364*/));
    // 0049a12e  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 0049a134  8d7d4a                 -lea edi, [ebp + 0x4a]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 0049a137  be10764900             -mov esi, 0x497610
    cpu.esi = 4814352 /*0x497610*/;
    // 0049a13c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049a13d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049a13e  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049a13f  bef0c35300             -mov esi, 0x53c3f0
    cpu.esi = 5489648 /*0x53c3f0*/;
    // 0049a144  8dbd4affffff           -lea edi, [ebp - 0xb6]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-182) /* -0xb6 */);
    // 0049a14a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0049a14b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049a14b  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049a14d  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049a14f  3c00                   +cmp al, 0
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
    // 0049a151  7410                   -je 0x49a163
    if (cpu.flags.zf)
    {
        goto L_0x0049a163;
    }
    // 0049a153  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049a156  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a159  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049a15c  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a15f  3c00                   +cmp al, 0
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
    // 0049a161  75e8                   -jne 0x49a14b
    if (!cpu.flags.zf)
    {
        goto L_0x0049a14b;
    }
L_0x0049a163:
    // 0049a163  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a164  8d854affffff           -lea eax, [ebp - 0xb6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-182) /* -0xb6 */);
    // 0049a16a  ba00c45300             -mov edx, 0x53c400
    cpu.edx = 5489664 /*0x53c400*/;
    // 0049a16f  e88c020000             -call 0x49a400
    cpu.esp -= 4;
    sub_49a400(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a174  8d854affffff           -lea eax, [ebp - 0xb6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-182) /* -0xb6 */);
    // 0049a17a  e8693e0500             -call 0x4edfe8
    cpu.esp -= 4;
    nfs3hp::watcom::fopen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a17f  894576                 -mov dword ptr [ebp + 0x76], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.eax;
    // 0049a182  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049a184  0f844b020000           -je 0x49a3d5
    if (cpu.flags.zf)
    {
        goto L_0x0049a3d5;
    }
    // 0049a18a  8d4d6e                 -lea ecx, [ebp + 0x6e]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(110) /* 0x6e */);
    // 0049a18d  8d5d6a                 -lea ebx, [ebp + 0x6a]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 0049a190  8d5566                 -lea edx, [ebp + 0x66]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 0049a193  8d4562                 -lea eax, [ebp + 0x62]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 0049a196  e895f1ffff             -call 0x499330
    cpu.esp -= 4;
    sub_499330(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a19b  8b5562                 -mov edx, dword ptr [ebp + 0x62]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 0049a19e  8b0dace55500           -mov ecx, dword ptr [0x55e5ac]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629356) /* 0x55e5ac */);
    // 0049a1a4  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049a1a5  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0049a1a7  7407                   -je 0x49a1b0
    if (cpu.flags.zf)
    {
        goto L_0x0049a1b0;
    }
    // 0049a1a9  b804c45300             -mov eax, 0x53c404
    cpu.eax = 5489668 /*0x53c404*/;
    // 0049a1ae  eb05                   -jmp 0x49a1b5
    goto L_0x0049a1b5;
L_0x0049a1b0:
    // 0049a1b0  b81cc45300             -mov eax, 0x53c41c
    cpu.eax = 5489692 /*0x53c41c*/;
L_0x0049a1b5:
    // 0049a1b5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a1b6  8b5d76                 -mov ebx, dword ptr [ebp + 0x76]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 0049a1b9  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049a1ba  e8c1a90500             -call 0x4f4b80
    cpu.esp -= 4;
    nfs3hp::watcom::fprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a1bf  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0049a1c2  8b7566                 -mov esi, dword ptr [ebp + 0x66]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 0049a1c5  8b3dace55500           -mov edi, dword ptr [0x55e5ac]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5629356) /* 0x55e5ac */);
    // 0049a1cb  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049a1cc  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0049a1ce  7407                   -je 0x49a1d7
    if (cpu.flags.zf)
    {
        goto L_0x0049a1d7;
    }
    // 0049a1d0  b834c45300             -mov eax, 0x53c434
    cpu.eax = 5489716 /*0x53c434*/;
    // 0049a1d5  eb05                   -jmp 0x49a1dc
    goto L_0x0049a1dc;
L_0x0049a1d7:
    // 0049a1d7  b84cc45300             -mov eax, 0x53c44c
    cpu.eax = 5489740 /*0x53c44c*/;
L_0x0049a1dc:
    // 0049a1dc  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a1dd  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 0049a1e0  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a1e1  e89aa90500             -call 0x4f4b80
    cpu.esp -= 4;
    nfs3hp::watcom::fprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a1e6  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0049a1e9  8b556a                 -mov edx, dword ptr [ebp + 0x6a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 0049a1ec  8b0dace55500           -mov ecx, dword ptr [0x55e5ac]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629356) /* 0x55e5ac */);
    // 0049a1f2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049a1f3  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0049a1f5  7407                   -je 0x49a1fe
    if (cpu.flags.zf)
    {
        goto L_0x0049a1fe;
    }
    // 0049a1f7  b864c45300             -mov eax, 0x53c464
    cpu.eax = 5489764 /*0x53c464*/;
    // 0049a1fc  eb05                   -jmp 0x49a203
    goto L_0x0049a203;
L_0x0049a1fe:
    // 0049a1fe  b87cc45300             -mov eax, 0x53c47c
    cpu.eax = 5489788 /*0x53c47c*/;
L_0x0049a203:
    // 0049a203  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a204  8b5d76                 -mov ebx, dword ptr [ebp + 0x76]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 0049a207  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049a208  e873a90500             -call 0x4f4b80
    cpu.esp -= 4;
    nfs3hp::watcom::fprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a20d  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0049a210  8b756e                 -mov esi, dword ptr [ebp + 0x6e]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */);
    // 0049a213  8b3dace55500           -mov edi, dword ptr [0x55e5ac]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5629356) /* 0x55e5ac */);
    // 0049a219  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049a21a  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0049a21c  7407                   -je 0x49a225
    if (cpu.flags.zf)
    {
        goto L_0x0049a225;
    }
    // 0049a21e  b894c45300             -mov eax, 0x53c494
    cpu.eax = 5489812 /*0x53c494*/;
    // 0049a223  eb05                   -jmp 0x49a22a
    goto L_0x0049a22a;
L_0x0049a225:
    // 0049a225  b8b4c45300             -mov eax, 0x53c4b4
    cpu.eax = 5489844 /*0x53c4b4*/;
L_0x0049a22a:
    // 0049a22a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a22b  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 0049a22e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a22f  e84ca90500             -call 0x4f4b80
    cpu.esp -= 4;
    nfs3hp::watcom::fprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a234  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0049a237  68d4c45300             -push 0x53c4d4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489876 /*0x53c4d4*/;
    cpu.esp -= 4;
    // 0049a23c  8b5576                 -mov edx, dword ptr [ebp + 0x76]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 0049a23f  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049a240  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x0049a242:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049a242  e839a90500             -call 0x4f4b80
    cpu.esp -= 4;
    nfs3hp::watcom::fprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a247  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0049a24a  3b1dd0f57900           +cmp ebx, dword ptr [0x79f5d0]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992784) /* 0x79f5d0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049a250  0f8d93010000           -jge 0x49a3e9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049a3e9;
    }
    // 0049a256  b42d                   -mov ah, 0x2d
    cpu.ah = 45 /*0x2d*/;
    // 0049a258  8d0cdd00000000         -lea ecx, [ebx*8]
    cpu.ecx = x86::reg32(cpu.ebx * 8);
    // 0049a25f  30d2                   -xor dl, dl
    cpu.dl ^= x86::reg8(x86::sreg8(cpu.dl));
    // 0049a261  88a51efdffff           -mov byte ptr [ebp - 0x2e2], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-738) /* -0x2e2 */) = cpu.ah;
    // 0049a267  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049a26c  88951ffdffff           -mov byte ptr [ebp - 0x2e1], dl
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-737) /* -0x2e1 */) = cpu.dl;
    // 0049a272  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0049a274  8b0401                 -mov eax, dword ptr [ecx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1);
    // 0049a277  89757a                 -mov dword ptr [ebp + 0x7a], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.esi;
    // 0049a27a  e801f3ffff             -call 0x499580
    cpu.esp -= 4;
    sub_499580(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a27f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049a281  7456                   -je 0x49a2d9
    if (cpu.flags.zf)
    {
        goto L_0x0049a2d9;
    }
    // 0049a283  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049a288  8b0401                 -mov eax, dword ptr [ecx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1);
    // 0049a28b  e8d0f1ffff             -call 0x499460
    cpu.esp -= 4;
    sub_499460(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a290  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049a292  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049a297  8b0401                 -mov eax, dword ptr [ecx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1);
    // 0049a29a  e8e1f1ffff             -call 0x499480
    cpu.esp -= 4;
    sub_499480(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a29f  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0049a2a1  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049a2a6  8b0401                 -mov eax, dword ptr [ecx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1);
    // 0049a2a9  8d5556                 -lea edx, [ebp + 0x56]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 0049a2ac  e8dff1ffff             -call 0x499490
    cpu.esp -= 4;
    sub_499490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a2b1  e88aebffff             -call 0x498e40
    cpu.esp -= 4;
    sub_498e40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a2b6  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a2b8  6804c55300             -push 0x53c504
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489924 /*0x53c504*/;
    cpu.esp -= 4;
    // 0049a2bd  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 0049a2c0  8d854afeffff           -lea eax, [ebp - 0x1b6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-438) /* -0x1b6 */);
    // 0049a2c6  b620                   -mov dh, 0x20
    cpu.dh = 32 /*0x20*/;
    // 0049a2c8  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a2c9  887572                 -mov byte ptr [ebp + 0x72], dh
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.dh;
    // 0049a2cc  e8bf530400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a2d1  83c408                 +add esp, 8
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
    // 0049a2d4  e984000000             -jmp 0x49a35d
    goto L_0x0049a35d;
L_0x0049a2d9:
    // 0049a2d9  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049a2de  8b0401                 -mov eax, dword ptr [ecx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1);
    // 0049a2e1  e87af1ffff             -call 0x499460
    cpu.esp -= 4;
    sub_499460(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a2e6  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049a2e8  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049a2ed  8b0401                 -mov eax, dword ptr [ecx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1);
    // 0049a2f0  e88bf1ffff             -call 0x499480
    cpu.esp -= 4;
    sub_499480(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a2f5  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0049a2f7  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049a2fc  8b0401                 -mov eax, dword ptr [ecx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1);
    // 0049a2ff  8d5556                 -lea edx, [ebp + 0x56]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 0049a302  e889f1ffff             -call 0x499490
    cpu.esp -= 4;
    sub_499490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a307  e834ebffff             -call 0x498e40
    cpu.esp -= 4;
    sub_498e40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a30c  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049a311  8b540104               -mov edx, dword ptr [ecx + eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */ + cpu.eax * 1);
    // 0049a315  8b0401                 -mov eax, dword ptr [ecx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1);
    // 0049a318  89557e                 -mov dword ptr [ebp + 0x7e], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.edx;
    // 0049a31b  e8a0a40500             -call 0x4f47c0
    cpu.esp -= 4;
    sub_4f47c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a320  8b54854a               -mov edx, dword ptr [ebp + eax*4 + 0x4a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */ + cpu.eax * 4);
    // 0049a324  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049a325  68dcc05300             -push 0x53c0dc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488860 /*0x53c0dc*/;
    cpu.esp -= 4;
    // 0049a32a  8d4572                 -lea eax, [ebp + 0x72]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 0049a32d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a32e  e85d530400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a333  a1c4f57900             -mov eax, dword ptr [0x79f5c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992772) /* 0x79f5c4 */);
    // 0049a338  8b0401                 -mov eax, dword ptr [ecx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1);
    // 0049a33b  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0049a33e  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0049a341  e89a720400             -call 0x4e15e0
    cpu.esp -= 4;
    nfs3hp::ea::memBlockName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a346  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a347  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0049a349  6818c55300             -push 0x53c518
    app->getMemory<x86::reg32>(cpu.esp-4) = 5489944 /*0x53c518*/;
    cpu.esp -= 4;
    // 0049a34e  8d854afeffff           -lea eax, [ebp - 0x1b6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-438) /* -0x1b6 */);
    // 0049a354  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a355  e836530400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a35a  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
L_0x0049a35d:
    // 0049a35d  8d4572                 -lea eax, [ebp + 0x72]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 0049a360  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a361  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0049a364  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049a365  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 0049a368  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a369  8d851efdffff           -lea eax, [ebp - 0x2e2]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-738) /* -0x2e2 */);
    // 0049a36f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a370  8d4556                 -lea eax, [ebp + 0x56]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 0049a373  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a374  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049a375  8b15ace55500           -mov edx, dword ptr [0x55e5ac]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629356) /* 0x55e5ac */);
    // 0049a37b  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049a37c  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0049a37e  7407                   -je 0x49a387
    if (cpu.flags.zf)
    {
        goto L_0x0049a387;
    }
    // 0049a380  b824c55300             -mov eax, 0x53c524
    cpu.eax = 5489956 /*0x53c524*/;
    // 0049a385  eb05                   -jmp 0x49a38c
    goto L_0x0049a38c;
L_0x0049a387:
    // 0049a387  b844c55300             -mov eax, 0x53c544
    cpu.eax = 5489988 /*0x53c544*/;
L_0x0049a38c:
    // 0049a38c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a38d  8d45ca                 -lea eax, [ebp - 0x36]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-54) /* -0x36 */);
    // 0049a390  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a391  e8fa520400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a396  83c424                 -add esp, 0x24
    (cpu.esp) += x86::reg32(x86::sreg32(36 /*0x24*/));
    // 0049a399  8d75ca                 -lea esi, [ebp - 0x36]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-54) /* -0x36 */);
    // 0049a39c  8dbd4afeffff           -lea edi, [ebp - 0x1b6]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-438) /* -0x1b6 */);
    // 0049a3a2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049a3a3  2bc9                   +sub ecx, ecx
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
    // 0049a3a5  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0049a3a6  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0049a3a8  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0049a3aa  4f                     -dec edi
    (cpu.edi)--;
L_0x0049a3ab:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049a3ab  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049a3ad  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049a3af  3c00                   +cmp al, 0
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
    // 0049a3b1  7410                   -je 0x49a3c3
    if (cpu.flags.zf)
    {
        goto L_0x0049a3c3;
    }
    // 0049a3b3  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049a3b6  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a3b9  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049a3bc  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a3bf  3c00                   +cmp al, 0
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
    // 0049a3c1  75e8                   -jne 0x49a3ab
    if (!cpu.flags.zf)
    {
        goto L_0x0049a3ab;
    }
L_0x0049a3c3:
    // 0049a3c3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a3c4  8d854afeffff           -lea eax, [ebp - 0x1b6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-438) /* -0x1b6 */);
    // 0049a3ca  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a3cb  8b4d76                 -mov ecx, dword ptr [ebp + 0x76]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 0049a3ce  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049a3cf  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049a3d0  e96dfeffff             -jmp 0x49a242
    goto L_0x0049a242;
L_0x0049a3d5:
    // 0049a3d5  6864c55300             -push 0x53c564
    app->getMemory<x86::reg32>(cpu.esp-4) = 5490020 /*0x53c564*/;
    cpu.esp -= 4;
    // 0049a3da  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0049a3dc  a1a4f57900             -mov eax, dword ptr [0x79f5a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992740) /* 0x79f5a4 */);
    // 0049a3e1  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0049a3e3  ff500c                 -call dword ptr [eax + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a3e6  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
L_0x0049a3e9:
    // 0049a3e9  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 0049a3ec  e80f3d0500             -call 0x4ee100
    cpu.esp -= 4;
    nfs3hp::watcom::fclose(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a3f1  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 0049a3f7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a3f8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a3f9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a3fa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a3fb  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a3fc  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a3fd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_49a400(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049a400  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049a401  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049a402  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049a403  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049a404  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049a405  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049a406  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049a408  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0049a40b  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0049a40d  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x0049a40f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049a40f  8d0433                 -lea eax, [ebx + esi]
    cpu.eax = x86::reg32(cpu.ebx + cpu.esi * 1);
    // 0049a412  80383f                 +cmp byte ptr [eax], 0x3f
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(63 /*0x3f*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0049a415  7403                   -je 0x49a41a
    if (cpu.flags.zf)
    {
        goto L_0x0049a41a;
    }
    // 0049a417  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049a418  ebf5                   -jmp 0x49a40f
    goto L_0x0049a40f;
L_0x0049a41a:
    // 0049a41a  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 0049a41c  eb06                   -jmp 0x49a424
    goto L_0x0049a424;
L_0x0049a41e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049a41e  41                     -inc ecx
    (cpu.ecx)++;
    // 0049a41f  83f964                 +cmp ecx, 0x64
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(100 /*0x64*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049a422  7d45                   -jge 0x49a469
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049a469;
    }
L_0x0049a424:
    // 0049a424  bf0a000000             -mov edi, 0xa
    cpu.edi = 10 /*0xa*/;
    // 0049a429  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0049a42b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049a42d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0049a430  f7ff                   -idiv edi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.edi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0049a432  ba0a000000             -mov edx, 0xa
    cpu.edx = 10 /*0xa*/;
    // 0049a437  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049a43c  8d3c33                 -lea edi, [ebx + esi]
    cpu.edi = x86::reg32(cpu.ebx + cpu.esi * 1);
    // 0049a43f  83c030                 -add eax, 0x30
    (cpu.eax) += x86::reg32(x86::sreg32(48 /*0x30*/));
    // 0049a442  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0049a445  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0049a447  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049a449  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049a44b  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0049a44e  f77dfc                 -idiv dword ptr [ebp - 4]
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0049a451  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049a453  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049a458  83c030                 -add eax, 0x30
    (cpu.eax) += x86::reg32(x86::sreg32(48 /*0x30*/));
    // 0049a45b  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049a45e  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0049a460  e8fb690400             -call 0x4e0e60
    cpu.esp -= 4;
    nfs3hp::ea::fileExists(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a465  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049a467  75b5                   -jne 0x49a41e
    if (!cpu.flags.zf)
    {
        goto L_0x0049a41e;
    }
L_0x0049a469:
    // 0049a469  83f964                 +cmp ecx, 0x64
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(100 /*0x64*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049a46c  7c0c                   -jl 0x49a47a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0049a47a;
    }
    // 0049a46e  c6441e0178             -mov byte ptr [esi + ebx + 1], 0x78
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */ + cpu.ebx * 1) = 120 /*0x78*/;
    // 0049a473  8a441e01               -mov al, byte ptr [esi + ebx + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */ + cpu.ebx * 1);
    // 0049a477  88041e                 -mov byte ptr [esi + ebx], al
    app->getMemory<x86::reg8>(cpu.esi + cpu.ebx * 1) = cpu.al;
L_0x0049a47a:
    // 0049a47a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049a47f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049a481  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a482  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a483  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a484  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a485  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a486  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a487  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_49a490(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049a490  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049a491  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049a492  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049a494  e8470e0500             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a499  ba00030000             -mov edx, 0x300
    cpu.edx = 768 /*0x300*/;
    // 0049a49e  a3b8e55500             -mov dword ptr [0x55e5b8], eax
    app->getMemory<x86::reg32>(x86::reg32(5629368) /* 0x55e5b8 */) = cpu.eax;
    // 0049a4a3  e8380e0500             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a4a8  a3b4ec5500             -mov dword ptr [0x55ecb4], eax
    app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */) = cpu.eax;
    // 0049a4ad  e82e0e0500             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a4b2  a3b8ec5500             -mov dword ptr [0x55ecb8], eax
    app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */) = cpu.eax;
    // 0049a4b7  b8f0f57900             -mov eax, 0x79f5f0
    cpu.eax = 7992816 /*0x79f5f0*/;
    // 0049a4bc  e84b620400             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a4c1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a4c2  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a4c3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49a4d0: native, nfs3hp::frontend::freeNetworkLocks */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_49a520(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049a520  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049a521  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049a522  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049a523  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049a524  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049a525  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049a526  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049a528  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0049a52b  e8b0450000             -call 0x49eae0
    cpu.esp -= 4;
    sub_49eae0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a530  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0049a532  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0049a535  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 0049a53a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049a53d  e84ec10000             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a542  0fbff8                 -movsx edi, ax
    cpu.edi = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0049a545  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049a547  83ffff                 +cmp edi, -1
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
    // 0049a54a  0f84f8000000           -je 0x49a648
    if (cpu.flags.zf)
    {
        goto L_0x0049a648;
    }
    // 0049a550  e80b7efdff             -call 0x472360
    cpu.esp -= 4;
    nfs3hp::port::fields::refreshCustomColours(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a555  a007d36f00             -mov al, byte ptr [0x6fd307]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328519) /* 0x6fd307 */);
    // 0049a55a  8845fc                 -mov byte ptr [ebp - 4], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.al;
    // 0049a55d  a008d36f00             -mov al, byte ptr [0x6fd308]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328520) /* 0x6fd308 */);
    // 0049a562  8845fd                 -mov byte ptr [ebp - 3], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-3) /* -0x3 */) = cpu.al;
    // 0049a565  a009d36f00             -mov al, byte ptr [0x6fd309]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328521) /* 0x6fd309 */);
    // 0049a56a  8845fe                 -mov byte ptr [ebp - 2], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-2) /* -0x2 */) = cpu.al;
    // 0049a56d  a00ad36f00             -mov al, byte ptr [0x6fd30a]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328522) /* 0x6fd30a */);
    // 0049a572  8845ff                 -mov byte ptr [ebp - 1], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-1) /* -0x1 */) = cpu.al;
    // 0049a575  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0049a57a  e8d1f4f9ff             -call 0x439a50
    cpu.esp -= 4;
    nfs3hp::load::carRecord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a57f  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049a581  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a583  8b15bcd26f00           -mov edx, dword ptr [0x6fd2bc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0049a589  a02cf97900             -mov al, byte ptr [0x79f92c]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993644) /* 0x79f92c */);
    // 0049a58e  39d0                   +cmp eax, edx
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
    // 0049a590  0f85a8000000           -jne 0x49a63e
    if (!cpu.flags.zf)
    {
        goto L_0x0049a63e;
    }
    // 0049a596  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049a598  8b15d4d26f00           -mov edx, dword ptr [0x6fd2d4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328468) /* 0x6fd2d4 */);
    // 0049a59e  a02df97900             -mov al, byte ptr [0x79f92d]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993645) /* 0x79f92d */);
    // 0049a5a3  39d0                   +cmp eax, edx
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
    // 0049a5a5  0f8593000000           -jne 0x49a63e
    if (!cpu.flags.zf)
    {
        goto L_0x0049a63e;
    }
    // 0049a5ab  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049a5ad  8b1508d56f00           -mov edx, dword ptr [0x6fd508]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329032) /* 0x6fd508 */);
    // 0049a5b3  a020f97900             -mov al, byte ptr [0x79f920]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993632) /* 0x79f920 */);
    // 0049a5b8  39d0                   +cmp eax, edx
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
    // 0049a5ba  0f857e000000           -jne 0x49a63e
    if (!cpu.flags.zf)
    {
        goto L_0x0049a63e;
    }
    // 0049a5c0  bafcd26f00             -mov edx, 0x6fd2fc
    cpu.edx = 7328508 /*0x6fd2fc*/;
    // 0049a5c5  b821f97900             -mov eax, 0x79f921
    cpu.eax = 7993633 /*0x79f921*/;
    // 0049a5ca  e8413d0500             -call 0x4ee310
    cpu.esp -= 4;
    nfs3hp::watcom::strcmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a5cf  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049a5d1  0f8567000000           -jne 0x49a63e
    if (!cpu.flags.zf)
    {
        goto L_0x0049a63e;
    }
    // 0049a5d7  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0049a5d9  0f855f000000           -jne 0x49a63e
    if (!cpu.flags.zf)
    {
        goto L_0x0049a63e;
    }
    // 0049a5df  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0049a5e1  8d04bd00000000         -lea eax, [edi*4]
    cpu.eax = x86::reg32(cpu.edi * 4);
    // 0049a5e8  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 0049a5ea  31fb                   -xor ebx, edi
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0049a5ec  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049a5ef  8b3dbcd26f00           -mov edi, dword ptr [0x6fd2bc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0049a5f5  8a9804f67900           -mov bl, byte ptr [eax + 0x79f604]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(7992836) /* 0x79f604 */);
    // 0049a5fb  39fb                   +cmp ebx, edi
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049a5fd  753f                   -jne 0x49a63e
    if (!cpu.flags.zf)
    {
        goto L_0x0049a63e;
    }
    // 0049a5ff  31fb                   -xor ebx, edi
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0049a601  8b15d4d26f00           -mov edx, dword ptr [0x6fd2d4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328468) /* 0x6fd2d4 */);
    // 0049a607  8a9805f67900           -mov bl, byte ptr [eax + 0x79f605]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(7992837) /* 0x79f605 */);
    // 0049a60d  39d3                   +cmp ebx, edx
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
    // 0049a60f  752d                   -jne 0x49a63e
    if (!cpu.flags.zf)
    {
        goto L_0x0049a63e;
    }
    // 0049a611  8a80f8f57900           -mov al, byte ptr [eax + 0x79f5f8]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(7992824) /* 0x79f5f8 */);
    // 0049a617  8b1d08d56f00           -mov ebx, dword ptr [0x6fd508]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329032) /* 0x6fd508 */);
    // 0049a61d  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049a622  39d8                   +cmp eax, ebx
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
    // 0049a624  7518                   -jne 0x49a63e
    if (!cpu.flags.zf)
    {
        goto L_0x0049a63e;
    }
    // 0049a626  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049a629  3b3d61f97900           +cmp edi, dword ptr [0x79f961]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7993697) /* 0x79f961 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049a62f  750d                   -jne 0x49a63e
    if (!cpu.flags.zf)
    {
        goto L_0x0049a63e;
    }
    // 0049a631  a168f97900             -mov eax, dword ptr [0x79f968]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7993704) /* 0x79f968 */);
    // 0049a636  3b8680030000           +cmp eax, dword ptr [esi + 0x380]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(896) /* 0x380 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049a63c  740a                   -je 0x49a648
    if (cpu.flags.zf)
    {
        goto L_0x0049a648;
    }
L_0x0049a63e:
    // 0049a63e  e89d020000             -call 0x49a8e0
    cpu.esp -= 4;
    sub_49a8e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a643  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
L_0x0049a648:
    // 0049a648  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049a650  0f8420010000           -je 0x49a776
    if (cpu.flags.zf)
    {
        goto L_0x0049a776;
    }
    // 0049a656  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a658  8b1db8d36f00           -mov ebx, dword ptr [0x6fd3b8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0049a65e  a0f1f87900             -mov al, byte ptr [0x79f8f1]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993585) /* 0x79f8f1 */);
    // 0049a663  39d8                   +cmp eax, ebx
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
    // 0049a665  0f8501010000           -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a66b  31d8                   -xor eax, ebx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049a66d  8b35acd46f00           -mov esi, dword ptr [0x6fd4ac]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 0049a673  a0f2f87900             -mov al, byte ptr [0x79f8f2]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993586) /* 0x79f8f2 */);
    // 0049a678  39f0                   +cmp eax, esi
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
    // 0049a67a  0f85ec000000           -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a680  31f0                   -xor eax, esi
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0049a682  8b3d04d56f00           -mov edi, dword ptr [0x6fd504]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7329028) /* 0x6fd504 */);
    // 0049a688  a0f3f87900             -mov al, byte ptr [0x79f8f3]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993587) /* 0x79f8f3 */);
    // 0049a68d  39f8                   +cmp eax, edi
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
    // 0049a68f  0f85d7000000           -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a695  31f8                   -xor eax, edi
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0049a697  8b15b4d46f00           -mov edx, dword ptr [0x6fd4b4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */);
    // 0049a69d  a0f4f87900             -mov al, byte ptr [0x79f8f4]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993588) /* 0x79f8f4 */);
    // 0049a6a2  39d0                   +cmp eax, edx
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
    // 0049a6a4  0f85c2000000           -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a6aa  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049a6ac  8b1db0d46f00           -mov ebx, dword ptr [0x6fd4b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328944) /* 0x6fd4b0 */);
    // 0049a6b2  a0f5f87900             -mov al, byte ptr [0x79f8f5]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993589) /* 0x79f8f5 */);
    // 0049a6b7  39d8                   +cmp eax, ebx
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
    // 0049a6b9  0f85ad000000           -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a6bf  31d8                   -xor eax, ebx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049a6c1  8b35d4d46f00           -mov esi, dword ptr [0x6fd4d4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 0049a6c7  a0f6f87900             -mov al, byte ptr [0x79f8f6]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993590) /* 0x79f8f6 */);
    // 0049a6cc  39f0                   +cmp eax, esi
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
    // 0049a6ce  0f8598000000           -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a6d4  31f0                   -xor eax, esi
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0049a6d6  8b3dd0d46f00           -mov edi, dword ptr [0x6fd4d0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7328976) /* 0x6fd4d0 */);
    // 0049a6dc  a008f97900             -mov al, byte ptr [0x79f908]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993608) /* 0x79f908 */);
    // 0049a6e1  39f8                   +cmp eax, edi
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
    // 0049a6e3  0f8583000000           -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a6e9  31f8                   -xor eax, edi
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0049a6eb  8b15e8d46f00           -mov edx, dword ptr [0x6fd4e8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */);
    // 0049a6f1  a009f97900             -mov al, byte ptr [0x79f909]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993609) /* 0x79f909 */);
    // 0049a6f6  39d0                   +cmp eax, edx
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
    // 0049a6f8  0f856e000000           -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a6fe  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049a700  8b1dc4d46f00           -mov ebx, dword ptr [0x6fd4c4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */);
    // 0049a706  a00af97900             -mov al, byte ptr [0x79f90a]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993610) /* 0x79f90a */);
    // 0049a70b  39d8                   +cmp eax, ebx
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
    // 0049a70d  0f8559000000           -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a713  31d8                   -xor eax, ebx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049a715  8b35ccd46f00           -mov esi, dword ptr [0x6fd4cc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328972) /* 0x6fd4cc */);
    // 0049a71b  a00bf97900             -mov al, byte ptr [0x79f90b]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993611) /* 0x79f90b */);
    // 0049a720  39f0                   +cmp eax, esi
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
    // 0049a722  7548                   -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a724  31f0                   -xor eax, esi
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0049a726  8b3dc8d46f00           -mov edi, dword ptr [0x6fd4c8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */);
    // 0049a72c  a00cf97900             -mov al, byte ptr [0x79f90c]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993612) /* 0x79f90c */);
    // 0049a731  39f8                   +cmp eax, edi
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
    // 0049a733  7537                   -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a735  31f8                   -xor eax, edi
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0049a737  8b15c0d46f00           -mov edx, dword ptr [0x6fd4c0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */);
    // 0049a73d  a00df97900             -mov al, byte ptr [0x79f90d]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993613) /* 0x79f90d */);
    // 0049a742  39d0                   +cmp eax, edx
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
    // 0049a744  7526                   -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a746  8b1db4d36f00           -mov ebx, dword ptr [0x6fd3b4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328692) /* 0x6fd3b4 */);
    // 0049a74c  a110f97900             -mov eax, dword ptr [0x79f910]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7993616) /* 0x79f910 */);
    // 0049a751  39d8                   +cmp eax, ebx
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
    // 0049a753  7517                   -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a755  31d8                   -xor eax, ebx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049a757  8b3570d46f00           -mov esi, dword ptr [0x6fd470]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328880) /* 0x6fd470 */);
    // 0049a75d  a014f97900             -mov al, byte ptr [0x79f914]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993620) /* 0x79f914 */);
    // 0049a762  39f0                   +cmp eax, esi
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
    // 0049a764  7506                   -jne 0x49a76c
    if (!cpu.flags.zf)
    {
        goto L_0x0049a76c;
    }
    // 0049a766  837df800               +cmp dword ptr [ebp - 8], 0
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
    // 0049a76a  740a                   -je 0x49a776
    if (cpu.flags.zf)
    {
        goto L_0x0049a776;
    }
L_0x0049a76c:
    // 0049a76c  e80f030000             -call 0x49aa80
    cpu.esp -= 4;
    sub_49aa80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a771  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
L_0x0049a776:
    // 0049a776  66833da8227a0000       +cmp word ptr [0x7a22a8], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004264) /* 0x7a22a8 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049a77e  740e                   -je 0x49a78e
    if (cpu.flags.zf)
    {
        goto L_0x0049a78e;
    }
    // 0049a780  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049a782  66890da8227a00         -mov word ptr [0x7a22a8], cx
    app->getMemory<x86::reg16>(x86::reg32(8004264) /* 0x7a22a8 */) = cpu.cx;
    // 0049a789  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
L_0x0049a78e:
    // 0049a78e  66833d9a227a0000       +cmp word ptr [0x7a229a], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004250) /* 0x7a229a */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049a796  7427                   -je 0x49a7bf
    if (cpu.flags.zf)
    {
        goto L_0x0049a7bf;
    }
    // 0049a798  a1b4ec5500             -mov eax, dword ptr [0x55ecb4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */);
    // 0049a79d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049a79e  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a7a4  8b15b4ec5500           -mov edx, dword ptr [0x55ecb4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */);
    // 0049a7aa  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0049a7ac  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049a7ad  66893d9a227a00         -mov word ptr [0x7a229a], di
    app->getMemory<x86::reg16>(x86::reg32(8004250) /* 0x7a229a */) = cpu.di;
    // 0049a7b4  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a7ba  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
L_0x0049a7bf:
    // 0049a7bf  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049a7c7  0f85f9000000           -jne 0x49a8c6
    if (!cpu.flags.zf)
    {
        goto L_0x0049a8c6;
    }
    // 0049a7cd  66833d98227a0000       +cmp word ptr [0x7a2298], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004248) /* 0x7a2298 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049a7d5  0f84eb000000           -je 0x49a8c6
    if (cpu.flags.zf)
    {
        goto L_0x0049a8c6;
    }
    // 0049a7db  8b0db8e55500           -mov ecx, dword ptr [0x55e5b8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629368) /* 0x55e5b8 */);
    // 0049a7e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049a7e2  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049a7e4  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a7ea  66891d98227a00         -mov word ptr [0x7a2298], bx
    app->getMemory<x86::reg16>(x86::reg32(8004248) /* 0x7a2298 */) = cpu.bx;
    // 0049a7f1  8b1db8e55500           -mov ebx, dword ptr [0x55e5b8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629368) /* 0x55e5b8 */);
    // 0049a7f7  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049a7f8  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a7fe  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a800  a0f1f87900             -mov al, byte ptr [0x79f8f1]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993585) /* 0x79f8f1 */);
    // 0049a805  a3b8d36f00             -mov dword ptr [0x6fd3b8], eax
    app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */) = cpu.eax;
    // 0049a80a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a80c  a0f2f87900             -mov al, byte ptr [0x79f8f2]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993586) /* 0x79f8f2 */);
    // 0049a811  a3acd46f00             -mov dword ptr [0x6fd4ac], eax
    app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */) = cpu.eax;
    // 0049a816  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a818  a0f3f87900             -mov al, byte ptr [0x79f8f3]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993587) /* 0x79f8f3 */);
    // 0049a81d  a304d56f00             -mov dword ptr [0x6fd504], eax
    app->getMemory<x86::reg32>(x86::reg32(7329028) /* 0x6fd504 */) = cpu.eax;
    // 0049a822  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a824  a0f4f87900             -mov al, byte ptr [0x79f8f4]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993588) /* 0x79f8f4 */);
    // 0049a829  a3b4d46f00             -mov dword ptr [0x6fd4b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */) = cpu.eax;
    // 0049a82e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a830  a0f5f87900             -mov al, byte ptr [0x79f8f5]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993589) /* 0x79f8f5 */);
    // 0049a835  a3b0d46f00             -mov dword ptr [0x6fd4b0], eax
    app->getMemory<x86::reg32>(x86::reg32(7328944) /* 0x6fd4b0 */) = cpu.eax;
    // 0049a83a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a83c  a0f6f87900             -mov al, byte ptr [0x79f8f6]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993590) /* 0x79f8f6 */);
    // 0049a841  a3d4d46f00             -mov dword ptr [0x6fd4d4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */) = cpu.eax;
    // 0049a846  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a848  a008f97900             -mov al, byte ptr [0x79f908]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993608) /* 0x79f908 */);
    // 0049a84d  a3d0d46f00             -mov dword ptr [0x6fd4d0], eax
    app->getMemory<x86::reg32>(x86::reg32(7328976) /* 0x6fd4d0 */) = cpu.eax;
    // 0049a852  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a854  a009f97900             -mov al, byte ptr [0x79f909]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993609) /* 0x79f909 */);
    // 0049a859  a3e8d46f00             -mov dword ptr [0x6fd4e8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */) = cpu.eax;
    // 0049a85e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a860  a00af97900             -mov al, byte ptr [0x79f90a]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993610) /* 0x79f90a */);
    // 0049a865  a3c4d46f00             -mov dword ptr [0x6fd4c4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */) = cpu.eax;
    // 0049a86a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a86c  a00bf97900             -mov al, byte ptr [0x79f90b]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993611) /* 0x79f90b */);
    // 0049a871  a3ccd46f00             -mov dword ptr [0x6fd4cc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328972) /* 0x6fd4cc */) = cpu.eax;
    // 0049a876  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a878  a00cf97900             -mov al, byte ptr [0x79f90c]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993612) /* 0x79f90c */);
    // 0049a87d  a3c8d46f00             -mov dword ptr [0x6fd4c8], eax
    app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */) = cpu.eax;
    // 0049a882  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a884  a00df97900             -mov al, byte ptr [0x79f90d]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993613) /* 0x79f90d */);
    // 0049a889  a3c0d46f00             -mov dword ptr [0x6fd4c0], eax
    app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */) = cpu.eax;
    // 0049a88e  a110f97900             -mov eax, dword ptr [0x79f910]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7993616) /* 0x79f910 */);
    // 0049a893  a3b4d36f00             -mov dword ptr [0x6fd3b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328692) /* 0x6fd3b4 */) = cpu.eax;
    // 0049a898  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049a89a  a014f97900             -mov al, byte ptr [0x79f914]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7993620) /* 0x79f914 */);
    // 0049a89f  8b35d4d46f00           -mov esi, dword ptr [0x6fd4d4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 0049a8a5  a370d46f00             -mov dword ptr [0x6fd470], eax
    app->getMemory<x86::reg32>(x86::reg32(7328880) /* 0x6fd470 */) = cpu.eax;
    // 0049a8aa  83fe04                 +cmp esi, 4
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
    // 0049a8ad  7c12                   -jl 0x49a8c1
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0049a8c1;
    }
    // 0049a8af  b8f8f87900             -mov eax, 0x79f8f8
    cpu.eax = 7993592 /*0x79f8f8*/;
    // 0049a8b4  e8d7f0f9ff             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a8b9  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0049a8bc  a3d4d46f00             -mov dword ptr [0x6fd4d4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */) = cpu.eax;
L_0x0049a8c1:
    // 0049a8c1  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
L_0x0049a8c6:
    // 0049a8c6  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049a8c8  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049a8ca  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a8cb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a8cc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a8cd  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a8ce  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a8cf  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a8d0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_49a8e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049a8e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049a8e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049a8e2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049a8e3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049a8e4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049a8e5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049a8e6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049a8e8  c60518f9790003         -mov byte ptr [0x79f918], 3
    app->getMemory<x86::reg8>(x86::reg32(7993624) /* 0x79f918 */) = 3 /*0x3*/;
    // 0049a8ef  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 0049a8f4  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049a8f7  e894bd0000             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a8fc  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0049a8fd  a31cf97900             -mov dword ptr [0x79f91c], eax
    app->getMemory<x86::reg32>(x86::reg32(7993628) /* 0x79f91c */) = cpu.eax;
    // 0049a902  83f8ff                 +cmp eax, -1
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
    // 0049a905  0f8462010000           -je 0x49aa6d
    if (cpu.flags.zf)
    {
        goto L_0x0049aa6d;
    }
    // 0049a90b  befcd26f00             -mov esi, 0x6fd2fc
    cpu.esi = 7328508 /*0x6fd2fc*/;
    // 0049a910  bf21f97900             -mov edi, 0x79f921
    cpu.edi = 7993633 /*0x79f921*/;
    // 0049a915  a008d56f00             -mov al, byte ptr [0x6fd508]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7329032) /* 0x6fd508 */);
    // 0049a91a  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 0049a91f  a220f97900             -mov byte ptr [0x79f920], al
    app->getMemory<x86::reg8>(x86::reg32(7993632) /* 0x79f920 */) = cpu.al;
    // 0049a924  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0049a925:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049a925  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049a927  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049a929  3c00                   +cmp al, 0
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
    // 0049a92b  7410                   -je 0x49a93d
    if (cpu.flags.zf)
    {
        goto L_0x0049a93d;
    }
    // 0049a92d  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049a930  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a933  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049a936  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049a939  3c00                   +cmp al, 0
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
    // 0049a93b  75e8                   -jne 0x49a925
    if (!cpu.flags.zf)
    {
        goto L_0x0049a925;
    }
L_0x0049a93d:
    // 0049a93d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a93e  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0049a943  bf31f97900             -mov edi, 0x79f931
    cpu.edi = 7993649 /*0x79f931*/;
    // 0049a948  e8d3ebf9ff             -call 0x439520
    cpu.esp -= 4;
    nfs3hp::load::carId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a94d  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049a94f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049a951  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049a952  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049a954  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0049a957  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049a959  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0049a95b  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0049a95e  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0049a960  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049a961  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0049a966  bb0f000000             -mov ebx, 0xf
    cpu.ebx = 15 /*0xf*/;
    // 0049a96b  e840f1f9ff             -call 0x439ab0
    cpu.esp -= 4;
    nfs3hp::port::fields::carText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a970  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0049a972  b841f97900             -mov eax, 0x79f941
    cpu.eax = 7993665 /*0x79f941*/;
    // 0049a977  e8b4640400             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a97c  30d2                   -xor dl, dl
    cpu.dl ^= x86::reg8(x86::sreg8(cpu.dl));
    // 0049a97e  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0049a983  881550f97900           -mov byte ptr [0x79f950], dl
    app->getMemory<x86::reg8>(x86::reg32(7993680) /* 0x79f950 */) = cpu.dl;
    // 0049a989  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0049a98e  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 0049a993  e818f1f9ff             -call 0x439ab0
    cpu.esp -= 4;
    nfs3hp::port::fields::carText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a998  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0049a99a  b851f97900             -mov eax, 0x79f951
    cpu.eax = 7993681 /*0x79f951*/;
    // 0049a99f  e88c640400             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a9a4  30f6                   -xor dh, dh
    cpu.dh ^= x86::reg8(x86::sreg8(cpu.dh));
    // 0049a9a6  883560f97900           -mov byte ptr [0x79f960], dh
    app->getMemory<x86::reg8>(x86::reg32(7993696) /* 0x79f960 */) = cpu.dh;
    // 0049a9ac  e8af79fdff             -call 0x472360
    cpu.esp -= 4;
    nfs3hp::port::fields::refreshCustomColours(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a9b1  a007d36f00             -mov al, byte ptr [0x6fd307]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328519) /* 0x6fd307 */);
    // 0049a9b6  a261f97900             -mov byte ptr [0x79f961], al
    app->getMemory<x86::reg8>(x86::reg32(7993697) /* 0x79f961 */) = cpu.al;
    // 0049a9bb  a008d36f00             -mov al, byte ptr [0x6fd308]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328520) /* 0x6fd308 */);
    // 0049a9c0  a262f97900             -mov byte ptr [0x79f962], al
    app->getMemory<x86::reg8>(x86::reg32(7993698) /* 0x79f962 */) = cpu.al;
    // 0049a9c5  a009d36f00             -mov al, byte ptr [0x6fd309]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328521) /* 0x6fd309 */);
    // 0049a9ca  a263f97900             -mov byte ptr [0x79f963], al
    app->getMemory<x86::reg8>(x86::reg32(7993699) /* 0x79f963 */) = cpu.al;
    // 0049a9cf  a00ad36f00             -mov al, byte ptr [0x6fd30a]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328522) /* 0x6fd30a */);
    // 0049a9d4  a264f97900             -mov byte ptr [0x79f964], al
    app->getMemory<x86::reg8>(x86::reg32(7993700) /* 0x79f964 */) = cpu.al;
    // 0049a9d9  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0049a9de  e86df0f9ff             -call 0x439a50
    cpu.esp -= 4;
    nfs3hp::load::carRecord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049a9e3  8b8080030000           -mov eax, dword ptr [eax + 0x380]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(896) /* 0x380 */);
    // 0049a9e9  a368f97900             -mov dword ptr [0x79f968], eax
    app->getMemory<x86::reg32>(x86::reg32(7993704) /* 0x79f968 */) = cpu.eax;
    // 0049a9ee  a0bcd26f00             -mov al, byte ptr [0x6fd2bc]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0049a9f3  a22cf97900             -mov byte ptr [0x79f92c], al
    app->getMemory<x86::reg8>(x86::reg32(7993644) /* 0x79f92c */) = cpu.al;
    // 0049a9f8  a0d4d26f00             -mov al, byte ptr [0x6fd2d4]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328468) /* 0x6fd2d4 */);
    // 0049a9fd  a22df97900             -mov byte ptr [0x79f92d], al
    app->getMemory<x86::reg8>(x86::reg32(7993645) /* 0x79f92d */) = cpu.al;
    // 0049aa02  a0dcd26f00             -mov al, byte ptr [0x6fd2dc]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328476) /* 0x6fd2dc */);
    // 0049aa07  a22ef97900             -mov byte ptr [0x79f92e], al
    app->getMemory<x86::reg8>(x86::reg32(7993646) /* 0x79f92e */) = cpu.al;
    // 0049aa0c  a0e0d26f00             -mov al, byte ptr [0x6fd2e0]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328480) /* 0x6fd2e0 */);
    // 0049aa11  a22ff97900             -mov byte ptr [0x79f92f], al
    app->getMemory<x86::reg8>(x86::reg32(7993647) /* 0x79f92f */) = cpu.al;
    // 0049aa16  a0e4d26f00             -mov al, byte ptr [0x6fd2e4]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328484) /* 0x6fd2e4 */);
    // 0049aa1b  a230f97900             -mov byte ptr [0x79f930], al
    app->getMemory<x86::reg8>(x86::reg32(7993648) /* 0x79f930 */) = cpu.al;
    // 0049aa20  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0049aa25  bf6cf97900             -mov edi, 0x79f96c
    cpu.edi = 7993708 /*0x79f96c*/;
    // 0049aa2a  e8e1eaf9ff             -call 0x439510
    cpu.esp -= 4;
    sub_439510(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049aa2f  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049aa31  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049aa32  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049aa33  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049aa34  8b151cf97900           -mov edx, dword ptr [0x79f91c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7993628) /* 0x79f91c */);
    // 0049aa3a  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0049aa41  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0049aa43  baf0f57900             -mov edx, 0x79f5f0
    cpu.edx = 7992816 /*0x79f5f0*/;
    // 0049aa48  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049aa4b  bb60000000             -mov ebx, 0x60
    cpu.ebx = 96 /*0x60*/;
    // 0049aa50  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049aa52  b818f97900             -mov eax, 0x79f918
    cpu.eax = 7993624 /*0x79f918*/;
    // 0049aa57  e894fa0400             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049aa5c  bb60000000             -mov ebx, 0x60
    cpu.ebx = 96 /*0x60*/;
    // 0049aa61  ba18f97900             -mov edx, 0x79f918
    cpu.edx = 7993624 /*0x79f918*/;
    // 0049aa66  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049aa68  e873b60000             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049aa6d:
    // 0049aa6d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049aa6e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049aa6f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049aa70  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049aa71  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049aa72  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049aa73  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_49aa80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049aa80  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049aa81  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049aa82  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049aa83  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049aa84  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049aa85  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049aa87  c605f0f8790002         -mov byte ptr [0x79f8f0], 2
    app->getMemory<x86::reg8>(x86::reg32(7993584) /* 0x79f8f0 */) = 2 /*0x2*/;
    // 0049aa8e  a0b8d36f00             -mov al, byte ptr [0x6fd3b8]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0049aa93  a2f1f87900             -mov byte ptr [0x79f8f1], al
    app->getMemory<x86::reg8>(x86::reg32(7993585) /* 0x79f8f1 */) = cpu.al;
    // 0049aa98  a0acd46f00             -mov al, byte ptr [0x6fd4ac]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 0049aa9d  a2f2f87900             -mov byte ptr [0x79f8f2], al
    app->getMemory<x86::reg8>(x86::reg32(7993586) /* 0x79f8f2 */) = cpu.al;
    // 0049aaa2  a004d56f00             -mov al, byte ptr [0x6fd504]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7329028) /* 0x6fd504 */);
    // 0049aaa7  a2f3f87900             -mov byte ptr [0x79f8f3], al
    app->getMemory<x86::reg8>(x86::reg32(7993587) /* 0x79f8f3 */) = cpu.al;
    // 0049aaac  a0b4d46f00             -mov al, byte ptr [0x6fd4b4]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328948) /* 0x6fd4b4 */);
    // 0049aab1  a2f4f87900             -mov byte ptr [0x79f8f4], al
    app->getMemory<x86::reg8>(x86::reg32(7993588) /* 0x79f8f4 */) = cpu.al;
    // 0049aab6  a0b0d46f00             -mov al, byte ptr [0x6fd4b0]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328944) /* 0x6fd4b0 */);
    // 0049aabb  a2f5f87900             -mov byte ptr [0x79f8f5], al
    app->getMemory<x86::reg8>(x86::reg32(7993589) /* 0x79f8f5 */) = cpu.al;
    // 0049aac0  a0d4d46f00             -mov al, byte ptr [0x6fd4d4]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 0049aac5  a2f6f87900             -mov byte ptr [0x79f8f6], al
    app->getMemory<x86::reg8>(x86::reg32(7993590) /* 0x79f8f6 */) = cpu.al;
    // 0049aaca  a0d0d46f00             -mov al, byte ptr [0x6fd4d0]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328976) /* 0x6fd4d0 */);
    // 0049aacf  a208f97900             -mov byte ptr [0x79f908], al
    app->getMemory<x86::reg8>(x86::reg32(7993608) /* 0x79f908 */) = cpu.al;
    // 0049aad4  a0e8d46f00             -mov al, byte ptr [0x6fd4e8]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7329000) /* 0x6fd4e8 */);
    // 0049aad9  a209f97900             -mov byte ptr [0x79f909], al
    app->getMemory<x86::reg8>(x86::reg32(7993609) /* 0x79f909 */) = cpu.al;
    // 0049aade  a0c4d46f00             -mov al, byte ptr [0x6fd4c4]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328964) /* 0x6fd4c4 */);
    // 0049aae3  a20af97900             -mov byte ptr [0x79f90a], al
    app->getMemory<x86::reg8>(x86::reg32(7993610) /* 0x79f90a */) = cpu.al;
    // 0049aae8  a0ccd46f00             -mov al, byte ptr [0x6fd4cc]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328972) /* 0x6fd4cc */);
    // 0049aaed  a20bf97900             -mov byte ptr [0x79f90b], al
    app->getMemory<x86::reg8>(x86::reg32(7993611) /* 0x79f90b */) = cpu.al;
    // 0049aaf2  a0c8d46f00             -mov al, byte ptr [0x6fd4c8]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328968) /* 0x6fd4c8 */);
    // 0049aaf7  a20cf97900             -mov byte ptr [0x79f90c], al
    app->getMemory<x86::reg8>(x86::reg32(7993612) /* 0x79f90c */) = cpu.al;
    // 0049aafc  a0c0d46f00             -mov al, byte ptr [0x6fd4c0]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328960) /* 0x6fd4c0 */);
    // 0049ab01  a20df97900             -mov byte ptr [0x79f90d], al
    app->getMemory<x86::reg8>(x86::reg32(7993613) /* 0x79f90d */) = cpu.al;
    // 0049ab06  a1b4d36f00             -mov eax, dword ptr [0x6fd3b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328692) /* 0x6fd3b4 */);
    // 0049ab0b  a310f97900             -mov dword ptr [0x79f910], eax
    app->getMemory<x86::reg32>(x86::reg32(7993616) /* 0x79f910 */) = cpu.eax;
    // 0049ab10  a070d46f00             -mov al, byte ptr [0x6fd470]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7328880) /* 0x6fd470 */);
    // 0049ab15  8a15f6f87900           -mov dl, byte ptr [0x79f8f6]
    cpu.dl = app->getMemory<x86::reg8>(x86::reg32(7993590) /* 0x79f8f6 */);
    // 0049ab1b  a214f97900             -mov byte ptr [0x79f914], al
    app->getMemory<x86::reg8>(x86::reg32(7993620) /* 0x79f914 */) = cpu.al;
    // 0049ab20  80fa04                 +cmp dl, 4
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
    // 0049ab23  7217                   -jb 0x49ab3c
    if (cpu.flags.cf)
    {
        goto L_0x0049ab3c;
    }
    // 0049ab25  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049ab27  88d0                   -mov al, dl
    cpu.al = cpu.dl;
    // 0049ab29  83e804                 -sub eax, 4
    (cpu.eax) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0049ab2c  bff8f87900             -mov edi, 0x79f8f8
    cpu.edi = 7993592 /*0x79f8f8*/;
    // 0049ab31  e8eae9f9ff             -call 0x439520
    cpu.esp -= 4;
    nfs3hp::load::carId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ab36  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049ab38  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049ab39  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049ab3a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049ab3b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x0049ab3c:
    // 0049ab3c  bb28000000             -mov ebx, 0x28
    cpu.ebx = 40 /*0x28*/;
    // 0049ab41  baf0f87900             -mov edx, 0x79f8f0
    cpu.edx = 7993584 /*0x79f8f0*/;
    // 0049ab46  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049ab48  e893b50000             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ab4d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ab4e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ab4f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ab50  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ab51  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ab52  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49ab60: native, nfs3hp::frontend::allPlayersReady */

/* align: skip 0x8b 0xc0 */
void Application::sub_49abd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049abd0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049abd1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049abd2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049abd3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049abd4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049abd6  833de0227a0000         +cmp dword ptr [0x7a22e0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004320) /* 0x7a22e0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049abdd  0f84e2000000           -je 0x49acc5
    if (cpu.flags.zf)
    {
        goto L_0x0049acc5;
    }
    // 0049abe3  66833d76227a0002       +cmp word ptr [0x7a2276], 2
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004214) /* 0x7a2276 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2 /*0x2*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049abeb  7d07                   -jge 0x49abf4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049abf4;
    }
    // 0049abed  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049abef  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049abf0  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049abf1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049abf2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049abf3  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049abf4:
    // 0049abf4  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049abfc  0f846c000000           -je 0x49ac6e
    if (cpu.flags.zf)
    {
        goto L_0x0049ac6e;
    }
    // 0049ac02  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0049ac04:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049ac04  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 0049ac09  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049ac0c  39c2                   +cmp edx, eax
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
    // 0049ac0e  7d32                   -jge 0x49ac42
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049ac42;
    }
    // 0049ac10  0fbfc2                 -movsx eax, dx
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.dx));
    // 0049ac13  e878ba0000             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ac18  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0049ac19  83f8ff                 +cmp eax, -1
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
    // 0049ac1c  7507                   -jne 0x49ac25
    if (!cpu.flags.zf)
    {
        goto L_0x0049ac25;
    }
    // 0049ac1e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049ac20  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac21  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac22  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac23  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac24  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049ac25:
    // 0049ac25  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0049ac27  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049ac2a  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049ac2c  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049ac2f  80b8f8f5790000         +cmp byte ptr [eax + 0x79f5f8], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(7992824) /* 0x79f5f8 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0049ac36  7507                   -jne 0x49ac3f
    if (!cpu.flags.zf)
    {
        goto L_0x0049ac3f;
    }
    // 0049ac38  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0049ac3a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac3b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac3c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac3d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac3e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049ac3f:
    // 0049ac3f  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049ac40  ebc2                   -jmp 0x49ac04
    goto L_0x0049ac04;
L_0x0049ac42:
    // 0049ac42  c70510e7550001000000   -mov dword ptr [0x55e710], 1
    app->getMemory<x86::reg32>(x86::reg32(5629712) /* 0x55e710 */) = 1 /*0x1*/;
    // 0049ac4c  e82f4a0000             -call 0x49f680
    cpu.esp -= 4;
    sub_49f680(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ac51  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049ac53  740a                   -je 0x49ac5f
    if (cpu.flags.zf)
    {
        goto L_0x0049ac5f;
    }
    // 0049ac55  66833d9a227a0000       +cmp word ptr [0x7a229a], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004250) /* 0x7a229a */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049ac5d  7420                   -je 0x49ac7f
    if (cpu.flags.zf)
    {
        goto L_0x0049ac7f;
    }
L_0x0049ac5f:
    // 0049ac5f  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049ac61  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049ac63  891d10e75500           -mov dword ptr [0x55e710], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629712) /* 0x55e710 */) = cpu.ebx;
    // 0049ac69  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac6a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac6b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac6c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac6d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049ac6e:
    // 0049ac6e  66833d9c227a0001       +cmp word ptr [0x7a229c], 1
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004252) /* 0x7a229c */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1 /*0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049ac76  7407                   -je 0x49ac7f
    if (cpu.flags.zf)
    {
        goto L_0x0049ac7f;
    }
    // 0049ac78  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049ac7a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac7b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac7c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac7d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ac7e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049ac7f:
    // 0049ac7f  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049ac87  740a                   -je 0x49ac93
    if (cpu.flags.zf)
    {
        goto L_0x0049ac93;
    }
    // 0049ac89  e8c2410000             -call 0x49ee50
    cpu.esp -= 4;
    sub_49ee50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ac8e  e8edfdffff             -call 0x49aa80
    cpu.esp -= 4;
    sub_49aa80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049ac93:
    // 0049ac93  e848fcffff             -call 0x49a8e0
    cpu.esp -= 4;
    sub_49a8e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ac98  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 0049ac9d  b8ca227a00             -mov eax, 0x7a22ca
    cpu.eax = 8004298 /*0x7a22ca*/;
    // 0049aca2  e8655a0400             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049aca7  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0049acac  b83c000000             -mov eax, 0x3c
    cpu.eax = 60 /*0x3c*/;
    // 0049acb1  e85abd0000             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049acb6  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0049acbb  b81e000000             -mov eax, 0x1e
    cpu.eax = 30 /*0x1e*/;
    // 0049acc0  e84bbd0000             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049acc5:
    // 0049acc5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049acca  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049accb  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049accc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049accd  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049acce  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_49acd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049acd0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049acd1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049acd2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049acd3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049acd4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049acd5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049acd6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049acd8  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0049acdb  e8d00d0000             -call 0x49bab0
    cpu.esp -= 4;
    sub_49bab0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ace0  ba1f000000             -mov edx, 0x1f
    cpu.edx = 31 /*0x1f*/;
    // 0049ace5  b81e000000             -mov eax, 0x1e
    cpu.eax = 30 /*0x1e*/;
    // 0049acea  e821bd0000             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049acef  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0049acf1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049acf1  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 0049acf6  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049acf9  39c1                   +cmp ecx, eax
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
    // 0049acfb  0f8df0000000           -jge 0x49adf1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049adf1;
    }
    // 0049ad01  0fbfc1                 -movsx eax, cx
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.cx));
    // 0049ad04  e887b90000             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ad09  0fbfd8                 -movsx ebx, ax
    cpu.ebx = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0049ad0c  83fbff                 +cmp ebx, -1
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
    // 0049ad0f  0f84d6000000           -je 0x49adeb
    if (cpu.flags.zf)
    {
        goto L_0x0049adeb;
    }
    // 0049ad15  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0049ad17  c1e302                 -shl ebx, 2
    cpu.ebx <<= 2 /*0x2*/ % 32;
    // 0049ad1a  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0049ad1c  b8f0f57900             -mov eax, 0x79f5f0
    cpu.eax = 7992816 /*0x79f5f0*/;
    // 0049ad21  c1e305                 -shl ebx, 5
    cpu.ebx <<= 5 /*0x5*/ % 32;
    // 0049ad24  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0049ad26  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0049ad29  8d7009                 -lea esi, [eax + 9]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(9) /* 0x9 */);
    // 0049ad2c  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0049ad33  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049ad35  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049ad38  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0049ad3a  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0049ad3d  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049ad3f  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 0049ad44  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0049ad46  8d7834                 -lea edi, [eax + 0x34]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(52) /* 0x34 */);
    // 0049ad49  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0049ad4a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049ad4a  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049ad4c  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049ad4e  3c00                   +cmp al, 0
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
    // 0049ad50  7410                   -je 0x49ad62
    if (cpu.flags.zf)
    {
        goto L_0x0049ad62;
    }
    // 0049ad52  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049ad55  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049ad58  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049ad5b  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049ad5e  3c00                   +cmp al, 0
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
    // 0049ad60  75e8                   -jne 0x49ad4a
    if (!cpu.flags.zf)
    {
        goto L_0x0049ad4a;
    }
L_0x0049ad62:
    // 0049ad62  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ad63  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049ad66  83c019                 -add eax, 0x19
    (cpu.eax) += x86::reg32(x86::sreg32(25 /*0x19*/));
    // 0049ad69  e822ecf9ff             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ad6e  898240d56f00           -mov dword ptr [edx + 0x6fd540], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329088) /* 0x6fd540 */) = cpu.eax;
    // 0049ad74  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049ad76  8a8305f67900           -mov al, byte ptr [ebx + 0x79f605]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(7992837) /* 0x79f605 */);
    // 0049ad7c  898248d56f00           -mov dword ptr [edx + 0x6fd548], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329096) /* 0x6fd548 */) = cpu.eax;
    // 0049ad82  8a8339f67900           -mov al, byte ptr [ebx + 0x79f639]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(7992889) /* 0x79f639 */);
    // 0049ad88  88824cd56f00           -mov byte ptr [edx + 0x6fd54c], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(7329100) /* 0x6fd54c */) = cpu.al;
    // 0049ad8e  8a833af67900           -mov al, byte ptr [ebx + 0x79f63a]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(7992890) /* 0x79f63a */);
    // 0049ad94  88824dd56f00           -mov byte ptr [edx + 0x6fd54d], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(7329101) /* 0x6fd54d */) = cpu.al;
    // 0049ad9a  8a833bf67900           -mov al, byte ptr [ebx + 0x79f63b]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(7992891) /* 0x79f63b */);
    // 0049ada0  88824ed56f00           -mov byte ptr [edx + 0x6fd54e], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(7329102) /* 0x6fd54e */) = cpu.al;
    // 0049ada6  8a833cf67900           -mov al, byte ptr [ebx + 0x79f63c]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(7992892) /* 0x79f63c */);
    // 0049adac  88824fd56f00           -mov byte ptr [edx + 0x6fd54f], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(7329103) /* 0x6fd54f */) = cpu.al;
    // 0049adb2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049adb4  8a8306f67900           -mov al, byte ptr [ebx + 0x79f606]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(7992838) /* 0x79f606 */);
    // 0049adba  898250d56f00           -mov dword ptr [edx + 0x6fd550], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329104) /* 0x6fd550 */) = cpu.eax;
    // 0049adc0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049adc2  8a8307f67900           -mov al, byte ptr [ebx + 0x79f607]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(7992839) /* 0x79f607 */);
    // 0049adc8  898254d56f00           -mov dword ptr [edx + 0x6fd554], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329108) /* 0x6fd554 */) = cpu.eax;
    // 0049adce  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0049add0  8dba88d56f00           -lea edi, [edx + 0x6fd588]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(7329160) /* 0x6fd588 */);
    // 0049add6  8a8308f67900           -mov al, byte ptr [ebx + 0x79f608]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(7992840) /* 0x79f608 */);
    // 0049addc  8db344f67900           -lea esi, [ebx + 0x79f644]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(7992900) /* 0x79f644 */);
    // 0049ade2  898258d56f00           -mov dword ptr [edx + 0x6fd558], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329112) /* 0x6fd558 */) = cpu.eax;
    // 0049ade8  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049ade9  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049adea  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x0049adeb:
    // 0049adeb  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049adec  e900ffffff             -jmp 0x49acf1
    goto L_0x0049acf1;
L_0x0049adf1:
    // 0049adf1  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 0049adf3  eb05                   -jmp 0x49adfa
    goto L_0x0049adfa;
L_0x0049adf5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049adf5  83f908                 +cmp ecx, 8
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049adf8  7d21                   -jge 0x49ae1b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049ae1b;
    }
L_0x0049adfa:
    // 0049adfa  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0049adfc  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0049ae03  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049ae05  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049ae08  30ca                   -xor dl, cl
    cpu.dl ^= x86::reg8(x86::sreg8(cpu.cl));
    // 0049ae0a  8890f8f57900           -mov byte ptr [eax + 0x79f5f8], dl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(7992824) /* 0x79f5f8 */) = cpu.dl;
    // 0049ae10  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0049ae12  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049ae13  8990f4f57900           -mov dword ptr [eax + 0x79f5f4], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7992820) /* 0x79f5f4 */) = cpu.edx;
    // 0049ae19  ebda                   -jmp 0x49adf5
    goto L_0x0049adf5;
L_0x0049ae1b:
    // 0049ae1b  e8f0b0faff             -call 0x445f10
    cpu.esp -= 4;
    nfs3hp::port::dialogs::closeDialog(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ae20  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049ae22  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ae23  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ae24  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ae25  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ae26  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ae27  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ae28  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49ae30: native, nfs3hp::frontend::lobbyDrawPlayers */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_49b2b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049b2b0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049b2b1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049b2b2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049b2b3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049b2b4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049b2b6  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0049b2b8  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 0049b2c0  7540                   -jne 0x49b302
    if (!cpu.flags.zf)
    {
        goto L_0x0049b302;
    }
    // 0049b2c2  8b15b8e55500           -mov edx, dword ptr [0x55e5b8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629368) /* 0x55e5b8 */);
    // 0049b2c8  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049b2c9  bff0f87900             -mov edi, 0x79f8f0
    cpu.edi = 7993584 /*0x79f8f0*/;
    // 0049b2ce  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049b2d4  b928000000             -mov ecx, 0x28
    cpu.ecx = 40 /*0x28*/;
    // 0049b2d9  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049b2da  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049b2dc  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0049b2df  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049b2e1  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0049b2e3  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0049b2e6  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0049b2e8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b2e9  8b0db8e55500           -mov ecx, dword ptr [0x55e5b8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629368) /* 0x55e5b8 */);
    // 0049b2ef  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0049b2f4  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049b2f5  66891d98227a00         -mov word ptr [0x7a2298], bx
    app->getMemory<x86::reg16>(x86::reg32(8004248) /* 0x7a2298 */) = cpu.bx;
    // 0049b2fc  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049b302:
    // 0049b302  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b303  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b304  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b305  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b306  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_49b310(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049b310  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049b311  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049b312  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049b313  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049b314  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049b316  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0049b319  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0049b31b  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0049b31e  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 0049b326  0f85ac000000           -jne 0x49b3d8
    if (!cpu.flags.zf)
    {
        goto L_0x0049b3d8;
    }
    // 0049b32c  8b15b4ec5500           -mov edx, dword ptr [0x55ecb4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */);
    // 0049b332  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049b333  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049b339  66833d78227a0000       +cmp word ptr [0x7a2278], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004216) /* 0x7a2278 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049b341  7c22                   -jl 0x49b365
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0049b365;
    }
    // 0049b343  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 0049b348  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049b34b  e840b30000             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049b350  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0049b351  3b4604                 +cmp eax, dword ptr [esi + 4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049b354  750f                   -jne 0x49b365
    if (!cpu.flags.zf)
    {
        goto L_0x0049b365;
    }
    // 0049b356  8b1db4ec5500           -mov ebx, dword ptr [0x55ecb4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */);
    // 0049b35c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049b35d  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049b363  eb73                   -jmp 0x49b3d8
    goto L_0x0049b3d8;
L_0x0049b365:
    // 0049b365  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049b368  8b5204                 -mov edx, dword ptr [edx + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 0049b36b  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0049b372  bff0f57900             -mov edi, 0x79f5f0
    cpu.edi = 7992816 /*0x79f5f0*/;
    // 0049b377  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0049b379  b960000000             -mov ecx, 0x60
    cpu.ecx = 96 /*0x60*/;
    // 0049b37e  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049b381  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049b384  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049b386  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049b387  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049b389  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0049b38c  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049b38e  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0049b390  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0049b393  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0049b395  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b396  8b35b4ec5500           -mov esi, dword ptr [0x55ecb4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */);
    // 0049b39c  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0049b3a1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049b3a2  66890d9a227a00         -mov word ptr [0x7a229a], cx
    app->getMemory<x86::reg16>(x86::reg32(8004250) /* 0x7a229a */) = cpu.cx;
    // 0049b3a9  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049b3af  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049b3b7  741f                   -je 0x49b3d8
    if (cpu.flags.zf)
    {
        goto L_0x0049b3d8;
    }
    // 0049b3b9  833de4227a0002         +cmp dword ptr [0x7a22e4], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004324) /* 0x7a22e4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049b3c0  7507                   -jne 0x49b3c9
    if (!cpu.flags.zf)
    {
        goto L_0x0049b3c9;
    }
    // 0049b3c2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049b3c4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b3c5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b3c6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b3c7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b3c8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049b3c9:
    // 0049b3c9  bb60000000             -mov ebx, 0x60
    cpu.ebx = 96 /*0x60*/;
    // 0049b3ce  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049b3d1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049b3d3  e808ad0000             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049b3d8:
    // 0049b3d8  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049b3da  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b3db  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b3dc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b3dd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b3de  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_49b3e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049b3e0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049b3e1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049b3e2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049b3e3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049b3e4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049b3e6  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0049b3e9  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0049b3ec  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049b3ee  8915bce55500           -mov dword ptr [0x55e5bc], edx
    app->getMemory<x86::reg32>(x86::reg32(5629372) /* 0x55e5bc */) = cpu.edx;
    // 0049b3f4  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049b3fc  750c                   -jne 0x49b40a
    if (!cpu.flags.zf)
    {
        goto L_0x0049b40a;
    }
    // 0049b3fe  c705bce5550001000000   -mov dword ptr [0x55e5bc], 1
    app->getMemory<x86::reg32>(x86::reg32(5629372) /* 0x55e5bc */) = 1 /*0x1*/;
    // 0049b408  eb5a                   -jmp 0x49b464
    goto L_0x0049b464;
L_0x0049b40a:
    // 0049b40a  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049b40d  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 0049b412  42                     -inc edx
    (cpu.edx)++;
    // 0049b413  bfe0f57900             -mov edi, 0x79f5e0
    cpu.edi = 7992800 /*0x79f5e0*/;
    // 0049b418  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0049b41a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049b41b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049b41d  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0049b420  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049b422  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0049b424  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0049b427  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0049b429  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b42a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049b42d  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 0049b430  a3a8fa7900             -mov dword ptr [0x79faa8], eax
    app->getMemory<x86::reg32>(x86::reg32(7994024) /* 0x79faa8 */) = cpu.eax;
    // 0049b435  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049b437  e854e5f9ff             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049b43c  83f8ff                 +cmp eax, -1
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
    // 0049b43f  7411                   -je 0x49b452
    if (cpu.flags.zf)
    {
        goto L_0x0049b452;
    }
    // 0049b441  c705bce5550001000000   -mov dword ptr [0x55e5bc], 1
    app->getMemory<x86::reg32>(x86::reg32(5629372) /* 0x55e5bc */) = 1 /*0x1*/;
    // 0049b44b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049b44d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b44e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b44f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b450  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b451  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049b452:
    // 0049b452  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049b455  b8bce55500             -mov eax, 0x55e5bc
    cpu.eax = 5629372 /*0x55e5bc*/;
    // 0049b45a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049b45c  8b5b18                 -mov ebx, dword ptr [ebx + 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */);
    // 0049b45f  e8dc000000             -call 0x49b540
    cpu.esp -= 4;
    nfs3hp::load::beginCarpReceive(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049b464:
    // 0049b464  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049b466  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b467  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b468  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b469  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b46a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_49b470(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049b470  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049b471  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049b472  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049b473  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049b474  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049b476  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0049b479  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0049b47c  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049b47e  8915bce55500           -mov dword ptr [0x55e5bc], edx
    app->getMemory<x86::reg32>(x86::reg32(5629372) /* 0x55e5bc */) = cpu.edx;
    // 0049b484  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049b48c  740c                   -je 0x49b49a
    if (cpu.flags.zf)
    {
        goto L_0x0049b49a;
    }
    // 0049b48e  c705bce5550001000000   -mov dword ptr [0x55e5bc], 1
    app->getMemory<x86::reg32>(x86::reg32(5629372) /* 0x55e5bc */) = 1 /*0x1*/;
    // 0049b498  eb5a                   -jmp 0x49b4f4
    goto L_0x0049b4f4;
L_0x0049b49a:
    // 0049b49a  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049b49d  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 0049b4a2  42                     -inc edx
    (cpu.edx)++;
    // 0049b4a3  bfe0f57900             -mov edi, 0x79f5e0
    cpu.edi = 7992800 /*0x79f5e0*/;
    // 0049b4a8  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0049b4aa  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049b4ab  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049b4ad  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0049b4b0  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049b4b2  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0049b4b4  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0049b4b7  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0049b4b9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b4ba  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049b4bd  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 0049b4c0  a3a8fa7900             -mov dword ptr [0x79faa8], eax
    app->getMemory<x86::reg32>(x86::reg32(7994024) /* 0x79faa8 */) = cpu.eax;
    // 0049b4c5  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049b4c7  e8c4e4f9ff             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049b4cc  83f8ff                 +cmp eax, -1
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
    // 0049b4cf  7411                   -je 0x49b4e2
    if (cpu.flags.zf)
    {
        goto L_0x0049b4e2;
    }
    // 0049b4d1  c705bce5550001000000   -mov dword ptr [0x55e5bc], 1
    app->getMemory<x86::reg32>(x86::reg32(5629372) /* 0x55e5bc */) = 1 /*0x1*/;
    // 0049b4db  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049b4dd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b4de  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b4df  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b4e0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b4e1  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049b4e2:
    // 0049b4e2  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049b4e5  b8bce55500             -mov eax, 0x55e5bc
    cpu.eax = 5629372 /*0x55e5bc*/;
    // 0049b4ea  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049b4ec  8b5b18                 -mov ebx, dword ptr [ebx + 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */);
    // 0049b4ef  e84c000000             -call 0x49b540
    cpu.esp -= 4;
    nfs3hp::load::beginCarpReceive(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049b4f4:
    // 0049b4f4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049b4f6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b4f7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b4f8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b4f9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b4fa  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_49b500(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049b500  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049b501  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049b503  b8bce55500             -mov eax, 0x55e5bc
    cpu.eax = 5629372 /*0x55e5bc*/;
    // 0049b508  4b                     -dec ebx
    (cpu.ebx)--;
    // 0049b509  42                     -inc edx
    (cpu.edx)++;
    // 0049b50a  e8e1000000             -call 0x49b5f0
    cpu.esp -= 4;
    nfs3hp::load::receiveCarpChunk(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049b50f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b510  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_49b520(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049b520  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049b521  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049b523  b8bce55500             -mov eax, 0x55e5bc
    cpu.eax = 5629372 /*0x55e5bc*/;
    // 0049b528  4b                     -dec ebx
    (cpu.ebx)--;
    // 0049b529  42                     -inc edx
    (cpu.edx)++;
    // 0049b52a  e8c1000000             -call 0x49b5f0
    cpu.esp -= 4;
    nfs3hp::load::receiveCarpChunk(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049b52f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049b530  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49b540: native, nfs3hp::load::beginCarpReceive */

/* sub_49b5f0: native, nfs3hp::load::receiveCarpChunk */

/* sub_49b800: native, nfs3hp::load::beginCarpSend */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_49b9b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049b9b0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049b9b1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049b9b2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049b9b3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049b9b4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049b9b6  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0049b9b9  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 0049b9bc  8955e8                 -mov dword ptr [ebp - 0x18], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.edx;
    // 0049b9bf  bad0000000             -mov edx, 0xd0
    cpu.edx = 208 /*0xd0*/;
    // 0049b9c4  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0049b9c6  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
L_0x0049b9c9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049b9c9  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0049b9cb  7e46                   -jle 0x49ba13
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0049ba13;
    }
    // 0049b9cd  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049b9d0  39cf                   +cmp edi, ecx
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049b9d2  7d04                   -jge 0x49b9d8
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049b9d8;
    }
    // 0049b9d4  89fe                   -mov esi, edi
    cpu.esi = cpu.edi;
    // 0049b9d6  eb02                   -jmp 0x49b9da
    goto L_0x0049b9da;
L_0x0049b9d8:
    // 0049b9d8  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
L_0x0049b9da:
    // 0049b9da  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049b9dd  8b4de8                 -mov ecx, dword ptr [ebp - 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0049b9e0  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0049b9e5  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0049b9e7  40                     -inc eax
    (cpu.eax)++;
    // 0049b9e8  e813d80400             -call 0x4e9200
    cpu.esp -= 4;
    nfs3hp::watcom::fread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049b9ed  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049b9ef  750a                   -jne 0x49b9fb
    if (!cpu.flags.zf)
    {
        goto L_0x0049b9fb;
    }
    // 0049b9f1  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0049b9f6  e9ab000000             -jmp 0x49baa6
    goto L_0x0049baa6;
L_0x0049b9fb:
    // 0049b9fb  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049b9fd  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049ba00  e82bbc0400             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ba05  8d5e01                 -lea ebx, [esi + 1]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049ba08  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049ba0a  e8d1a60000             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ba0f  29f7                   +sub edi, esi
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0049ba11  ebb6                   -jmp 0x49b9c9
    goto L_0x0049b9c9;
L_0x0049ba13:
    // 0049ba13  8b1d78f97900           -mov ebx, dword ptr [0x79f978]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7993720) /* 0x79f978 */);
    // 0049ba19  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0049ba1b  740c                   -je 0x49ba29
    if (cpu.flags.zf)
    {
        goto L_0x0049ba29;
    }
    // 0049ba1d  8d55e8                 -lea edx, [ebp - 0x18]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0049ba20  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0049ba22  e889fff9ff             -call 0x43b9b0
    cpu.esp -= 4;
    nfs3hp::ea::closeViv(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ba27  eb08                   -jmp 0x49ba31
    goto L_0x0049ba31;
L_0x0049ba29:
    // 0049ba29  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0049ba2c  e8cf260500             -call 0x4ee100
    cpu.esp -= 4;
    nfs3hp::watcom::fclose(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049ba31:
    // 0049ba31  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0049ba33  be2c010000             -mov esi, 0x12c
    cpu.esi = 300 /*0x12c*/;
    // 0049ba38  897dec                 -mov dword ptr [ebp - 0x14], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edi;
    // 0049ba3b  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 0049ba3e  eb55                   -jmp 0x49ba95
    goto L_0x0049ba95;
L_0x0049ba40:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049ba40  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
L_0x0049ba43:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049ba43  be7cf97900             -mov esi, 0x79f97c
    cpu.esi = 7993724 /*0x79f97c*/;
    // 0049ba48  8b7df0                 -mov edi, dword ptr [ebp - 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049ba4b  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049ba4e  47                     -inc edi
    (cpu.edi)++;
    // 0049ba4f  01ce                   -add esi, ecx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0049ba51  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049ba54  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049ba57  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049ba58  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049ba5a  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0049ba5d  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049ba5f  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0049ba61  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0049ba64  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0049ba66  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ba67  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049ba6a  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049ba6d  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049ba6f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049ba71  895dec                 -mov dword ptr [ebp - 0x14], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.ebx;
    // 0049ba74  e8b7bb0400             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ba79  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049ba7c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049ba7e  43                     -inc ebx
    (cpu.ebx)++;
    // 0049ba7f  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049ba82  e859a60000             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ba87  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049ba8a  29c6                   -sub esi, eax
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0049ba8c  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 0049ba8f  837dfc00               +cmp dword ptr [ebp - 4], 0
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
    // 0049ba93  7e0f                   -jle 0x49baa4
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0049baa4;
    }
L_0x0049ba95:
    // 0049ba95  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049ba98  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049ba9b  39d0                   +cmp eax, edx
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
    // 0049ba9d  7da1                   -jge 0x49ba40
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049ba40;
    }
    // 0049ba9f  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0049baa2  eb9f                   -jmp 0x49ba43
    goto L_0x0049ba43;
L_0x0049baa4:
    // 0049baa4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0049baa6:
    // 0049baa6  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049baa8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049baa9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049baaa  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049baab  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049baac  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_49bab0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049bab0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049bab1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049bab2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049bab3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049bab4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049bab5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049bab6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049bab8  81ece0010000           -sub esp, 0x1e0
    (cpu.esp) -= x86::reg32(x86::sreg32(480 /*0x1e0*/));
    // 0049babe  81ed9a000000           -sub ebp, 0x9a
    (cpu.ebp) -= x86::reg32(x86::sreg32(154 /*0x9a*/));
    // 0049bac4  b815010000             -mov eax, 0x115
    cpu.eax = 277 /*0x115*/;
    // 0049bac9  e81291faff             -call 0x444be0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogMessageById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bace  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049bad0  e89bf6faff             -call 0x44b170
    cpu.esp -= 4;
    nfs3hp::port::buttons::redrawMenu(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bad5  41                     -inc ecx
    (cpu.ecx)++;
L_0x0049bad6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049bad6  83f90f                 +cmp ecx, 0xf
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(15 /*0xf*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049bad9  7d08                   -jge 0x49bae3
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049bae3;
    }
    // 0049badb  e890f6faff             -call 0x44b170
    cpu.esp -= 4;
    nfs3hp::port::buttons::redrawMenu(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bae0  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049bae1  ebf3                   -jmp 0x49bad6
    goto L_0x0049bad6;
L_0x0049bae3:
    // 0049bae3  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049baeb  7422                   -je 0x49bb0f
    if (cpu.flags.zf)
    {
        goto L_0x0049bb0f;
    }
    // 0049baed  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0049baef:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049baef  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 0049baf4  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049baf7  39c1                   +cmp ecx, eax
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
    // 0049baf9  0f8d8c000000           -jge 0x49bb8b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049bb8b;
    }
    // 0049baff  b83c000000             -mov eax, 0x3c
    cpu.eax = 60 /*0x3c*/;
    // 0049bb04  8d510f                 -lea edx, [ecx + 0xf]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(15) /* 0xf */);
    // 0049bb07  e804af0000             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bb0c  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049bb0d  ebe0                   -jmp 0x49baef
    goto L_0x0049baef;
L_0x0049bb0f:
    // 0049bb0f  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 0049bb14  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049bb17  e874ab0000             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bb1c  0fbff8                 -movsx edi, ax
    cpu.edi = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0049bb1f  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0049bb24  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049bb26  e8f5d9f9ff             -call 0x439520
    cpu.esp -= 4;
    nfs3hp::load::carId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bb2b  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
L_0x0049bb2d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049bb2d  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 0049bb32  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049bb35  39c1                   +cmp ecx, eax
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
    // 0049bb37  7d52                   -jge 0x49bb8b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049bb8b;
    }
    // 0049bb39  0fbfc1                 -movsx eax, cx
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.cx));
    // 0049bb3c  e84fab0000             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bb41  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0049bb42  83f8ff                 +cmp eax, -1
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
    // 0049bb45  7434                   -je 0x49bb7b
    if (cpu.flags.zf)
    {
        goto L_0x0049bb7b;
    }
    // 0049bb47  39f8                   +cmp eax, edi
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
    // 0049bb49  7530                   -jne 0x49bb7b
    if (!cpu.flags.zf)
    {
        goto L_0x0049bb7b;
    }
    // 0049bb4b  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0049bb4d  e8ded9f9ff             -call 0x439530
    cpu.esp -= 4;
    sub_439530(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bb52  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049bb54  7425                   -je 0x49bb7b
    if (cpu.flags.zf)
    {
        goto L_0x0049bb7b;
    }
    // 0049bb56  b40f                   -mov ah, 0xf
    cpu.ah = 15 /*0xf*/;
    // 0049bb58  8b15bcd26f00           -mov edx, dword ptr [0x6fd2bc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0049bb5e  886562                 -mov byte ptr [ebp + 0x62], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(98) /* 0x62 */) = cpu.ah;
    // 0049bb61  8d4562                 -lea eax, [ebp + 0x62]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 0049bb64  e897fcffff             -call 0x49b800
    cpu.esp -= 4;
    nfs3hp::load::beginCarpSend(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bb69  b210                   -mov dl, 0x10
    cpu.dl = 16 /*0x10*/;
    // 0049bb6b  8b5d7a                 -mov ebx, dword ptr [ebp + 0x7a]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 0049bb6e  88558e                 -mov byte ptr [ebp - 0x72], dl
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-114) /* -0x72 */) = cpu.dl;
    // 0049bb71  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0049bb73  8d458e                 -lea eax, [ebp - 0x72]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-114) /* -0x72 */);
    // 0049bb76  e835feffff             -call 0x49b9b0
    cpu.esp -= 4;
    sub_49b9b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049bb7b:
    // 0049bb7b  b83c000000             -mov eax, 0x3c
    cpu.eax = 60 /*0x3c*/;
    // 0049bb80  8d510f                 -lea edx, [ecx + 0xf]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(15) /* 0xf */);
    // 0049bb83  e888ae0000             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bb88  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049bb89  eba2                   -jmp 0x49bb2d
    goto L_0x0049bb2d;
L_0x0049bb8b:
    // 0049bb8b  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049bb93  0f84ad000000           -je 0x49bc46
    if (cpu.flags.zf)
    {
        goto L_0x0049bc46;
    }
    // 0049bb99  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0049bb9b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049bb9b  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 0049bba0  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049bba3  39c1                   +cmp ecx, eax
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
    // 0049bba5  0f8db9000000           -jge 0x49bc64
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049bc64;
    }
    // 0049bbab  0fbfc1                 -movsx eax, cx
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.cx));
    // 0049bbae  e8ddaa0000             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bbb3  0fbfd0                 -movsx edx, ax
    cpu.edx = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0049bbb6  83faff                 +cmp edx, -1
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
    // 0049bbb9  0f8474000000           -je 0x49bc33
    if (cpu.flags.zf)
    {
        goto L_0x0049bc33;
    }
    // 0049bbbf  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0049bbc6  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0049bbc8  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049bbcb  05f0f57900             -add eax, 0x79f5f0
    (cpu.eax) += x86::reg32(x86::sreg32(7992816 /*0x79f5f0*/));
    // 0049bbd0  83c019                 -add eax, 0x19
    (cpu.eax) += x86::reg32(x86::sreg32(25 /*0x19*/));
    // 0049bbd3  e8b8ddf9ff             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bbd8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0049bbda  83f8ff                 +cmp eax, -1
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
    // 0049bbdd  750d                   -jne 0x49bbec
    if (!cpu.flags.zf)
    {
        goto L_0x0049bbec;
    }
    // 0049bbdf  68b0c55300             -push 0x53c5b0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5490096 /*0x53c5b0*/;
    cpu.esp -= 4;
    // 0049bbe4  e82754f6ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bbe9  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x0049bbec:
    // 0049bbec  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0049bbf3  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0049bbf5  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049bbf8  05f0f57900             -add eax, 0x79f5f0
    (cpu.eax) += x86::reg32(x86::sreg32(7992816 /*0x79f5f0*/));
    // 0049bbfd  83c019                 -add eax, 0x19
    (cpu.eax) += x86::reg32(x86::sreg32(25 /*0x19*/));
    // 0049bc00  e82bd9f9ff             -call 0x439530
    cpu.esp -= 4;
    sub_439530(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bc05  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049bc07  742a                   -je 0x49bc33
    if (cpu.flags.zf)
    {
        goto L_0x0049bc33;
    }
    // 0049bc09  b611                   -mov dh, 0x11
    cpu.dh = 17 /*0x11*/;
    // 0049bc0b  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0049bc0e  88757e                 -mov byte ptr [ebp + 0x7e], dh
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.dh;
    // 0049bc11  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0049bc13  e8e8fbffff             -call 0x49b800
    cpu.esp -= 4;
    nfs3hp::load::beginCarpSend(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bc18  b312                   -mov bl, 0x12
    cpu.bl = 18 /*0x12*/;
    // 0049bc1a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0049bc1c  8d85bafeffff           -lea eax, [ebp - 0x146]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-326) /* -0x146 */);
    // 0049bc22  889dbafeffff           -mov byte ptr [ebp - 0x146], bl
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-326) /* -0x146 */) = cpu.bl;
    // 0049bc28  8b9d96000000           -mov ebx, dword ptr [ebp + 0x96]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(150) /* 0x96 */);
    // 0049bc2e  e87dfdffff             -call 0x49b9b0
    cpu.esp -= 4;
    sub_49b9b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049bc33:
    // 0049bc33  b83c000000             -mov eax, 0x3c
    cpu.eax = 60 /*0x3c*/;
    // 0049bc38  8d5117                 -lea edx, [ecx + 0x17]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(23) /* 0x17 */);
    // 0049bc3b  e8d0ad0000             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bc40  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049bc41  e955ffffff             -jmp 0x49bb9b
    goto L_0x0049bb9b;
L_0x0049bc46:
    // 0049bc46  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0049bc48:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049bc48  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 0049bc4d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0049bc50  39c1                   +cmp ecx, eax
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
    // 0049bc52  7d10                   -jge 0x49bc64
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049bc64;
    }
    // 0049bc54  b83c000000             -mov eax, 0x3c
    cpu.eax = 60 /*0x3c*/;
    // 0049bc59  8d5117                 -lea edx, [ecx + 0x17]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(23) /* 0x17 */);
    // 0049bc5c  e8afad0000             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049bc61  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049bc62  ebe4                   -jmp 0x49bc48
    goto L_0x0049bc48;
L_0x0049bc64:
    // 0049bc64  8da59a000000           -lea esp, [ebp + 0x9a]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(154) /* 0x9a */);
    // 0049bc6a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049bc6b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049bc6c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049bc6d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049bc6e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049bc6f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049bc70  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_49bc80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049bc80  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049bc81  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049bc82  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049bc84  c7400800000000         -mov dword ptr [eax + 8], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 0049bc8b  c7400c00000000         -mov dword ptr [eax + 0xc], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = 0 /*0x0*/;
    // 0049bc92  c74004ff0f0000         -mov dword ptr [eax + 4], 0xfff
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = 4095 /*0xfff*/;
    // 0049bc99  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 0049bc9b  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0049bc9d  eb08                   -jmp 0x49bca7
    goto L_0x0049bca7;
L_0x0049bc9f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049bc9f  81fa00100000           +cmp edx, 0x1000
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4096 /*0x1000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049bca5  7d20                   -jge 0x49bcc7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049bcc7;
    }
L_0x0049bca7:
    // 0049bca7  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0049bca9  c1e105                 +shl ecx, 5
    {
        x86::reg8 tmp = 5 /*0x5*/ % 32;
        x86::reg32& op = cpu.ecx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 0049bcac  c744011880008037       -mov dword ptr [ecx + eax + 0x18], 0x37800080
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */ + cpu.eax * 1) = 931135616 /*0x37800080*/;
    // 0049bcb4  c744011c00ff7f3f       -mov dword ptr [ecx + eax + 0x1c], 0x3f7fff00
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */ + cpu.eax * 1) = 1065352960 /*0x3f7fff00*/;
    // 0049bcbc  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049bcbd  c744012400000000       -mov dword ptr [ecx + eax + 0x24], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */ + cpu.eax * 1) = 0 /*0x0*/;
    // 0049bcc5  ebd8                   -jmp 0x49bc9f
    goto L_0x0049bc9f;
L_0x0049bcc7:
    // 0049bcc7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049bcc8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049bcc9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49bcd0: native, nfs3hp::port::text::takeQuad */

/* sub_49bcf0: native, nfs3hp::dead::stub_0x49bcf0 */

/* align: skip 0x8b 0xc0 */
void Application::sub_49bd10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049bd10  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049bd11  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049bd12  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049bd14  8b7008                 -mov esi, dword ptr [eax + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 0049bd17  8931                   -mov dword ptr [ecx], esi
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.esi;
    // 0049bd19  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0049bd1b  c1e105                 -shl ecx, 5
    cpu.ecx <<= 5 /*0x5*/ % 32;
    // 0049bd1e  8d7010                 -lea esi, [eax + 0x10]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0049bd21  01ce                   -add esi, ecx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0049bd23  8933                   -mov dword ptr [ebx], esi
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.esi;
    // 0049bd25  015008                 -add dword ptr [eax + 8], edx
    (app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */)) += x86::reg32(x86::sreg32(cpu.edx));
    // 0049bd28  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049bd29  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049bd2a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49bd30: native, nfs3hp::port::text::queueQuad */

/* sub_49bdc0: native, nfs3hp::port::text::unqueueQuad */

/* sub_49be40: native, nfs3hp::port::text::flushQuads */

/* sub_49bef0: native, nfs3hp::frontend::hudSurfaceReset */

/* sub_49bf40: native, nfs3hp::frontend::hudSurfaceTakeVertices */

/* sub_49bf60: native, nfs3hp::graphics::flushLineMesh */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_49bfb0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049bfb0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049bfb1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049bfb3  0905ece55500           -or dword ptr [0x55e5ec], eax
    app->getMemory<x86::reg32>(x86::reg32(5629420) /* 0x55e5ec */) |= x86::reg32(x86::sreg32(cpu.eax));
    // 0049bfb9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049bfba  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49bfc0: native, nfs3hp::frontend::setPointer */

/* sub_49c000: native, nfs3hp::port::text::mouseMenuKey */

/* sub_49c080: native, nfs3hp::frontend::mouseKey */

/* sub_49c0e0: native, nfs3hp::frontend::connectSetPointer */

/* sub_49c140: native, nfs3hp::frontend::menuPointerLoadArt */

/* sub_49c1b0: native, nfs3hp::dead::stub_0x49c1b0 */

/* sub_49c1c0: native, nfs3hp::frontend::menuPointerListLine */

}
