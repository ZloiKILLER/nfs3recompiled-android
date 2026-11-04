#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_native_loading.py): defined in native_loading.cpp.
bool fceRecords(win32::WinApplication* app, x86::CPU& cpu);

/* sub_49c1f0: native, nfs3hp::frontend::pointerInBox */

/* sub_49c240: native, nfs3hp::frontend::menuPointerOverRect */

/* sub_49c280: native, nfs3hp::dead::stub_0x49c280 */

/* sub_49c290: native, nfs3hp::dead::stub_0x49c290 */

/* sub_49c2a0: native, nfs3hp::frontend::menuPointerFreeArt */

/* sub_49c2d0: native, nfs3hp::dead::stub_0x49c2d0 */

/* sub_49c310: native, nfs3hp::dead::stub_0x49c310 */

/* sub_49c3b0: native, nfs3hp::dead::stub_0x49c3b0 */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
/* data blob: 0000803f0000000000000000000000000000803f0000000000000000000000000000803f8d80000000008d9200000000 */
void Application::sub_49c410(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049c410  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049c411  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049c413  e808fb0100             -call 0x4bbf20
    cpu.esp -= 4;
    sub_4bbf20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049c418  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c419  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_49c420(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049c420  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049c421  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049c422  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049c423  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049c424  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049c425  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049c427  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0049c42a  833d603a7a0000         +cmp dword ptr [0x7a3a60], 0
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
    // 0049c431  7511                   -jne 0x49c444
    if (!cpu.flags.zf)
    {
        goto L_0x0049c444;
    }
    // 0049c433  8b5a04                 -mov ebx, dword ptr [edx + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 0049c436  ff7518                 -push dword ptr [ebp + 0x18]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    // 0049c439  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049c43a  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 0049c43c  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049c43d  e8beaf0200             -call 0x4c7400
    cpu.esp -= 4;
    sub_4c7400(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049c442  eb0c                   -jmp 0x49c450
    goto L_0x0049c450;
L_0x0049c444:
    // 0049c444  8b7a04                 -mov edi, dword ptr [edx + 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 0049c447  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049c448  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 0049c44a  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049c44b  e88085f9ff             -call 0x4349d0
    cpu.esp -= 4;
    nfs3hp::collide::drawMirroredRecords(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049c450:
    // 0049c450  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049c453  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0049c455  e816fa0100             -call 0x4bbe70
    cpu.esp -= 4;
    sub_4bbe70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049c45a  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049c45d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049c45e  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049c45f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049c461  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c462  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c463  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c464  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c465  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c466  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_49c470(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049c470  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049c471  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049c472  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049c473  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049c475  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0049c478  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0049c47a  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049c47d  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0049c47f  e83cfb0100             -call 0x4bbfc0
    cpu.esp -= 4;
    sub_4bbfc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049c484  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049c487  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049c488  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049c489  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049c48b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049c48d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c48e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c48f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c490  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_49c4a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049c4a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049c4a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049c4a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049c4a3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049c4a4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049c4a5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049c4a7  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0049c4aa  8975f4                 -mov dword ptr [ebp - 0xc], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.esi;
    // 0049c4ad  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049c4af  8b1d083d7a00           -mov ebx, dword ptr [0x7a3d08]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8011016) /* 0x7a3d08 */);
    // 0049c4b5  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0049c4b8  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 0049c4bb  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0049c4bd  7402                   -je 0x49c4c1
    if (cpu.flags.zf)
    {
        goto L_0x0049c4c1;
    }
    // 0049c4bf  8913                   -mov dword ptr [ebx], edx
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.edx;
L_0x0049c4c1:
    // 0049c4c1  8b1d0c3d7a00           -mov ebx, dword ptr [0x7a3d0c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8011020) /* 0x7a3d0c */);
L_0x0049c4c7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049c4c7  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0049c4c9  0f847f000000           -je 0x49c54e
    if (cpu.flags.zf)
    {
        goto L_0x0049c54e;
    }
    // 0049c4cf  ba04010000             -mov edx, 0x104
    cpu.edx = 260 /*0x104*/;
    // 0049c4d4  8d45f0                 -lea eax, [ebp - 0x10]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049c4d7  b921000000             -mov ecx, 0x21
    cpu.ecx = 33 /*0x21*/;
    // 0049c4dc  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 0049c4de  e8fdf80100             -call 0x4bbde0
    cpu.esp -= 4;
    sub_4bbde0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049c4e3  8b7df0                 -mov edi, dword ptr [ebp - 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049c4e6  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049c4e8  837df800               +cmp dword ptr [ebp - 8], 0
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
    // 0049c4ec  740a                   -je 0x49c4f8
    if (cpu.flags.zf)
    {
        goto L_0x0049c4f8;
    }
    // 0049c4ee  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049c4f1  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049c4f4  8906                   -mov dword ptr [esi], eax
    app->getMemory<x86::reg32>(cpu.esi) = cpu.eax;
    // 0049c4f6  eb06                   -jmp 0x49c4fe
    goto L_0x0049c4fe;
L_0x0049c4f8:
    // 0049c4f8  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049c4fb  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
L_0x0049c4fe:
    // 0049c4fe  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049c501  8b75f0                 -mov esi, dword ptr [ebp - 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049c504  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0049c507  0584000000             +add eax, 0x84
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(132 /*0x84*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0049c50c  894608                 -mov dword ptr [esi + 8], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0049c50f  8b75f0                 -mov esi, dword ptr [ebp - 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049c512  8d5020                 -lea edx, [eax + 0x20]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(32) /* 0x20 */);
    // 0049c515  89560c                 -mov dword ptr [esi + 0xc], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */) = cpu.edx;
    // 0049c518  8b75f0                 -mov esi, dword ptr [ebp - 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049c51b  8d5040                 -lea edx, [eax + 0x40]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 0049c51e  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 0049c523  895610                 -mov dword ptr [esi + 0x10], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */) = cpu.edx;
    // 0049c526  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0049c528  8b7308                 -mov esi, dword ptr [ebx + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 0049c52b  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049c52d  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 0049c532  8b730c                 -mov esi, dword ptr [ebx + 0xc]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 0049c535  8d7820                 -lea edi, [eax + 0x20]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(32) /* 0x20 */);
    // 0049c538  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049c53a  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 0049c53f  8b7310                 -mov esi, dword ptr [ebx + 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */);
    // 0049c542  8d7840                 -lea edi, [eax + 0x40]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 0049c545  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049c547  8b1b                   -mov ebx, dword ptr [ebx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0049c549  e979ffffff             -jmp 0x49c4c7
    goto L_0x0049c4c7;
L_0x0049c54e:
    // 0049c54e  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049c551  8b7df4                 -mov edi, dword ptr [ebp - 0xc]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049c554  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 0049c557  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049c55a  8d75e8                 -lea esi, [ebp - 0x18]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0049c55d  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0049c560  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049c561  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049c562  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049c565  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049c567  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c568  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c569  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c56a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c56b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049c56c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49c570: native, nfs3hp::graphics::showPicture */

/* sub_49c5c0: native, nfs3hp::graphics::textureFormatFor */

/* sub_49c600: native, nfs3hp::graphics::allocTexture */

/* sub_49c620: native, nfs3hp::graphics::updateTexture */

/* sub_49c700: native, nfs3hp::load::readCarColors */

/* sub_49c880: native, nfs3hp::load::rgbToHsv */

/* sub_49ca50: native, nfs3hp::load::hsvToRgb */

/* sub_49cc50: native, nfs3hp::load::tintPixel */

/* sub_49cd00: native, nfs3hp::load::scaleTexture */

/* sub_49cea0: native, nfs3hp::load::fitTexture */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_49cfa0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (fceRecords(app, cpu.sync())) /* port: native (tools/apply_native_loading.py) */
    {
        cpu.reload();
        return;
    }
    // 0049cfa0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049cfa1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049cfa2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049cfa3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049cfa4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049cfa5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049cfa6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049cfa8  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0049cfab  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049cfad  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0049cfb0  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0049cfb2  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049cfb4  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049cfb7  81c6041f0000           -add esi, 0x1f04
    (cpu.esi) += x86::reg32(x86::sreg32(7940 /*0x1f04*/));
    // 0049cfbd  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049cfbf  8b8604e1ffff           -mov eax, dword ptr [esi - 0x1efc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(-7932) /* -0x1efc */);
    // 0049cfc5  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 0049cfc8  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049cfcb  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049cfcd  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049cfcf  b80cc65300             -mov eax, 0x53c60c
    cpu.eax = 5490188 /*0x53c60c*/;
    // 0049cfd4  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
    // 0049cfd7  e844460400             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049cfdc  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049cfde  8986fce0ffff           -mov dword ptr [esi - 0x1f04], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(-7940) /* -0x1f04 */) = cpu.eax;
L_0x0049cfe4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049cfe4  3b99f8000000           +cmp ebx, dword ptr [ecx + 0xf8]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(248) /* 0xf8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049cfea  0f8d2f010000           -jge 0x49d11f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049d11f;
    }
    // 0049cff0  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049cff3  8b711c                 -mov esi, dword ptr [ecx + 0x1c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */);
    // 0049cff6  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 0049cff8  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0049cffb  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 0049d002  8d3401                 -lea esi, [ecx + eax]
    cpu.esi = x86::reg32(cpu.ecx + cpu.eax * 1);
    // 0049d005  8b86fc030000           -mov eax, dword ptr [esi + 0x3fc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(1020) /* 0x3fc */);
    // 0049d00b  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049d00e  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049d011  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049d014  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049d016  035118                 -add edx, dword ptr [ecx + 0x18]
    (cpu.edx) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */)));
    // 0049d019  8b86fc050000           -mov eax, dword ptr [esi + 0x5fc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(1532) /* 0x5fc */);
    // 0049d01f  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0049d022  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0049d025  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0049d028  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 0049d02b  8b7dec                 -mov edi, dword ptr [ebp - 0x14]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049d02e  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0049d031  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 0049d033  8bb6fc050000           -mov esi, dword ptr [esi + 0x5fc]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(1532) /* 0x5fc */);
    // 0049d039  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049d03b  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0049d03d  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049d040  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049d042  c1e602                 -shl esi, 2
    cpu.esi <<= 2 /*0x2*/ % 32;
    // 0049d045  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 0049d047  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 0049d049  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0049d04b  8975f8                 -mov dword ptr [ebp - 8], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.esi;
L_0x0049d04e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049d04e  8b7df8                 -mov edi, dword ptr [ebp - 8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049d051  3bbc99fc040000         +cmp edi, dword ptr [ecx + ebx*4 + 0x4fc]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(1276) /* 0x4fc */ + cpu.ebx * 4)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049d058  7d16                   -jge 0x49d070
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049d070;
    }
    // 0049d05a  89fe                   -mov esi, edi
    cpu.esi = cpu.edi;
    // 0049d05c  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049d05f  c1e605                 -shl esi, 5
    cpu.esi <<= 5 /*0x5*/ % 32;
    // 0049d062  01fe                   +add esi, edi
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0049d064  c7461400000000         -mov dword ptr [esi + 0x14], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */) = 0 /*0x0*/;
    // 0049d06b  ff45f8                 +inc dword ptr [ebp - 8]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049d06e  ebde                   -jmp 0x49d04e
    goto L_0x0049d04e;
L_0x0049d070:
    // 0049d070  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
