#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4262a0: native, nfs3hp::frontend::simCameraWaitThenHandOn */

/* sub_426310: native, nfs3hp::frontend::simCameraBackFromWait */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_426370(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00426370  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00426371  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00426372  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00426373  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00426374  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00426376  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00426378  8b7004                 -mov esi, dword ptr [eax + 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0042637b  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0042637d  7452                   -je 0x4263d1
    if (cpu.flags.zf)
    {
        goto L_0x004263d1;
    }
    // 0042637f  8b7e1c                 -mov edi, dword ptr [esi + 0x1c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(28) /* 0x1c */);
    // 00426382  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00426383  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00426385  8bb6a0000000           -mov esi, dword ptr [esi + 0xa0]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(160) /* 0xa0 */);
    // 0042638b  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0042638c  8bb89c000000           -mov edi, dword ptr [eax + 0x9c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(156) /* 0x9c */);
    // 00426392  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00426393  8bb098000000           -mov esi, dword ptr [eax + 0x98]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(152) /* 0x98 */);
    // 00426399  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0042639a  e851a8ffff             -call 0x420bf0
    cpu.esp -= 4;
    nfs3hp::trackdata::inTunnel(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042639f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004263a1  7425                   -je 0x4263c8
    if (cpu.flags.zf)
    {
        goto L_0x004263c8;
    }
    // 004263a3  8bb984000000           -mov edi, dword ptr [ecx + 0x84]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(132) /* 0x84 */);
    // 004263a9  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004263aa  8b411c                 -mov eax, dword ptr [ecx + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */);
    // 004263ad  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004263ae  8b7118                 -mov esi, dword ptr [ecx + 0x18]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */);
    // 004263b1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004263b2  8b7914                 -mov edi, dword ptr [ecx + 0x14]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */);
    // 004263b5  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004263b6  e835a8ffff             -call 0x420bf0
    cpu.esp -= 4;
    nfs3hp::trackdata::inTunnel(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004263bb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004263bd  7409                   -je 0x4263c8
    if (cpu.flags.zf)
    {
        goto L_0x004263c8;
    }
    // 004263bf  ff04d514105e00         +inc dword ptr [edx*8 + 0x5e1014]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(6164500) /* 0x5e1014 */ + cpu.edx * 8);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004263c6  eb09                   -jmp 0x4263d1
    goto L_0x004263d1;
L_0x004263c8:
    // 004263c8  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004263ca  8934d514105e00         -mov dword ptr [edx*8 + 0x5e1014], esi
    app->getMemory<x86::reg32>(x86::reg32(6164500) /* 0x5e1014 */ + cpu.edx * 8) = cpu.esi;
L_0x004263d1:
    // 004263d1  83fb08                 +cmp ebx, 8
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
    // 004263d4  7414                   -je 0x4263ea
    if (cpu.flags.zf)
    {
        goto L_0x004263ea;
    }
    // 004263d6  8b410c                 -mov eax, dword ptr [ecx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 004263d9  8b0485082f5500         -mov eax, dword ptr [eax*4 + 0x552f08]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5582600) /* 0x552f08 */ + cpu.eax * 4);
    // 004263e0  c7802401000000000000   -mov dword ptr [eax + 0x124], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(292) /* 0x124 */) = 0 /*0x0*/;
L_0x004263ea:
    // 004263ea  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004263eb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004263ec  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004263ed  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004263ee  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_426450: native, nfs3hp::frontend::simCameraStep */

/* sub_426560: native, nfs3hp::frontend::simCameraForcedMode */

/* sub_426680: native, nfs3hp::frontend::simCamerasStep */

/* sub_4267a0: native, nfs3hp::frontend::carSaveFrame */

/* sub_4267f0: native, nfs3hp::frontend::trackSaveFrame */

/* sub_426870: native, nfs3hp::frontend::carRecord */

/* sub_426b00: native, nfs3hp::frontend::trackRecord */

/* sub_426c20: native, nfs3hp::frontend::car1EditClicked */

/* sub_426c40: native, nfs3hp::frontend::car2EditClicked */

/* sub_426c60: native, nfs3hp::frontend::car1ColourClicked */

/* sub_426ce0: native, nfs3hp::frontend::car2ColourClicked */

/* sub_426d60: native, nfs3hp::frontend::car1OnEnter */

/* sub_426e20: native, nfs3hp::frontend::car1OnFrame */

/* sub_426ee0: native, nfs3hp::frontend::car1OnExit */

/* sub_426ef0: native, nfs3hp::frontend::car2OnEnter */

/* sub_426f80: native, nfs3hp::frontend::car2OnFrame */

/* sub_427040: native, nfs3hp::frontend::car2OnExit */

/* sub_427050: native, nfs3hp::frontend::opponentFitCar */

/* sub_4270c0: native, nfs3hp::frontend::opponentFitArt */

/* sub_427190: native, nfs3hp::frontend::opponentOnFrame */

/* sub_4272a0: native, nfs3hp::frontend::opponentOnEnter */

/* sub_4272e0: native, nfs3hp::frontend::opponentOnExit */

/* sub_4272f0: native, nfs3hp::frontend::tuningLoad */

/* sub_427370: native, nfs3hp::frontend::tuningSave */

/* sub_4273e0: native, nfs3hp::frontend::tuningEngineMoved */

/* sub_427470: native, nfs3hp::frontend::tuningBrakesMoved */

/* sub_427500: native, nfs3hp::frontend::tuningSteeringMoved */

/* sub_427590: native, nfs3hp::frontend::tuningGearsMoved */

/* sub_427620: native, nfs3hp::frontend::tuningSuspensionMoved */

/* sub_4276b0: native, nfs3hp::frontend::tuningAeroMoved */

/* sub_427740: native, nfs3hp::frontend::tuningTiresMoved */

/* sub_4277d0: native, nfs3hp::frontend::tuningSetSliders */

/* sub_4279b0: native, nfs3hp::frontend::tuningRememberSliders */

/* sub_427ac0: native, nfs3hp::frontend::tuningSlidersMoved */

/* sub_427c20: native, nfs3hp::frontend::tuningBindSliders */

/* sub_427d00: native, nfs3hp::frontend::tuningLoadAdvanced */

/* sub_427d80: native, nfs3hp::frontend::tuningDefaultsAnswered */

/* sub_427e10: native, nfs3hp::frontend::tuningDoneKey */

/* sub_427ea0: native, nfs3hp::frontend::tuningCancelKey */

/* sub_427f40: native, nfs3hp::frontend::tuningCarListKey */

/* sub_427fb0: native, nfs3hp::frontend::tuningDefaultsKey */

/* sub_428020: native, nfs3hp::frontend::tuningBindDoneCancel */

/* sub_428060: native, nfs3hp::frontend::tuningFitButtons */

/* sub_4281e0: native, nfs3hp::frontend::tuningPrepare */

/* sub_428360: native, nfs3hp::frontend::tuningOnEnter */

/* sub_4283b0: native, nfs3hp::frontend::tuningOnExit */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4283d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004283d0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004283d1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004283d3  e8f8af0700             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004283d8  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004283df  7407                   -je 0x4283e8
    if (cpu.flags.zf)
    {
        goto L_0x004283e8;
    }
    // 004283e1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004283e6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004283e7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004283e8:
    // 004283e8  e873f80800             -call 0x4b7c60
    cpu.esp -= 4;
    sub_4b7c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004283ed  e8deaf0700             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004283f2  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004283f9  7407                   -je 0x428402
    if (cpu.flags.zf)
    {
        goto L_0x00428402;
    }
    // 004283fb  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00428400  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428401  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00428402:
    // 00428402  e889410000             -call 0x42c590
    cpu.esp -= 4;
    nfs3hp::decompiled::resetHandlingTunings(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428407  e8c4af0700             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042840c  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 00428413  7407                   -je 0x42841c
    if (cpu.flags.zf)
    {
        goto L_0x0042841c;
    }
    // 00428415  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0042841a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042841b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0042841c:
    // 0042841c  e8df420000             -call 0x42c700
    cpu.esp -= 4;
    nfs3hp::raceload::loadRaceCars(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428421  e8aaaf0700             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428426  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 0042842d  7407                   -je 0x428436
    if (cpu.flags.zf)
    {
        goto L_0x00428436;
    }
    // 0042842f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00428434  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428435  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00428436:
    // 00428436  e8b5430000             -call 0x42c7f0
    cpu.esp -= 4;
    nfs3hp::raceload::setTimeStep(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042843b  e890af0700             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428440  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 00428447  7407                   -je 0x428450
    if (cpu.flags.zf)
    {
        goto L_0x00428450;
    }
    // 00428449  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0042844e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042844f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00428450:
    // 00428450  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00428452  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428453  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_428460(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00428460  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00428462  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00428464  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00428466  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00428468  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0042846a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0042846c  8d442000               -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 00428470  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00428471  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00428473  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428474  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_428480: native, nfs3hp::dead::stub_0x428480 */

/* sub_4284b0: native, nfs3hp::dead::stub_0x4284b0 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4284e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  x86::reg16 fsw = 0;
    // 004284e0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004284e1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004284e2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004284e3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004284e5  83ec1c                 -sub esp, 0x1c
    (cpu.esp) -= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 004284e8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004284ea  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 004284ec  83b8dc07000008         +cmp dword ptr [eax + 0x7dc], 8
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(2012) /* 0x7dc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004284f3  0f8deb000000           -jge 0x4285e4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004285e4;
    }
    // 004284f9  d9ee                   +fldz 
    fp0 = 0.0;
    // 004284fb  d85d14                 +fcomp dword ptr [ebp + 0x14]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(20) /* 0x14 */)));
    // 004284fe  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00428500  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00428501  7607                   -jbe 0x42850a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0042850a;
    }
    // 00428503  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00428505  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 00428508  eb35                   -jmp 0x42853f
    goto L_0x0042853f;
L_0x0042850a:
    // 0042850a  d94514                 +fld dword ptr [ebp + 0x14]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(20) /* 0x14 */));
    // 0042850d  dd55e4                 +fst qword ptr [ebp - 0x1c]
    app->getMemory<double>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = double(fp0);
    // 00428510  dc1d98715300           +fcomp qword ptr [0x537198]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5468568) /* 0x537198 */)));
    // 00428516  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00428518  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00428519  760f                   -jbe 0x42852a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0042852a;
    }
    // 0042851b  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0042851d  b900002440             -mov ecx, 0x40240000
    cpu.ecx = 1076101120 /*0x40240000*/;
    // 00428522  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00428525  894df8                 -mov dword ptr [ebp - 8], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ecx;
    // 00428528  eb0c                   -jmp 0x428536
    goto L_0x00428536;
L_0x0042852a:
    // 0042852a  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0042852d  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00428530  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00428533  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
L_0x00428536:
    // 00428536  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00428539  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0042853c  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
L_0x0042853f:
    // 0042853f  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 00428542  8b8edc070000           -mov ecx, dword ptr [esi + 0x7dc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2012) /* 0x7dc */);
    // 00428548  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0042854f  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00428551  8994c6e0070000         -mov dword ptr [esi + eax*8 + 0x7e0], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2016) /* 0x7e0 */ + cpu.eax * 8) = cpu.edx;
    // 00428558  8b96dc070000           -mov edx, dword ptr [esi + 0x7dc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2012) /* 0x7dc */);
    // 0042855e  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00428565  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00428567  899cc6e4070000         -mov dword ptr [esi + eax*8 + 0x7e4], ebx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2020) /* 0x7e4 */ + cpu.eax * 8) = cpu.ebx;
    // 0042856e  8b96dc070000           -mov edx, dword ptr [esi + 0x7dc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2012) /* 0x7dc */);
    // 00428574  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0042857b  dd45ec                 -fld qword ptr [ebp - 0x14]
    fp0 = x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-20) /* -0x14 */));
    // 0042857e  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00428580  dc0da0715300           -fmul qword ptr [0x5371a0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5468576) /* 0x5371a0 */)));
    // 00428586  89bcc6e8070000         -mov dword ptr [esi + eax*8 + 0x7e8], edi
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2024) /* 0x7e8 */ + cpu.eax * 8) = cpu.edi;
    // 0042858d  8b96dc070000           -mov edx, dword ptr [esi + 0x7dc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2012) /* 0x7dc */);
    // 00428593  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0042859a  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0042859c  8b5510                 -mov edx, dword ptr [ebp + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 0042859f  8994c6ec070000         -mov dword ptr [esi + eax*8 + 0x7ec], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2028) /* 0x7ec */ + cpu.eax * 8) = cpu.edx;
    // 004285a6  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004285a9  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004285ac  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004285ad  8b8edc070000           -mov ecx, dword ptr [esi + 0x7dc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2012) /* 0x7dc */);
    // 004285b3  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004285b5  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004285bc  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004285be  8994c6f0070000         -mov dword ptr [esi + eax*8 + 0x7f0], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2032) /* 0x7f0 */ + cpu.eax * 8) = cpu.edx;
    // 004285c5  8b96dc070000           -mov edx, dword ptr [esi + 0x7dc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2012) /* 0x7dc */);
    // 004285cb  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004285d2  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004285d4  8b5518                 -mov edx, dword ptr [ebp + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 004285d7  8994c6f4070000         -mov dword ptr [esi + eax*8 + 0x7f4], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2036) /* 0x7f4 */ + cpu.eax * 8) = cpu.edx;
    // 004285de  ff86dc070000           -inc dword ptr [esi + 0x7dc]
    (app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2012) /* 0x7dc */))++;