L_0x0049d072:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049d072  897df4                 -mov dword ptr [ebp - 0xc], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edi;
    // 0049d075  8b7df4                 -mov edi, dword ptr [ebp - 0xc]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049d078  3bbc99fc060000         +cmp edi, dword ptr [ecx + ebx*4 + 0x6fc]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(1788) /* 0x6fc */ + cpu.ebx * 4)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049d07f  0f8d94000000           -jge 0x49d119
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049d119;
    }
    // 0049d085  8b7204                 -mov esi, dword ptr [edx + 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 0049d088  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049d08b  c1e605                 -shl esi, 5
    cpu.esi <<= 5 /*0x5*/ % 32;
    // 0049d08e  01f7                   -add edi, esi
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.esi));
    // 0049d090  897808                 -mov dword ptr [eax + 8], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.edi;
    // 0049d093  8b7208                 -mov esi, dword ptr [edx + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 0049d096  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049d099  c1e605                 -shl esi, 5
    cpu.esi <<= 5 /*0x5*/ % 32;
    // 0049d09c  01f7                   -add edi, esi
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.esi));
    // 0049d09e  89780c                 -mov dword ptr [eax + 0xc], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = cpu.edi;
    // 0049d0a1  8b720c                 -mov esi, dword ptr [edx + 0xc]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    // 0049d0a4  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049d0a7  c1e605                 -shl esi, 5
    cpu.esi <<= 5 /*0x5*/ % 32;
    // 0049d0aa  66c740040300           -mov word ptr [eax + 4], 3
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */) = 3 /*0x3*/;
    // 0049d0b0  01f7                   -add edi, esi
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.esi));
    // 0049d0b2  897810                 -mov dword ptr [eax + 0x10], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */) = cpu.edi;
    // 0049d0b5  8b7220                 -mov esi, dword ptr [edx + 0x20]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(32) /* 0x20 */);
    // 0049d0b8  897020                 -mov dword ptr [eax + 0x20], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(32) /* 0x20 */) = cpu.esi;
    // 0049d0bb  8b722c                 -mov esi, dword ptr [edx + 0x2c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(44) /* 0x2c */);
    // 0049d0be  897024                 -mov dword ptr [eax + 0x24], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */) = cpu.esi;
    // 0049d0c1  8b7224                 -mov esi, dword ptr [edx + 0x24]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(36) /* 0x24 */);
    // 0049d0c4  897028                 -mov dword ptr [eax + 0x28], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(40) /* 0x28 */) = cpu.esi;
    // 0049d0c7  8b7230                 -mov esi, dword ptr [edx + 0x30]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(48) /* 0x30 */);
    // 0049d0ca  89702c                 -mov dword ptr [eax + 0x2c], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(44) /* 0x2c */) = cpu.esi;
    // 0049d0cd  8b7228                 -mov esi, dword ptr [edx + 0x28]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(40) /* 0x28 */);
    // 0049d0d0  897030                 -mov dword ptr [eax + 0x30], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */) = cpu.esi;
    // 0049d0d3  8b7234                 -mov esi, dword ptr [edx + 0x34]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(52) /* 0x34 */);
    // 0049d0d6  897034                 -mov dword ptr [eax + 0x34], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(52) /* 0x34 */) = cpu.esi;
    // 0049d0d9  668b32                 -mov si, word ptr [edx]
    cpu.si = app->getMemory<x86::reg16>(cpu.edx);
    // 0049d0dc  66897006               -mov word ptr [eax + 6], si
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */) = cpu.si;
    // 0049d0e0  f6421c04               +test byte ptr [edx + 0x1c], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.edx + x86::reg32(28) /* 0x1c */) & 4 /*0x4*/));
    // 0049d0e4  7404                   -je 0x49d0ea
    if (cpu.flags.zf)
    {
        goto L_0x0049d0ea;
    }
    // 0049d0e6  80480701               -or byte ptr [eax + 7], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(7) /* 0x7 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0049d0ea:
    // 0049d0ea  f6421c08               +test byte ptr [edx + 0x1c], 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.edx + x86::reg32(28) /* 0x1c */) & 8 /*0x8*/));
    // 0049d0ee  7404                   -je 0x49d0f4
    if (cpu.flags.zf)
    {
        goto L_0x0049d0f4;
    }
    // 0049d0f0  80480702               -or byte ptr [eax + 7], 2
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(7) /* 0x7 */) |= x86::reg8(x86::sreg8(2 /*0x2*/));
L_0x0049d0f4:
    // 0049d0f4  f6421c01               +test byte ptr [edx + 0x1c], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.edx + x86::reg32(28) /* 0x1c */) & 1 /*0x1*/));
    // 0049d0f8  7404                   -je 0x49d0fe
    if (cpu.flags.zf)
    {
        goto L_0x0049d0fe;
    }
    // 0049d0fa  80480704               -or byte ptr [eax + 7], 4
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(7) /* 0x7 */) |= x86::reg8(x86::sreg8(4 /*0x4*/));
L_0x0049d0fe:
    // 0049d0fe  f6421c02               +test byte ptr [edx + 0x1c], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.edx + x86::reg32(28) /* 0x1c */) & 2 /*0x2*/));
    // 0049d102  7404                   -je 0x49d108
    if (cpu.flags.zf)
    {
        goto L_0x0049d108;
    }
    // 0049d104  80480708               -or byte ptr [eax + 7], 8
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(7) /* 0x7 */) |= x86::reg8(x86::sreg8(8 /*0x8*/));
L_0x0049d108:
    // 0049d108  8b7df4                 -mov edi, dword ptr [ebp - 0xc]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049d10b  83c238                 -add edx, 0x38
    (cpu.edx) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 0049d10e  47                     -inc edi
    (cpu.edi)++;
    // 0049d10f  0584000000             +add eax, 0x84
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(132 /*0x84*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0049d114  e959ffffff             -jmp 0x49d072
    goto L_0x0049d072;
L_0x0049d119:
    // 0049d119  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049d11a  e9c5feffff             -jmp 0x49cfe4
    goto L_0x0049cfe4;
L_0x0049d11f:
    // 0049d11f  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049d121  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049d123  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d124  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d125  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d126  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d127  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d128  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d129  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49d130: native, nfs3hp::load::loadArtTextures */

/* sub_49d240: native, nfs3hp::load::nearestModelColor */

/* sub_49d320: native, nfs3hp::load::loadTgaTexture */

/* sub_49d450: native, nfs3hp::load::loadCarModel */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_49d5a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049d5a0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049d5a1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049d5a2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049d5a3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049d5a5  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0049d5a7  8b08                   -mov ecx, dword ptr [eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax);
    // 0049d5a9  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0049d5ab  7407                   -je 0x49d5b4
    if (cpu.flags.zf)
    {
        goto L_0x0049d5b4;
    }
    // 0049d5ad  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049d5af  e8dc420400             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049d5b4:
    // 0049d5b4  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049d5b6  e8d5420400             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049d5bb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d5bc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d5bd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d5be  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_49d5c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  x86::reg16 fsw = 0;
    // 0049d5c0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049d5c1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049d5c2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049d5c3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049d5c5  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0049d5c8  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0049d5ca  8955f0                 -mov dword ptr [ebp - 0x10], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edx;
    // 0049d5cd  895dec                 -mov dword ptr [ebp - 0x14], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.ebx;
    // 0049d5d0  894df4                 -mov dword ptr [ebp - 0xc], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ecx;
    // 0049d5d3  ba00ff7f47             -mov edx, 0x477fff00
    cpu.edx = 1199570688 /*0x477fff00*/;
    // 0049d5d8  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0049d5da  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
L_0x0049d5dd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049d5dd  3bb7f8000000           +cmp esi, dword ptr [edi + 0xf8]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi + x86::reg32(248) /* 0xf8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049d5e3  7d47                   -jge 0x49d62c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049d62c;
    }
    // 0049d5e5  8b5d14                 -mov ebx, dword ptr [ebp + 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 0049d5e8  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049d5e9  8b4510                 -mov eax, dword ptr [ebp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 0049d5ec  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049d5ed  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049d5f0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049d5f1  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049d5f4  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049d5f7  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049d5f8  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0049d5fa  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 0049d601  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049d602  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 0049d604  8d8ffc000000           -lea ecx, [edi + 0xfc]
    cpu.ecx = x86::reg32(cpu.edi + x86::reg32(252) /* 0xfc */);
    // 0049d60a  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049d60d  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049d60f  01c1                   +add ecx, eax
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
    // 0049d611  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0049d613  e808080000             -call 0x49de20
    cpu.esp -= 4;
    sub_49de20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0049d618  d955f8                 +fst dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 0049d61b  d85dfc                 +fcomp dword ptr [ebp - 4]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0049d61e  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 0049d620  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0049d621  7306                   -jae 0x49d629
    if (!cpu.flags.cf)
    {
        goto L_0x0049d629;
    }
    // 0049d623  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049d626  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
L_0x0049d629:
    // 0049d629  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049d62a  ebb1                   -jmp 0x49d5dd
    goto L_0x0049d5dd;
L_0x0049d62c:
    // 0049d62c  d945fc                 -fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 0049d62f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049d631  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d632  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d633  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d634  c20800                 -ret 8
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fp0;
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* sub_49d640: native, nfs3hp::decompiled::resetSunLighting */

/* align: skip  */
void Application::sub_49d720(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049d720  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049d721  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049d723  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049d728  c1e018                 -shl eax, 0x18
    cpu.eax <<= 24 /*0x18*/ % 32;
    // 0049d72b  a3e4e65500             -mov dword ptr [0x55e6e4], eax
    app->getMemory<x86::reg32>(x86::reg32(5629668) /* 0x55e6e4 */) = cpu.eax;
    // 0049d730  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049d732  88d0                   -mov al, dl
    cpu.al = cpu.dl;
    // 0049d734  c1e018                 -shl eax, 0x18
    cpu.eax <<= 24 /*0x18*/ % 32;
    // 0049d737  0dffffff00             -or eax, 0xffffff
    cpu.eax |= x86::reg32(x86::sreg32(16777215 /*0xffffff*/));
    // 0049d73c  a3e8e65500             -mov dword ptr [0x55e6e8], eax
    app->getMemory<x86::reg32>(x86::reg32(5629672) /* 0x55e6e8 */) = cpu.eax;
    // 0049d741  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d742  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_49d750(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049d750  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049d751  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049d753  a39ce65500             -mov dword ptr [0x55e69c], eax
    app->getMemory<x86::reg32>(x86::reg32(5629596) /* 0x55e69c */) = cpu.eax;
    // 0049d758  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d759  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_49d760(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049d760  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049d761  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049d762  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049d763  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049d765  bfa0e65500             -mov edi, 0x55e6a0
    cpu.edi = 5629600 /*0x55e6a0*/;
    // 0049d76a  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049d76c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049d76d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049d76e  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049d76f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d770  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d771  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d772  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_49d780(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2;
    // 0049d780  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049d781  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049d782  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049d783  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049d785  83ec54                 -sub esp, 0x54
    (cpu.esp) -= x86::reg32(x86::sreg32(84 /*0x54*/));
    // 0049d788  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0049d78a  ff7008                 -push dword ptr [eax + 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    // 0049d78d  ff30                   -push dword ptr [eax]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    // 0049d78f  e82cd20400             -call 0x4ea9c0
    cpu.esp -= 4;
    sub_4ea9c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0049d794  d94208                 -fld dword ptr [edx + 8]
    fp1 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */));
    // 0049d797  d8c8                   -fmul st(0)
    fp1 = fp::mul(fp1, x86::Float(fp1));
    // 0049d799  d902                   -fld dword ptr [edx]
    fp2 = x86::Float(app->getMemory<float>(cpu.edx));
    // 0049d79b  d8c8                   -fmul st(0)
    fp2 = fp::mul(fp2, x86::Float(fp2));
    // 0049d79d  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0049d7a0  dec1                   -faddp st(1)
    fp1 = fp::add(fp1, fp2);
    // 0049d7a2  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0049d7a4  d8058cc65300           -fadd dword ptr [0x53c68c]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(x86::reg32(5490316) /* 0x53c68c */)));
    // 0049d7aa  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0049d7ac  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp1);
    // 0049d7af  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp0);
    // 0049d7b2  e869d10400             -call 0x4ea920
    cpu.esp -= 4;
    nfs3hp::frontend::simSquareRoot(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0049d7b7  ff7204                 -push dword ptr [edx + 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    // 0049d7ba  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 0049d7bd  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 0049d7c0  bbece65500             -mov ebx, 0x55e6ec
    cpu.ebx = 5629676 /*0x55e6ec*/;
    // 0049d7c5  e8f6d10400             -call 0x4ea9c0
    cpu.esp -= 4;
    sub_4ea9c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0049d7ca  ff75f4                 -push dword ptr [ebp - 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    cpu.esp -= 4;
    // 0049d7cd  dc0590c65300           -fadd qword ptr [0x53c690]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(x86::reg32(5490320) /* 0x53c690 */)));
    // 0049d7d3  8d45d0                 -lea eax, [ebp - 0x30]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0049d7d6  d95dfc                 -fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 0049d7d9  e802260400             -call 0x4dfde0
    cpu.esp -= 4;
    nfs3hp::ea::yRotation(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049d7de  ff75fc                 -push dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    // 0049d7e1  8d45ac                 -lea eax, [ebp - 0x54]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 0049d7e4  8d55ac                 -lea edx, [ebp - 0x54]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 0049d7e7  e894250400             -call 0x4dfd80
    cpu.esp -= 4;
    nfs3hp::ea::xRotation(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049d7ec  8d45d0                 -lea eax, [ebp - 0x30]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0049d7ef  e88c2a0400             -call 0x4e0280
    cpu.esp -= 4;
    nfs3hp::vertices::multiplyMatrices(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049d7f4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049d7f6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d7f7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d7f8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d7f9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49d800: native, nfs3hp::render::glareColours */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_49d8f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049d8f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049d8f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049d8f2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049d8f3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049d8f4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049d8f5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049d8f6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049d8f8  b903000000             -mov ecx, 3
    cpu.ecx = 3 /*0x3*/;
    // 0049d8fd  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0049d8ff  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049d901  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0049d904  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0049d906  81e3ff000000           -and ebx, 0xff
    cpu.ebx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049d90c  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049d90e  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0049d910  f7f1                   -div ecx
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = cpu.ecx;
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 0049d912  c1fe08                 -sar esi, 8
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (8 /*0x8*/ % 32));
    // 0049d915  81e6ff000000           -and esi, 0xff
    cpu.esi &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049d91b  a3b8e65500             -mov dword ptr [0x55e6b8], eax
    app->getMemory<x86::reg32>(x86::reg32(5629624) /* 0x55e6b8 */) = cpu.eax;
    // 0049d920  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049d922  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0049d924  f7f1                   -div ecx
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = cpu.ecx;
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 0049d926  81e7ff000000           -and edi, 0xff
    cpu.edi &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049d92c  a3bce65500             -mov dword ptr [0x55e6bc], eax
    app->getMemory<x86::reg32>(x86::reg32(5629628) /* 0x55e6bc */) = cpu.eax;
    // 0049d931  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0049d933  d1e8                   -shr eax, 1
    cpu.eax >>= 1 /*0x1*/ % 32;
    // 0049d935  a3c0e65500             -mov dword ptr [0x55e6c0], eax
    app->getMemory<x86::reg32>(x86::reg32(5629632) /* 0x55e6c0 */) = cpu.eax;
    // 0049d93a  8d041b                 -lea eax, [ebx + ebx]
    cpu.eax = x86::reg32(cpu.ebx + cpu.ebx * 1);
    // 0049d93d  3dff000000             +cmp eax, 0xff
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
    // 0049d942  7605                   -jbe 0x49d949
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0049d949;
    }
    // 0049d944  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
L_0x0049d949:
    // 0049d949  a3c4e65500             -mov dword ptr [0x55e6c4], eax
    app->getMemory<x86::reg32>(x86::reg32(5629636) /* 0x55e6c4 */) = cpu.eax;
    // 0049d94e  8d0436                 -lea eax, [esi + esi]
    cpu.eax = x86::reg32(cpu.esi + cpu.esi * 1);
    // 0049d951  3dff000000             +cmp eax, 0xff
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
    // 0049d956  7605                   -jbe 0x49d95d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0049d95d;
    }
    // 0049d958  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
L_0x0049d95d:
    // 0049d95d  a3c8e65500             -mov dword ptr [0x55e6c8], eax
    app->getMemory<x86::reg32>(x86::reg32(5629640) /* 0x55e6c8 */) = cpu.eax;
    // 0049d962  8d043f                 -lea eax, [edi + edi]
    cpu.eax = x86::reg32(cpu.edi + cpu.edi * 1);
    // 0049d965  3dff000000             +cmp eax, 0xff
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
    // 0049d96a  7605                   -jbe 0x49d971
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0049d971;
    }
    // 0049d96c  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
L_0x0049d971:
    // 0049d971  8b15c4e65500           -mov edx, dword ptr [0x55e6c4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629636) /* 0x55e6c4 */);
    // 0049d977  2b15b8e65500           -sub edx, dword ptr [0x55e6b8]
    (cpu.edx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629624) /* 0x55e6b8 */)));
    // 0049d97d  8b0dbce65500           -mov ecx, dword ptr [0x55e6bc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629628) /* 0x55e6bc */);
    // 0049d983  8915d0e65500           -mov dword ptr [0x55e6d0], edx
    app->getMemory<x86::reg32>(x86::reg32(5629648) /* 0x55e6d0 */) = cpu.edx;
    // 0049d989  8b15c8e65500           -mov edx, dword ptr [0x55e6c8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629640) /* 0x55e6c8 */);
    // 0049d98f  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049d991  8915d4e65500           -mov dword ptr [0x55e6d4], edx
    app->getMemory<x86::reg32>(x86::reg32(5629652) /* 0x55e6d4 */) = cpu.edx;
    // 0049d997  8b15c0e65500           -mov edx, dword ptr [0x55e6c0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629632) /* 0x55e6c0 */);
    // 0049d99d  a3cce65500             -mov dword ptr [0x55e6cc], eax
    app->getMemory<x86::reg32>(x86::reg32(5629644) /* 0x55e6cc */) = cpu.eax;
    // 0049d9a2  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0049d9a4  a3d8e65500             -mov dword ptr [0x55e6d8], eax
    app->getMemory<x86::reg32>(x86::reg32(5629656) /* 0x55e6d8 */) = cpu.eax;
    // 0049d9a9  893db4e65500           -mov dword ptr [0x55e6b4], edi
    app->getMemory<x86::reg32>(x86::reg32(5629620) /* 0x55e6b4 */) = cpu.edi;
    // 0049d9af  8935b0e65500           -mov dword ptr [0x55e6b0], esi
    app->getMemory<x86::reg32>(x86::reg32(5629616) /* 0x55e6b0 */) = cpu.esi;
    // 0049d9b5  891dace65500           -mov dword ptr [0x55e6ac], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629612) /* 0x55e6ac */) = cpu.ebx;
    // 0049d9bb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d9bc  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d9bd  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d9be  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d9bf  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d9c0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049d9c1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49d9d0: native, nfs3hp::vertices::objectVerticesNearest */

/* sub_49da30: native, nfs3hp::vertices::shadeNormals */

/* sub_49db70: native, nfs3hp::vertices::envMapCoords */

/* sub_49dbf0: native, nfs3hp::vertices::cullPolygons */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_49de20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 0049de20  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049de21  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049de22  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049de23  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049de25  81ece0000000           -sub esp, 0xe0
    (cpu.esp) -= x86::reg32(x86::sreg32(224 /*0xe0*/));
    // 0049de2b  83ed6e                 -sub ebp, 0x6e
    (cpu.ebp) -= x86::reg32(x86::sreg32(110 /*0x6e*/));
    // 0049de2e  89456a                 -mov dword ptr [ebp + 0x6a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */) = cpu.eax;
    // 0049de31  895562                 -mov dword ptr [ebp + 0x62], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */) = cpu.edx;
    // 0049de34  895d5e                 -mov dword ptr [ebp + 0x5e], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */) = cpu.ebx;
    // 0049de37  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0049de39  8b0dace65500           -mov ecx, dword ptr [0x55e6ac]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629612) /* 0x55e6ac */);
    // 0049de3f  a1b0e65500             -mov eax, dword ptr [0x55e6b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629616) /* 0x55e6b0 */);
    // 0049de44  c1e110                 -shl ecx, 0x10
    cpu.ecx <<= 16 /*0x10*/ % 32;
    // 0049de47  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 0049de4a  81c9000000ff           -or ecx, 0xff000000
    cpu.ecx |= x86::reg32(x86::sreg32(4278190080 /*0xff000000*/));
    // 0049de50  09c8                   -or eax, ecx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049de52  8b0db4e65500           -mov ecx, dword ptr [0x55e6b4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629620) /* 0x55e6b4 */);
    // 0049de58  8d7d8e                 -lea edi, [ebp - 0x72]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-114) /* -0x72 */);
    // 0049de5b  09c1                   -or ecx, eax
    cpu.ecx |= x86::reg32(x86::sreg32(cpu.eax));
    // 0049de5d  bee0c34900             -mov esi, 0x49c3e0
    cpu.esi = 4834272 /*0x49c3e0*/;
    // 0049de62  894d4a                 -mov dword ptr [ebp + 0x4a], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */) = cpu.ecx;
    // 0049de65  b909000000             -mov ecx, 9
    cpu.ecx = 9 /*0x9*/;
    // 0049de6a  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049de6c  b909000000             -mov ecx, 9
    cpu.ecx = 9 /*0x9*/;
    // 0049de71  8d7dfa                 -lea edi, [ebp - 6]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 0049de74  be98445600             -mov esi, 0x564498
    cpu.esi = 5653656 /*0x564498*/;
    // 0049de79  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049de7b  8d7d2a                 -lea edi, [ebp + 0x2a]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(42) /* 0x2a */);
    // 0049de7e  be8c445600             -mov esi, 0x56448c
    cpu.esi = 5653644 /*0x56448c*/;
    // 0049de83  8b4d6a                 -mov ecx, dword ptr [ebp + 0x6a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 0049de86  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049de87  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049de88  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049de89  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0049de8b  7508                   -jne 0x49de95
    if (!cpu.flags.zf)
    {
        goto L_0x0049de95;
    }
    // 0049de8d  894d5a                 -mov dword ptr [ebp + 0x5a], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */) = cpu.ecx;
    // 0049de90  e9b8010000             -jmp 0x49e04d
    goto L_0x0049e04d;
L_0x0049de95:
    // 0049de95  8b4562                 -mov eax, dword ptr [ebp + 0x62]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 0049de98  3b81f8000000           +cmp eax, dword ptr [ecx + 0xf8]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(248) /* 0xf8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049de9e  7c11                   -jl 0x49deb1
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0049deb1;
    }
    // 0049dea0  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0049dea2  897d5a                 -mov dword ptr [ebp + 0x5a], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */) = cpu.edi;
    // 0049dea5  d9455a                 -fld dword ptr [ebp + 0x5a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */));
    // 0049dea8  8d656e                 -lea esp, [ebp + 0x6e]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(110) /* 0x6e */);
    // 0049deab  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049deac  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049dead  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049deae  c21400                 -ret 0x14
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fp0;
    cpu.esp += 4+20 /*0x14*/;
    return;
L_0x0049deb1:
    // 0049deb1  8b5d6a                 -mov ebx, dword ptr [ebp + 0x6a]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 0049deb4  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049deb7  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049deb9  8b83fc040000           -mov eax, dword ptr [ebx + 0x4fc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(1276) /* 0x4fc */);
    // 0049debf  894552                 -mov dword ptr [ebp + 0x52], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */) = cpu.eax;
    // 0049dec2  8b456a                 -mov eax, dword ptr [ebp + 0x6a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 0049dec5  81c1041f0000           -add ecx, 0x1f04
    (cpu.ecx) += x86::reg32(x86::sreg32(7940 /*0x1f04*/));
    // 0049decb  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0049dece  895d42                 -mov dword ptr [ebp + 0x42], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */) = cpu.ebx;
    // 0049ded1  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0049ded3  8b9bfc030000           -mov ebx, dword ptr [ebx + 0x3fc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(1020) /* 0x3fc */);
    // 0049ded9  894556                 -mov dword ptr [ebp + 0x56], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */) = cpu.eax;
    // 0049dedc  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 0049dee3  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049dee5  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049dee8  8b5d56                 -mov ebx, dword ptr [ebp + 0x56]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 0049deeb  894546                 -mov dword ptr [ebp + 0x46], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */) = cpu.eax;
    // 0049deee  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049def0  8b456a                 -mov eax, dword ptr [ebp + 0x6a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 0049def3  8b7814                 -mov edi, dword ptr [eax + 0x14]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 0049def6  8b7546                 -mov esi, dword ptr [ebp + 0x46]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 0049def9  01cf                   -add edi, ecx
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0049defb  01f7                   -add edi, esi
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.esi));
    // 0049defd  8b701c                 -mov esi, dword ptr [eax + 0x1c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 0049df00  8b4024                 -mov eax, dword ptr [eax + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */);
    // 0049df03  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0049df05  894566                 -mov dword ptr [ebp + 0x66], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */) = cpu.eax;
    // 0049df08  8b4542                 -mov eax, dword ptr [ebp + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */);
    // 0049df0b  8b80fc030000           -mov eax, dword ptr [eax + 0x3fc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1020) /* 0x3fc */);
    // 0049df11  01ce                   -add esi, ecx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0049df13  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049df16  8b4d66                 -mov ecx, dword ptr [ebp + 0x66]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 0049df19  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049df1b  8b4546                 -mov eax, dword ptr [ebp + 0x46]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 0049df1e  895d56                 -mov dword ptr [ebp + 0x56], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */) = cpu.ebx;
    // 0049df21  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049df23  8b5d5e                 -mov ebx, dword ptr [ebp + 0x5e]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 0049df26  894d66                 -mov dword ptr [ebp + 0x66], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */) = cpu.ecx;
    // 0049df29  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0049df2b  7506                   -jne 0x49df33
    if (!cpu.flags.zf)
    {
        goto L_0x0049df33;
    }
    // 0049df2d  8d45fa                 -lea eax, [ebp - 6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 0049df30  89455e                 -mov dword ptr [ebp + 0x5e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */) = cpu.eax;
L_0x0049df33:
    // 0049df33  837d7e00               +cmp dword ptr [ebp + 0x7e], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049df37  7506                   -jne 0x49df3f
    if (!cpu.flags.zf)
    {
        goto L_0x0049df3f;
    }
    // 0049df39  8d45fa                 -lea eax, [ebp - 6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 0049df3c  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