L_0x004285e4:
    // 004285e4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004285e6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004285e7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004285e8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004285e9  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4285f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004285f0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004285f1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004285f2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004285f4  0524020000             -add eax, 0x224
    (cpu.eax) += x86::reg32(x86::sreg32(548 /*0x224*/));
    // 004285f9  c7400400000000         -mov dword ptr [eax + 4], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = 0 /*0x0*/;
    // 00428600  c7400800000000         -mov dword ptr [eax + 8], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 00428607  c7400c00000000         -mov dword ptr [eax + 0xc], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = 0 /*0x0*/;
    // 0042860e  c7401000000000         -mov dword ptr [eax + 0x10], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */) = 0 /*0x0*/;
    // 00428615  c7401400020000         -mov dword ptr [eax + 0x14], 0x200
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */) = 512 /*0x200*/;
    // 0042861c  c7403c00000000         -mov dword ptr [eax + 0x3c], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */) = 0 /*0x0*/;
    // 00428623  c7404000000000         -mov dword ptr [eax + 0x40], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 0 /*0x0*/;
    // 0042862a  c7404400000000         -mov dword ptr [eax + 0x44], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(68) /* 0x44 */) = 0 /*0x0*/;
    // 00428631  c7404c00000000         -mov dword ptr [eax + 0x4c], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(76) /* 0x4c */) = 0 /*0x0*/;
    // 00428638  c7405400000000         -mov dword ptr [eax + 0x54], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(84) /* 0x54 */) = 0 /*0x0*/;
    // 0042863f  c7405c00000000         -mov dword ptr [eax + 0x5c], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(92) /* 0x5c */) = 0 /*0x0*/;
    // 00428646  c7406400000000         -mov dword ptr [eax + 0x64], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 0 /*0x0*/;
    // 0042864d  c7404800000000         -mov dword ptr [eax + 0x48], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(72) /* 0x48 */) = 0 /*0x0*/;
    // 00428654  c7405000000000         -mov dword ptr [eax + 0x50], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(80) /* 0x50 */) = 0 /*0x0*/;
    // 0042865b  c7405800000000         -mov dword ptr [eax + 0x58], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(88) /* 0x58 */) = 0 /*0x0*/;
    // 00428662  c7406000000000         -mov dword ptr [eax + 0x60], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(96) /* 0x60 */) = 0 /*0x0*/;
    // 00428669  c7406800000000         -mov dword ptr [eax + 0x68], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(104) /* 0x68 */) = 0 /*0x0*/;
    // 00428670  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00428672  c70000000000           -mov dword ptr [eax], 0
    app->getMemory<x86::reg32>(cpu.eax) = 0 /*0x0*/;
    // 00428678  eb05                   -jmp 0x42867f
    goto L_0x0042867f;
L_0x0042867a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0042867a  83fa08                 +cmp edx, 8
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
    // 0042867d  7d13                   -jge 0x428692
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00428692;
    }
L_0x0042867f:
    // 0042867f  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00428680  c744901800000000       -mov dword ptr [eax + edx*4 + 0x18], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */ + cpu.edx * 4) = 0 /*0x0*/;
    // 00428688  c744907000000000       -mov dword ptr [eax + edx*4 + 0x70], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(112) /* 0x70 */ + cpu.edx * 4) = 0 /*0x0*/;
    // 00428690  ebe8                   -jmp 0x42867a
    goto L_0x0042867a;
L_0x00428692:
    // 00428692  c7809800000000000000   -mov dword ptr [eax + 0x98], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(152) /* 0x98 */) = 0 /*0x0*/;
    // 0042869c  c7809c00000000000000   -mov dword ptr [eax + 0x9c], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(156) /* 0x9c */) = 0 /*0x0*/;
    // 004286a6  c780a000000000000000   -mov dword ptr [eax + 0xa0], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(160) /* 0xa0 */) = 0 /*0x0*/;
    // 004286b0  c780a800000000000000   -mov dword ptr [eax + 0xa8], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(168) /* 0xa8 */) = 0 /*0x0*/;
    // 004286ba  c780ac00000000000000   -mov dword ptr [eax + 0xac], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(172) /* 0xac */) = 0 /*0x0*/;
    // 004286c4  c780b000000000000000   -mov dword ptr [eax + 0xb0], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(176) /* 0xb0 */) = 0 /*0x0*/;
    // 004286ce  c780b400000000000000   -mov dword ptr [eax + 0xb4], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(180) /* 0xb4 */) = 0 /*0x0*/;
    // 004286d8  c780b800000000000000   -mov dword ptr [eax + 0xb8], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(184) /* 0xb8 */) = 0 /*0x0*/;
    // 004286e2  c780bc00000000000000   -mov dword ptr [eax + 0xbc], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(188) /* 0xbc */) = 0 /*0x0*/;
    // 004286ec  c7809400000000000000   -mov dword ptr [eax + 0x94], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(148) /* 0x94 */) = 0 /*0x0*/;
    // 004286f6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004286f7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004286f8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_428700(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00428700  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00428701  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00428702  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00428703  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00428704  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00428705  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00428706  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00428708  8b1d98fd5e00           -mov ebx, dword ptr [0x5efd98]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(6225304) /* 0x5efd98 */);
    // 0042870e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00428710  8b8020020000           -mov eax, dword ptr [eax + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(544) /* 0x220 */);
    // 00428716  8b4818                 -mov ecx, dword ptr [eax + 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 00428719  a19cfd5e00             -mov eax, dword ptr [0x5efd9c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225308) /* 0x5efd9c */);
    // 0042871e  891485c8fa5e00         -mov dword ptr [eax*4 + 0x5efac8], edx
    app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */ + cpu.eax * 4) = cpu.edx;
    // 00428725  891485c8fc5e00         -mov dword ptr [eax*4 + 0x5efcc8], edx
    app->getMemory<x86::reg32>(x86::reg32(6225096) /* 0x5efcc8 */ + cpu.eax * 4) = cpu.edx;
    // 0042872c  89148588fc5e00         -mov dword ptr [eax*4 + 0x5efc88], edx
    app->getMemory<x86::reg32>(x86::reg32(6225032) /* 0x5efc88 */ + cpu.eax * 4) = cpu.edx;
    // 00428733  c7820002000000000000   -mov dword ptr [edx + 0x200], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(512) /* 0x200 */) = 0 /*0x0*/;
    // 0042873d  c782f4010000ffffffff   -mov dword ptr [edx + 0x1f4], 0xffffffff
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(500) /* 0x1f4 */) = 4294967295 /*0xffffffff*/;
    // 00428747  8b359cfd5e00           -mov esi, dword ptr [0x5efd9c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(6225308) /* 0x5efd9c */);
    // 0042874d  8982f0010000           -mov dword ptr [edx + 0x1f0], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(496) /* 0x1f0 */) = cpu.eax;
    // 00428753  46                     -inc esi
    (cpu.esi)++;
    // 00428754  8b8220020000           -mov eax, dword ptr [edx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(544) /* 0x220 */);
    // 0042875a  c782f8010000ffffffff   -mov dword ptr [edx + 0x1f8], 0xffffffff
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(504) /* 0x1f8 */) = 4294967295 /*0xffffffff*/;
    // 00428764  89359cfd5e00           -mov dword ptr [0x5efd9c], esi
    app->getMemory<x86::reg32>(x86::reg32(6225308) /* 0x5efd9c */) = cpu.esi;
    // 0042876a  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 0042876d  8dbae4010000           -lea edi, [edx + 0x1e4]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(484) /* 0x1e4 */);
    // 00428773  e8380b0100             -call 0x4392b0
    cpu.esp -= 4;
    nfs3hp::load::carFolder(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428778  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0042877a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0042877b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0042877b  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0042877d  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0042877f  3c00                   +cmp al, 0
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
    // 00428781  7410                   -je 0x428793
    if (cpu.flags.zf)
    {
        goto L_0x00428793;
    }
    // 00428783  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00428786  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00428789  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0042878c  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0042878f  3c00                   +cmp al, 0
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
    // 00428791  75e8                   -jne 0x42877b
    if (!cpu.flags.zf)
    {
        goto L_0x0042877b;
    }
L_0x00428793:
    // 00428793  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428794  f6c101                 +test cl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 1 /*0x1*/));
    // 00428797  0f84c4000000           -je 0x428861
    if (cpu.flags.zf)
    {
        goto L_0x00428861;
    }
    // 0042879d  a1a4fd5e00             -mov eax, dword ptr [0x5efda4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */);
    // 004287a2  8982f4010000           -mov dword ptr [edx + 0x1f4], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(500) /* 0x1f4 */) = cpu.eax;
    // 004287a8  8b3da4fd5e00           -mov edi, dword ptr [0x5efda4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */);
    // 004287ae  89148548fa5e00         -mov dword ptr [eax*4 + 0x5efa48], edx
    app->getMemory<x86::reg32>(x86::reg32(6224456) /* 0x5efa48 */ + cpu.eax * 4) = cpu.edx;
    // 004287b5  8b1d98fd5e00           -mov ebx, dword ptr [0x5efd98]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(6225304) /* 0x5efd98 */);
    // 004287bb  8aa200020000           -mov ah, byte ptr [edx + 0x200]
    cpu.ah = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */);
    // 004287c1  47                     -inc edi
    (cpu.edi)++;
    // 004287c2  80cc04                 -or ah, 4
    cpu.ah |= x86::reg8(x86::sreg8(4 /*0x4*/));
    // 004287c5  893da4fd5e00           -mov dword ptr [0x5efda4], edi
    app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */) = cpu.edi;
    // 004287cb  88a200020000           -mov byte ptr [edx + 0x200], ah
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */) = cpu.ah;
    // 004287d1  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 004287d4  7440                   -je 0x428816
    if (cpu.flags.zf)
    {
        goto L_0x00428816;
    }
    // 004287d6  a188fd5e00             -mov eax, dword ptr [0x5efd88]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225288) /* 0x5efd88 */);
    // 004287db  89148588fa5e00         -mov dword ptr [eax*4 + 0x5efa88], edx
    app->getMemory<x86::reg32>(x86::reg32(6224520) /* 0x5efa88 */ + cpu.eax * 4) = cpu.edx;
    // 004287e2  a178305500             -mov eax, dword ptr [0x553078]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5582968) /* 0x553078 */);
    // 004287e7  89148508fc5e00         -mov dword ptr [eax*4 + 0x5efc08], edx
    app->getMemory<x86::reg32>(x86::reg32(6224904) /* 0x5efc08 */ + cpu.eax * 4) = cpu.edx;
    // 004287ee  8a8200020000           -mov al, byte ptr [edx + 0x200]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */);
    // 004287f4  0c20                   +or al, 0x20
    cpu.clear_co();
    cpu.set_szp((cpu.al |= x86::reg8(x86::sreg8(32 /*0x20*/))));
    // 004287f6  8b3578305500           -mov esi, dword ptr [0x553078]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5582968) /* 0x553078 */);
    // 004287fc  888200020000           -mov byte ptr [edx + 0x200], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */) = cpu.al;
    // 00428802  a188fd5e00             -mov eax, dword ptr [0x5efd88]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225288) /* 0x5efd88 */);
    // 00428807  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00428808  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00428809  893578305500           -mov dword ptr [0x553078], esi
    app->getMemory<x86::reg32>(x86::reg32(5582968) /* 0x553078 */) = cpu.esi;
    // 0042880f  a388fd5e00             -mov dword ptr [0x5efd88], eax
    app->getMemory<x86::reg32>(x86::reg32(6225288) /* 0x5efd88 */) = cpu.eax;
    // 00428814  eb45                   -jmp 0x42885b
    goto L_0x0042885b;
L_0x00428816:
    // 00428816  a1a0fd5e00             -mov eax, dword ptr [0x5efda0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225312) /* 0x5efda0 */);
    // 0042881b  8982f8010000           -mov dword ptr [edx + 0x1f8], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(504) /* 0x1f8 */) = cpu.eax;
    // 00428821  89148548fc5e00         -mov dword ptr [eax*4 + 0x5efc48], edx
    app->getMemory<x86::reg32>(x86::reg32(6224968) /* 0x5efc48 */ + cpu.eax * 4) = cpu.edx;
    // 00428828  a174305500             -mov eax, dword ptr [0x553074]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5582964) /* 0x553074 */);
    // 0042882d  89148508fd5e00         -mov dword ptr [eax*4 + 0x5efd08], edx
    app->getMemory<x86::reg32>(x86::reg32(6225160) /* 0x5efd08 */ + cpu.eax * 4) = cpu.edx;
    // 00428834  8aa200020000           -mov ah, byte ptr [edx + 0x200]
    cpu.ah = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */);
    // 0042883a  80cc01                 -or ah, 1
    cpu.ah |= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 0042883d  8b3da0fd5e00           -mov edi, dword ptr [0x5efda0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(6225312) /* 0x5efda0 */);
    // 00428843  88a200020000           -mov byte ptr [edx + 0x200], ah
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */) = cpu.ah;
    // 00428849  a174305500             -mov eax, dword ptr [0x553074]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5582964) /* 0x553074 */);
    // 0042884e  47                     -inc edi
    (cpu.edi)++;
    // 0042884f  40                     -inc eax
    (cpu.eax)++;
    // 00428850  893da0fd5e00           -mov dword ptr [0x5efda0], edi
    app->getMemory<x86::reg32>(x86::reg32(6225312) /* 0x5efda0 */) = cpu.edi;
    // 00428856  a374305500             -mov dword ptr [0x553074], eax
    app->getMemory<x86::reg32>(x86::reg32(5582964) /* 0x553074 */) = cpu.eax;
L_0x0042885b:
    // 0042885b  891d98fd5e00           -mov dword ptr [0x5efd98], ebx
    app->getMemory<x86::reg32>(x86::reg32(6225304) /* 0x5efd98 */) = cpu.ebx;