L_0x0049df3f:
    // 0049df3f  83bd8600000000         +cmp dword ptr [ebp + 0x86], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(134) /* 0x86 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049df46  7509                   -jne 0x49df51
    if (!cpu.flags.zf)
    {
        goto L_0x0049df51;
    }
    // 0049df48  8d45fa                 -lea eax, [ebp - 6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 0049df4b  898586000000           -mov dword ptr [ebp + 0x86], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(134) /* 0x86 */) = cpu.eax;
L_0x0049df51:
    // 0049df51  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0049df53  7503                   -jne 0x49df58
    if (!cpu.flags.zf)
    {
        goto L_0x0049df58;
    }
    // 0049df55  8d552a                 -lea edx, [ebp + 0x2a]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(42) /* 0x2a */);
L_0x0049df58:
    // 0049df58  83bd8200000000         +cmp dword ptr [ebp + 0x82], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(130) /* 0x82 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049df5f  7509                   -jne 0x49df6a
    if (!cpu.flags.zf)
    {
        goto L_0x0049df6a;
    }
    // 0049df61  8d452a                 -lea eax, [ebp + 0x2a]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(42) /* 0x2a */);
    // 0049df64  898582000000           -mov dword ptr [ebp + 0x82], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(130) /* 0x82 */) = cpu.eax;
L_0x0049df6a:
    // 0049df6a  83bd8a00000000         +cmp dword ptr [ebp + 0x8a], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(138) /* 0x8a */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049df71  7509                   -jne 0x49df7c
    if (!cpu.flags.zf)
    {
        goto L_0x0049df7c;
    }
    // 0049df73  8d452a                 -lea eax, [ebp + 0x2a]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(42) /* 0x2a */);
    // 0049df76  89858a000000           -mov dword ptr [ebp + 0x8a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(138) /* 0x8a */) = cpu.eax;