L_0x00428861:
    // 00428861  8b1d98fd5e00           -mov ebx, dword ptr [0x5efd98]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(6225304) /* 0x5efd98 */);
    // 00428867  f6c102                 +test cl, 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 2 /*0x2*/));
    // 0042886a  7464                   -je 0x4288d0
    if (cpu.flags.zf)
    {
        goto L_0x004288d0;
    }
    // 0042886c  89149d08fb5e00         -mov dword ptr [ebx*4 + 0x5efb08], edx
    app->getMemory<x86::reg32>(x86::reg32(6224648) /* 0x5efb08 */ + cpu.ebx * 4) = cpu.edx;
    // 00428873  808a0002000002         -or byte ptr [edx + 0x200], 2
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */) |= x86::reg8(x86::sreg8(2 /*0x2*/));
    // 0042887a  a194fd5e00             -mov eax, dword ptr [0x5efd94]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225300) /* 0x5efd94 */);
    // 0042887f  89148588fb5e00         -mov dword ptr [eax*4 + 0x5efb88], edx
    app->getMemory<x86::reg32>(x86::reg32(6224776) /* 0x5efb88 */ + cpu.eax * 4) = cpu.edx;
    // 00428886  8aa200020000           -mov ah, byte ptr [edx + 0x200]
    cpu.ah = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */);
    // 0042888c  8b3594fd5e00           -mov esi, dword ptr [0x5efd94]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(6225300) /* 0x5efd94 */);
    // 00428892  80cc08                 -or ah, 8
    cpu.ah |= x86::reg8(x86::sreg8(8 /*0x8*/));
    // 00428895  8b3da0fd5e00           -mov edi, dword ptr [0x5efda0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(6225312) /* 0x5efda0 */);
    // 0042889b  88a200020000           -mov byte ptr [edx + 0x200], ah
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */) = cpu.ah;
    // 004288a1  a1a0fd5e00             -mov eax, dword ptr [0x5efda0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225312) /* 0x5efda0 */);
    // 004288a6  43                     -inc ebx
    (cpu.ebx)++;
    // 004288a7  8982f8010000           -mov dword ptr [edx + 0x1f8], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(504) /* 0x1f8 */) = cpu.eax;
    // 004288ad  46                     -inc esi
    (cpu.esi)++;
    // 004288ae  89148548fc5e00         -mov dword ptr [eax*4 + 0x5efc48], edx
    app->getMemory<x86::reg32>(x86::reg32(6224968) /* 0x5efc48 */ + cpu.eax * 4) = cpu.edx;
    // 004288b5  47                     -inc edi
    (cpu.edi)++;
    // 004288b6  8a8200020000           -mov al, byte ptr [edx + 0x200]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */);
    // 004288bc  893594fd5e00           -mov dword ptr [0x5efd94], esi
    app->getMemory<x86::reg32>(x86::reg32(6225300) /* 0x5efd94 */) = cpu.esi;
    // 004288c2  0c01                   -or al, 1
    cpu.al |= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 004288c4  893da0fd5e00           -mov dword ptr [0x5efda0], edi
    app->getMemory<x86::reg32>(x86::reg32(6225312) /* 0x5efda0 */) = cpu.edi;
    // 004288ca  888200020000           -mov byte ptr [edx + 0x200], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */) = cpu.al;
L_0x004288d0:
    // 004288d0  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 004288d3  742d                   -je 0x428902
    if (cpu.flags.zf)
    {
        goto L_0x00428902;
    }
    // 004288d5  89149d08fb5e00         -mov dword ptr [ebx*4 + 0x5efb08], edx
    app->getMemory<x86::reg32>(x86::reg32(6224648) /* 0x5efb08 */ + cpu.ebx * 4) = cpu.edx;
    // 004288dc  808a0002000002         -or byte ptr [edx + 0x200], 2
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */) |= x86::reg8(x86::sreg8(2 /*0x2*/));
    // 004288e3  a18cfd5e00             -mov eax, dword ptr [0x5efd8c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225292) /* 0x5efd8c */);
    // 004288e8  89148548fb5e00         -mov dword ptr [eax*4 + 0x5efb48], edx
    app->getMemory<x86::reg32>(x86::reg32(6224712) /* 0x5efb48 */ + cpu.eax * 4) = cpu.edx;
    // 004288ef  808a0002000010         -or byte ptr [edx + 0x200], 0x10
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
    // 004288f6  a18cfd5e00             -mov eax, dword ptr [0x5efd8c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225292) /* 0x5efd8c */);
    // 004288fb  40                     -inc eax
    (cpu.eax)++;
    // 004288fc  43                     -inc ebx
    (cpu.ebx)++;
    // 004288fd  a38cfd5e00             -mov dword ptr [0x5efd8c], eax
    app->getMemory<x86::reg32>(x86::reg32(6225292) /* 0x5efd8c */) = cpu.eax;
L_0x00428902:
    // 00428902  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 00428905  7430                   -je 0x428937
    if (cpu.flags.zf)
    {
        goto L_0x00428937;
    }
    // 00428907  f6c101                 +test cl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 1 /*0x1*/));
    // 0042890a  752b                   -jne 0x428937
    if (!cpu.flags.zf)
    {
        goto L_0x00428937;
    }
    // 0042890c  89149d08fb5e00         -mov dword ptr [ebx*4 + 0x5efb08], edx
    app->getMemory<x86::reg32>(x86::reg32(6224648) /* 0x5efb08 */ + cpu.ebx * 4) = cpu.edx;
    // 00428913  808a0002000002         -or byte ptr [edx + 0x200], 2
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */) |= x86::reg8(x86::sreg8(2 /*0x2*/));
    // 0042891a  a188fd5e00             -mov eax, dword ptr [0x5efd88]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225288) /* 0x5efd88 */);
    // 0042891f  89148588fa5e00         -mov dword ptr [eax*4 + 0x5efa88], edx
    app->getMemory<x86::reg32>(x86::reg32(6224520) /* 0x5efa88 */ + cpu.eax * 4) = cpu.edx;
    // 00428926  808a0002000020         -or byte ptr [edx + 0x200], 0x20
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */) |= x86::reg8(x86::sreg8(32 /*0x20*/));
    // 0042892d  8d4801                 -lea ecx, [eax + 1]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 00428930  43                     -inc ebx
    (cpu.ebx)++;
    // 00428931  890d88fd5e00           -mov dword ptr [0x5efd88], ecx
    app->getMemory<x86::reg32>(x86::reg32(6225288) /* 0x5efd88 */) = cpu.ecx;
L_0x00428937:
    // 00428937  8b8220020000           -mov eax, dword ptr [edx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(544) /* 0x220 */);
    // 0042893d  8b3530dc6f00           -mov esi, dword ptr [0x6fdc30]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7330864) /* 0x6fdc30 */);
    // 00428943  3b7014                 +cmp esi, dword ptr [eax + 0x14]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428946  7510                   -jne 0x428958
    if (!cpu.flags.zf)
    {
        goto L_0x00428958;
    }
    // 00428948  833db8d36f0002         +cmp dword ptr [0x6fd3b8], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0042894f  7507                   -jne 0x428958
    if (!cpu.flags.zf)
    {
        goto L_0x00428958;
    }
    // 00428951  808a0002000080         -or byte ptr [edx + 0x200], 0x80
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(512) /* 0x200 */) |= x86::reg8(x86::sreg8(128 /*0x80*/));
L_0x00428958:
    // 00428958  891d98fd5e00           -mov dword ptr [0x5efd98], ebx
    app->getMemory<x86::reg32>(x86::reg32(6225304) /* 0x5efd98 */) = cpu.ebx;
    // 0042895e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042895f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428960  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428961  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428962  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428963  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428964  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_428970(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00428970  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00428971  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00428972  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00428973  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00428975  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00428977  c780e400000000000000   -mov dword ptr [eax + 0xe4], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(228) /* 0xe4 */) = 0 /*0x0*/;
    // 00428981  c780e800000000000000   -mov dword ptr [eax + 0xe8], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(232) /* 0xe8 */) = 0 /*0x0*/;
    // 0042898b  c780ec00000000000000   -mov dword ptr [eax + 0xec], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(236) /* 0xec */) = 0 /*0x0*/;
    // 00428995  c780f804000000000000   -mov dword ptr [eax + 0x4f8], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1272) /* 0x4f8 */) = 0 /*0x0*/;
    // 0042899f  c780fc04000000000000   -mov dword ptr [eax + 0x4fc], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1276) /* 0x4fc */) = 0 /*0x0*/;
    // 004289a9  c7800005000000000000   -mov dword ptr [eax + 0x500], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1280) /* 0x500 */) = 0 /*0x0*/;
    // 004289b3  c780e004000000000000   -mov dword ptr [eax + 0x4e0], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1248) /* 0x4e0 */) = 0 /*0x0*/;
    // 004289bd  c780e404000000000000   -mov dword ptr [eax + 0x4e4], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1252) /* 0x4e4 */) = 0 /*0x0*/;
    // 004289c7  c780e804000000000000   -mov dword ptr [eax + 0x4e8], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1256) /* 0x4e8 */) = 0 /*0x0*/;
    // 004289d1  c7802405000000000000   -mov dword ptr [eax + 0x524], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1316) /* 0x524 */) = 0 /*0x0*/;
    // 004289db  ffb0bc000000           -push dword ptr [eax + 0xbc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(188) /* 0xbc */);
    cpu.esp -= 4;
    // 004289e1  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004289e4  ba20000000             -mov edx, 0x20
    cpu.edx = 32 /*0x20*/;
    // 004289e9  e8c258feff             -call 0x40e2b0
    cpu.esp -= 4;
    nfs3hp::aiplan::roadSliceIndex(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004289ee  898138060000           -mov dword ptr [ecx + 0x638], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(1592) /* 0x638 */) = cpu.eax;
    // 004289f4  8d8168010000           -lea eax, [ecx + 0x168]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(360) /* 0x168 */);
    // 004289fa  e80d7d0b00             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004289ff  66c781640100000100     -mov word ptr [ecx + 0x164], 1
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(356) /* 0x164 */) = 1 /*0x1*/;
    // 00428a08  c7816801000000000000   -mov dword ptr [ecx + 0x168], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(360) /* 0x168 */) = 0 /*0x0*/;
    // 00428a12  c7816c01000000000000   -mov dword ptr [ecx + 0x16c], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(364) /* 0x16c */) = 0 /*0x0*/;
    // 00428a1c  c6818b00000000         -mov byte ptr [ecx + 0x8b], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(139) /* 0x8b */) = 0 /*0x0*/;
    // 00428a23  c7818401000000000000   -mov dword ptr [ecx + 0x184], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(388) /* 0x184 */) = 0 /*0x0*/;
    // 00428a2d  c7819c09000000000000   -mov dword ptr [ecx + 0x99c], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(2460) /* 0x99c */) = 0 /*0x0*/;
    // 00428a37  c781a009000000000000   -mov dword ptr [ecx + 0x9a0], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(2464) /* 0x9a0 */) = 0 /*0x0*/;
    // 00428a41  c781d408000000000000   -mov dword ptr [ecx + 0x8d4], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(2260) /* 0x8d4 */) = 0 /*0x0*/;
    // 00428a4b  c781d008000000000000   -mov dword ptr [ecx + 0x8d0], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(2256) /* 0x8d0 */) = 0 /*0x0*/;
    // 00428a55  c7811c09000000000000   -mov dword ptr [ecx + 0x91c], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(2332) /* 0x91c */) = 0 /*0x0*/;
    // 00428a5f  c7812009000000000000   -mov dword ptr [ecx + 0x920], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(2336) /* 0x920 */) = 0 /*0x0*/;
    // 00428a69  c7812409000000000000   -mov dword ptr [ecx + 0x924], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(2340) /* 0x924 */) = 0 /*0x0*/;
    // 00428a73  a1c8fd5e00             -mov eax, dword ptr [0x5efdc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225352) /* 0x5efdc8 */);
    // 00428a78  c7812809000000000000   -mov dword ptr [ecx + 0x928], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(2344) /* 0x928 */) = 0 /*0x0*/;
    // 00428a82  898174010000           -mov dword ptr [ecx + 0x174], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(372) /* 0x174 */) = cpu.eax;
    // 00428a88  8aa121060000           -mov ah, byte ptr [ecx + 0x621]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(1569) /* 0x621 */);
    // 00428a8e  c7812c09000000000000   -mov dword ptr [ecx + 0x92c], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(2348) /* 0x92c */) = 0 /*0x0*/;
    // 00428a98  80e4bf                 -and ah, 0xbf
    cpu.ah &= x86::reg8(x86::sreg8(191 /*0xbf*/));
    // 00428a9b  88a121060000           -mov byte ptr [ecx + 0x621], ah
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(1569) /* 0x621 */) = cpu.ah;
    // 00428aa1  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00428aa3  c781c808000000000000   -mov dword ptr [ecx + 0x8c8], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(2248) /* 0x8c8 */) = 0 /*0x0*/;
    // 00428aad  e8ed1c0800             -call 0x4aa79f
    cpu.esp -= 4;
    nfs3hp::handling::resetDriveState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428ab2  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00428ab4  e8f78afdff             -call 0x4015b0
    cpu.esp -= 4;
    nfs3hp::airace::stopIfCopNear(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428ab9  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00428abb  e860410000             -call 0x42cc20
    cpu.esp -= 4;
    nfs3hp::raceload::countLaps(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428ac0  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00428ac2  e849430000             -call 0x42ce10
    cpu.esp -= 4;
    nfs3hp::raceload::carRaceDistance(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428ac7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428ac8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428ac9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428aca  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_428ad0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4;
  x86::reg16 fsw = 0;
    // 00428ad0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00428ad1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00428ad2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00428ad3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00428ad4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00428ad5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00428ad7  81ecb0000000           -sub esp, 0xb0
    (cpu.esp) -= x86::reg32(x86::sreg32(176 /*0xb0*/));
    // 00428add  83ed76                 -sub ebp, 0x76
    (cpu.ebp) -= x86::reg32(x86::sreg32(118 /*0x76*/));
    // 00428ae0  89456e                 -mov dword ptr [ebp + 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.eax;
    // 00428ae3  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 00428ae5  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00428ae7  ffb58e000000           -push dword ptr [ebp + 0x8e]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(142) /* 0x8e */);
    cpu.esp -= 4;
    // 00428aed  e8be57feff             -call 0x40e2b0
    cpu.esp -= 4;
    nfs3hp::aiplan::roadSliceIndex(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428af2  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00428af4  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00428af6  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00428af8  e82357feff             -call 0x40e220
    cpu.esp -= 4;
    nfs3hp::aiplan::nodeHasLane(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428afd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00428aff  0f8448020000           -je 0x428d4d
    if (cpu.flags.zf)
    {
        goto L_0x00428d4d;
    }
    // 00428b05  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00428b07:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00428b07  3b159cfd5e00           +cmp edx, dword ptr [0x5efd9c]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6225308) /* 0x5efd9c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428b0d  0f8d82000000           -jge 0x428b95
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00428b95;
    }
    // 00428b13  8b0495c8fa5e00         -mov eax, dword ptr [edx*4 + 0x5efac8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */ + cpu.edx * 4);
    // 00428b1a  8b701c                 -mov esi, dword ptr [eax + 0x1c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 00428b1d  8b8838060000           -mov ecx, dword ptr [eax + 0x638]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1592) /* 0x638 */);
    // 00428b23  3b456e                 +cmp eax, dword ptr [ebp + 0x6e]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428b26  0f8463000000           -je 0x428b8f
    if (cpu.flags.zf)
    {
        goto L_0x00428b8f;
    }
    // 00428b2c  80b88800000000         +cmp byte ptr [eax + 0x88], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(136) /* 0x88 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00428b33  0f8556000000           -jne 0x428b8f
    if (!cpu.flags.zf)
    {
        goto L_0x00428b8f;
    }
    // 00428b39  80b88900000000         +cmp byte ptr [eax + 0x89], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(137) /* 0x89 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00428b40  744d                   -je 0x428b8f
    if (cpu.flags.zf)
    {
        goto L_0x00428b8f;
    }
    // 00428b42  39f9                   +cmp ecx, edi
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428b44  740c                   -je 0x428b52
    if (cpu.flags.zf)
    {
        goto L_0x00428b52;
    }
    // 00428b46  8d41ff                 -lea eax, [ecx - 1]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(-1) /* -0x1 */);
    // 00428b49  39c7                   +cmp edi, eax
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
    // 00428b4b  7405                   -je 0x428b52
    if (cpu.flags.zf)
    {
        goto L_0x00428b52;
    }
    // 00428b4d  41                     -inc ecx
    (cpu.ecx)++;
    // 00428b4e  39f9                   +cmp ecx, edi
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428b50  753d                   -jne 0x428b8f
    if (!cpu.flags.zf)
    {
        goto L_0x00428b8f;
    }
L_0x00428b52:
    // 00428b52  39f3                   +cmp ebx, esi
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
    // 00428b54  7507                   -jne 0x428b5d
    if (!cpu.flags.zf)
    {
        goto L_0x00428b5d;
    }
    // 00428b56  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00428b58  e9f0010000             -jmp 0x428d4d
    goto L_0x00428d4d;
L_0x00428b5d:
    // 00428b5d  8d46ff                 -lea eax, [esi - 1]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(-1) /* -0x1 */);
    // 00428b60  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00428b62  7d06                   -jge 0x428b6a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00428b6a;
    }
    // 00428b64  030540d95d00           -add eax, dword ptr [0x5dd940]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
L_0x00428b6a:
    // 00428b6a  39c3                   +cmp ebx, eax
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
    // 00428b6c  7507                   -jne 0x428b75
    if (!cpu.flags.zf)
    {
        goto L_0x00428b75;
    }
    // 00428b6e  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00428b70  e9d8010000             -jmp 0x428d4d
    goto L_0x00428d4d;
L_0x00428b75:
    // 00428b75  8d4601                 -lea eax, [esi + 1]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00428b78  8b3540d95d00           -mov esi, dword ptr [0x5dd940]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00428b7e  39f0                   +cmp eax, esi
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
    // 00428b80  7c02                   -jl 0x428b84
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00428b84;
    }
    // 00428b82  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
L_0x00428b84:
    // 00428b84  39c3                   +cmp ebx, eax
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
    // 00428b86  7507                   -jne 0x428b8f
    if (!cpu.flags.zf)
    {
        goto L_0x00428b8f;
    }
L_0x00428b88:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00428b88  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00428b8a  e9be010000             -jmp 0x428d4d
    goto L_0x00428d4d;
L_0x00428b8f:
    // 00428b8f  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00428b90  e972ffffff             -jmp 0x428b07
    goto L_0x00428b07;
L_0x00428b95:
    // 00428b95  8d04dd00000000         -lea eax, [ebx*8]
    cpu.eax = x86::reg32(cpu.ebx * 8);
    // 00428b9c  8d1403                 -lea edx, [ebx + eax]
    cpu.edx = x86::reg32(cpu.ebx + cpu.eax * 1);
    // 00428b9f  a148d95d00             -mov eax, dword ptr [0x5dd948]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 00428ba4  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00428ba7  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00428ba9  8b5015                 -mov edx, dword ptr [eax + 0x15]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(21) /* 0x15 */);
    // 00428bac  c1fa18                 -sar edx, 0x18
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (24 /*0x18*/ % 32));
    // 00428baf  c1e209                 -shl edx, 9
    cpu.edx <<= 9 /*0x9*/ % 32;
    // 00428bb2  895572                 -mov dword ptr [ebp + 0x72], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.edx;
    // 00428bb5  dd05a8715300           -fld qword ptr [0x5371a8]
    fp0 = x86::Float(app->getMemory<double>(x86::reg32(5468584) /* 0x5371a8 */));
    // 00428bbb  db4572                 -fild dword ptr [ebp + 0x72]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */)));
    // 00428bbe  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 00428bc0  db00                   -fild dword ptr [eax]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax)));
    // 00428bc2  db4004                 -fild dword ptr [eax + 4]
    fp3 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */)));
    // 00428bc5  db4008                 -fild dword ptr [eax + 8]
    fp4 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 00428bc8  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp4;
        fp4 = fp1;
        fp1 = tmp;
    }
    // 00428bca  d95d4a                 -fstp dword ptr [ebp + 0x4a]
    app->getMemory<float>(cpu.ebp + x86::reg32(74) /* 0x4a */) = float(fp4);
    // 00428bcd  8b5016                 -mov edx, dword ptr [eax + 0x16]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(22) /* 0x16 */);
    // 00428bd0  c1fa18                 -sar edx, 0x18
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (24 /*0x18*/ % 32));
    // 00428bd3  c1e209                 -shl edx, 9
    cpu.edx <<= 9 /*0x9*/ % 32;
    // 00428bd6  895572                 -mov dword ptr [ebp + 0x72], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.edx;
    // 00428bd9  db4572                 -fild dword ptr [ebp + 0x72]
    fp4 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */)));
    // 00428bdc  d8cc                   -fmul st(4)
    fp4 = fp::mul(fp4, x86::Float(fp0));
    // 00428bde  d95d4e                 -fstp dword ptr [ebp + 0x4e]
    app->getMemory<float>(cpu.ebp + x86::reg32(78) /* 0x4e */) = float(fp4);
    // 00428be1  8b5017                 -mov edx, dword ptr [eax + 0x17]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(23) /* 0x17 */);
    // 00428be4  c1fa18                 -sar edx, 0x18
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (24 /*0x18*/ % 32));
    // 00428be7  c1e209                 -shl edx, 9
    cpu.edx <<= 9 /*0x9*/ % 32;
    // 00428bea  895572                 -mov dword ptr [ebp + 0x72], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.edx;
    // 00428bed  db4572                 -fild dword ptr [ebp + 0x72]
    fp4 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */)));
    // 00428bf0  d8cc                   -fmul st(4)
    fp4 = fp::mul(fp4, x86::Float(fp0));
    // 00428bf2  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp4;
        fp4 = fp2;
        fp2 = tmp;
    }
    // 00428bf4  d8cc                   -fmul st(4)
    fp4 = fp::mul(fp4, x86::Float(fp0));
    // 00428bf6  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp4;
        fp4 = fp2;
        fp2 = tmp;
    }
    // 00428bf8  d95d52                 -fstp dword ptr [ebp + 0x52]
    app->getMemory<float>(cpu.ebp + x86::reg32(82) /* 0x52 */) = float(fp4);
    // 00428bfb  8b5011                 -mov edx, dword ptr [eax + 0x11]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(17) /* 0x11 */);
    // 00428bfe  c1fa18                 -sar edx, 0x18
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (24 /*0x18*/ % 32));
    // 00428c01  c1e209                 -shl edx, 9
    cpu.edx <<= 9 /*0x9*/ % 32;
    // 00428c04  895572                 -mov dword ptr [ebp + 0x72], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.edx;
    // 00428c07  db4572                 -fild dword ptr [ebp + 0x72]
    fp4 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */)));
    // 00428c0a  d8cc                   -fmul st(4)
    fp4 = fp::mul(fp4, x86::Float(fp0));
    // 00428c0c  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp4;
        fp4 = fp3;
        fp3 = tmp;
    }
    // 00428c0e  d8cc                   -fmul st(4)
    fp4 = fp::mul(fp4, x86::Float(fp0));
    // 00428c10  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp4;
        fp4 = fp3;
        fp3 = tmp;
    }
    // 00428c12  d95d62                 -fstp dword ptr [ebp + 0x62]
    app->getMemory<float>(cpu.ebp + x86::reg32(98) /* 0x62 */) = float(fp4);
    // 00428c15  8b5012                 -mov edx, dword ptr [eax + 0x12]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(18) /* 0x12 */);
    // 00428c18  c1fa18                 -sar edx, 0x18
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (24 /*0x18*/ % 32));
    // 00428c1b  c1e209                 -shl edx, 9
    cpu.edx <<= 9 /*0x9*/ % 32;
    // 00428c1e  895572                 -mov dword ptr [ebp + 0x72], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.edx;
    // 00428c21  db4572                 -fild dword ptr [ebp + 0x72]
    fp4 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */)));
    // 00428c24  d8cc                   -fmul st(4)
    fp4 = fp::mul(fp4, x86::Float(fp0));
    // 00428c26  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp4;
        fp4 = fp1;
        fp1 = tmp;
    }
    // 00428c28  d8cc                   -fmul st(4)
    fp4 = fp::mul(fp4, x86::Float(fp0));
    // 00428c2a  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp4;
        fp4 = fp1;
        fp1 = tmp;
    }
    // 00428c2c  d95d66                 -fstp dword ptr [ebp + 0x66]
    app->getMemory<float>(cpu.ebp + x86::reg32(102) /* 0x66 */) = float(fp4);
    // 00428c2f  8b4013                 -mov eax, dword ptr [eax + 0x13]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(19) /* 0x13 */);
    // 00428c32  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 00428c35  c1e009                 -shl eax, 9
    cpu.eax <<= 9 /*0x9*/ % 32;
    // 00428c38  894572                 -mov dword ptr [ebp + 0x72], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.eax;
    // 00428c3b  db4572                 -fild dword ptr [ebp + 0x72]
    fp4 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */)));
    // 00428c3e  decc                   -fmulp st(4)
    fp0 = fp::mul(fp0, fp4);
    // 00428c40  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 00428c42  d95d56                 -fstp dword ptr [ebp + 0x56]
    app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */) = float(fp3);
    // 00428c45  8d454a                 -lea eax, [ebp + 0x4a]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 00428c48  d95d5a                 -fstp dword ptr [ebp + 0x5a]
    app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */) = float(fp2);
    // 00428c4b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00428c4c  d95d5e                 -fstp dword ptr [ebp + 0x5e]
    app->getMemory<float>(cpu.ebp + x86::reg32(94) /* 0x5e */) = float(fp1);
    // 00428c4f  8d554a                 -lea edx, [ebp + 0x4a]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 00428c52  ffb58e000000           -push dword ptr [ebp + 0x8e]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(142) /* 0x8e */);
    cpu.esp -= 4;
    // 00428c58  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00428c5d  d95d6a                 -fstp dword ptr [ebp + 0x6a]
    app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */) = float(fp0);
    // 00428c60  e8eb730b00             -call 0x4e0050
    cpu.esp -= 4;
    nfs3hp::vertices::scaleVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428c65  8d4562                 -lea eax, [ebp + 0x62]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 00428c68  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00428c69  8d5562                 -lea edx, [ebp + 0x62]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 00428c6c  ffb596000000           -push dword ptr [ebp + 0x96]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(150) /* 0x96 */);
    cpu.esp -= 4;
    // 00428c72  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00428c77  8d75c6                 -lea esi, [ebp - 0x3a]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-58) /* -0x3a */);
    // 00428c7a  e8d1730b00             -call 0x4e0050
    cpu.esp -= 4;
    nfs3hp::vertices::scaleVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428c7f  d9454a                 -fld dword ptr [ebp + 0x4a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(74) /* 0x4a */));
    // 00428c82  d9454e                 -fld dword ptr [ebp + 0x4e]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(78) /* 0x4e */));
    // 00428c85  d94552                 -fld dword ptr [ebp + 0x52]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(82) /* 0x52 */));
    // 00428c88  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00428c8d  8d453e                 -lea eax, [ebp + 0x3e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(62) /* 0x3e */);
    // 00428c90  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00428c92  d84562                 -fadd dword ptr [ebp + 0x62]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(98) /* 0x62 */)));
    // 00428c95  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00428c97  d84566                 -fadd dword ptr [ebp + 0x66]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(102) /* 0x66 */)));
    // 00428c9a  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00428c9c  d8456a                 -fadd dword ptr [ebp + 0x6a]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 00428c9f  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00428ca1  d84556                 -fadd dword ptr [ebp + 0x56]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */)));
    // 00428ca4  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00428ca6  d8455a                 -fadd dword ptr [ebp + 0x5a]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */)));
    // 00428ca9  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00428cab  d8455e                 -fadd dword ptr [ebp + 0x5e]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(94) /* 0x5e */)));
    // 00428cae  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00428cb0  d95d3e                 -fstp dword ptr [ebp + 0x3e]
    app->getMemory<float>(cpu.ebp + x86::reg32(62) /* 0x3e */) = float(fp2);
    // 00428cb3  d95d42                 -fstp dword ptr [ebp + 0x42]
    app->getMemory<float>(cpu.ebp + x86::reg32(66) /* 0x42 */) = float(fp1);
    // 00428cb6  d95d46                 -fstp dword ptr [ebp + 0x46]
    app->getMemory<float>(cpu.ebp + x86::reg32(70) /* 0x46 */) = float(fp0);
    // 00428cb9  e8e278ffff             -call 0x4205a0
    cpu.esp -= 4;
    nfs3hp::trackdata::snapToTrack(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428cbe  837dda00               +cmp dword ptr [ebp - 0x26], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-38) /* -0x26 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428cc2  0f84c0feffff           -je 0x428b88
    if (cpu.flags.zf)
    {
        goto L_0x00428b88;
    }
    // 00428cc8  837dde00               +cmp dword ptr [ebp - 0x22], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-34) /* -0x22 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428ccc  750d                   -jne 0x428cdb
    if (!cpu.flags.zf)
    {
        goto L_0x00428cdb;
    }
    // 00428cce  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00428cd0  8d6576                 -lea esp, [ebp + 0x76]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 00428cd3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428cd4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428cd5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428cd6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428cd7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428cd8  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
L_0x00428cdb:
    // 00428cdb  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00428ce0  8d45c6                 -lea eax, [ebp - 0x3a]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-58) /* -0x3a */);
    // 00428ce3  e8f87affff             -call 0x4207e0
    cpu.esp -= 4;
    nfs3hp::trackdata::positionSurface(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428ce8  a840                   +test al, 0x40
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 64 /*0x40*/));
    // 00428cea  754f                   -jne 0x428d3b
    if (!cpu.flags.zf)
    {
        goto L_0x00428d3b;
    }
    // 00428cec  a880                   +test al, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 128 /*0x80*/));
    // 00428cee  754b                   -jne 0x428d3b
    if (!cpu.flags.zf)
    {
        goto L_0x00428d3b;
    }
    // 00428cf0  a83f                   +test al, 0x3f
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 63 /*0x3f*/));
    // 00428cf2  7447                   -je 0x428d3b
    if (cpu.flags.zf)
    {
        goto L_0x00428d3b;
    }
    // 00428cf4  83e03f                 -and eax, 0x3f
    cpu.eax &= x86::reg32(x86::sreg32(63 /*0x3f*/));
    // 00428cf7  83f80e                 +cmp eax, 0xe
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(14 /*0xe*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428cfa  743f                   -je 0x428d3b
    if (cpu.flags.zf)
    {
        goto L_0x00428d3b;
    }
    // 00428cfc  837dda00               +cmp dword ptr [ebp - 0x26], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-38) /* -0x26 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428d00  740c                   -je 0x428d0e
    if (cpu.flags.zf)
    {
        goto L_0x00428d0e;
    }
    // 00428d02  8b45e2                 -mov eax, dword ptr [ebp - 0x1e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-30) /* -0x1e */);
    // 00428d05  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00428d07  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00428d0a  01c0                   +add eax, eax
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00428d0c  eb1c                   -jmp 0x428d2a
    goto L_0x00428d2a;