L_0x0049df7c:
    // 0049df7c  8d451e                 -lea eax, [ebp + 0x1e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(30) /* 0x1e */);
    // 0049df7f  8b8d82000000           -mov ecx, dword ptr [ebp + 0x82]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 0049df85  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049df86  8b5d7e                 -mov ebx, dword ptr [ebp + 0x7e]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0049df89  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049df8e  e89d240400             -call 0x4e0430
    cpu.esp -= 4;
    nfs3hp::vertices::transformPoints(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049df93  8d4536                 -lea eax, [ebp + 0x36]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(54) /* 0x36 */);
    // 0049df96  8b8d8a000000           -mov ecx, dword ptr [ebp + 0x8a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(138) /* 0x8a */);
    // 0049df9c  8b9d86000000           -mov ebx, dword ptr [ebp + 0x86]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(134) /* 0x86 */);
    // 0049dfa2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049dfa3  8d551e                 -lea edx, [ebp + 0x1e]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(30) /* 0x1e */);
    // 0049dfa6  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049dfab  e880240400             -call 0x4e0430
    cpu.esp -= 4;
    nfs3hp::vertices::transformPoints(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049dfb0  8d5db2                 -lea ebx, [ebp - 0x4e]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-78) /* -0x4e */);
    // 0049dfb3  8b557e                 -mov edx, dword ptr [ebp + 0x7e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0049dfb6  8b455e                 -mov eax, dword ptr [ebp + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 0049dfb9  e8c2220400             -call 0x4e0280
    cpu.esp -= 4;
    nfs3hp::vertices::multiplyMatrices(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049dfbe  8d5d8e                 -lea ebx, [ebp - 0x72]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-114) /* -0x72 */);
    // 0049dfc1  8b9586000000           -mov edx, dword ptr [ebp + 0x86]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(134) /* 0x86 */);
    // 0049dfc7  8d45b2                 -lea eax, [ebp - 0x4e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-78) /* -0x4e */);
    // 0049dfca  e8b1220400             -call 0x4e0280
    cpu.esp -= 4;
    nfs3hp::vertices::multiplyMatrices(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049dfcf  8b554a                 -mov edx, dword ptr [ebp + 0x4a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 0049dfd2  8d4d8e                 -lea ecx, [ebp - 0x72]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-114) /* -0x72 */);
    // 0049dfd5  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049dfd6  8d4536                 -lea eax, [ebp + 0x36]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(54) /* 0x36 */);
    // 0049dfd9  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0049dfdb  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049dfdc  8b5556                 -mov edx, dword ptr [ebp + 0x56]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 0049dfdf  8b4552                 -mov eax, dword ptr [ebp + 0x52]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 0049dfe2  e8e9f9ffff             -call 0x49d9d0
    cpu.esp -= 4;
    nfs3hp::vertices::objectVerticesNearest(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0049dfe7  8b0d9ce65500           -mov ecx, dword ptr [0x55e69c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629596) /* 0x55e69c */);
    // 0049dfed  d95d4e                 -fstp dword ptr [ebp + 0x4e]
    app->getMemory<float>(cpu.ebp + x86::reg32(78) /* 0x4e */) = float(fp0);
    // 0049dff0  83f901                 +cmp ecx, 1
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
    // 0049dff3  7515                   -jne 0x49e00a
    if (!cpu.flags.zf)
    {
        goto L_0x0049e00a;
    }
    // 0049dff5  8d45b2                 -lea eax, [ebp - 0x4e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-78) /* -0x4e */);
    // 0049dff8  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049dff9  8b4d66                 -mov ecx, dword ptr [ebp + 0x66]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 0049dffc  8b5552                 -mov edx, dword ptr [ebp + 0x52]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 0049dfff  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049e000  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0049e002  8b456a                 -mov eax, dword ptr [ebp + 0x6a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 0049e005  e826faffff             -call 0x49da30
    cpu.esp -= 4;
    nfs3hp::vertices::shadeNormals(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049e00a:
    // 0049e00a  83bd8e00000000         +cmp dword ptr [ebp + 0x8e], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(142) /* 0x8e */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049e011  7423                   -je 0x49e036
    if (cpu.flags.zf)
    {
        goto L_0x0049e036;
    }
    // 0049e013  8d5dd6                 -lea ebx, [ebp - 0x2a]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-42) /* -0x2a */);
    // 0049e016  baece65500             -mov edx, 0x55e6ec
    cpu.edx = 5629676 /*0x55e6ec*/;
    // 0049e01b  8d45b2                 -lea eax, [ebp - 0x4e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-78) /* -0x4e */);
    // 0049e01e  e85d220400             -call 0x4e0280
    cpu.esp -= 4;
    nfs3hp::vertices::multiplyMatrices(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e023  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0049e025  8d45d6                 -lea eax, [ebp - 0x2a]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-42) /* -0x2a */);
    // 0049e028  8b5d66                 -mov ebx, dword ptr [ebp + 0x66]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 0049e02b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049e02c  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0049e02e  8b4552                 -mov eax, dword ptr [ebp + 0x52]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 0049e031  e83afbffff             -call 0x49db70
    cpu.esp -= 4;
    nfs3hp::vertices::envMapCoords(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049e036:
    // 0049e036  8b9d8e000000           -mov ebx, dword ptr [ebp + 0x8e]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(142) /* 0x8e */);
    // 0049e03c  8b5562                 -mov edx, dword ptr [ebp + 0x62]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 0049e03f  8b456a                 -mov eax, dword ptr [ebp + 0x6a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 0049e042  e8a9fbffff             -call 0x49dbf0
    cpu.esp -= 4;
    nfs3hp::vertices::cullPolygons(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e047  8b454e                 -mov eax, dword ptr [ebp + 0x4e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */);
    // 0049e04a  89455a                 -mov dword ptr [ebp + 0x5a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */) = cpu.eax;
L_0x0049e04d:
    // 0049e04d  d9455a                 -fld dword ptr [ebp + 0x5a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */));
    // 0049e050  8d656e                 -lea esp, [ebp + 0x6e]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(110) /* 0x6e */);
    // 0049e053  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e054  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e055  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e056  c21400                 -ret 0x14
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fp0;
    cpu.esp += 4+20 /*0x14*/;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
/* data blob: 00000000000000000000000000000000000000000000008d80000000008d5200 */
void Application::sub_49e080(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049e080  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049e081  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049e082  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049e083  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049e084  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049e085  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049e086  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049e088  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 0049e08d  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0049e092  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0049e097  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 0049e09c  b401                   -mov ah, 1
    cpu.ah = 1 /*0x1*/;
    // 0049e09e  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0049e0a0  8915e0227a00           -mov dword ptr [0x7a22e0], edx
    app->getMemory<x86::reg32>(x86::reg32(8004320) /* 0x7a22e0 */) = cpu.edx;
    // 0049e0a6  890de4227a00           -mov dword ptr [0x7a22e4], ecx
    app->getMemory<x86::reg32>(x86::reg32(8004324) /* 0x7a22e4 */) = cpu.ecx;
    // 0049e0ac  891d0cd56f00           -mov dword ptr [0x6fd50c], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329036) /* 0x6fd50c */) = cpu.ebx;
    // 0049e0b2  8935b0d36f00           -mov dword ptr [0x6fd3b0], esi
    app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */) = cpu.esi;
    // 0049e0b8  893d1ce75500           -mov dword ptr [0x55e71c], edi
    app->getMemory<x86::reg32>(x86::reg32(5629724) /* 0x55e71c */) = cpu.edi;
    // 0049e0be  882514227a00           -mov byte ptr [0x7a2214], ah
    app->getMemory<x86::reg8>(x86::reg32(8004116) /* 0x7a2214 */) = cpu.ah;
    // 0049e0c4  befcd26f00             -mov esi, 0x6fd2fc
    cpu.esi = 7328508 /*0x6fd2fc*/;
    // 0049e0c9  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049e0cb  bf15227a00             -mov edi, 0x7a2215
    cpu.edi = 8004117 /*0x7a2215*/;
    // 0049e0d0  6689157a227a00         -mov word ptr [0x7a227a], dx
    app->getMemory<x86::reg16>(x86::reg32(8004218) /* 0x7a227a */) = cpu.dx;
    // 0049e0d7  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0049e0d8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049e0d8  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049e0da  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049e0dc  3c00                   +cmp al, 0
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
    // 0049e0de  7410                   -je 0x49e0f0
    if (cpu.flags.zf)
    {
        goto L_0x0049e0f0;
    }
    // 0049e0e0  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049e0e3  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049e0e6  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049e0e9  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049e0ec  3c00                   +cmp al, 0
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
    // 0049e0ee  75e8                   -jne 0x49e0d8
    if (!cpu.flags.zf)
    {
        goto L_0x0049e0d8;
    }
L_0x0049e0f0:
    // 0049e0f0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e0f1  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0049e0f3  40                     -inc eax
    (cpu.eax)++;
    // 0049e0f4  66c7044578227a00ffff   -mov word ptr [eax*2 + 0x7a2278], 0xffff
    app->getMemory<x86::reg16>(x86::reg32(8004216) /* 0x7a2278 */ + cpu.eax * 2) = 65535 /*0xffff*/;
L_0x0049e0fe:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049e0fe  83f808                 +cmp eax, 8
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
    // 0049e101  7d0d                   -jge 0x49e110
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049e110;
    }
    // 0049e103  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049e104  66c7044578227a00ffff   -mov word ptr [eax*2 + 0x7a2278], 0xffff
    app->getMemory<x86::reg16>(x86::reg32(8004216) /* 0x7a2278 */ + cpu.eax * 2) = 65535 /*0xffff*/;
    // 0049e10e  ebee                   -jmp 0x49e0fe
    goto L_0x0049e0fe;
L_0x0049e110:
    // 0049e110  e83b150000             -call 0x49f650
    cpu.esp -= 4;
    nfs3hp::frontend::connectClearQueue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e115  e8e6830000             -call 0x4a6500
    cpu.esp -= 4;
    sub_4a6500(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e11a  e871f6f8ff             -call 0x42d790
    cpu.esp -= 4;
    nfs3hp::raceload::openMessages(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e11f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049e124  bfa01f7a00             -mov edi, 0x7a1fa0
    cpu.edi = 8003488 /*0x7a1fa0*/;
    // 0049e129  a360e85500             -mov dword ptr [0x55e860], eax
    app->getMemory<x86::reg32>(x86::reg32(5630048) /* 0x55e860 */) = cpu.eax;
    // 0049e12e  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049e130  49                     -dec ecx
    (cpu.ecx)--;
    // 0049e131  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0049e133  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0049e135  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 0049e137  49                     -dec ecx
    (cpu.ecx)--;
    // 0049e138  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0049e13a  0f84a8000000           -je 0x49e1e8
    if (cpu.flags.zf)
    {
        goto L_0x0049e1e8;
    }
    // 0049e140  bee01f7a00             -mov esi, 0x7a1fe0
    cpu.esi = 8003552 /*0x7a1fe0*/;
    // 0049e145  bffcd26f00             -mov edi, 0x6fd2fc
    cpu.edi = 7328508 /*0x6fd2fc*/;
    // 0049e14a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0049e14b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049e14b  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049e14d  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049e14f  3c00                   +cmp al, 0
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
    // 0049e151  7410                   -je 0x49e163
    if (cpu.flags.zf)
    {
        goto L_0x0049e163;
    }
    // 0049e153  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049e156  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049e159  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049e15c  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049e15f  3c00                   +cmp al, 0
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
    // 0049e161  75e8                   -jne 0x49e14b
    if (!cpu.flags.zf)
    {
        goto L_0x0049e14b;
    }
L_0x0049e163:
    // 0049e163  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e164  a100207a00             -mov eax, dword ptr [0x7a2000]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8003584) /* 0x7a2000 */);
    // 0049e169  a3b8d36f00             -mov dword ptr [0x6fd3b8], eax
    app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */) = cpu.eax;
    // 0049e16e  a104207a00             -mov eax, dword ptr [0x7a2004]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8003588) /* 0x7a2004 */);
    // 0049e173  a3acd46f00             -mov dword ptr [0x6fd4ac], eax
    app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */) = cpu.eax;
    // 0049e178  a10c207a00             -mov eax, dword ptr [0x7a200c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8003596) /* 0x7a200c */);
    // 0049e17d  a3b4d46f00             -mov dword ptr [0x6fd4b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */) = cpu.eax;
    // 0049e182  83f808                 +cmp eax, 8
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
    // 0049e185  7507                   -jne 0x49e18e
    if (!cpu.flags.zf)
    {
        goto L_0x0049e18e;
    }
    // 0049e187  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0049e18c  eb0e                   -jmp 0x49e19c
    goto L_0x0049e19c;
L_0x0049e18e:
    // 0049e18e  83f804                 +cmp eax, 4
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
    // 0049e191  7507                   -jne 0x49e19a
    if (!cpu.flags.zf)
    {
        goto L_0x0049e19a;
    }
    // 0049e193  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049e198  eb02                   -jmp 0x49e19c
    goto L_0x0049e19c;
L_0x0049e19a:
    // 0049e19a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0049e19c:
    // 0049e19c  a3b0d46f00             -mov dword ptr [0x6fd4b0], eax
    app->getMemory<x86::reg32>(x86::reg32(7328944) /* 0x6fd4b0 */) = cpu.eax;
    // 0049e1a1  a110207a00             -mov eax, dword ptr [0x7a2010]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8003600) /* 0x7a2010 */);
    // 0049e1a6  a370d46f00             -mov dword ptr [0x6fd470], eax
    app->getMemory<x86::reg32>(x86::reg32(7328880) /* 0x6fd470 */) = cpu.eax;
    // 0049e1ab  a114207a00             -mov eax, dword ptr [0x7a2014]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8003604) /* 0x7a2014 */);
    // 0049e1b0  bba01f7a00             -mov ebx, 0x7a1fa0
    cpu.ebx = 8003488 /*0x7a1fa0*/;
    // 0049e1b5  a3d4d46f00             -mov dword ptr [0x6fd4d4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */) = cpu.eax;
    // 0049e1ba  a108207a00             -mov eax, dword ptr [0x7a2008]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8003592) /* 0x7a2008 */);
    // 0049e1bf  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049e1c1  e84ab8f9ff             -call 0x439a10
    cpu.esp -= 4;
    sub_439a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e1c6  b8fcd26f00             -mov eax, 0x6fd2fc
    cpu.eax = 7328508 /*0x6fd2fc*/;
    // 0049e1cb  8b0ddc1f7a00           -mov ecx, dword ptr [0x7a1fdc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8003548) /* 0x7a1fdc */);
    // 0049e1d1  668b15fc1f7a00         -mov dx, word ptr [0x7a1ffc]
    cpu.dx = app->getMemory<x86::reg16>(x86::reg32(8003580) /* 0x7a1ffc */);
    // 0049e1d8  e823790000             -call 0x4a5b00
    cpu.esp -= 4;
    sub_4a5b00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e1dd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049e1df  7428                   -je 0x49e209
    if (cpu.flags.zf)
    {
        goto L_0x0049e209;
    }
    // 0049e1e1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049e1e6  eb23                   -jmp 0x49e20b
    goto L_0x0049e20b;
L_0x0049e1e8:
    // 0049e1e8  b90a1a0000             -mov ecx, 0x1a0a
    cpu.ecx = 6666 /*0x1a0a*/;
    // 0049e1ed  bbbcc65300             -mov ebx, 0x53c6bc
    cpu.ebx = 5490364 /*0x53c6bc*/;
    // 0049e1f2  b8fcd26f00             -mov eax, 0x6fd2fc
    cpu.eax = 7328508 /*0x6fd2fc*/;
    // 0049e1f7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049e1f9  e802790000             -call 0x4a5b00
    cpu.esp -= 4;
    sub_4a5b00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e1fe  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049e200  7407                   -je 0x49e209
    if (cpu.flags.zf)
    {
        goto L_0x0049e209;
    }
    // 0049e202  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049e207  eb02                   -jmp 0x49e20b
    goto L_0x0049e20b;
L_0x0049e209:
    // 0049e209  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0049e20b:
    // 0049e20b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e20c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e20d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e20e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e20f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e210  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e211  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_49e220(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049e220  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049e221  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049e222  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049e223  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049e225  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049e22a  bbfeffffff             -mov ebx, 0xfffffffe
    cpu.ebx = 4294967294 /*0xfffffffe*/;
    // 0049e22f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049e231  e85a810000             -call 0x4a6390
    cpu.esp -= 4;
    nfs3hp::ea::resetNetwork(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e236  668915da227a00         -mov word ptr [0x7a22da], dx
    app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */) = cpu.dx;
    // 0049e23d  66891d78227a00         -mov word ptr [0x7a2278], bx
    app->getMemory<x86::reg16>(x86::reg32(8004216) /* 0x7a2278 */) = cpu.bx;
    // 0049e244  66891576227a00         -mov word ptr [0x7a2276], dx
    app->getMemory<x86::reg16>(x86::reg32(8004214) /* 0x7a2276 */) = cpu.dx;
    // 0049e24b  e830feffff             -call 0x49e080
    cpu.esp -= 4;
    sub_49e080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e250  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049e252  0f8476000000           -je 0x49e2ce
    if (cpu.flags.zf)
    {
        goto L_0x0049e2ce;
    }
L_0x0049e258:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049e258  66833d96227a0000       +cmp word ptr [0x7a2296], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004246) /* 0x7a2296 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049e260  7531                   -jne 0x49e293
    if (!cpu.flags.zf)
    {
        goto L_0x0049e293;
    }
    // 0049e262  e849830000             -call 0x4a65b0
    cpu.esp -= 4;
    sub_4a65b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e267  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049e269  750a                   -jne 0x49e275
    if (!cpu.flags.zf)
    {
        goto L_0x0049e275;
    }
    // 0049e26b  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 0049e273  74e3                   -je 0x49e258
    if (cpu.flags.zf)
    {
        goto L_0x0049e258;
    }
L_0x0049e275:
    // 0049e275  b8c2000000             -mov eax, 0xc2
    cpu.eax = 194 /*0xc2*/;
    // 0049e27a  e86169faff             -call 0x444be0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogMessageById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e27f  66c705dc227a000100     -mov word ptr [0x7a22dc], 1
    app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */) = 1 /*0x1*/;
    // 0049e288  e853830000             -call 0x4a65e0
    cpu.esp -= 4;
    nfs3hp::ea::closeNetwork(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e28d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049e28f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e290  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e291  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e292  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049e293:
    // 0049e293  a18c227a00             -mov eax, dword ptr [0x7a228c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004236) /* 0x7a228c */);
    // 0049e298  a3e4227a00             -mov dword ptr [0x7a22e4], eax
    app->getMemory<x86::reg32>(x86::reg32(8004324) /* 0x7a22e4 */) = cpu.eax;
    // 0049e29d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049e29f  e83c8a0000             -call 0x4a6ce0
    cpu.esp -= 4;
    sub_4a6ce0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e2a4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049e2a6  7521                   -jne 0x49e2c9
    if (!cpu.flags.zf)
    {
        goto L_0x0049e2c9;
    }
    // 0049e2a8  b8c2000000             -mov eax, 0xc2
    cpu.eax = 194 /*0xc2*/;
    // 0049e2ad  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0049e2b2  e82969faff             -call 0x444be0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogMessageById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e2b7  668915dc227a00         -mov word ptr [0x7a22dc], dx
    app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */) = cpu.dx;
    // 0049e2be  e81d830000             -call 0x4a65e0
    cpu.esp -= 4;
    nfs3hp::ea::closeNetwork(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e2c3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049e2c5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e2c6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e2c7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e2c8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049e2c9:
    // 0049e2c9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0049e2ce:
    // 0049e2ce  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e2cf  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e2d0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e2d1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_49e2e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049e2e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049e2e1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049e2e2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049e2e3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049e2e5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049e2ea  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0049e2ef  e89c800000             -call 0x4a6390
    cpu.esp -= 4;
    nfs3hp::ea::resetNetwork(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e2f4  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049e2f6  668915da227a00         -mov word ptr [0x7a22da], dx
    app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */) = cpu.dx;
    // 0049e2fd  66891d78227a00         -mov word ptr [0x7a2278], bx
    app->getMemory<x86::reg16>(x86::reg32(8004216) /* 0x7a2278 */) = cpu.bx;
    // 0049e304  66891576227a00         -mov word ptr [0x7a2276], dx
    app->getMemory<x86::reg16>(x86::reg32(8004214) /* 0x7a2276 */) = cpu.dx;
    // 0049e30b  e870fdffff             -call 0x49e080
    cpu.esp -= 4;
    sub_49e080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e310  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049e312  7405                   -je 0x49e319
    if (cpu.flags.zf)
    {
        goto L_0x0049e319;
    }
    // 0049e314  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0049e319:
    // 0049e319  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e31a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e31b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e31c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_49e320(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049e320  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049e321  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049e322  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049e323  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049e325  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0049e328  8855ec                 -mov byte ptr [ebp - 0x14], dl
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.dl;
    // 0049e32b  833d10e7550000         +cmp dword ptr [0x55e710], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629712) /* 0x55e710 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049e332  7523                   -jne 0x49e357
    if (!cpu.flags.zf)
    {
        goto L_0x0049e357;
    }
    // 0049e334  8b0de0017a00           -mov ecx, dword ptr [0x7a01e0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7995872) /* 0x7a01e0 */);
    // 0049e33a  83f907                 +cmp ecx, 7
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049e33d  7d18                   -jge 0x49e357
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049e357;
    }
    // 0049e33f  8d5901                 -lea ebx, [ecx + 1]
    cpu.ebx = x86::reg32(cpu.ecx + x86::reg32(1) /* 0x1 */);
    // 0049e342  8d55ec                 -lea edx, [ebp - 0x14]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049e345  891de0017a00           -mov dword ptr [0x7a01e0], ebx
    app->getMemory<x86::reg32>(x86::reg32(7995872) /* 0x7a01e0 */) = cpu.ebx;
    // 0049e34b  e850130000             -call 0x49f6a0
    cpu.esp -= 4;
    nfs3hp::frontend::connectEnqueuePeer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049e350  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049e355  eb02                   -jmp 0x49e359
    goto L_0x0049e359;
L_0x0049e357:
    // 0049e357  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0049e359:
    // 0049e359  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049e35b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e35c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e35d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049e35e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49e360: native, nfs3hp::frontend::connectConnectionAvailable */

/* sub_49e3d0: native, nfs3hp::frontend::connectListSessions */

/* sub_49e5f0: native, nfs3hp::frontend::connectCreateSession */

/* sub_49e800: native, nfs3hp::frontend::connectJoinSession */

/* sub_49e9d0: native, nfs3hp::frontend::connectUseTcp */

/* sub_49ea70: native, nfs3hp::frontend::connectFindAddress */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_49eae0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049eae0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049eae1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049eae2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049eae3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049eae4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049eae5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049eae6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049eae8  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 0049eaeb  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049eaed  668955fc               -mov word ptr [ebp - 4], dx
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.dx;
    // 0049eaf1  833de0227a0003         +cmp dword ptr [0x7a22e0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004320) /* 0x7a22e0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049eaf8  7407                   -je 0x49eb01
    if (cpu.flags.zf)
    {
        goto L_0x0049eb01;
    }
    // 0049eafa  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0049eafc  e960010000             -jmp 0x49ec61
    goto L_0x0049ec61;