L_0x00428d0e:
    // 00428d0e  8b55d6                 -mov edx, dword ptr [ebp - 0x2a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-42) /* -0x2a */);
    // 00428d11  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 00428d18  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00428d1a  8b1548d95d00           -mov edx, dword ptr [0x5dd948]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 00428d20  8b44820e               -mov eax, dword ptr [edx + eax*4 + 0xe]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(14) /* 0xe */ + cpu.eax * 4);
    // 00428d24  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 00428d27  c1e009                 +shl eax, 9
    {
        x86::reg8 tmp = 9 /*0x9*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
L_0x00428d2a:
    // 00428d2a  894572                 -mov dword ptr [ebp + 0x72], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.eax;
    // 00428d2d  db4572                 +fild dword ptr [ebp + 0x72]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */)));
    // 00428d30  dc1db0715300           +fcomp qword ptr [0x5371b0]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5468592) /* 0x5371b0 */)));
    // 00428d36  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00428d38  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00428d39  730d                   -jae 0x428d48
    if (!cpu.flags.cf)
    {
        goto L_0x00428d48;
    }
L_0x00428d3b:
    // 00428d3b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00428d3d  8d6576                 -lea esp, [ebp + 0x76]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 00428d40  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428d41  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428d42  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428d43  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428d44  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428d45  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
L_0x00428d48:
    // 00428d48  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x00428d4d:
    // 00428d4d  8d6576                 -lea esp, [ebp + 0x76]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 00428d50  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428d51  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428d52  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428d53  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428d54  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428d55  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_428d60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    // 00428d60  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00428d61  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00428d62  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00428d63  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00428d64  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00428d65  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00428d67  83ec30                 -sub esp, 0x30
    (cpu.esp) -= x86::reg32(x86::sreg32(48 /*0x30*/));
    // 00428d6a  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00428d6d  8955f0                 -mov dword ptr [ebp - 0x10], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edx;
    // 00428d70  8d7ddc                 -lea edi, [ebp - 0x24]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00428d73  be60844200             -mov esi, 0x428460
    cpu.esi = 4359264 /*0x428460*/;
    // 00428d78  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00428d7a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00428d7b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00428d7c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00428d7d  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 00428d80  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00428d83  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00428d84  8b4de0                 -mov ecx, dword ptr [ebp - 0x20]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00428d87  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00428d88  8b5ddc                 -mov ebx, dword ptr [ebp - 0x24]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00428d8b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00428d8d  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00428d8e  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00428d91  be0a000000             -mov esi, 0xa
    cpu.esi = 10 /*0xa*/;
    // 00428d96  e835fdffff             -call 0x428ad0
    cpu.esp -= 4;
    sub_428ad0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428d9b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00428d9d  0f85a0010000           -jne 0x428f43
    if (!cpu.flags.zf)
    {
        goto L_0x00428f43;
    }
L_0x00428da3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00428da3  4e                     -dec esi
    (cpu.esi)--;
    // 00428da4  83feff                 +cmp esi, -1
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
    // 00428da7  0f8496010000           -je 0x428f43
    if (cpu.flags.zf)
    {
        goto L_0x00428f43;
    }
    // 00428dad  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00428db0  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 00428db7  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00428db9  a148d95d00             -mov eax, dword ptr [0x5dd948]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 00428dbe  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00428dc1  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00428dc3  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00428dc5  8a5013                 -mov dl, byte ptr [eax + 0x13]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(19) /* 0x13 */);
    // 00428dc8  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00428dca  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00428dcc  80e20f                 -and dl, 0xf
    cpu.dl &= x86::reg8(x86::sreg8(15 /*0xf*/));
    // 00428dcf  88d3                   -mov bl, dl
    cpu.bl = cpu.dl;
    // 00428dd1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00428dd3  8a5017                 -mov dl, byte ptr [eax + 0x17]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(23) /* 0x17 */);
    // 00428dd6  c1e20f                 -shl edx, 0xf
    cpu.edx <<= 15 /*0xf*/ % 32;
    // 00428dd9  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00428ddc  c1ff04                 -sar edi, 4
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (4 /*0x4*/ % 32));
    // 00428ddf  dd05b8715300           -fld qword ptr [0x5371b8]
    fp0 = x86::Float(app->getMemory<double>(x86::reg32(5468600) /* 0x5371b8 */));
    // 00428de5  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00428de8  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 00428dea  83e70f                 -and edi, 0xf
    cpu.edi &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 00428ded  8a401b                 -mov al, byte ptr [eax + 0x1b]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(27) /* 0x1b */);
    // 00428df0  d95dec                 -fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fp1);
    // 00428df3  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00428df8  c1e00f                 -shl eax, 0xf
    cpu.eax <<= 15 /*0xf*/ % 32;
    // 00428dfb  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00428dfe  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00428e01  dec9                   -fmulp st(1)
    fp0 = fp::mul(fp0, fp1);
    // 00428e03  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00428e05  d95de8                 -fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp0);
    // 00428e08  39df                   +cmp edi, ebx
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428e0a  7e04                   -jle 0x428e10
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00428e10;
    }
    // 00428e0c  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00428e0e  eb02                   -jmp 0x428e12
    goto L_0x00428e12;
L_0x00428e10:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00428e10  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
L_0x00428e12:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00428e12  39c1                   +cmp ecx, eax
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
    // 00428e14  0f8ded000000           -jge 0x428f07
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00428f07;
    }
    // 00428e1a  39d9                   +cmp ecx, ebx
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
    // 00428e1c  7f6a                   -jg 0x428e88
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00428e88;
    }
    // 00428e1e  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 00428e21  d945e8                 -fld dword ptr [ebp - 0x18]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */));
    // 00428e24  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00428e27  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 00428e29  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00428e2b  dc0dc0715300           -fmul qword ptr [0x5371c0]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5468608) /* 0x5371c0 */)));
    // 00428e31  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00428e34  8b55e0                 -mov edx, dword ptr [ebp - 0x20]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00428e37  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00428e38  dec1                   -faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 00428e3a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00428e3b  d95ddc                 -fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp0);
    // 00428e3e  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00428e41  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00428e42  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00428e45  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00428e48  e883fcffff             -call 0x428ad0
    cpu.esp -= 4;
    sub_428ad0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428e4d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00428e4f  7437                   -je 0x428e88
    if (cpu.flags.zf)
    {
        goto L_0x00428e88;
    }
    // 00428e51  d945dc                 -fld dword ptr [ebp - 0x24]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */));
    // 00428e54  dc0dc8715300           -fmul qword ptr [0x5371c8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5468616) /* 0x5371c8 */)));
    // 00428e5a  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00428e5d  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00428e60  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428e61  d945e0                 -fld dword ptr [ebp - 0x20]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */));
    // 00428e64  dc0dc8715300           -fmul qword ptr [0x5371c8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5468616) /* 0x5371c8 */)));
    // 00428e6a  8945d0                 -mov dword ptr [ebp - 0x30], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.eax;
    // 00428e6d  83ec04                 +sub esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00428e70  db1c24                 +fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00428e73  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428e74  d945e4                 +fld dword ptr [ebp - 0x1c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */));
    // 00428e77  dc0dc8715300           +fmul qword ptr [0x5371c8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5468616) /* 0x5371c8 */)));
    // 00428e7d  8d5dd0                 -lea ebx, [ebp - 0x30]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 00428e80  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00428e83  e9d8000000             -jmp 0x428f60
    goto L_0x00428f60;