L_0x0049eb01:
    // 0049eb01  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049eb09  750e                   -jne 0x49eb19
    if (!cpu.flags.zf)
    {
        goto L_0x0049eb19;
    }
    // 0049eb0b  e8b07e0000             -call 0x4a69c0
    cpu.esp -= 4;
    sub_4a69c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049eb10  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049eb12  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049eb13  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049eb14  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049eb15  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049eb16  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049eb17  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049eb18  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049eb19:
    // 0049eb19  833de4227a0002         +cmp dword ptr [0x7a22e4], 2
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
    // 0049eb20  7448                   -je 0x49eb6a
    if (cpu.flags.zf)
    {
        goto L_0x0049eb6a;
    }
    // 0049eb22  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0049eb24  eb1a                   -jmp 0x49eb40
    goto L_0x0049eb40;
L_0x0049eb26:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049eb26  05e8227a00             -add eax, 0x7a22e8
    (cpu.eax) += x86::reg32(x86::sreg32(8004328 /*0x7a22e8*/));
    // 0049eb2b  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0049eb30  e87b010000             -call 0x49ecb0
    cpu.esp -= 4;
    nfs3hp::ea::dropMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049eb35  66894dfc               -mov word ptr [ebp - 4], cx
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.cx;
L_0x0049eb39:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049eb39  42                     -inc edx
    (cpu.edx)++;
    // 0049eb3a  6683fa07               +cmp dx, 7
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(7 /*0x7*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049eb3e  7d2a                   -jge 0x49eb6a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049eb6a;
    }
L_0x0049eb40:
    // 0049eb40  0fbfca                 -movsx ecx, dx
    cpu.ecx = x86::reg32(static_cast<x86::sreg16>(cpu.dx));
    // 0049eb43  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0049eb4a  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049eb4c  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049eb4f  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0049eb51  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0049eb54  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0049eb56  83b80c237a0000         +cmp dword ptr [eax + 0x7a230c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8004364) /* 0x7a230c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049eb5d  75da                   -jne 0x49eb39
    if (!cpu.flags.zf)
    {
        goto L_0x0049eb39;
    }
    // 0049eb5f  83b8f8227a0000         +cmp dword ptr [eax + 0x7a22f8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8004344) /* 0x7a22f8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049eb66  75be                   -jne 0x49eb26
    if (!cpu.flags.zf)
    {
        goto L_0x0049eb26;
    }
    // 0049eb68  ebcf                   -jmp 0x49eb39
    goto L_0x0049eb39;
L_0x0049eb6a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049eb6a  e8110b0000             -call 0x49f680
    cpu.esp -= 4;
    sub_49f680(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049eb6f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049eb71  0f85cd000000           -jne 0x49ec44
    if (!cpu.flags.zf)
    {
        goto L_0x0049ec44;
    }
    // 0049eb77  8d45dc                 -lea eax, [ebp - 0x24]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0049eb7a  e8b10b0000             -call 0x49f730
    cpu.esp -= 4;
    sub_49f730(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049eb7f  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0049eb81  eb07                   -jmp 0x49eb8a
    goto L_0x0049eb8a;
L_0x0049eb83:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049eb83  42                     -inc edx
    (cpu.edx)++;
    // 0049eb84  6683fa08               +cmp dx, 8
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(8 /*0x8*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049eb88  7d4d                   -jge 0x49ebd7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049ebd7;
    }
L_0x0049eb8a:
    // 0049eb8a  0fbff2                 -movsx esi, dx
    cpu.esi = x86::reg32(static_cast<x86::sreg16>(cpu.dx));
    // 0049eb8d  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 0049eb94  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 0049eb96  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049eb99  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0049eb9b  83bcc10c237a0000       +cmp dword ptr [ecx + eax*8 + 0x7a230c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8004364) /* 0x7a230c */ + cpu.eax * 8);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049eba3  75de                   -jne 0x49eb83
    if (!cpu.flags.zf)
    {
        goto L_0x0049eb83;
    }
    // 0049eba5  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 0049ebac  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 0049ebae  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049ebb1  bfdc257a00             -mov edi, 0x7a25dc
    cpu.edi = 8005084 /*0x7a25dc*/;
    // 0049ebb6  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 0049ebb8  8d75dc                 -lea esi, [ebp - 0x24]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0049ebbb  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049ebbd  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0049ebbe:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049ebbe  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049ebc0  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0049ebc2  3c00                   +cmp al, 0
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
    // 0049ebc4  7410                   -je 0x49ebd6
    if (cpu.flags.zf)
    {
        goto L_0x0049ebd6;
    }
    // 0049ebc6  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0049ebc9  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049ebcc  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049ebcf  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049ebd2  3c00                   +cmp al, 0
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
    // 0049ebd4  75e8                   -jne 0x49ebbe
    if (!cpu.flags.zf)
    {
        goto L_0x0049ebbe;
    }
L_0x0049ebd6:
    // 0049ebd6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0049ebd7:
    // 0049ebd7  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 0049ebdc  0580020000             -add eax, 0x280
    (cpu.eax) += x86::reg32(x86::sreg32(640 /*0x280*/));
    // 0049ebe1  0fbfd2                 -movsx edx, dx
    cpu.edx = x86::reg32(static_cast<x86::sreg16>(cpu.dx));
    // 0049ebe4  a314e75500             -mov dword ptr [0x55e714], eax
    app->getMemory<x86::reg32>(x86::reg32(5629716) /* 0x55e714 */) = cpu.eax;
    // 0049ebe9  8d45e7                 -lea eax, [ebp - 0x19]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-25) /* -0x19 */);
    // 0049ebec  e8bf030000             -call 0x49efb0
    cpu.esp -= 4;
    nfs3hp::frontend::connectSetUpEndpoint(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ebf1  8b15e4227a00           -mov edx, dword ptr [0x7a22e4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8004324) /* 0x7a22e4 */);
    // 0049ebf7  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049ebf9  83fa02                 +cmp edx, 2
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
    // 0049ebfc  7420                   -je 0x49ec1e
    if (cpu.flags.zf)
    {
        goto L_0x0049ec1e;
    }
    // 0049ebfe  b990037a00             -mov ecx, 0x7a0390
    cpu.ecx = 7996304 /*0x7a0390*/;
    // 0049ec03  bb07000000             -mov ebx, 7
    cpu.ebx = 7 /*0x7*/;
    // 0049ec08  bad2000000             -mov edx, 0xd2
    cpu.edx = 210 /*0xd2*/;
    // 0049ec0d  e83e6c0500             -call 0x4f5850
    cpu.esp -= 4;
    sub_4f5850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ec12  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049ec14  7508                   -jne 0x49ec1e
    if (!cpu.flags.zf)
    {
        goto L_0x0049ec1e;
    }
    // 0049ec16  ff0de0017a00           +dec dword ptr [0x7a01e0]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(7995872) /* 0x7a01e0 */);
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0049ec1c  eb17                   -jmp 0x49ec35
    goto L_0x0049ec35;
L_0x0049ec1e:
    // 0049ec1e  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 0049ec23  66013d76227a00         +add word ptr [0x7a2276], di
    {
        auto tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004214) /* 0x7a2276 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(cpu.di));
        x86::reg16 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) == (1 & (tmp2 >> 15));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0049ec2a  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0049ec2c  66897dfc               -mov word ptr [ebp - 4], di
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.di;
    // 0049ec30  e8ab800000             -call 0x4a6ce0
    cpu.esp -= 4;
    sub_4a6ce0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049ec35:
    // 0049ec35  c70514e75500ffffff0f   -mov dword ptr [0x55e714], 0xfffffff
    app->getMemory<x86::reg32>(x86::reg32(5629716) /* 0x55e714 */) = 268435455 /*0xfffffff*/;
    // 0049ec3f  e926ffffff             -jmp 0x49eb6a
    goto L_0x0049eb6a;
L_0x0049ec44:
    // 0049ec44  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049ec47  66a196227a00           -mov ax, word ptr [0x7a2296]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(8004246) /* 0x7a2296 */);
    // 0049ec4d  6609c2                 +or dx, ax
    cpu.clear_co();
    cpu.set_szp((cpu.dx |= x86::reg16(x86::sreg16(cpu.ax))));
    // 0049ec50  668955fc               -mov word ptr [ebp - 4], dx
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.dx;
    // 0049ec54  7405                   -je 0x49ec5b
    if (cpu.flags.zf)
    {
        goto L_0x0049ec5b;
    }
    // 0049ec56  e8657a0000             -call 0x4a66c0
    cpu.esp -= 4;
    sub_4a66c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049ec5b:
    // 0049ec5b  8b45fa                 -mov eax, dword ptr [ebp - 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 0049ec5e  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
L_0x0049ec61:
    // 0049ec61  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049ec63  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ec64  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ec65  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ec66  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ec67  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ec68  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ec69  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_49ec70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049ec70  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049ec71  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049ec72  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049ec73  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049ec74  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049ec76  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x0049ec7b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049ec7b  3b1514d56f00           +cmp edx, dword ptr [0x6fd514]
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
    // 0049ec81  7d28                   -jge 0x49ecab
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049ecab;
    }
    // 0049ec83  8d0c12                 -lea ecx, [edx + edx]
    cpu.ecx = x86::reg32(cpu.edx + cpu.edx * 1);
    // 0049ec86  6683b9ca227a0002       +cmp word ptr [ecx + 0x7a22ca], 2
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8004298) /* 0x7a22ca */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2 /*0x2*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0049ec8e  7518                   -jne 0x49eca8
    if (!cpu.flags.zf)
    {
        goto L_0x0049eca8;
    }
    // 0049ec90  8b0495f0217a00         -mov eax, dword ptr [edx*4 + 0x7a21f0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004080) /* 0x7a21f0 */ + cpu.edx * 4);
    // 0049ec97  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 0049ec9c  e80f000000             -call 0x49ecb0
    cpu.esp -= 4;
    nfs3hp::ea::dropMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049eca1  6689b1ca227a00         -mov word ptr [ecx + 0x7a22ca], si
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8004298) /* 0x7a22ca */) = cpu.si;
L_0x0049eca8:
    // 0049eca8  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049eca9  ebd0                   -jmp 0x49ec7b
    goto L_0x0049ec7b;
L_0x0049ecab:
    // 0049ecab  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ecac  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ecad  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ecae  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ecaf  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49ecb0: native, nfs3hp::ea::dropMachine */

/* sub_49ecf0: native, nfs3hp::ea::announceMachineLeft */

/* align: skip 0x90 */
void Application::sub_49edb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049edb0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049edb1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049edb2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049edb3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049edb4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049edb6  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 0049edbb  bfca227a00             -mov edi, 0x7a22ca
    cpu.edi = 8004298 /*0x7a22ca*/;
    // 0049edc0  8d7202                 -lea esi, [edx + 2]
    cpu.esi = x86::reg32(cpu.edx + x86::reg32(2) /* 0x2 */);
    // 0049edc3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049edc4  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049edc6  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0049edc9  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049edcb  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0049edcd  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0049edd0  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0049edd2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049edd3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049edd4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049edd5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049edd6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049edd7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_49ede0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049ede0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049ede1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049ede2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049ede4  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049edec  0f845b000000           -je 0x49ee4d
    if (cpu.flags.zf)
    {
        goto L_0x0049ee4d;
    }
    // 0049edf2  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 0049edfa  7551                   -jne 0x49ee4d
    if (!cpu.flags.zf)
    {
        goto L_0x0049ee4d;
    }
    // 0049edfc  833d60e8550000         +cmp dword ptr [0x55e860], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630048) /* 0x55e860 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049ee03  7548                   -jne 0x49ee4d
    if (!cpu.flags.zf)
    {
        goto L_0x0049ee4d;
    }
    // 0049ee05  833de0227a0003         +cmp dword ptr [0x7a22e0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004320) /* 0x7a22e0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049ee0c  753f                   -jne 0x49ee4d
    if (!cpu.flags.zf)
    {
        goto L_0x0049ee4d;
    }
    // 0049ee0e  833de4227a0002         +cmp dword ptr [0x7a22e4], 2
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
    // 0049ee15  742c                   -je 0x49ee43
    if (cpu.flags.zf)
    {
        goto L_0x0049ee43;
    }
    // 0049ee17  a11ce75500             -mov eax, dword ptr [0x55e71c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629724) /* 0x55e71c */);
    // 0049ee1c  e89f030000             -call 0x49f1c0
    cpu.esp -= 4;
    nfs3hp::ea::sessionIndex(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ee21  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0049ee23  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049ee26  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0049ee28  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049ee2b  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0049ee2d  833c8538e7550000       +cmp dword ptr [eax*4 + 0x55e738], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629752) /* 0x55e738 */ + cpu.eax * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049ee35  740c                   -je 0x49ee43
    if (cpu.flags.zf)
    {
        goto L_0x0049ee43;
    }
    // 0049ee37  b890037a00             -mov eax, 0x7a0390
    cpu.eax = 7996304 /*0x7a0390*/;
    // 0049ee3c  e82f610500             -call 0x4f4f70
    cpu.esp -= 4;
    sub_4f4f70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ee41  85c0                   -test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