L_0x00428e88:
    // 00428e88  39f9                   +cmp ecx, edi
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428e8a  7f6b                   -jg 0x428ef7
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00428ef7;
    }
    // 00428e8c  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 00428e8f  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00428e92  d9e0                   -fchs 
    fp0 = -fp0;
    // 00428e94  d945ec                 -fld dword ptr [ebp - 0x14]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */));
    // 00428e97  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00428e99  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 00428e9b  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00428e9d  dc0dc0715300           -fmul qword ptr [0x5371c0]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5468608) /* 0x5371c0 */)));
    // 00428ea3  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00428ea6  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00428ea9  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00428eaa  dee9                   -fsubp st(1)
    fp0 = fp::sub(fp0, fp1);
    // 00428eac  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00428ead  d95ddc                 -fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp0);
    // 00428eb0  8b55dc                 -mov edx, dword ptr [ebp - 0x24]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00428eb3  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00428eb4  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00428eb7  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00428eba  e811fcffff             -call 0x428ad0
    cpu.esp -= 4;
    sub_428ad0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428ebf  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00428ec1  7434                   -je 0x428ef7
    if (cpu.flags.zf)
    {
        goto L_0x00428ef7;
    }
    // 00428ec3  d945dc                 -fld dword ptr [ebp - 0x24]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */));
    // 00428ec6  dc0dc8715300           -fmul qword ptr [0x5371c8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5468616) /* 0x5371c8 */)));
    // 00428ecc  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00428ecf  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00428ed2  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428ed3  d945e0                 -fld dword ptr [ebp - 0x20]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */));
    // 00428ed6  dc0dc8715300           -fmul qword ptr [0x5371c8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5468616) /* 0x5371c8 */)));
    // 00428edc  8945d0                 -mov dword ptr [ebp - 0x30], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.eax;
    // 00428edf  83ec04                 +sub esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00428ee2  db1c24                 +fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00428ee5  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428ee6  d945e4                 +fld dword ptr [ebp - 0x1c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */));
    // 00428ee9  dc0dc8715300           +fmul qword ptr [0x5371c8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5468616) /* 0x5371c8 */)));
    // 00428eef  8d5dd0                 -lea ebx, [ebp - 0x30]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 00428ef2  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00428ef5  eb69                   -jmp 0x428f60
    goto L_0x00428f60;
L_0x00428ef7:
    // 00428ef7  41                     -inc ecx
    (cpu.ecx)++;
    // 00428ef8  39df                   +cmp edi, ebx
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428efa  0f8e10ffffff           -jle 0x428e10
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00428e10;
    }
    // 00428f00  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00428f02  e90bffffff             -jmp 0x428e12
    goto L_0x00428e12;
L_0x00428f07:
    // 00428f07  833dc0d46f0000         +cmp dword ptr [0x6fd4c0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00428f0e  7410                   -je 0x428f20
    if (cpu.flags.zf)
    {
        goto L_0x00428f20;
    }
    // 00428f10  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00428f13  48                     -dec eax
    (cpu.eax)--;
    // 00428f14  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00428f16  7d18                   -jge 0x428f30
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00428f30;
    }
    // 00428f18  030540d95d00           +add eax, dword ptr [0x5dd940]
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00428f1e  eb10                   -jmp 0x428f30
    goto L_0x00428f30;
L_0x00428f20:
    // 00428f20  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00428f23  8b3d40d95d00           -mov edi, dword ptr [0x5dd940]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00428f29  40                     -inc eax
    (cpu.eax)++;
    // 00428f2a  39f8                   +cmp eax, edi
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
    // 00428f2c  7c02                   -jl 0x428f30
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00428f30;
    }
    // 00428f2e  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
L_0x00428f30:
    // 00428f30  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00428f33  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00428f35  894de0                 -mov dword ptr [ebp - 0x20], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.ecx;
    // 00428f38  894de4                 -mov dword ptr [ebp - 0x1c], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.ecx;
    // 00428f3b  894ddc                 -mov dword ptr [ebp - 0x24], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.ecx;
    // 00428f3e  e960feffff             -jmp 0x428da3
    goto L_0x00428da3;
L_0x00428f43:
    // 00428f43  d9ee                   -fldz 
    fp0 = 0.0;
    // 00428f45  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00428f48  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00428f4b  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428f4c  d9ee                   -fldz 
    fp0 = 0.0;
    // 00428f4e  8d5dd0                 -lea ebx, [ebp - 0x30]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 00428f51  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00428f54  8945d0                 -mov dword ptr [ebp - 0x30], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.eax;
    // 00428f57  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00428f5a  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00428f5d  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428f5e  d9ee                   -fldz 
    fp0 = 0.0;
L_0x00428f60:
    // 00428f60  8945d4                 -mov dword ptr [ebp - 0x2c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.eax;
    // 00428f63  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00428f66  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00428f69  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428f6a  8b0dc0d46f00           -mov ecx, dword ptr [0x6fd4c0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */);
    // 00428f70  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
    // 00428f73  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00428f76  e8257b0700             -call 0x4a0aa0
    cpu.esp -= 4;
    nfs3hp::frontend::simTrackPlaceOnRoad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00428f7b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00428f7d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428f7e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428f7f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428f80  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428f81  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00428f82  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_428f90: native, nfs3hp::racestart::respawnCar */

/* sub_429100: native, nfs3hp::racestart::resetCar */

/* sub_429260: native, nfs3hp::racestart::stuckCheck */

/* sub_429490: native, nfs3hp::dead::stub_0x429490 */

/* sub_4294c0: native, nfs3hp::racestart::clearRespawn */

/* sub_4294e0: native, nfs3hp::racestart::shakeAmplitude */

/* sub_4295a0: native, nfs3hp::racestart::startShake */

/* sub_4296c0: native, nfs3hp::racestart::startIdleShake */

/* sub_429790: native, nfs3hp::racestart::shakeWave */

/* sub_4297d0: native, nfs3hp::racestart::objectContacts */

/* sub_429880: native, nfs3hp::racestart::wheelSlip */

/* sub_4299a0: native, nfs3hp::racestart::wheelCells */

/* sub_429b40: native, nfs3hp::racestart::wheelSurface */

/* sub_429ba0: native, nfs3hp::racestart::endSkidMark */

/* sub_429d20: native, nfs3hp::racestart::extendSkidMark */

/* sub_429f10: native, nfs3hp::racestart::skidSounds */

/* sub_42a130: native, nfs3hp::racestart::wheelSpray */

/* sub_42a280: native, nfs3hp::racestart::surfaceEffects */

/* sub_42a480: native, nfs3hp::racestart::wheelSkid */

/* sub_42a560: native, nfs3hp::racestart::wheelEffects */

/* sub_42a7d0: native, nfs3hp::racestart::updateShake */

/* sub_42ac40: native, nfs3hp::racestart::crashEffects */

/* sub_42b480: native, nfs3hp::racestart::respawnEffects */

/* sub_42b660: native, nfs3hp::racestart::stopCarEffects */

/* sub_42b780: native, nfs3hp::racestart::carEffects */

/* sub_42b7b0: native, nfs3hp::racestart::airborneSpin */

/* sub_42b900: native, nfs3hp::racestart::updateCar */

/* sub_42ba00: native, nfs3hp::racestart::defaultGridPlace */

/* sub_42bb80: native, nfs3hp::racestart::loadStartGrid */

/* sub_42bc70: native, nfs3hp::racestart::gridPlace */

/* sub_42bcb0: native, nfs3hp::racestart::placeOnGrid */

/* sub_42c2a0: native, nfs3hp::raceload::loadCarPerformance */

/* sub_42c350: native, nfs3hp::raceload::loadRaceCar */

/* sub_42c400: native, nfs3hp::raceload::setCarHandlers */

/* sub_42c520: native, nfs3hp::raceload::freeCarShapes */

/* sub_42c530: native, nfs3hp::raceload::restartRaceCars */

/* sub_42c590: native, nfs3hp::decompiled::resetHandlingTunings */

/* sub_42c660: native, nfs3hp::raceload::clearCarSlots */

/* sub_42c680: native, nfs3hp::raceload::clearRaceCounts */

/* sub_42c6d0: native, nfs3hp::raceload::clearCarList */

/* sub_42c700: native, nfs3hp::raceload::loadRaceCars */

}