L_0x0049ee43:
    // 0049ee43  c70560e8550001000000   -mov dword ptr [0x55e860], 1
    app->getMemory<x86::reg32>(x86::reg32(5630048) /* 0x55e860 */) = 1 /*0x1*/;
L_0x0049ee4d:
    // 0049ee4d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ee4e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049ee4f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_49ee50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049ee50  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049ee51  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049ee52  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049ee53  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049ee55  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0049ee5d  0f8461000000           -je 0x49eec4
    if (cpu.flags.zf)
    {
        goto L_0x0049eec4;
    }
    // 0049ee63  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 0049ee6b  7557                   -jne 0x49eec4
    if (!cpu.flags.zf)
    {
        goto L_0x0049eec4;
    }
    // 0049ee6d  833d60e8550000         +cmp dword ptr [0x55e860], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630048) /* 0x55e860 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049ee74  744e                   -je 0x49eec4
    if (cpu.flags.zf)
    {
        goto L_0x0049eec4;
    }
    // 0049ee76  833de0227a0003         +cmp dword ptr [0x7a22e0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004320) /* 0x7a22e0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049ee7d  7545                   -jne 0x49eec4
    if (!cpu.flags.zf)
    {
        goto L_0x0049eec4;
    }
    // 0049ee7f  833de4227a0002         +cmp dword ptr [0x7a22e4], 2
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
    // 0049ee86  742e                   -je 0x49eeb6
    if (cpu.flags.zf)
    {
        goto L_0x0049eeb6;
    }
    // 0049ee88  a11ce75500             -mov eax, dword ptr [0x55e71c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629724) /* 0x55e71c */);
    // 0049ee8d  e82e030000             -call 0x49f1c0
    cpu.esp -= 4;
    nfs3hp::ea::sessionIndex(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049ee92  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0049ee94  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049ee97  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0049ee99  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049ee9c  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0049ee9e  833c8538e7550000       +cmp dword ptr [eax*4 + 0x55e738], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5629752) /* 0x55e738 */ + cpu.eax * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049eea6  740e                   -je 0x49eeb6
    if (cpu.flags.zf)
    {
        goto L_0x0049eeb6;
    }
    // 0049eea8  b890037a00             -mov eax, 0x7a0390
    cpu.eax = 7996304 /*0x7a0390*/;
    // 0049eead  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049eeaf  e89c600500             -call 0x4f4f50
    cpu.esp -= 4;
    nfs3hp::ea::netShutGame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049eeb4  85c0                   -test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
L_0x0049eeb6:
    // 0049eeb6  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0049eeb8  893d10e75500           -mov dword ptr [0x55e710], edi
    app->getMemory<x86::reg32>(x86::reg32(5629712) /* 0x55e710 */) = cpu.edi;
    // 0049eebe  893d60e85500           -mov dword ptr [0x55e860], edi
    app->getMemory<x86::reg32>(x86::reg32(5630048) /* 0x55e860 */) = cpu.edi;
L_0x0049eec4:
    // 0049eec4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049eec5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049eec6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049eec7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49eed0: native, nfs3hp::ea::endNetworkGame */

/* sub_49efb0: native, nfs3hp::frontend::connectSetUpEndpoint */

/* sub_49f030: native, nfs3hp::frontend::connectEnumerateServices */

/* sub_49f150: native, nfs3hp::frontend::connectFindConnections */

/* sub_49f1c0: native, nfs3hp::ea::sessionIndex */

/* sub_49f200: native, nfs3hp::frontend::connectServiceFound */

/* sub_49f380: native, nfs3hp::frontend::connectSessionFound */

/* sub_49f470: native, nfs3hp::frontend::connectQueuePeerByName */

/* sub_49f4b0: native, nfs3hp::frontend::connectQueuePeer */

/* sub_49f520: native, nfs3hp::frontend::connectClearPeerSlots */

/* sub_49f550: native, nfs3hp::ea::forgetPendingPlayer */

/* sub_49f5c0: native, nfs3hp::frontend::connectRememberPeer */

/* sub_49f650: native, nfs3hp::frontend::connectClearQueue */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_49f680(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049f680  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049f681  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049f683  833dec017a0000         +cmp dword ptr [0x7a01ec], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7995884) /* 0x7a01ec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049f68a  7404                   -je 0x49f690
    if (cpu.flags.zf)
    {
        goto L_0x0049f690;
    }
    // 0049f68c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049f68e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f68f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049f690:
    // 0049f690  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049f695  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f696  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49f6a0: native, nfs3hp::frontend::connectEnqueuePeer */

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_49f730(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049f730  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049f731  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049f732  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049f733  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049f734  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049f735  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049f737  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0049f739  8b35e4017a00           -mov esi, dword ptr [0x7a01e4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7995876) /* 0x7a01e4 */);
    // 0049f73f  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0049f741  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049f744  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 0049f746  bef0017a00             -mov esi, 0x7a01f0
    cpu.esi = 7995888 /*0x7a01f0*/;
    // 0049f74b  b91f000000             -mov ecx, 0x1f
    cpu.ecx = 31 /*0x1f*/;
    // 0049f750  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049f752  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049f753  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049f755  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0049f758  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049f75a  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0049f75c  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0049f75f  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0049f761  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f762  8b15e4017a00           -mov edx, dword ptr [0x7a01e4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7995876) /* 0x7a01e4 */);
    // 0049f768  42                     -inc edx
    (cpu.edx)++;
    // 0049f769  be06000000             -mov esi, 6
    cpu.esi = 6 /*0x6*/;
    // 0049f76e  8915e4017a00           -mov dword ptr [0x7a01e4], edx
    app->getMemory<x86::reg32>(x86::reg32(7995876) /* 0x7a01e4 */) = cpu.edx;
    // 0049f774  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049f776  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0049f779  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0049f77b  ff0dec017a00           -dec dword ptr [0x7a01ec]
    (app->getMemory<x86::reg32>(x86::reg32(7995884) /* 0x7a01ec */))--;
    // 0049f781  8915e4017a00           -mov dword ptr [0x7a01e4], edx
    app->getMemory<x86::reg32>(x86::reg32(7995876) /* 0x7a01e4 */) = cpu.edx;
    // 0049f787  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f788  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f789  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f78a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f78b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f78c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 */
void Application::sub_49f790(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049f790  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049f791  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049f792  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049f793  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049f795  81eca4010000           -sub esp, 0x1a4
    (cpu.esp) -= x86::reg32(x86::sreg32(420 /*0x1a4*/));
    // 0049f79b  ba9c010000             -mov edx, 0x19c
    cpu.edx = 412 /*0x19c*/;
    // 0049f7a0  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049f7a2  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 0049f7a5  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 0049f7a8  e863000000             -call 0x49f810
    cpu.esp -= 4;
    sub_49f810(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f7ad  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049f7af  7407                   -je 0x49f7b8
    if (cpu.flags.zf)
    {
        goto L_0x0049f7b8;
    }
    // 0049f7b1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049f7b6  eb48                   -jmp 0x49f800
    goto L_0x0049f800;
L_0x0049f7b8:
    // 0049f7b8  e8a3030000             -call 0x49fb60
    cpu.esp -= 4;
    sub_49fb60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f7bd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049f7bf  7506                   -jne 0x49f7c7
    if (!cpu.flags.zf)
    {
        goto L_0x0049f7c7;
    }
    // 0049f7c1  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049f7c3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f7c4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f7c5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f7c6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049f7c7:
    // 0049f7c7  8d45fc                 -lea eax, [ebp - 4]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049f7ca  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049f7cb  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049f7ce  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049f7cf  8d855cfeffff           -lea eax, [ebp - 0x1a4]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-420) /* -0x1a4 */);
    // 0049f7d5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049f7d6  89955cfeffff           -mov dword ptr [ebp - 0x1a4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-420) /* -0x1a4 */) = cpu.edx;
    // 0049f7dc  ff15cc057a00           -call dword ptr [0x7a05cc]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(7996876) /* 0x7a05cc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f7e2  837df800               +cmp dword ptr [ebp - 8], 0
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
    // 0049f7e6  7506                   -jne 0x49f7ee
    if (!cpu.flags.zf)
    {
        goto L_0x0049f7ee;
    }
    // 0049f7e8  837dfc00               +cmp dword ptr [ebp - 4], 0
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
    // 0049f7ec  7410                   -je 0x49f7fe
    if (cpu.flags.zf)
    {
        goto L_0x0049f7fe;
    }
L_0x0049f7ee:
    // 0049f7ee  e85d040000             -call 0x49fc50
    cpu.esp -= 4;
    sub_49fc50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f7f3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049f7f8  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049f7fa  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f7fb  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f7fc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f7fd  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049f7fe:
    // 0049f7fe  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0049f800:
    // 0049f800  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049f802  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f803  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f804  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f805  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_49f810(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049f810  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049f811  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049f812  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049f813  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049f815  81ec90010000           -sub esp, 0x190
    (cpu.esp) -= x86::reg32(x86::sreg32(400 /*0x190*/));
    // 0049f81b  6a3f                   -push 0x3f
    app->getMemory<x86::reg32>(cpu.esp-4) = 63 /*0x3f*/;
    cpu.esp -= 4;
    // 0049f81d  2eff1550475300         -call dword ptr cs:[0x534750]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457744) /* 0x534750 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f824  a801                   +test al, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 1 /*0x1*/));
    // 0049f826  7407                   -je 0x49f82f
    if (cpu.flags.zf)
    {
        goto L_0x0049f82f;
    }
    // 0049f828  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049f82d  eb41                   -jmp 0x49f870
    goto L_0x0049f870;
L_0x0049f82f:
    // 0049f82f  8d8570feffff           -lea eax, [ebp - 0x190]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-400) /* -0x190 */);
    // 0049f835  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049f836  6801010000             -push 0x101
    app->getMemory<x86::reg32>(cpu.esp-4) = 257 /*0x101*/;
    cpu.esp -= 4;
    // 0049f83b  e870460900             -call 0x533eb0
    cpu.esp -= 4;
    sub_533eb0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f840  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049f842  752a                   -jne 0x49f86e
    if (!cpu.flags.zf)
    {
        goto L_0x0049f86e;
    }
    // 0049f844  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049f845  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0049f847  6a02                   -push 2
    app->getMemory<x86::reg32>(cpu.esp-4) = 2 /*0x2*/;
    cpu.esp -= 4;
    // 0049f849  e85c460900             -call 0x533eaa
    cpu.esp -= 4;
    sub_533eaa(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f84e  83f8ff                 +cmp eax, -1
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
    // 0049f851  7416                   -je 0x49f869
    if (cpu.flags.zf)
    {
        goto L_0x0049f869;
    }
    // 0049f853  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049f854  e84b460900             -call 0x533ea4
    cpu.esp -= 4;
    sub_533ea4(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f859  e840460900             -call 0x533e9e
    cpu.esp -= 4;
    sub_533e9e(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f85e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049f863  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049f865  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f866  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f867  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f868  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049f869:
    // 0049f869  e830460900             -call 0x533e9e
    cpu.esp -= 4;
    sub_533e9e(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049f86e:
    // 0049f86e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0049f870:
    // 0049f870  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049f872  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f873  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f874  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f875  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_49f880(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049f880  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049f881  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049f882  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049f883  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049f884  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049f885  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049f886  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049f888  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0049f88b  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049f88d  c745f8c0190000         -mov dword ptr [ebp - 8], 0x19c0
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = 6592 /*0x19c0*/;
    // 0049f894  e8c7020000             -call 0x49fb60
    cpu.esp -= 4;
    sub_49fb60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f899  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049f89b  0f8490000000           -je 0x49f931
    if (cpu.flags.zf)
    {
        goto L_0x0049f931;
    }
    // 0049f8a1  68e4057a00             -push 0x7a05e4
    app->getMemory<x86::reg32>(cpu.esp-4) = 7996900 /*0x7a05e4*/;
    cpu.esp -= 4;
    // 0049f8a6  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049f8a9  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049f8aa  68e8057a00             -push 0x7a05e8
    app->getMemory<x86::reg32>(cpu.esp-4) = 7996904 /*0x7a05e8*/;
    cpu.esp -= 4;
    // 0049f8af  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049f8b1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049f8b2  bb08010000             -mov ebx, 0x108
    cpu.ebx = 264 /*0x108*/;
    // 0049f8b7  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049f8b8  890de4057a00           -mov dword ptr [0x7a05e4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7996900) /* 0x7a05e4 */) = cpu.ecx;
    // 0049f8be  891de8057a00           -mov dword ptr [0x7a05e8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7996904) /* 0x7a05e8 */) = cpu.ebx;
    // 0049f8c4  ff15d0057a00           -call dword ptr [0x7a05d0]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(7996880) /* 0x7a05d0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f8ca  83f808                 +cmp eax, 8
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
    // 0049f8cd  720b                   -jb 0x49f8da
    if (cpu.flags.cf)
    {
        goto L_0x0049f8da;
    }
    // 0049f8cf  7611                   -jbe 0x49f8e2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0049f8e2;
    }
    // 0049f8d1  3d5b020000             +cmp eax, 0x25b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(603 /*0x25b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049f8d6  740a                   -je 0x49f8e2
    if (cpu.flags.zf)
    {
        goto L_0x0049f8e2;
    }
    // 0049f8d8  eb04                   -jmp 0x49f8de
    goto L_0x0049f8de;
L_0x0049f8da:
    // 0049f8da  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049f8dc  7404                   -je 0x49f8e2
    if (cpu.flags.zf)
    {
        goto L_0x0049f8e2;
    }
L_0x0049f8de:
    // 0049f8de  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0049f8e0  eb4f                   -jmp 0x49f931
    goto L_0x0049f931;
L_0x0049f8e2:
    // 0049f8e2  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0049f8e4  7446                   -je 0x49f92c
    if (cpu.flags.zf)
    {
        goto L_0x0049f92c;
    }
    // 0049f8e6  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0049f8e8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049f8e8  3b0de4057a00           +cmp ecx, dword ptr [0x7a05e4]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7996900) /* 0x7a05e4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049f8ee  733c                   -jae 0x49f92c
    if (!cpu.flags.cf)
    {
        goto L_0x0049f92c;
    }
    // 0049f8f0  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049f8f2  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 0049f8f5  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0049f8f7  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0049f8fa  05e8057a00             -add eax, 0x7a05e8
    (cpu.eax) += x86::reg32(x86::sreg32(7996904 /*0x7a05e8*/));
    // 0049f8ff  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0049f902  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0049f905  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0049f90c  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0049f90e  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049f911  01c8                   +add eax, ecx
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
    // 0049f913  bb14000000             -mov ebx, 0x14
    cpu.ebx = 20 /*0x14*/;
    // 0049f918  8d3c06                 -lea edi, [esi + eax]
    cpu.edi = x86::reg32(cpu.esi + cpu.eax * 1);
    // 0049f91b  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049f91e  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0049f920  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049f921  e80a150400             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049f926  c6471400               -mov byte ptr [edi + 0x14], 0
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(20) /* 0x14 */) = 0 /*0x0*/;
    // 0049f92a  ebbc                   -jmp 0x49f8e8
    goto L_0x0049f8e8;
L_0x0049f92c:
    // 0049f92c  a1e4057a00             -mov eax, dword ptr [0x7a05e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7996900) /* 0x7a05e4 */);
L_0x0049f931:
    // 0049f931  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049f933  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f934  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f935  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f936  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f937  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f938  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049f939  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49f940: native, nfs3hp::dead::stub_0x49f940 */

/* sub_49fa90: native, nfs3hp::dead::stub_0x49fa90 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_49fb10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049fb10  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049fb11  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049fb13  833dc0057a0000         +cmp dword ptr [0x7a05c0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7996864) /* 0x7a05c0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049fb1a  743d                   -je 0x49fb59
    if (cpu.flags.zf)
    {
        goto L_0x0049fb59;
    }
    // 0049fb1c  833dc8057a0000         +cmp dword ptr [0x7a05c8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7996872) /* 0x7a05c8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049fb23  7434                   -je 0x49fb59
    if (cpu.flags.zf)
    {
        goto L_0x0049fb59;
    }
    // 0049fb25  833dcc057a0000         +cmp dword ptr [0x7a05cc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7996876) /* 0x7a05cc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049fb2c  742b                   -je 0x49fb59
    if (cpu.flags.zf)
    {
        goto L_0x0049fb59;
    }
    // 0049fb2e  833dd0057a0000         +cmp dword ptr [0x7a05d0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7996880) /* 0x7a05d0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049fb35  7422                   -je 0x49fb59
    if (cpu.flags.zf)
    {
        goto L_0x0049fb59;
    }
    // 0049fb37  833dd4057a0000         +cmp dword ptr [0x7a05d4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7996884) /* 0x7a05d4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049fb3e  7419                   -je 0x49fb59
    if (cpu.flags.zf)
    {
        goto L_0x0049fb59;
    }
    // 0049fb40  833dd8057a0000         +cmp dword ptr [0x7a05d8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7996888) /* 0x7a05d8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049fb47  7410                   -je 0x49fb59
    if (cpu.flags.zf)
    {
        goto L_0x0049fb59;
    }
    // 0049fb49  833ddc057a0000         +cmp dword ptr [0x7a05dc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7996892) /* 0x7a05dc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049fb50  7407                   -je 0x49fb59
    if (cpu.flags.zf)
    {
        goto L_0x0049fb59;
    }
    // 0049fb52  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049fb57  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fb58  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049fb59:
    // 0049fb59  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049fb5b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fb5c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_49fb60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049fb60  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049fb61  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049fb62  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049fb63  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049fb64  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049fb65  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049fb66  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049fb68  e8a3ffffff             -call 0x49fb10
    cpu.esp -= 4;
    sub_49fb10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049fb6d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049fb6f  740a                   -je 0x49fb7b
    if (cpu.flags.zf)
    {
        goto L_0x0049fb7b;
    }
    // 0049fb71  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049fb76  e9c2000000             -jmp 0x49fc3d
    goto L_0x0049fc3d;
L_0x0049fb7b:
    // 0049fb7b  681cc75300             -push 0x53c71c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5490460 /*0x53c71c*/;
    cpu.esp -= 4;
    // 0049fb80  2eff158c455300         -call dword ptr cs:[0x53458c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457292) /* 0x53458c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049fb87  a3c0057a00             -mov dword ptr [0x7a05c0], eax
    app->getMemory<x86::reg32>(x86::reg32(7996864) /* 0x7a05c0 */) = cpu.eax;
    // 0049fb8c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049fb8e  7507                   -jne 0x49fb97
    if (!cpu.flags.zf)
    {
        goto L_0x0049fb97;
    }
    // 0049fb90  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fb91  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fb92  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fb93  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fb94  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fb95  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fb96  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049fb97:
    // 0049fb97  6828c75300             -push 0x53c728
    app->getMemory<x86::reg32>(cpu.esp-4) = 5490472 /*0x53c728*/;
    cpu.esp -= 4;
    // 0049fb9c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049fb9d  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049fba4  6834c75300             -push 0x53c734
    app->getMemory<x86::reg32>(cpu.esp-4) = 5490484 /*0x53c734*/;
    cpu.esp -= 4;
    // 0049fba9  8b15c0057a00           -mov edx, dword ptr [0x7a05c0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7996864) /* 0x7a05c0 */);
    // 0049fbaf  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049fbb0  a3c8057a00             -mov dword ptr [0x7a05c8], eax
    app->getMemory<x86::reg32>(x86::reg32(7996872) /* 0x7a05c8 */) = cpu.eax;
    // 0049fbb5  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049fbbc  6848c75300             -push 0x53c748
    app->getMemory<x86::reg32>(cpu.esp-4) = 5490504 /*0x53c748*/;
    cpu.esp -= 4;
    // 0049fbc1  8b0dc0057a00           -mov ecx, dword ptr [0x7a05c0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7996864) /* 0x7a05c0 */);
    // 0049fbc7  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049fbc8  a3cc057a00             -mov dword ptr [0x7a05cc], eax
    app->getMemory<x86::reg32>(x86::reg32(7996876) /* 0x7a05cc */) = cpu.eax;
    // 0049fbcd  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049fbd4  6858c75300             -push 0x53c758
    app->getMemory<x86::reg32>(cpu.esp-4) = 5490520 /*0x53c758*/;
    cpu.esp -= 4;
    // 0049fbd9  8b1dc0057a00           -mov ebx, dword ptr [0x7a05c0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7996864) /* 0x7a05c0 */);
    // 0049fbdf  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049fbe0  a3d0057a00             -mov dword ptr [0x7a05d0], eax
    app->getMemory<x86::reg32>(x86::reg32(7996880) /* 0x7a05d0 */) = cpu.eax;
    // 0049fbe5  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049fbec  6870c75300             -push 0x53c770
    app->getMemory<x86::reg32>(cpu.esp-4) = 5490544 /*0x53c770*/;
    cpu.esp -= 4;
    // 0049fbf1  8b35c0057a00           -mov esi, dword ptr [0x7a05c0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7996864) /* 0x7a05c0 */);
    // 0049fbf7  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049fbf8  a3d4057a00             -mov dword ptr [0x7a05d4], eax
    app->getMemory<x86::reg32>(x86::reg32(7996884) /* 0x7a05d4 */) = cpu.eax;
    // 0049fbfd  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049fc04  687cc75300             -push 0x53c77c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5490556 /*0x53c77c*/;
    cpu.esp -= 4;
    // 0049fc09  8b3dc0057a00           -mov edi, dword ptr [0x7a05c0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7996864) /* 0x7a05c0 */);
    // 0049fc0f  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0049fc10  a3d8057a00             -mov dword ptr [0x7a05d8], eax
    app->getMemory<x86::reg32>(x86::reg32(7996888) /* 0x7a05d8 */) = cpu.eax;
    // 0049fc15  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049fc1c  a3dc057a00             -mov dword ptr [0x7a05dc], eax
    app->getMemory<x86::reg32>(x86::reg32(7996892) /* 0x7a05dc */) = cpu.eax;
    // 0049fc21  e8eafeffff             -call 0x49fb10
    cpu.esp -= 4;
    sub_49fb10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049fc26  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0049fc28  740c                   -je 0x49fc36
    if (cpu.flags.zf)
    {
        goto L_0x0049fc36;
    }
    // 0049fc2a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049fc2f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc30  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc31  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc32  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc33  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc34  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc35  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049fc36:
    // 0049fc36  e815000000             -call 0x49fc50
    cpu.esp -= 4;
    sub_49fc50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049fc3b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0049fc3d:
    // 0049fc3d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc3e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc3f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc40  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc41  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc42  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc43  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_49fc50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0049fc50  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049fc51  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049fc52  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0049fc53  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0049fc55  8b15c0057a00           -mov edx, dword ptr [0x7a05c0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7996864) /* 0x7a05c0 */);
    // 0049fc5b  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0049fc5d  7417                   -je 0x49fc76
    if (cpu.flags.zf)
    {
        goto L_0x0049fc76;
    }
    // 0049fc5f  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049fc60  2eff15dc445300         -call dword ptr cs:[0x5344dc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457116) /* 0x5344dc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049fc67  ba24000000             -mov edx, 0x24
    cpu.edx = 36 /*0x24*/;
    // 0049fc6c  b8c0057a00             -mov eax, 0x7a05c0
    cpu.eax = 7996864 /*0x7a05c0*/;
    // 0049fc71  e8960a0400             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049fc76:
    // 0049fc76  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc77  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc78  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049fc79  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_49fc80: native, nfs3hp::dead::stub_0x49fc80 */

/* sub_49fda0: native, nfs3hp::dead::stub_0x49fda0 */

/* sub_49fdd0: native, nfs3hp::dead::stub_0x49fdd0 */

/* sub_49fde0: native, nfs3hp::frontend::simCarTiltedOver */

/* sub_49fe70: native, nfs3hp::port::conv::fn49fe70 */

/* sub_49fee0: native, nfs3hp::frontend::simSliceHeading */

/* sub_49ffa0: native, nfs3hp::frontend::simCarTrackFrame */

/* sub_4a00c0: native, nfs3hp::frontend::simNodeGap */

/* sub_4a00d0: native, nfs3hp::port::conv::fn4a00d0 */

/* sub_4a0140: native, nfs3hp::frontend::simCarOntoRoad */

/* sub_4a0440: native, nfs3hp::port::conv::fn4a0440 */

/* sub_4a0580: native, nfs3hp::frontend::simCarNearPlayers */

/* sub_4a08c0: native, nfs3hp::frontend::simCarTrackEnvironment */

/* sub_4a0980: native, nfs3hp::port::conv::fn4a0980 */

/* sub_4a0a10: native, nfs3hp::port::conv::fn4a0a10 */

/* sub_4a0aa0: native, nfs3hp::frontend::simTrackPlaceOnRoad */

/* sub_4a0d20: native, nfs3hp::port::conv::fn4a0d20 */

/* sub_4a0f30: native, nfs3hp::port::conv::fn4a0f30 */

}
