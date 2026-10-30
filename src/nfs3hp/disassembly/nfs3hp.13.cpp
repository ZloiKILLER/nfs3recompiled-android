#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_44a660: native, nfs3hp::frontend::menuShownNow */

/* sub_44a680: native, nfs3hp::dead::stub_0x44a680 */

/* sub_44a690: native, nfs3hp::port::loop::releaseMouse */

/* sub_44a6a0: native, nfs3hp::dead::stub_0x44a6a0 */

/* sub_44a6b0: native, nfs3hp::port::loop::itemUnderMouse */

/* sub_44a860: native, nfs3hp::port::loop::itemsShown */

/* sub_44a8b0: native, nfs3hp::port::loop::passItems */

/* sub_44a960: native, nfs3hp::frontend::menuShowItems */

/* sub_44a9e0: native, nfs3hp::port::loop::hideItems */

/* sub_44aa30: native, nfs3hp::port::loop::leaveScreen */

/* sub_44aad0: native, nfs3hp::port::loop::resetItems */

/* sub_44ab40: native, nfs3hp::port::loop::enterScreen */

/* sub_44ac90: native, nfs3hp::port::buttons::loadShapes */

/* sub_44ad60: native, nfs3hp::dead::stub_0x44ad60 */

/* sub_44adb0: native, nfs3hp::dead::stub_0x44adb0 */

/* sub_44ae00: native, nfs3hp::frontend::menuCurrentName */

/* sub_44ae20: native, nfs3hp::dead::stub_0x44ae20 */

/* sub_44ae60: native, nfs3hp::port::loop::menuByName */

/* sub_44aeb0: native, nfs3hp::port::loop::currentMenu */

/* sub_44aec0: native, nfs3hp::port::loop::popMenu */

/* sub_44af10: native, nfs3hp::port::loop::pushMenu */

/* sub_44af50: native, nfs3hp::port::loop::setHistory */

/* sub_44af90: native, nfs3hp::port::loop::player1NameAnswer */

/* sub_44b040: native, nfs3hp::port::loop::player2NameAnswer */

/* sub_44b0f0: native, nfs3hp::port::loop::quitAnswer */

/* sub_44b170: native, nfs3hp::port::buttons::redrawMenu */

/* sub_44b1a0: native, nfs3hp::port::loop::waitWhileSuspended */

/* sub_44b230: native, nfs3hp::port::loop::menuLoop */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_44b950(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044b950  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044b951  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044b952  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044b953  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044b954  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044b955  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044b956  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044b958  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044b95d  a14cbb6f00             -mov eax, dword ptr [0x6fbb4c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7322444) /* 0x6fbb4c */);
    // 0044b962  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044b964  891534925500           -mov dword ptr [0x559234], edx
    app->getMemory<x86::reg32>(x86::reg32(5607988) /* 0x559234 */) = cpu.edx;
    // 0044b96a  890d54925500           -mov dword ptr [0x559254], ecx
    app->getMemory<x86::reg32>(x86::reg32(5608020) /* 0x559254 */) = cpu.ecx;
    // 0044b970  890d28925500           -mov dword ptr [0x559228], ecx
    app->getMemory<x86::reg32>(x86::reg32(5607976) /* 0x559228 */) = cpu.ecx;
    // 0044b976  e8d55d0800             -call 0x4d1750
    cpu.esp -= 4;
    nfs3hp::port::text::loadLanguageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044b97b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044b97d  40                     -inc eax
    (cpu.eax)++;
    // 0044b97e  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0044b980  893485fc276600         -mov dword ptr [eax*4 + 0x6627fc], esi
    app->getMemory<x86::reg32>(x86::reg32(6694908) /* 0x6627fc */ + cpu.eax * 4) = cpu.esi;
L_0x0044b987:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044b987  83f814                 +cmp eax, 0x14
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044b98a  7d0c                   -jge 0x44b998
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0044b998;
    }
    // 0044b98c  40                     -inc eax
    (cpu.eax)++;
    // 0044b98d  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 0044b98f  893485fc276600         -mov dword ptr [eax*4 + 0x6627fc], esi
    app->getMemory<x86::reg32>(x86::reg32(6694908) /* 0x6627fc */ + cpu.eax * 4) = cpu.esi;
    // 0044b996  ebef                   -jmp 0x44b987
    goto L_0x0044b987;
L_0x0044b998:
    // 0044b998  bf05000000             -mov edi, 5
    cpu.edi = 5 /*0x5*/;
    // 0044b99d  8a252eeb5500           -mov ah, byte ptr [0x55eb2e]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */);
    // 0044b9a3  893d6c296600           -mov dword ptr [0x66296c], edi
    app->getMemory<x86::reg32>(x86::reg32(6695276) /* 0x66296c */) = cpu.edi;
    // 0044b9a9  f6c420                 +test ah, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 32 /*0x20*/));
    // 0044b9ac  743a                   -je 0x44b9e8
    if (cpu.flags.zf)
    {
        goto L_0x0044b9e8;
    }
    // 0044b9ae  a1c8fa5e00             -mov eax, dword ptr [0x5efac8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */);
    // 0044b9b3  f6800002000001         +test byte ptr [eax + 0x200], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 1 /*0x1*/));
    // 0044b9ba  7409                   -je 0x44b9c5
    if (cpu.flags.zf)
    {
        goto L_0x0044b9c5;
    }
    // 0044b9bc  83b8c402000003         +cmp dword ptr [eax + 0x2c4], 3
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
    // 0044b9c3  7417                   -je 0x44b9dc
    if (cpu.flags.zf)
    {
        goto L_0x0044b9dc;
    }
L_0x0044b9c5:
    // 0044b9c5  a1c8fa5e00             -mov eax, dword ptr [0x5efac8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */);
    // 0044b9ca  f6800002000001         +test byte ptr [eax + 0x200], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 1 /*0x1*/));
    // 0044b9d1  7415                   -je 0x44b9e8
    if (cpu.flags.zf)
    {
        goto L_0x0044b9e8;
    }
    // 0044b9d3  83b8c402000004         +cmp dword ptr [eax + 0x2c4], 4
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
    // 0044b9da  750c                   -jne 0x44b9e8
    if (!cpu.flags.zf)
    {
        goto L_0x0044b9e8;
    }
L_0x0044b9dc:
    // 0044b9dc  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044b9e1  b854905300             -mov eax, 0x539054
    cpu.eax = 5476436 /*0x539054*/;
    // 0044b9e6  eb1f                   -jmp 0x44ba07
    goto L_0x0044ba07;
L_0x0044b9e8:
    // 0044b9e8  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044b9ed  b85c905300             -mov eax, 0x53905c
    cpu.eax = 5476444 /*0x53905c*/;
    // 0044b9f2  e819dbffff             -call 0x449510
    cpu.esp -= 4;
    nfs3hp::port::menus::loadMenus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044b9f7  f6052eeb550020         +test byte ptr [0x55eb2e], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */) & 32 /*0x20*/));
    // 0044b9fe  740c                   -je 0x44ba0c
    if (cpu.flags.zf)
    {
        goto L_0x0044ba0c;
    }
    // 0044ba00  b864905300             -mov eax, 0x539064
    cpu.eax = 5476452 /*0x539064*/;
    // 0044ba05  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044ba07:
    // 0044ba07  e804dbffff             -call 0x449510
    cpu.esp -= 4;
    nfs3hp::port::menus::loadMenus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044ba0c:
    // 0044ba0c  bbd0406000             -mov ebx, 0x6040d0
    cpu.ebx = 6308048 /*0x6040d0*/;
    // 0044ba11  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 0044ba16  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044ba18  891d30925500           -mov dword ptr [0x559230], ebx
    app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */) = cpu.ebx;
    // 0044ba1e  e81df1ffff             -call 0x44ab40
    cpu.esp -= 4;
    nfs3hp::port::loop::enterScreen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ba23  893530768b00           -mov dword ptr [0x8b7630], esi
    app->getMemory<x86::reg32>(x86::reg32(9139760) /* 0x8b7630 */) = cpu.esi;
    // 0044ba29  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba2a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba2b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba2c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba2d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba2e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba2f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_44ba30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044ba30  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044ba31  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044ba32  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044ba33  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044ba34  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044ba35  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044ba36  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044ba38  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044ba3d  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044ba3f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044ba41  890d54925500           -mov dword ptr [0x559254], ecx
    app->getMemory<x86::reg32>(x86::reg32(5608020) /* 0x559254 */) = cpu.ecx;
    // 0044ba47  891534925500           -mov dword ptr [0x559234], edx
    app->getMemory<x86::reg32>(x86::reg32(5607988) /* 0x559234 */) = cpu.edx;
    // 0044ba4d  40                     -inc eax
    (cpu.eax)++;
    // 0044ba4e  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044ba50  891c85fc276600         -mov dword ptr [eax*4 + 0x6627fc], ebx
    app->getMemory<x86::reg32>(x86::reg32(6694908) /* 0x6627fc */ + cpu.eax * 4) = cpu.ebx;
L_0x0044ba57:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044ba57  83f814                 +cmp eax, 0x14
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ba5a  7d0c                   -jge 0x44ba68
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0044ba68;
    }
    // 0044ba5c  40                     -inc eax
    (cpu.eax)++;
    // 0044ba5d  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0044ba5f  891c85fc276600         -mov dword ptr [eax*4 + 0x6627fc], ebx
    app->getMemory<x86::reg32>(x86::reg32(6694908) /* 0x6627fc */ + cpu.eax * 4) = cpu.ebx;
    // 0044ba66  ebef                   -jmp 0x44ba57
    goto L_0x0044ba57;
L_0x0044ba68:
    // 0044ba68  be05000000             -mov esi, 5
    cpu.esi = 5 /*0x5*/;
    // 0044ba6d  bfd0406000             -mov edi, 0x6040d0
    cpu.edi = 6308048 /*0x6040d0*/;
    // 0044ba72  89356c296600           -mov dword ptr [0x66296c], esi
    app->getMemory<x86::reg32>(x86::reg32(6695276) /* 0x66296c */) = cpu.esi;
    // 0044ba78  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0044ba7a  893d30925500           -mov dword ptr [0x559230], edi
    app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */) = cpu.edi;
    // 0044ba80  e8bbf0ffff             -call 0x44ab40
    cpu.esp -= 4;
    nfs3hp::port::loop::enterScreen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ba85  c70530768b0001000000   -mov dword ptr [0x8b7630], 1
    app->getMemory<x86::reg32>(x86::reg32(9139760) /* 0x8b7630 */) = 1 /*0x1*/;
    // 0044ba8f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba90  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba91  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba92  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba93  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba94  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ba95  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_44bac0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0044bac0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044bac1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044bac2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044bac3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044bac4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044bac5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044bac6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044bac8  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0044bacb  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0044bad0  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044bad2  8b3554925500           -mov esi, dword ptr [0x559254]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5608020) /* 0x559254 */);
    // 0044bad8  894df4                 -mov dword ptr [ebp - 0xc], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ecx;
    // 0044badb  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044badd  740f                   -je 0x44baee
    if (cpu.flags.zf)
    {
        goto L_0x0044baee;
    }
    // 0044badf  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0044bae1  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0044bae3  893d54925500           -mov dword ptr [0x559254], edi
    app->getMemory<x86::reg32>(x86::reg32(5608020) /* 0x559254 */) = cpu.edi;
    // 0044bae9  e982000000             -jmp 0x44bb70
    goto L_0x0044bb70;
L_0x0044baee:
    // 0044baee  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044baf3  e8b8ebffff             -call 0x44a6b0
    cpu.esp -= 4;
    nfs3hp::port::loop::itemUnderMouse(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044baf8  8b1530925500           -mov edx, dword ptr [0x559230]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bafe  8b7218                 -mov esi, dword ptr [edx + 0x18]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(24) /* 0x18 */);
    // 0044bb01  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0044bb03  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 0044bb06  39c6                   +cmp esi, eax
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
    // 0044bb08  7418                   -je 0x44bb22
    if (cpu.flags.zf)
    {
        goto L_0x0044bb22;
    }
    // 0044bb0a  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0044bb0c  e83fe3ffff             -call 0x449e50
    cpu.esp -= 4;
    nfs3hp::port::loop::selectable(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bb11  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044bb13  740d                   -je 0x44bb22
    if (cpu.flags.zf)
    {
        goto L_0x0044bb22;
    }
    // 0044bb15  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bb1a  668b501a               -mov dx, word ptr [eax + 0x1a]
    cpu.dx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(26) /* 0x1a */);
    // 0044bb1e  6689501c               -mov word ptr [eax + 0x1c], dx
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(28) /* 0x1c */) = cpu.dx;
L_0x0044bb22:
    // 0044bb22  83ffff                 +cmp edi, -1
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
    // 0044bb25  750d                   -jne 0x44bb34
    if (!cpu.flags.zf)
    {
        goto L_0x0044bb34;
    }
    // 0044bb27  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bb2c  66c7401a6400           -mov word ptr [eax + 0x1a], 0x64
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(26) /* 0x1a */) = 100 /*0x64*/;
    // 0044bb32  eb09                   -jmp 0x44bb3d
    goto L_0x0044bb3d;
L_0x0044bb34:
    // 0044bb34  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bb39  6689781a               -mov word ptr [eax + 0x1a], di
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(26) /* 0x1a */) = cpu.di;
L_0x0044bb3d:
    // 0044bb3d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044bb42  e8195e0000             -call 0x451960
    cpu.esp -= 4;
    nfs3hp::port::text::menuKey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bb47  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044bb49  6685c0                 +test ax, ax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.ax & cpu.ax));
    // 0044bb4c  7422                   -je 0x44bb70
    if (cpu.flags.zf)
    {
        goto L_0x0044bb70;
    }
    // 0044bb4e  e85d7dffff             -call 0x4438b0
    cpu.esp -= 4;
    nfs3hp::port::dialogs::dialogShown(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bb53  84c0                   +test al, al
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & cpu.al));
    // 0044bb55  740a                   -je 0x44bb61
    if (cpu.flags.zf)
    {
        goto L_0x0044bb61;
    }
    // 0044bb57  0fbfc6                 -movsx eax, si
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.si));
    // 0044bb5a  e801a3ffff             -call 0x445e60
    cpu.esp -= 4;
    nfs3hp::port::dialogs::dialogKey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bb5f  eb0d                   -jmp 0x44bb6e
    goto L_0x0044bb6e;
L_0x0044bb61:
    // 0044bb61  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bb66  0fbfd6                 -movsx edx, si
    cpu.edx = x86::reg32(static_cast<x86::sreg16>(cpu.si));
    // 0044bb69  e862e7ffff             -call 0x44a2d0
    cpu.esp -= 4;
    nfs3hp::port::loop::handleKey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044bb6e:
    // 0044bb6e  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x0044bb70:
    // 0044bb70  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044bb72  0f845e010000           -je 0x44bcd6
    if (cpu.flags.zf)
    {
        goto L_0x0044bcd6;
    }
    // 0044bb78  890d6c296600           -mov dword ptr [0x66296c], ecx
    app->getMemory<x86::reg32>(x86::reg32(6695276) /* 0x66296c */) = cpu.ecx;
    // 0044bb7e  8d41fb                 -lea eax, [ecx - 5]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(-5) /* -0x5 */);
    // 0044bb81  83f807                 +cmp eax, 7
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044bb84  0f8718010000           -ja 0x44bca2
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0044bca2;
    }
    // 0044bb8a  ff248598ba4400         -jmp dword ptr [eax*4 + 0x44ba98]
    cpu.ip = app->getMemory<x86::reg32>(4504216 + cpu.eax * 4); goto dynamic_jump;
  case 0x0044bb91:
    // 0044bb91  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bb96  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0044bb98  e893eeffff             -call 0x44aa30
    cpu.esp -= 4;
    nfs3hp::port::loop::leaveScreen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bb9d  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0044bb9f  e9fe000000             -jmp 0x44bca2
    goto L_0x0044bca2;
  case 0x0044bba4:
    // 0044bba4  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bba9  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0044bbab  e880eeffff             -call 0x44aa30
    cpu.esp -= 4;
    nfs3hp::port::loop::leaveScreen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bbb0  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bbb5  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0044bbba  e881efffff             -call 0x44ab40
    cpu.esp -= 4;
    nfs3hp::port::loop::enterScreen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bbbf  e9de000000             -jmp 0x44bca2
    goto L_0x0044bca2;
  case 0x0044bbc4:
    // 0044bbc4  8b3d30925500           -mov edi, dword ptr [0x559230]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bbca  8b4716                 -mov eax, dword ptr [edi + 0x16]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(22) /* 0x16 */);
    // 0044bbcd  8b5718                 -mov edx, dword ptr [edi + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(24) /* 0x18 */);
    // 0044bbd0  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0044bbd3  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0044bbd6  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0044bbd8  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0044bbdb  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0044bbde  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0044bbe1  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044bbe3  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0044bbe6  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044bbe8  bae04e6000             -mov edx, 0x604ee0
    cpu.edx = 6311648 /*0x604ee0*/;
    // 0044bbed  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0044bbf0  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0044bbf2  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0044bbf4  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0044bbf7  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0044bbf9  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0044bbfc  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044bbff  e83ce6ffff             -call 0x44a240
    cpu.esp -= 4;
    nfs3hp::port::loop::escapeButton(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bc04  39f8                   +cmp eax, edi
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
    // 0044bc06  750a                   -jne 0x44bc12
    if (!cpu.flags.zf)
    {
        goto L_0x0044bc12;
    }
    // 0044bc08  b903000000             -mov ecx, 3
    cpu.ecx = 3 /*0x3*/;
    // 0044bc0d  e99a000000             -jmp 0x44bcac
    goto L_0x0044bcac;
L_0x0044bc12:
    // 0044bc12  833f05                 +cmp dword ptr [edi], 5
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edi);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044bc15  7503                   -jne 0x44bc1a
    if (!cpu.flags.zf)
    {
        goto L_0x0044bc1a;
    }
    // 0044bc17  8b7740                 -mov esi, dword ptr [edi + 0x40]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(64) /* 0x40 */);
L_0x0044bc1a:
    // 0044bc1a  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044bc1c  0f8476000000           -je 0x44bc98
    if (cpu.flags.zf)
    {
        goto L_0x0044bc98;
    }
    // 0044bc22  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044bc24  895df4                 -mov dword ptr [ebp - 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;
L_0x0044bc27:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044bc27  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0044bc2a  3b05b00b6600           +cmp eax, dword ptr [0x660bb0]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6687664) /* 0x660bb0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044bc30  7d1e                   -jge 0x44bc50
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0044bc50;
    }
    // 0044bc32  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044bc34  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0044bc37  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0044bc39  8b1485e4406000         -mov edx, dword ptr [eax*4 + 0x6040e4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6308068) /* 0x6040e4 */ + cpu.eax * 4);
    // 0044bc40  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0044bc42  e8c9260a00             -call 0x4ee310
    cpu.esp -= 4;
    nfs3hp::watcom::strcmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bc47  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044bc49  7405                   -je 0x44bc50
    if (cpu.flags.zf)
    {
        goto L_0x0044bc50;
    }
    // 0044bc4b  ff45f4                 +inc dword ptr [ebp - 0xc]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044bc4e  ebd7                   -jmp 0x44bc27
    goto L_0x0044bc27;
L_0x0044bc50:
    // 0044bc50  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0044bc53  3b05b00b6600           +cmp eax, dword ptr [0x660bb0]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6687664) /* 0x660bb0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044bc59  7d36                   -jge 0x44bc91
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0044bc91;
    }
    // 0044bc5b  e8b0f2ffff             -call 0x44af10
    cpu.esp -= 4;
    nfs3hp::port::loop::pushMenu(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bc60  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bc65  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0044bc67  e8c4edffff             -call 0x44aa30
    cpu.esp -= 4;
    nfs3hp::port::loop::leaveScreen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bc6c  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0044bc6f  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 0044bc76  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0044bc78  bad0406000             -mov edx, 0x6040d0
    cpu.edx = 6308048 /*0x6040d0*/;
    // 0044bc7d  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0044bc80  01c2                   +add edx, eax
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
    // 0044bc82  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0044bc84  891530925500           -mov dword ptr [0x559230], edx
    app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */) = cpu.edx;
    // 0044bc8a  e8b1eeffff             -call 0x44ab40
    cpu.esp -= 4;
    nfs3hp::port::loop::enterScreen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bc8f  eb0c                   -jmp 0x44bc9d
    goto L_0x0044bc9d;
L_0x0044bc91:
    // 0044bc91  b903000000             -mov ecx, 3
    cpu.ecx = 3 /*0x3*/;
    // 0044bc96  eb05                   -jmp 0x44bc9d
    goto L_0x0044bc9d;
L_0x0044bc98:
    // 0044bc98  b903000000             -mov ecx, 3
    cpu.ecx = 3 /*0x3*/;
L_0x0044bc9d:
    // 0044bc9d  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
  [[fallthrough]];
  case 0x0044bca2:
L_0x0044bca2:
    // 0044bca2  83f903                 +cmp ecx, 3
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
    // 0044bca5  7405                   -je 0x44bcac
    if (cpu.flags.zf)
    {
        goto L_0x0044bcac;
    }
    // 0044bca7  83f901                 +cmp ecx, 1
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
    // 0044bcaa  752a                   -jne 0x44bcd6
    if (!cpu.flags.zf)
    {
        goto L_0x0044bcd6;
    }
L_0x0044bcac:
    // 0044bcac  8b3530925500           -mov esi, dword ptr [0x559230]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bcb2  81fed0406000           +cmp esi, 0x6040d0
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6308048 /*0x6040d0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044bcb8  7504                   -jne 0x44bcbe
    if (!cpu.flags.zf)
    {
        goto L_0x0044bcbe;
    }
    // 0044bcba  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0044bcbc  eb18                   -jmp 0x44bcd6
    goto L_0x0044bcd6;
L_0x0044bcbe:
    // 0044bcbe  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0044bcc0  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0044bcc2  e869edffff             -call 0x44aa30
    cpu.esp -= 4;
    nfs3hp::port::loop::leaveScreen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bcc7  e8f4f1ffff             -call 0x44aec0
    cpu.esp -= 4;
    nfs3hp::port::loop::popMenu(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bccc  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bcd1  e86aeeffff             -call 0x44ab40
    cpu.esp -= 4;
    nfs3hp::port::loop::enterScreen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044bcd6:
    // 0044bcd6  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0044bcd8  741c                   -je 0x44bcf6
    if (cpu.flags.zf)
    {
        goto L_0x0044bcf6;
    }
    // 0044bcda  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bcdf  8b7810                 -mov edi, dword ptr [eax + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0044bce2  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044bce4  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0044bce6  0f84d1000000           -je 0x44bdbd
    if (cpu.flags.zf)
    {
        goto L_0x0044bdbd;
    }
    // 0044bcec  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044bcee  ff5210                 -call dword ptr [edx + 0x10]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bcf1  e9c7000000             -jmp 0x44bdbd
    goto L_0x0044bdbd;
L_0x0044bcf6:
    // 0044bcf6  83f90b                 +cmp ecx, 0xb
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(11 /*0xb*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044bcf9  7209                   -jb 0x44bd04
    if (cpu.flags.cf)
    {
        goto L_0x0044bd04;
    }
    // 0044bcfb  761e                   -jbe 0x44bd1b
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044bd1b;
    }
    // 0044bcfd  83f90c                 +cmp ecx, 0xc
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
    // 0044bd00  7409                   -je 0x44bd0b
    if (cpu.flags.zf)
    {
        goto L_0x0044bd0b;
    }
    // 0044bd02  eb37                   -jmp 0x44bd3b
    goto L_0x0044bd3b;
L_0x0044bd04:
    // 0044bd04  83f90a                 +cmp ecx, 0xa
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(10 /*0xa*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044bd07  7422                   -je 0x44bd2b
    if (cpu.flags.zf)
    {
        goto L_0x0044bd2b;
    }
    // 0044bd09  eb30                   -jmp 0x44bd3b
    goto L_0x0044bd3b;
L_0x0044bd0b:
    // 0044bd0b  b903000000             -mov ecx, 3
    cpu.ecx = 3 /*0x3*/;
    // 0044bd10  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044bd12  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0044bd14  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd15  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd16  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd17  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd18  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd19  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd1a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044bd1b:
    // 0044bd1b  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 0044bd20  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044bd22  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0044bd24  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd25  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd26  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd27  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd28  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd29  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd2a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044bd2b:
    // 0044bd2b  b905000000             -mov ecx, 5
    cpu.ecx = 5 /*0x5*/;
    // 0044bd30  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044bd32  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0044bd34  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd35  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd36  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd37  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd38  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd39  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd3a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044bd3b:
    // 0044bd3b  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bd40  8b5016                 -mov edx, dword ptr [eax + 0x16]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(22) /* 0x16 */);
    // 0044bd43  8b4018                 -mov eax, dword ptr [eax + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0044bd46  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0044bd49  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0044bd4c  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0044bd4e  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0044bd55  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044bd57  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0044bd5a  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044bd5c  b9e04e6000             -mov ecx, 0x604ee0
    cpu.ecx = 6311648 /*0x604ee0*/;
    // 0044bd61  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0044bd64  01c1                   +add ecx, eax
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
    // 0044bd66  7453                   -je 0x44bdbb
    if (cpu.flags.zf)
    {
        goto L_0x0044bdbb;
    }
    // 0044bd68  8b4110                 -mov eax, dword ptr [ecx + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */);
    // 0044bd6b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044bd6d  744c                   -je 0x44bdbb
    if (cpu.flags.zf)
    {
        goto L_0x0044bdbb;
    }
    // 0044bd6f  ba68905300             -mov edx, 0x539068
    cpu.edx = 5476456 /*0x539068*/;
    // 0044bd74  e8476cffff             -call 0x4429c0
    cpu.esp -= 4;
    nfs3hp::port::menus::codelinkMatches(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bd79  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044bd7b  7410                   -je 0x44bd8d
    if (cpu.flags.zf)
    {
        goto L_0x0044bd8d;
    }
    // 0044bd7d  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0044bd82  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044bd84  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0044bd86  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd87  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd88  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd89  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd8a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd8b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bd8c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044bd8d:
    // 0044bd8d  ba78905300             -mov edx, 0x539078
    cpu.edx = 5476472 /*0x539078*/;
    // 0044bd92  8b4110                 -mov eax, dword ptr [ecx + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */);
    // 0044bd95  e8266cffff             -call 0x4429c0
    cpu.esp -= 4;
    nfs3hp::port::menus::codelinkMatches(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bd9a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044bd9c  7410                   -je 0x44bdae
    if (cpu.flags.zf)
    {
        goto L_0x0044bdae;
    }
    // 0044bd9e  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0044bda3  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044bda5  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0044bda7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bda8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bda9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdaa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdab  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdac  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdad  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044bdae:
    // 0044bdae  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044bdb0  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044bdb2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0044bdb4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdb5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdb6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdb7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdb8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdb9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdba  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044bdbb:
    // 0044bdbb  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0044bdbd:
    // 0044bdbd  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044bdbf  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0044bdc1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdc2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdc3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdc4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdc5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdc6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044bdc7  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_44bdd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044bdd0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044bdd1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044bdd2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044bdd4  833d6492550000         +cmp dword ptr [0x559264], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5608036) /* 0x559264 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044bddb  7505                   -jne 0x44bde2
    if (!cpu.flags.zf)
    {
        goto L_0x0044bde2;
    }
    // 0044bddd  e8feb70500             -call 0x4a75e0
    cpu.esp -= 4;
    sub_4a75e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044bde2:
    // 0044bde2  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044bde7  e804e7ffff             -call 0x44a4f0
    cpu.esp -= 4;
    nfs3hp::port::loop::drawItems(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bdec  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044bdee  e8bd7affff             -call 0x4438b0
    cpu.esp -= 4;
    nfs3hp::port::dialogs::dialogShown(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bdf3  84c0                   +test al, al
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & cpu.al));
    // 0044bdf5  7407                   -je 0x44bdfe
    if (cpu.flags.zf)
    {
        goto L_0x0044bdfe;
    }
    // 0044bdf7  e854a1ffff             -call 0x445f50
    cpu.esp -= 4;
    nfs3hp::port::dialogs::dialogPass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044bdfc  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
L_0x0044bdfe:
    // 0044bdfe  e83d550800             -call 0x4d1340
    cpu.esp -= 4;
    nfs3hp::port::text::flushFonts(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044be03  e8588dfeff             -call 0x434b60
    cpu.esp -= 4;
    nfs3hp::collide::drawPointer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044be08  a130925500             -mov eax, dword ptr [0x559230]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5607984) /* 0x559230 */);
    // 0044be0d  e89eeaffff             -call 0x44a8b0
    cpu.esp -= 4;
    nfs3hp::port::loop::passItems(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044be12  891554925500           -mov dword ptr [0x559254], edx
    app->getMemory<x86::reg32>(x86::reg32(5608020) /* 0x559254 */) = cpu.edx;
    // 0044be18  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0044be1a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044be1b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044be1c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_44be20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044be20  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044be21  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044be22  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044be24  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044be26  891534925500           -mov dword ptr [0x559234], edx
    app->getMemory<x86::reg32>(x86::reg32(5607988) /* 0x559234 */) = cpu.edx;
    // 0044be2c  e8dfa0ffff             -call 0x445f10
    cpu.esp -= 4;
    nfs3hp::port::dialogs::closeDialog(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044be31  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044be32  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044be33  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_44be60: native, nfs3hp::port::loop::fitRaceSettings */

/* align: skip 0x00 0x00 */
void Application::sub_44c2d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044c2d0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044c2d1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044c2d2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044c2d3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044c2d5  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044c2d7  baac905300             -mov edx, 0x5390ac
    cpu.edx = 5476524 /*0x5390ac*/;
    // 0044c2dc  e85f67ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c2e1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c2e3  0f8468000000           -je 0x44c351
    if (cpu.flags.zf)
    {
        goto L_0x0044c351;
    }
    // 0044c2e9  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
    // 0044c2ed  bab4905300             -mov edx, 0x5390b4
    cpu.edx = 5476532 /*0x5390b4*/;
    // 0044c2f2  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c2f4  e84767ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c2f9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c2fb  7454                   -je 0x44c351
    if (cpu.flags.zf)
    {
        goto L_0x0044c351;
    }
    // 0044c2fd  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
    // 0044c301  bac0905300             -mov edx, 0x5390c0
    cpu.edx = 5476544 /*0x5390c0*/;
    // 0044c306  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c308  e83367ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c30d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c30f  7440                   -je 0x44c351
    if (cpu.flags.zf)
    {
        goto L_0x0044c351;
    }
    // 0044c311  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
    // 0044c315  bac8905300             -mov edx, 0x5390c8
    cpu.edx = 5476552 /*0x5390c8*/;
    // 0044c31a  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c31c  e81f67ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c321  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c323  742c                   -je 0x44c351
    if (cpu.flags.zf)
    {
        goto L_0x0044c351;
    }
    // 0044c325  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
    // 0044c329  bad0905300             -mov edx, 0x5390d0
    cpu.edx = 5476560 /*0x5390d0*/;
    // 0044c32e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c330  e80b67ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c335  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c337  7418                   -je 0x44c351
    if (cpu.flags.zf)
    {
        goto L_0x0044c351;
    }
    // 0044c339  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
    // 0044c33d  bad8905300             -mov edx, 0x5390d8
    cpu.edx = 5476568 /*0x5390d8*/;
    // 0044c342  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c344  e8f766ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c349  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c34b  7404                   -je 0x44c351
    if (cpu.flags.zf)
    {
        goto L_0x0044c351;
    }
    // 0044c34d  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c351:
    // 0044c351  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c352  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c353  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c354  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_44c360(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044c360  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044c361  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044c362  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044c363  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044c365  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044c367  bae0905300             -mov edx, 0x5390e0
    cpu.edx = 5476576 /*0x5390e0*/;
    // 0044c36c  e8cf66ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c371  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c373  7404                   -je 0x44c379
    if (cpu.flags.zf)
    {
        goto L_0x0044c379;
    }
    // 0044c375  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c379:
    // 0044c379  baf0905300             -mov edx, 0x5390f0
    cpu.edx = 5476592 /*0x5390f0*/;
    // 0044c37e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c380  e8bb66ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c385  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c387  7404                   -je 0x44c38d
    if (cpu.flags.zf)
    {
        goto L_0x0044c38d;
    }
    // 0044c389  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c38d:
    // 0044c38d  bafc905300             -mov edx, 0x5390fc
    cpu.edx = 5476604 /*0x5390fc*/;
    // 0044c392  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c394  e8a766ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c399  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c39b  7404                   -je 0x44c3a1
    if (cpu.flags.zf)
    {
        goto L_0x0044c3a1;
    }
    // 0044c39d  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c3a1:
    // 0044c3a1  ba0c915300             -mov edx, 0x53910c
    cpu.edx = 5476620 /*0x53910c*/;
    // 0044c3a6  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c3a8  e89366ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c3ad  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c3af  7404                   -je 0x44c3b5
    if (cpu.flags.zf)
    {
        goto L_0x0044c3b5;
    }
    // 0044c3b1  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c3b5:
    // 0044c3b5  ba18915300             -mov edx, 0x539118
    cpu.edx = 5476632 /*0x539118*/;
    // 0044c3ba  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c3bc  e87f66ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c3c1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c3c3  7404                   -je 0x44c3c9
    if (cpu.flags.zf)
    {
        goto L_0x0044c3c9;
    }
    // 0044c3c5  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c3c9:
    // 0044c3c9  ba24915300             -mov edx, 0x539124
    cpu.edx = 5476644 /*0x539124*/;
    // 0044c3ce  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c3d0  e86b66ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c3d5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c3d7  7404                   -je 0x44c3dd
    if (cpu.flags.zf)
    {
        goto L_0x0044c3dd;
    }
    // 0044c3d9  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c3dd:
    // 0044c3dd  ba2c915300             -mov edx, 0x53912c
    cpu.edx = 5476652 /*0x53912c*/;
    // 0044c3e2  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c3e4  e85766ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c3e9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c3eb  7404                   -je 0x44c3f1
    if (cpu.flags.zf)
    {
        goto L_0x0044c3f1;
    }
    // 0044c3ed  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c3f1:
    // 0044c3f1  ba3c915300             -mov edx, 0x53913c
    cpu.edx = 5476668 /*0x53913c*/;
    // 0044c3f6  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c3f8  e84366ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c3fd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c3ff  7404                   -je 0x44c405
    if (cpu.flags.zf)
    {
        goto L_0x0044c405;
    }
    // 0044c401  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c405:
    // 0044c405  ba4c915300             -mov edx, 0x53914c
    cpu.edx = 5476684 /*0x53914c*/;
    // 0044c40a  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c40c  e82f66ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c411  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c413  7404                   -je 0x44c419
    if (cpu.flags.zf)
    {
        goto L_0x0044c419;
    }
    // 0044c415  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c419:
    // 0044c419  ba60915300             -mov edx, 0x539160
    cpu.edx = 5476704 /*0x539160*/;
    // 0044c41e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c420  e81b66ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c425  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c427  7404                   -je 0x44c42d
    if (cpu.flags.zf)
    {
        goto L_0x0044c42d;
    }
    // 0044c429  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c42d:
    // 0044c42d  ba74915300             -mov edx, 0x539174
    cpu.edx = 5476724 /*0x539174*/;
    // 0044c432  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c434  e80766ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c439  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c43b  7404                   -je 0x44c441
    if (cpu.flags.zf)
    {
        goto L_0x0044c441;
    }
    // 0044c43d  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c441:
    // 0044c441  ba88915300             -mov edx, 0x539188
    cpu.edx = 5476744 /*0x539188*/;
    // 0044c446  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c448  e8f365ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c44d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c44f  7404                   -je 0x44c455
    if (cpu.flags.zf)
    {
        goto L_0x0044c455;
    }
    // 0044c451  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c455:
    // 0044c455  ba9c915300             -mov edx, 0x53919c
    cpu.edx = 5476764 /*0x53919c*/;
    // 0044c45a  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c45c  e8df65ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c461  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c463  7404                   -je 0x44c469
    if (cpu.flags.zf)
    {
        goto L_0x0044c469;
    }
    // 0044c465  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c469:
    // 0044c469  baac915300             -mov edx, 0x5391ac
    cpu.edx = 5476780 /*0x5391ac*/;
    // 0044c46e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c470  e8cb65ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c475  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c477  7404                   -je 0x44c47d
    if (cpu.flags.zf)
    {
        goto L_0x0044c47d;
    }
    // 0044c479  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c47d:
    // 0044c47d  bab8915300             -mov edx, 0x5391b8
    cpu.edx = 5476792 /*0x5391b8*/;
    // 0044c482  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c484  e8b765ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c489  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c48b  7404                   -je 0x44c491
    if (cpu.flags.zf)
    {
        goto L_0x0044c491;
    }
    // 0044c48d  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044c491:
    // 0044c491  bacc915300             -mov edx, 0x5391cc
    cpu.edx = 5476812 /*0x5391cc*/;
    // 0044c496  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c498  e8a365ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c49d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c49f  7404                   -je 0x44c4a5
    if (cpu.flags.zf)
    {
        goto L_0x0044c4a5;
    }
    // 0044c4a1  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044c4a5:
    // 0044c4a5  bae0915300             -mov edx, 0x5391e0
    cpu.edx = 5476832 /*0x5391e0*/;
    // 0044c4aa  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c4ac  e88f65ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c4b1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c4b3  7404                   -je 0x44c4b9
    if (cpu.flags.zf)
    {
        goto L_0x0044c4b9;
    }
    // 0044c4b5  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044c4b9:
    // 0044c4b9  baf8915300             -mov edx, 0x5391f8
    cpu.edx = 5476856 /*0x5391f8*/;
    // 0044c4be  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c4c0  e87b65ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c4c5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c4c7  7404                   -je 0x44c4cd
    if (cpu.flags.zf)
    {
        goto L_0x0044c4cd;
    }
    // 0044c4c9  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044c4cd:
    // 0044c4cd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c4ce  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c4cf  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c4d0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_44c4e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044c4e0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044c4e1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044c4e2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044c4e3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044c4e5  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044c4e7  ba0c925300             -mov edx, 0x53920c
    cpu.edx = 5476876 /*0x53920c*/;
    // 0044c4ec  e84f65ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c4f1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c4f3  7404                   -je 0x44c4f9
    if (cpu.flags.zf)
    {
        goto L_0x0044c4f9;
    }
    // 0044c4f5  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c4f9:
    // 0044c4f9  ba1c925300             -mov edx, 0x53921c
    cpu.edx = 5476892 /*0x53921c*/;
    // 0044c4fe  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c500  e83b65ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c505  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c507  7404                   -je 0x44c50d
    if (cpu.flags.zf)
    {
        goto L_0x0044c50d;
    }
    // 0044c509  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c50d:
    // 0044c50d  ba2c925300             -mov edx, 0x53922c
    cpu.edx = 5476908 /*0x53922c*/;
    // 0044c512  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c514  e82765ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c519  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c51b  7404                   -je 0x44c521
    if (cpu.flags.zf)
    {
        goto L_0x0044c521;
    }
    // 0044c51d  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c521:
    // 0044c521  ba3c925300             -mov edx, 0x53923c
    cpu.edx = 5476924 /*0x53923c*/;
    // 0044c526  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c528  e81365ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c52d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c52f  7404                   -je 0x44c535
    if (cpu.flags.zf)
    {
        goto L_0x0044c535;
    }
    // 0044c531  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c535:
    // 0044c535  ba4c925300             -mov edx, 0x53924c
    cpu.edx = 5476940 /*0x53924c*/;
    // 0044c53a  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c53c  e8ff64ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c541  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c543  7404                   -je 0x44c549
    if (cpu.flags.zf)
    {
        goto L_0x0044c549;
    }
    // 0044c545  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c549:
    // 0044c549  ba5c925300             -mov edx, 0x53925c
    cpu.edx = 5476956 /*0x53925c*/;
    // 0044c54e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c550  e8eb64ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c555  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c557  7404                   -je 0x44c55d
    if (cpu.flags.zf)
    {
        goto L_0x0044c55d;
    }
    // 0044c559  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c55d:
    // 0044c55d  bab8915300             -mov edx, 0x5391b8
    cpu.edx = 5476792 /*0x5391b8*/;
    // 0044c562  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c564  e8d764ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c569  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c56b  7404                   -je 0x44c571
    if (cpu.flags.zf)
    {
        goto L_0x0044c571;
    }
    // 0044c56d  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c571:
    // 0044c571  bacc915300             -mov edx, 0x5391cc
    cpu.edx = 5476812 /*0x5391cc*/;
    // 0044c576  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c578  e8c364ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c57d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c57f  7404                   -je 0x44c585
    if (cpu.flags.zf)
    {
        goto L_0x0044c585;
    }
    // 0044c581  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c585:
    // 0044c585  bae0915300             -mov edx, 0x5391e0
    cpu.edx = 5476832 /*0x5391e0*/;
    // 0044c58a  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c58c  e8af64ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c591  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c593  7404                   -je 0x44c599
    if (cpu.flags.zf)
    {
        goto L_0x0044c599;
    }
    // 0044c595  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c599:
    // 0044c599  baf8915300             -mov edx, 0x5391f8
    cpu.edx = 5476856 /*0x5391f8*/;
    // 0044c59e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c5a0  e89b64ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c5a5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c5a7  7404                   -je 0x44c5ad
    if (cpu.flags.zf)
    {
        goto L_0x0044c5ad;
    }
    // 0044c5a9  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c5ad:
    // 0044c5ad  ba68925300             -mov edx, 0x539268
    cpu.edx = 5476968 /*0x539268*/;
    // 0044c5b2  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c5b4  e88764ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c5b9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c5bb  7404                   -je 0x44c5c1
    if (cpu.flags.zf)
    {
        goto L_0x0044c5c1;
    }
    // 0044c5bd  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c5c1:
    // 0044c5c1  ba80925300             -mov edx, 0x539280
    cpu.edx = 5476992 /*0x539280*/;
    // 0044c5c6  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c5c8  e87364ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c5cd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c5cf  7404                   -je 0x44c5d5
    if (cpu.flags.zf)
    {
        goto L_0x0044c5d5;
    }
    // 0044c5d1  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c5d5:
    // 0044c5d5  ba94925300             -mov edx, 0x539294
    cpu.edx = 5477012 /*0x539294*/;
    // 0044c5da  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c5dc  e85f64ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c5e1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c5e3  7404                   -je 0x44c5e9
    if (cpu.flags.zf)
    {
        goto L_0x0044c5e9;
    }
    // 0044c5e5  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c5e9:
    // 0044c5e9  baa4925300             -mov edx, 0x5392a4
    cpu.edx = 5477028 /*0x5392a4*/;
    // 0044c5ee  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044c5f0  e84b64ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c5f5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c5f7  7404                   -je 0x44c5fd
    if (cpu.flags.zf)
    {
        goto L_0x0044c5fd;
    }
    // 0044c5f9  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044c5fd:
    // 0044c5fd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c5fe  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c5ff  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c600  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_44c610(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044c610  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044c611  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044c612  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044c614  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044c616  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044c618  83f901                 +cmp ecx, 1
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
    // 0044c61b  724b                   -jb 0x44c668
    if (cpu.flags.cf)
    {
        goto L_0x0044c668;
    }
    // 0044c61d  7608                   -jbe 0x44c627
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044c627;
    }
    // 0044c61f  83f903                 +cmp ecx, 3
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
    // 0044c622  7438                   -je 0x44c65c
    if (cpu.flags.zf)
    {
        goto L_0x0044c65c;
    }
    // 0044c624  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c625  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c626  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044c627:
    // 0044c627  e89480ffff             -call 0x4446c0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogFocusedButton(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c62c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c62e  750f                   -jne 0x44c63f
    if (!cpu.flags.zf)
    {
        goto L_0x0044c63f;
    }
    // 0044c630  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0044c632  7403                   -je 0x44c637
    if (cpu.flags.zf)
    {
        goto L_0x0044c637;
    }
    // 0044c634  c60201                 -mov byte ptr [edx], 1
    app->getMemory<x86::reg8>(cpu.edx) = 1 /*0x1*/;
L_0x0044c637:
    // 0044c637  a168925500             -mov eax, dword ptr [0x559268]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608040) /* 0x559268 */);
    // 0044c63c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c63d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c63e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044c63f:
    // 0044c63f  83f801                 +cmp eax, 1
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
    // 0044c642  7509                   -jne 0x44c64d
    if (!cpu.flags.zf)
    {
        goto L_0x0044c64d;
    }
    // 0044c644  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0044c646  740c                   -je 0x44c654
    if (cpu.flags.zf)
    {
        goto L_0x0044c654;
    }
    // 0044c648  c60201                 -mov byte ptr [edx], 1
    app->getMemory<x86::reg8>(cpu.edx) = 1 /*0x1*/;
    // 0044c64b  eb07                   -jmp 0x44c654
    goto L_0x0044c654;
L_0x0044c64d:
    // 0044c64d  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0044c64f  7403                   -je 0x44c654
    if (cpu.flags.zf)
    {
        goto L_0x0044c654;
    }
    // 0044c651  c60200                 -mov byte ptr [edx], 0
    app->getMemory<x86::reg8>(cpu.edx) = 0 /*0x0*/;
L_0x0044c654:
    // 0044c654  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044c659  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c65a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c65b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044c65c:
    // 0044c65c  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0044c65e  7403                   -je 0x44c663
    if (cpu.flags.zf)
    {
        goto L_0x0044c663;
    }
    // 0044c660  c60201                 -mov byte ptr [edx], 1
    app->getMemory<x86::reg8>(cpu.edx) = 1 /*0x1*/;
L_0x0044c663:
    // 0044c663  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
L_0x0044c668:
    // 0044c668  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c669  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c66a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_44c670(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044c670  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044c671  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044c672  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044c674  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044c676  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044c678  83f901                 +cmp ecx, 1
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
    // 0044c67b  724b                   -jb 0x44c6c8
    if (cpu.flags.cf)
    {
        goto L_0x0044c6c8;
    }
    // 0044c67d  7608                   -jbe 0x44c687
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044c687;
    }
    // 0044c67f  83f903                 +cmp ecx, 3
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
    // 0044c682  7438                   -je 0x44c6bc
    if (cpu.flags.zf)
    {
        goto L_0x0044c6bc;
    }
    // 0044c684  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c685  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c686  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044c687:
    // 0044c687  e83480ffff             -call 0x4446c0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogFocusedButton(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c68c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c68e  750f                   -jne 0x44c69f
    if (!cpu.flags.zf)
    {
        goto L_0x0044c69f;
    }
    // 0044c690  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0044c692  7403                   -je 0x44c697
    if (cpu.flags.zf)
    {
        goto L_0x0044c697;
    }
    // 0044c694  c60201                 -mov byte ptr [edx], 1
    app->getMemory<x86::reg8>(cpu.edx) = 1 /*0x1*/;
L_0x0044c697:
    // 0044c697  b806000000             -mov eax, 6
    cpu.eax = 6 /*0x6*/;
    // 0044c69c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c69d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c69e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044c69f:
    // 0044c69f  83f801                 +cmp eax, 1
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
    // 0044c6a2  7509                   -jne 0x44c6ad
    if (!cpu.flags.zf)
    {
        goto L_0x0044c6ad;
    }
    // 0044c6a4  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0044c6a6  740c                   -je 0x44c6b4
    if (cpu.flags.zf)
    {
        goto L_0x0044c6b4;
    }
    // 0044c6a8  c60201                 -mov byte ptr [edx], 1
    app->getMemory<x86::reg8>(cpu.edx) = 1 /*0x1*/;
    // 0044c6ab  eb07                   -jmp 0x44c6b4
    goto L_0x0044c6b4;
L_0x0044c6ad:
    // 0044c6ad  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0044c6af  7403                   -je 0x44c6b4
    if (cpu.flags.zf)
    {
        goto L_0x0044c6b4;
    }
    // 0044c6b1  c60200                 -mov byte ptr [edx], 0
    app->getMemory<x86::reg8>(cpu.edx) = 0 /*0x0*/;
L_0x0044c6b4:
    // 0044c6b4  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044c6b9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c6ba  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c6bb  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044c6bc:
    // 0044c6bc  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0044c6be  7403                   -je 0x44c6c3
    if (cpu.flags.zf)
    {
        goto L_0x0044c6c3;
    }
    // 0044c6c0  c60201                 -mov byte ptr [edx], 1
    app->getMemory<x86::reg8>(cpu.edx) = 1 /*0x1*/;
L_0x0044c6c3:
    // 0044c6c3  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
L_0x0044c6c8:
    // 0044c6c8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c6c9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c6ca  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_44c6d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044c6d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044c6d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044c6d2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044c6d3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044c6d5  891568925500           -mov dword ptr [0x559268], edx
    app->getMemory<x86::reg32>(x86::reg32(5608040) /* 0x559268 */) = cpu.edx;
    // 0044c6db  e870510800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c6e0  a310446600             -mov dword ptr [0x664410], eax
    app->getMemory<x86::reg32>(x86::reg32(6702096) /* 0x664410 */) = cpu.eax;
    // 0044c6e5  b8c4000000             -mov eax, 0xc4
    cpu.eax = 196 /*0xc4*/;
    // 0044c6ea  e861510800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c6ef  a314446600             -mov dword ptr [0x664414], eax
    app->getMemory<x86::reg32>(x86::reg32(6702100) /* 0x664414 */) = cpu.eax;
    // 0044c6f4  b8c5000000             -mov eax, 0xc5
    cpu.eax = 197 /*0xc5*/;
    // 0044c6f9  e852510800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c6fe  6810c64400             -push 0x44c610
    app->getMemory<x86::reg32>(cpu.esp-4) = 4507152 /*0x44c610*/;
    cpu.esp -= 4;
    // 0044c703  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044c705  b914446600             -mov ecx, 0x664414
    cpu.ecx = 6702100 /*0x664414*/;
    // 0044c70a  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0044c70c  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 0044c711  ba10446600             -mov edx, 0x664410
    cpu.edx = 6702096 /*0x664410*/;
    // 0044c716  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044c718  a318446600             -mov dword ptr [0x664418], eax
    app->getMemory<x86::reg32>(x86::reg32(6702104) /* 0x664418 */) = cpu.eax;
    // 0044c71d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044c722  e8a97fffff             -call 0x4446d0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogOpenQuestion(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c727  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c728  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c729  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c72a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_44c730(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044c730  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044c731  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044c732  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044c734  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0044c736  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044c738  83f801                 +cmp eax, 1
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
    // 0044c73b  0f825d000000           -jb 0x44c79e
    if (cpu.flags.cf)
    {
        goto L_0x0044c79e;
    }
    // 0044c741  760a                   -jbe 0x44c74d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044c74d;
    }
    // 0044c743  83f803                 +cmp eax, 3
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
    // 0044c746  744a                   -je 0x44c792
    if (cpu.flags.zf)
    {
        goto L_0x0044c792;
    }
    // 0044c748  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0044c74a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c74b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c74c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044c74d:
    // 0044c74d  e86e7fffff             -call 0x4446c0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogFocusedButton(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c752  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c754  7c2b                   -jl 0x44c781
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0044c781;
    }
    // 0044c756  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0044c759  8b90882c6600           -mov edx, dword ptr [eax + 0x662c88]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6696072) /* 0x662c88 */);
    // 0044c75f  83fa02                 +cmp edx, 2
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
    // 0044c762  7509                   -jne 0x44c76d
    if (!cpu.flags.zf)
    {
        goto L_0x0044c76d;
    }
    // 0044c764  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044c766  7431                   -je 0x44c799
    if (cpu.flags.zf)
    {
        goto L_0x0044c799;
    }
    // 0044c768  c60101                 -mov byte ptr [ecx], 1
    app->getMemory<x86::reg8>(cpu.ecx) = 1 /*0x1*/;
    // 0044c76b  eb2c                   -jmp 0x44c799
    goto L_0x0044c799;
L_0x0044c76d:
    // 0044c76d  8b80702c6600           -mov eax, dword ptr [eax + 0x662c70]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6696048) /* 0x662c70 */);
    // 0044c773  e858ffffff             -call 0x44c6d0
    cpu.esp -= 4;
    sub_44c6d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c778  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044c77a  740c                   -je 0x44c788
    if (cpu.flags.zf)
    {
        goto L_0x0044c788;
    }
    // 0044c77c  c60100                 -mov byte ptr [ecx], 0
    app->getMemory<x86::reg8>(cpu.ecx) = 0 /*0x0*/;
    // 0044c77f  eb07                   -jmp 0x44c788
    goto L_0x0044c788;
L_0x0044c781:
    // 0044c781  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044c783  7403                   -je 0x44c788
    if (cpu.flags.zf)
    {
        goto L_0x0044c788;
    }
    // 0044c785  c60100                 -mov byte ptr [ecx], 0
    app->getMemory<x86::reg8>(cpu.ecx) = 0 /*0x0*/;
L_0x0044c788:
    // 0044c788  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 0044c78d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0044c78f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c790  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c791  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044c792:
    // 0044c792  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044c794  7403                   -je 0x44c799
    if (cpu.flags.zf)
    {
        goto L_0x0044c799;
    }
    // 0044c796  c60101                 -mov byte ptr [ecx], 1
    app->getMemory<x86::reg8>(cpu.ecx) = 1 /*0x1*/;
L_0x0044c799:
    // 0044c799  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
L_0x0044c79e:
    // 0044c79e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0044c7a0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c7a1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c7a2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_44c7b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044c7b0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044c7b1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044c7b2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044c7b3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044c7b4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044c7b5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044c7b6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044c7b8  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044c7bf  7e11                   -jle 0x44c7d2
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0044c7d2;
    }
    // 0044c7c1  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0044c7c9  7507                   -jne 0x44c7d2
    if (!cpu.flags.zf)
    {
        goto L_0x0044c7d2;
    }
    // 0044c7cb  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044c7d0  eb02                   -jmp 0x44c7d4
    goto L_0x0044c7d4;
L_0x0044c7d2:
    // 0044c7d2  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044c7d4:
    // 0044c7d4  8b1da0d36f00           -mov ebx, dword ptr [0x6fd3a0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
    // 0044c7da  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0044c7dc  83fb02                 +cmp ebx, 2
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
    // 0044c7df  7c07                   -jl 0x44c7e8
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0044c7e8;
    }
    // 0044c7e1  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044c7e6  eb02                   -jmp 0x44c7ea
    goto L_0x0044c7ea;
L_0x0044c7e8:
    // 0044c7e8  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044c7ea:
    // 0044c7ea  b8db010000             -mov eax, 0x1db
    cpu.eax = 475 /*0x1db*/;
    // 0044c7ef  e85c500800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c7f4  a31c446600             -mov dword ptr [0x66441c], eax
    app->getMemory<x86::reg32>(x86::reg32(6702108) /* 0x66441c */) = cpu.eax;
    // 0044c7f9  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0044c7fb  0f84a2000000           -je 0x44c8a3
    if (cpu.flags.zf)
    {
        goto L_0x0044c8a3;
    }
    // 0044c801  a1b4367d00             -mov eax, dword ptr [0x7d36b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */);
    // 0044c806  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044c808  7507                   -jne 0x44c811
    if (!cpu.flags.zf)
    {
        goto L_0x0044c811;
    }
    // 0044c80a  b8dc010000             -mov eax, 0x1dc
    cpu.eax = 476 /*0x1dc*/;
    // 0044c80f  eb05                   -jmp 0x44c816
    goto L_0x0044c816;
L_0x0044c811:
    // 0044c811  b8dd010000             -mov eax, 0x1dd
    cpu.eax = 477 /*0x1dd*/;
L_0x0044c816:
    // 0044c816  e835500800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c81b  a320446600             -mov dword ptr [0x664420], eax
    app->getMemory<x86::reg32>(x86::reg32(6702112) /* 0x664420 */) = cpu.eax;
    // 0044c820  b8ef010000             -mov eax, 0x1ef
    cpu.eax = 495 /*0x1ef*/;
    // 0044c825  be0c000000             -mov esi, 0xc
    cpu.esi = 12 /*0xc*/;
    // 0044c82a  bf0a000000             -mov edi, 0xa
    cpu.edi = 10 /*0xa*/;
    // 0044c82f  ba00010000             -mov edx, 0x100
    cpu.edx = 256 /*0x100*/;
    // 0044c834  b9fc000000             -mov ecx, 0xfc
    cpu.ecx = 252 /*0xfc*/;
    // 0044c839  e812500800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c83e  a324446600             -mov dword ptr [0x664424], eax
    app->getMemory<x86::reg32>(x86::reg32(6702116) /* 0x664424 */) = cpu.eax;
    // 0044c843  b8c6000000             -mov eax, 0xc6
    cpu.eax = 198 /*0xc6*/;
    // 0044c848  bb03000000             -mov ebx, 3
    cpu.ebx = 3 /*0x3*/;
    // 0044c84d  e8fe4f0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c852  6830c74400             -push 0x44c730
    app->getMemory<x86::reg32>(cpu.esp-4) = 4507440 /*0x44c730*/;
    cpu.esp -= 4;
    // 0044c857  a328446600             -mov dword ptr [0x664428], eax
    app->getMemory<x86::reg32>(x86::reg32(6702120) /* 0x664428 */) = cpu.eax;
    // 0044c85c  8935882c6600           -mov dword ptr [0x662c88], esi
    app->getMemory<x86::reg32>(x86::reg32(6696072) /* 0x662c88 */) = cpu.esi;
    // 0044c862  8915702c6600           -mov dword ptr [0x662c70], edx
    app->getMemory<x86::reg32>(x86::reg32(6696048) /* 0x662c70 */) = cpu.edx;
    // 0044c868  890d742c6600           -mov dword ptr [0x662c74], ecx
    app->getMemory<x86::reg32>(x86::reg32(6696052) /* 0x662c74 */) = cpu.ecx;
    // 0044c86e  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0044c870  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044c875  b920446600             -mov ecx, 0x664420
    cpu.ecx = 6702112 /*0x664420*/;
    // 0044c87a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0044c87b  ba1c446600             -mov edx, 0x66441c
    cpu.edx = 6702108 /*0x66441c*/;
    // 0044c880  a3902c6600             -mov dword ptr [0x662c90], eax
    app->getMemory<x86::reg32>(x86::reg32(6696080) /* 0x662c90 */) = cpu.eax;
    // 0044c885  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044c887  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044c88c  893d8c2c6600           -mov dword ptr [0x662c8c], edi
    app->getMemory<x86::reg32>(x86::reg32(6696076) /* 0x662c8c */) = cpu.edi;
    // 0044c892  e8397effff             -call 0x4446d0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogOpenQuestion(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c897  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044c89c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c89d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c89e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c89f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c8a0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c8a1  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c8a2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044c8a3:
    // 0044c8a3  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044c8a5  0f8492000000           -je 0x44c93d
    if (cpu.flags.zf)
    {
        goto L_0x0044c93d;
    }
    // 0044c8ab  b8df010000             -mov eax, 0x1df
    cpu.eax = 479 /*0x1df*/;
    // 0044c8b0  bb0b000000             -mov ebx, 0xb
    cpu.ebx = 11 /*0xb*/;
    // 0044c8b5  be0a000000             -mov esi, 0xa
    cpu.esi = 10 /*0xa*/;
    // 0044c8ba  bf02000000             -mov edi, 2
    cpu.edi = 2 /*0x2*/;
    // 0044c8bf  e88c4f0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c8c4  a320446600             -mov dword ptr [0x664420], eax
    app->getMemory<x86::reg32>(x86::reg32(6702112) /* 0x664420 */) = cpu.eax;
    // 0044c8c9  b8ef010000             -mov eax, 0x1ef
    cpu.eax = 495 /*0x1ef*/;
    // 0044c8ce  bafc000000             -mov edx, 0xfc
    cpu.edx = 252 /*0xfc*/;
    // 0044c8d3  e8784f0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c8d8  a324446600             -mov dword ptr [0x664424], eax
    app->getMemory<x86::reg32>(x86::reg32(6702116) /* 0x664424 */) = cpu.eax;
    // 0044c8dd  b8c6000000             -mov eax, 0xc6
    cpu.eax = 198 /*0xc6*/;
    // 0044c8e2  b920446600             -mov ecx, 0x664420
    cpu.ecx = 6702112 /*0x664420*/;
    // 0044c8e7  e8644f0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c8ec  6830c74400             -push 0x44c730
    app->getMemory<x86::reg32>(cpu.esp-4) = 4507440 /*0x44c730*/;
    cpu.esp -= 4;
    // 0044c8f1  a328446600             -mov dword ptr [0x664428], eax
    app->getMemory<x86::reg32>(x86::reg32(6702120) /* 0x664428 */) = cpu.eax;
    // 0044c8f6  891d882c6600           -mov dword ptr [0x662c88], ebx
    app->getMemory<x86::reg32>(x86::reg32(6696072) /* 0x662c88 */) = cpu.ebx;
    // 0044c8fc  89358c2c6600           -mov dword ptr [0x662c8c], esi
    app->getMemory<x86::reg32>(x86::reg32(6696076) /* 0x662c8c */) = cpu.esi;
    // 0044c902  8915742c6600           -mov dword ptr [0x662c74], edx
    app->getMemory<x86::reg32>(x86::reg32(6696052) /* 0x662c74 */) = cpu.edx;
    // 0044c908  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0044c90a  b801010000             -mov eax, 0x101
    cpu.eax = 257 /*0x101*/;
    // 0044c90f  bb03000000             -mov ebx, 3
    cpu.ebx = 3 /*0x3*/;
    // 0044c914  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044c915  ba1c446600             -mov edx, 0x66441c
    cpu.edx = 6702108 /*0x66441c*/;
    // 0044c91a  a3702c6600             -mov dword ptr [0x662c70], eax
    app->getMemory<x86::reg32>(x86::reg32(6696048) /* 0x662c70 */) = cpu.eax;
    // 0044c91f  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044c921  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044c926  893d902c6600           -mov dword ptr [0x662c90], edi
    app->getMemory<x86::reg32>(x86::reg32(6696080) /* 0x662c90 */) = cpu.edi;
    // 0044c92c  e89f7dffff             -call 0x4446d0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogOpenQuestion(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c931  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044c936  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c937  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c938  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c939  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c93a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c93b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044c93c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044c93d:
    // 0044c93d  bafd000000             -mov edx, 0xfd
    cpu.edx = 253 /*0xfd*/;
    // 0044c942  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044c947  83f801                 +cmp eax, 1
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
    // 0044c94a  7213                   -jb 0x44c95f
    if (cpu.flags.cf)
    {
        goto L_0x0044c95f;
    }
    // 0044c94c  760c                   -jbe 0x44c95a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044c95a;
    }
    // 0044c94e  83f802                 +cmp eax, 2
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
    // 0044c951  750c                   -jne 0x44c95f
    if (!cpu.flags.zf)
    {
        goto L_0x0044c95f;
    }
    // 0044c953  baff000000             -mov edx, 0xff
    cpu.edx = 255 /*0xff*/;
    // 0044c958  eb05                   -jmp 0x44c95f
    goto L_0x0044c95f;
L_0x0044c95a:
    // 0044c95a  bafe000000             -mov edx, 0xfe
    cpu.edx = 254 /*0xfe*/;
L_0x0044c95f:
    // 0044c95f  b8dd010000             -mov eax, 0x1dd
    cpu.eax = 477 /*0x1dd*/;
    // 0044c964  b90b000000             -mov ecx, 0xb
    cpu.ecx = 11 /*0xb*/;
    // 0044c969  e8e24e0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c96e  a320446600             -mov dword ptr [0x664420], eax
    app->getMemory<x86::reg32>(x86::reg32(6702112) /* 0x664420 */) = cpu.eax;
    // 0044c973  b8dc010000             -mov eax, 0x1dc
    cpu.eax = 476 /*0x1dc*/;
    // 0044c978  bb0c000000             -mov ebx, 0xc
    cpu.ebx = 12 /*0xc*/;
    // 0044c97d  e8ce4e0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c982  a324446600             -mov dword ptr [0x664424], eax
    app->getMemory<x86::reg32>(x86::reg32(6702116) /* 0x664424 */) = cpu.eax;
    // 0044c987  b8ef010000             -mov eax, 0x1ef
    cpu.eax = 495 /*0x1ef*/;
    // 0044c98c  be0a000000             -mov esi, 0xa
    cpu.esi = 10 /*0xa*/;
    // 0044c991  e8ba4e0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c996  a328446600             -mov dword ptr [0x664428], eax
    app->getMemory<x86::reg32>(x86::reg32(6702120) /* 0x664428 */) = cpu.eax;
    // 0044c99b  b8c6000000             -mov eax, 0xc6
    cpu.eax = 198 /*0xc6*/;
    // 0044c9a0  bf02000000             -mov edi, 2
    cpu.edi = 2 /*0x2*/;
    // 0044c9a5  e8a64e0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044c9aa  6830c74400             -push 0x44c730
    app->getMemory<x86::reg32>(cpu.esp-4) = 4507440 /*0x44c730*/;
    cpu.esp -= 4;
    // 0044c9af  a32c446600             -mov dword ptr [0x66442c], eax
    app->getMemory<x86::reg32>(x86::reg32(6702124) /* 0x66442c */) = cpu.eax;
    // 0044c9b4  890d882c6600           -mov dword ptr [0x662c88], ecx
    app->getMemory<x86::reg32>(x86::reg32(6696072) /* 0x662c88 */) = cpu.ecx;
    // 0044c9ba  891d8c2c6600           -mov dword ptr [0x662c8c], ebx
    app->getMemory<x86::reg32>(x86::reg32(6696076) /* 0x662c8c */) = cpu.ebx;
    // 0044c9c0  8935902c6600           -mov dword ptr [0x662c90], esi
    app->getMemory<x86::reg32>(x86::reg32(6696080) /* 0x662c90 */) = cpu.esi;
    // 0044c9c6  8915702c6600           -mov dword ptr [0x662c70], edx
    app->getMemory<x86::reg32>(x86::reg32(6696048) /* 0x662c70 */) = cpu.edx;
    // 0044c9cc  8915742c6600           -mov dword ptr [0x662c74], edx
    app->getMemory<x86::reg32>(x86::reg32(6696052) /* 0x662c74 */) = cpu.edx;
    // 0044c9d2  b8fc000000             -mov eax, 0xfc
    cpu.eax = 252 /*0xfc*/;
    // 0044c9d7  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0044c9d9  b920446600             -mov ecx, 0x664420
    cpu.ecx = 6702112 /*0x664420*/;
    // 0044c9de  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 0044c9e3  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0044c9e5  ba1c446600             -mov edx, 0x66441c
    cpu.edx = 6702108 /*0x66441c*/;
    // 0044c9ea  a3782c6600             -mov dword ptr [0x662c78], eax
    app->getMemory<x86::reg32>(x86::reg32(6696056) /* 0x662c78 */) = cpu.eax;
    // 0044c9ef  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044c9f1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044c9f6  893d942c6600           -mov dword ptr [0x662c94], edi
    app->getMemory<x86::reg32>(x86::reg32(6696084) /* 0x662c94 */) = cpu.edi;
    // 0044c9fc  e8cf7cffff             -call 0x4446d0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogOpenQuestion(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ca01  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044ca06  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ca07  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ca08  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ca09  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ca0a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ca0b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ca0c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_44ca10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044ca10  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044ca11  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044ca12  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044ca13  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044ca14  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044ca16  b8ea010000             -mov eax, 0x1ea
    cpu.eax = 490 /*0x1ea*/;
    // 0044ca1b  e8304e0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ca20  a334446600             -mov dword ptr [0x664434], eax
    app->getMemory<x86::reg32>(x86::reg32(6702132) /* 0x664434 */) = cpu.eax;
    // 0044ca25  b8c4000000             -mov eax, 0xc4
    cpu.eax = 196 /*0xc4*/;
    // 0044ca2a  e8214e0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ca2f  a338446600             -mov dword ptr [0x664438], eax
    app->getMemory<x86::reg32>(x86::reg32(6702136) /* 0x664438 */) = cpu.eax;
    // 0044ca34  b8c5000000             -mov eax, 0xc5
    cpu.eax = 197 /*0xc5*/;
    // 0044ca39  e8124e0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ca3e  6870c64400             -push 0x44c670
    app->getMemory<x86::reg32>(cpu.esp-4) = 4507248 /*0x44c670*/;
    cpu.esp -= 4;
    // 0044ca43  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044ca45  b938446600             -mov ecx, 0x664438
    cpu.ecx = 6702136 /*0x664438*/;
    // 0044ca4a  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044ca4c  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 0044ca51  ba34446600             -mov edx, 0x664434
    cpu.edx = 6702132 /*0x664434*/;
    // 0044ca56  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044ca58  a33c446600             -mov dword ptr [0x66443c], eax
    app->getMemory<x86::reg32>(x86::reg32(6702140) /* 0x66443c */) = cpu.eax;
    // 0044ca5d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044ca62  e8697cffff             -call 0x4446d0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogOpenQuestion(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ca67  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044ca6c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ca6d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ca6e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ca6f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ca70  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_44ca80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044ca80  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044ca81  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044ca83  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044ca88  83f801                 +cmp eax, 1
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
    // 0044ca8b  7217                   -jb 0x44caa4
    if (cpu.flags.cf)
    {
        goto L_0x0044caa4;
    }
    // 0044ca8d  7607                   -jbe 0x44ca96
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044ca96;
    }
    // 0044ca8f  83f802                 +cmp eax, 2
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
    // 0044ca92  7409                   -je 0x44ca9d
    if (cpu.flags.zf)
    {
        goto L_0x0044ca9d;
    }
    // 0044ca94  eb0e                   -jmp 0x44caa4
    goto L_0x0044caa4;
L_0x0044ca96:
    // 0044ca96  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044ca9b  eb02                   -jmp 0x44ca9f
    goto L_0x0044ca9f;
L_0x0044ca9d:
    // 0044ca9d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044ca9f:
    // 0044ca9f  e88cafffff             -call 0x447a30
    cpu.esp -= 4;
    sub_447a30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044caa4:
    // 0044caa4  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044caa9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044caaa  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_44cab0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044cab0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044cab1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044cab2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044cab3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044cab4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044cab5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044cab6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044cab8  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0044cabb  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0044cabe  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044cac0  689a99193f             -push 0x3f19999a
    app->getMemory<x86::reg32>(cpu.esp-4) = 1058642330 /*0x3f19999a*/;
    cpu.esp -= 4;
    // 0044cac5  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
    // 0044cac8  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 0044cacb  e8604a0000             -call 0x451530
    cpu.esp -= 4;
    nfs3hp::port::text::dimBackground(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cad0  bab4925300             -mov edx, 0x5392b4
    cpu.edx = 5477044 /*0x5392b4*/;
    // 0044cad5  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044cad8  e8635fffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cadd  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044cadf  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044cae1  7462                   -je 0x44cb45
    if (cpu.flags.zf)
    {
        goto L_0x0044cb45;
    }
    // 0044cae3  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044caea  7406                   -je 0x44caf2
    if (cpu.flags.zf)
    {
        goto L_0x0044caf2;
    }
    // 0044caec  66c740080401           -mov word ptr [eax + 8], 0x104
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */) = 260 /*0x104*/;
L_0x0044caf2:
    // 0044caf2  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044caf9  7f44                   -jg 0x44cb3f
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0044cb3f;
    }
    // 0044cafb  833da0d36f0000         +cmp dword ptr [0x6fd3a0], 0
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
    // 0044cb02  753b                   -jne 0x44cb3f
    if (!cpu.flags.zf)
    {
        goto L_0x0044cb3f;
    }
    // 0044cb04  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044cb09  83f801                 +cmp eax, 1
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
    // 0044cb0c  7516                   -jne 0x44cb24
    if (!cpu.flags.zf)
    {
        goto L_0x0044cb24;
    }
    // 0044cb0e  b890040000             -mov eax, 0x490
    cpu.eax = 1168 /*0x490*/;
    // 0044cb13  e8384d0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cb18  c7426480ca4400         -mov dword ptr [edx + 0x64], 0x44ca80
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(100) /* 0x64 */) = 4508288 /*0x44ca80*/;
    // 0044cb1f  89423c                 -mov dword ptr [edx + 0x3c], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */) = cpu.eax;
    // 0044cb22  eb21                   -jmp 0x44cb45
    goto L_0x0044cb45;
L_0x0044cb24:
    // 0044cb24  83f802                 +cmp eax, 2
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
    // 0044cb27  7516                   -jne 0x44cb3f
    if (!cpu.flags.zf)
    {
        goto L_0x0044cb3f;
    }
    // 0044cb29  b891040000             -mov eax, 0x491
    cpu.eax = 1169 /*0x491*/;
    // 0044cb2e  e81d4d0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cb33  c7426480ca4400         -mov dword ptr [edx + 0x64], 0x44ca80
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(100) /* 0x64 */) = 4508288 /*0x44ca80*/;
    // 0044cb3a  89423c                 -mov dword ptr [edx + 0x3c], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */) = cpu.eax;
    // 0044cb3d  eb06                   -jmp 0x44cb45
    goto L_0x0044cb45;
L_0x0044cb3f:
    // 0044cb3f  66814a040110           -or word ptr [edx + 4], 0x1001
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(4) /* 0x4 */) |= x86::reg16(x86::sreg16(4097 /*0x1001*/));
L_0x0044cb45:
    // 0044cb45  bac0925300             -mov edx, 0x5392c0
    cpu.edx = 5477056 /*0x5392c0*/;
    // 0044cb4a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044cb4d  e8ee5effff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cb52  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044cb54  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044cb56  0f846c020000           -je 0x44cdc8
    if (cpu.flags.zf)
    {
        goto L_0x0044cdc8;
    }
    // 0044cb5c  8b1db8d36f00           -mov ebx, dword ptr [0x6fd3b8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044cb62  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0044cb64  0f8585000000           -jne 0x44cbef
    if (!cpu.flags.zf)
    {
        goto L_0x0044cbef;
    }
    // 0044cb6a  b8d5000000             -mov eax, 0xd5
    cpu.eax = 213 /*0xd5*/;
    // 0044cb6f  bf6c925500             -mov edi, 0x55926c
    cpu.edi = 5608044 /*0x55926c*/;
    // 0044cb74  e8d74c0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cb79  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044cb7b  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0044cb7c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cb7c  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044cb7e  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044cb80  3c00                   +cmp al, 0
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
    // 0044cb82  7410                   -je 0x44cb94
    if (cpu.flags.zf)
    {
        goto L_0x0044cb94;
    }
    // 0044cb84  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044cb87  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cb8a  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044cb8d  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cb90  3c00                   +cmp al, 0
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
    // 0044cb92  75e8                   -jne 0x44cb7c
    if (!cpu.flags.zf)
    {
        goto L_0x0044cb7c;
    }
L_0x0044cb94:
    // 0044cb94  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cb95  bec8925300             -mov esi, 0x5392c8
    cpu.esi = 5477064 /*0x5392c8*/;
    // 0044cb9a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044cb9b  2bc9                   +sub ecx, ecx
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
    // 0044cb9d  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044cb9e  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0044cba0  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0044cba2  4f                     -dec edi
    (cpu.edi)--;
L_0x0044cba3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cba3  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044cba5  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044cba7  3c00                   +cmp al, 0
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
    // 0044cba9  7410                   -je 0x44cbbb
    if (cpu.flags.zf)
    {
        goto L_0x0044cbbb;
    }
    // 0044cbab  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044cbae  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cbb1  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044cbb4  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cbb7  3c00                   +cmp al, 0
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
    // 0044cbb9  75e8                   -jne 0x44cba3
    if (!cpu.flags.zf)
    {
        goto L_0x0044cba3;
    }
L_0x0044cbbb:
    // 0044cbbb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cbbc  b8ba010000             -mov eax, 0x1ba
    cpu.eax = 442 /*0x1ba*/;
    // 0044cbc1  e88a4c0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cbc6  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044cbc8  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044cbc9  2bc9                   +sub ecx, ecx
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
    // 0044cbcb  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044cbcc  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0044cbce  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0044cbd0  4f                     -dec edi
    (cpu.edi)--;
L_0x0044cbd1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cbd1  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044cbd3  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044cbd5  3c00                   +cmp al, 0
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
    // 0044cbd7  7410                   -je 0x44cbe9
    if (cpu.flags.zf)
    {
        goto L_0x0044cbe9;
    }
    // 0044cbd9  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044cbdc  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cbdf  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044cbe2  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cbe5  3c00                   +cmp al, 0
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
    // 0044cbe7  75e8                   -jne 0x44cbd1
    if (!cpu.flags.zf)
    {
        goto L_0x0044cbd1;
    }
L_0x0044cbe9:
    // 0044cbe9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cbea  e9d2010000             -jmp 0x44cdc1
    goto L_0x0044cdc1;
L_0x0044cbef:
    // 0044cbef  83fb01                 +cmp ebx, 1
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
    // 0044cbf2  0f8585000000           -jne 0x44cc7d
    if (!cpu.flags.zf)
    {
        goto L_0x0044cc7d;
    }
    // 0044cbf8  b8d6000000             -mov eax, 0xd6
    cpu.eax = 214 /*0xd6*/;
    // 0044cbfd  bf6c925500             -mov edi, 0x55926c
    cpu.edi = 5608044 /*0x55926c*/;
    // 0044cc02  e8494c0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cc07  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044cc09  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0044cc0a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cc0a  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044cc0c  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044cc0e  3c00                   +cmp al, 0
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
    // 0044cc10  7410                   -je 0x44cc22
    if (cpu.flags.zf)
    {
        goto L_0x0044cc22;
    }
    // 0044cc12  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044cc15  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cc18  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044cc1b  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cc1e  3c00                   +cmp al, 0
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
    // 0044cc20  75e8                   -jne 0x44cc0a
    if (!cpu.flags.zf)
    {
        goto L_0x0044cc0a;
    }
L_0x0044cc22:
    // 0044cc22  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cc23  bec8925300             -mov esi, 0x5392c8
    cpu.esi = 5477064 /*0x5392c8*/;
    // 0044cc28  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044cc29  2bc9                   +sub ecx, ecx
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
    // 0044cc2b  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044cc2c  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0044cc2e  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0044cc30  4f                     -dec edi
    (cpu.edi)--;
L_0x0044cc31:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cc31  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044cc33  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044cc35  3c00                   +cmp al, 0
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
    // 0044cc37  7410                   -je 0x44cc49
    if (cpu.flags.zf)
    {
        goto L_0x0044cc49;
    }
    // 0044cc39  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044cc3c  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cc3f  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044cc42  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cc45  3c00                   +cmp al, 0
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
    // 0044cc47  75e8                   -jne 0x44cc31
    if (!cpu.flags.zf)
    {
        goto L_0x0044cc31;
    }
L_0x0044cc49:
    // 0044cc49  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cc4a  b8ba010000             -mov eax, 0x1ba
    cpu.eax = 442 /*0x1ba*/;
    // 0044cc4f  e8fc4b0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cc54  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044cc56  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044cc57  2bc9                   +sub ecx, ecx
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
    // 0044cc59  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044cc5a  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0044cc5c  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0044cc5e  4f                     -dec edi
    (cpu.edi)--;
L_0x0044cc5f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cc5f  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044cc61  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044cc63  3c00                   +cmp al, 0
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
    // 0044cc65  7410                   -je 0x44cc77
    if (cpu.flags.zf)
    {
        goto L_0x0044cc77;
    }
    // 0044cc67  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044cc6a  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cc6d  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044cc70  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cc73  3c00                   +cmp al, 0
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
    // 0044cc75  75e8                   -jne 0x44cc5f
    if (!cpu.flags.zf)
    {
        goto L_0x0044cc5f;
    }
L_0x0044cc77:
    // 0044cc77  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cc78  e944010000             -jmp 0x44cdc1
    goto L_0x0044cdc1;
L_0x0044cc7d:
    // 0044cc7d  83fb02                 +cmp ebx, 2
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
    // 0044cc80  0f8585000000           -jne 0x44cd0b
    if (!cpu.flags.zf)
    {
        goto L_0x0044cd0b;
    }
    // 0044cc86  b8d7000000             -mov eax, 0xd7
    cpu.eax = 215 /*0xd7*/;
    // 0044cc8b  bf6c925500             -mov edi, 0x55926c
    cpu.edi = 5608044 /*0x55926c*/;
    // 0044cc90  e8bb4b0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cc95  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044cc97  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0044cc98:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cc98  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044cc9a  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044cc9c  3c00                   +cmp al, 0
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
    // 0044cc9e  7410                   -je 0x44ccb0
    if (cpu.flags.zf)
    {
        goto L_0x0044ccb0;
    }
    // 0044cca0  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044cca3  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cca6  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044cca9  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044ccac  3c00                   +cmp al, 0
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
    // 0044ccae  75e8                   -jne 0x44cc98
    if (!cpu.flags.zf)
    {
        goto L_0x0044cc98;
    }
L_0x0044ccb0:
    // 0044ccb0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ccb1  bec8925300             -mov esi, 0x5392c8
    cpu.esi = 5477064 /*0x5392c8*/;
    // 0044ccb6  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044ccb7  2bc9                   +sub ecx, ecx
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
    // 0044ccb9  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044ccba  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0044ccbc  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0044ccbe  4f                     -dec edi
    (cpu.edi)--;
L_0x0044ccbf:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044ccbf  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044ccc1  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044ccc3  3c00                   +cmp al, 0
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
    // 0044ccc5  7410                   -je 0x44ccd7
    if (cpu.flags.zf)
    {
        goto L_0x0044ccd7;
    }
    // 0044ccc7  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044ccca  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cccd  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044ccd0  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044ccd3  3c00                   +cmp al, 0
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
    // 0044ccd5  75e8                   -jne 0x44ccbf
    if (!cpu.flags.zf)
    {
        goto L_0x0044ccbf;
    }
L_0x0044ccd7:
    // 0044ccd7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ccd8  b8ba010000             -mov eax, 0x1ba
    cpu.eax = 442 /*0x1ba*/;
    // 0044ccdd  e86e4b0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cce2  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044cce4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044cce5  2bc9                   +sub ecx, ecx
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
    // 0044cce7  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044cce8  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0044ccea  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0044ccec  4f                     -dec edi
    (cpu.edi)--;
L_0x0044cced:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cced  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044ccef  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044ccf1  3c00                   +cmp al, 0
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
    // 0044ccf3  7410                   -je 0x44cd05
    if (cpu.flags.zf)
    {
        goto L_0x0044cd05;
    }
    // 0044ccf5  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044ccf8  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044ccfb  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044ccfe  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cd01  3c00                   +cmp al, 0
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
    // 0044cd03  75e8                   -jne 0x44cced
    if (!cpu.flags.zf)
    {
        goto L_0x0044cced;
    }
L_0x0044cd05:
    // 0044cd05  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cd06  e9b6000000             -jmp 0x44cdc1
    goto L_0x0044cdc1;
L_0x0044cd0b:
    // 0044cd0b  83fb03                 +cmp ebx, 3
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
    // 0044cd0e  0f8582000000           -jne 0x44cd96
    if (!cpu.flags.zf)
    {
        goto L_0x0044cd96;
    }
    // 0044cd14  b8dc000000             -mov eax, 0xdc
    cpu.eax = 220 /*0xdc*/;
    // 0044cd19  bf6c925500             -mov edi, 0x55926c
    cpu.edi = 5608044 /*0x55926c*/;
    // 0044cd1e  e82d4b0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cd23  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044cd25  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0044cd26:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cd26  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044cd28  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044cd2a  3c00                   +cmp al, 0
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
    // 0044cd2c  7410                   -je 0x44cd3e
    if (cpu.flags.zf)
    {
        goto L_0x0044cd3e;
    }
    // 0044cd2e  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044cd31  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cd34  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044cd37  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cd3a  3c00                   +cmp al, 0
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
    // 0044cd3c  75e8                   -jne 0x44cd26
    if (!cpu.flags.zf)
    {
        goto L_0x0044cd26;
    }
L_0x0044cd3e:
    // 0044cd3e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cd3f  bec8925300             -mov esi, 0x5392c8
    cpu.esi = 5477064 /*0x5392c8*/;
    // 0044cd44  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044cd45  2bc9                   +sub ecx, ecx
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
    // 0044cd47  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044cd48  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0044cd4a  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0044cd4c  4f                     -dec edi
    (cpu.edi)--;
L_0x0044cd4d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cd4d  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044cd4f  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044cd51  3c00                   +cmp al, 0
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
    // 0044cd53  7410                   -je 0x44cd65
    if (cpu.flags.zf)
    {
        goto L_0x0044cd65;
    }
    // 0044cd55  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044cd58  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cd5b  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044cd5e  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cd61  3c00                   +cmp al, 0
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
    // 0044cd63  75e8                   -jne 0x44cd4d
    if (!cpu.flags.zf)
    {
        goto L_0x0044cd4d;
    }
L_0x0044cd65:
    // 0044cd65  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cd66  b8ba010000             -mov eax, 0x1ba
    cpu.eax = 442 /*0x1ba*/;
    // 0044cd6b  e8e04a0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cd70  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044cd72  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044cd73  2bc9                   +sub ecx, ecx
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
    // 0044cd75  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044cd76  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0044cd78  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0044cd7a  4f                     -dec edi
    (cpu.edi)--;
L_0x0044cd7b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cd7b  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044cd7d  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044cd7f  3c00                   +cmp al, 0
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
    // 0044cd81  7410                   -je 0x44cd93
    if (cpu.flags.zf)
    {
        goto L_0x0044cd93;
    }
    // 0044cd83  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044cd86  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cd89  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044cd8c  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cd8f  3c00                   +cmp al, 0
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
    // 0044cd91  75e8                   -jne 0x44cd7b
    if (!cpu.flags.zf)
    {
        goto L_0x0044cd7b;
    }
L_0x0044cd93:
    // 0044cd93  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cd94  eb2b                   -jmp 0x44cdc1
    goto L_0x0044cdc1;
L_0x0044cd96:
    // 0044cd96  b804010000             -mov eax, 0x104
    cpu.eax = 260 /*0x104*/;
    // 0044cd9b  bf6c925500             -mov edi, 0x55926c
    cpu.edi = 5608044 /*0x55926c*/;
    // 0044cda0  e8ab4a0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cda5  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044cda7  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0044cda8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044cda8  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0044cdaa  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0044cdac  3c00                   +cmp al, 0
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
    // 0044cdae  7410                   -je 0x44cdc0
    if (cpu.flags.zf)
    {
        goto L_0x0044cdc0;
    }
    // 0044cdb0  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044cdb3  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cdb6  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0044cdb9  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0044cdbc  3c00                   +cmp al, 0
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
    // 0044cdbe  75e8                   -jne 0x44cda8
    if (!cpu.flags.zf)
    {
        goto L_0x0044cda8;
    }
L_0x0044cdc0:
    // 0044cdc0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0044cdc1:
    // 0044cdc1  c7423c6c925500         -mov dword ptr [edx + 0x3c], 0x55926c
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */) = 5608044 /*0x55926c*/;
L_0x0044cdc8:
    // 0044cdc8  f6052eeb550020         +test byte ptr [0x55eb2e], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */) & 32 /*0x20*/));
    // 0044cdcf  7416                   -je 0x44cde7
    if (cpu.flags.zf)
    {
        goto L_0x0044cde7;
    }
    // 0044cdd1  bacc925300             -mov edx, 0x5392cc
    cpu.edx = 5477068 /*0x5392cc*/;
    // 0044cdd6  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044cdd9  e8625cffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cdde  c74040dc925300         -mov dword ptr [eax + 0x40], 0x5392dc
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 5477084 /*0x5392dc*/;
    // 0044cde5  eb18                   -jmp 0x44cdff
    goto L_0x0044cdff;
L_0x0044cde7:
    // 0044cde7  bacc925300             -mov edx, 0x5392cc
    cpu.edx = 5477068 /*0x5392cc*/;
    // 0044cdec  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044cdef  e84c5cffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cdf4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044cdf6  7407                   -je 0x44cdff
    if (cpu.flags.zf)
    {
        goto L_0x0044cdff;
    }
    // 0044cdf8  c74064b0c74400         -mov dword ptr [eax + 0x64], 0x44c7b0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4507568 /*0x44c7b0*/;
L_0x0044cdff:
    // 0044cdff  bae0925300             -mov edx, 0x5392e0
    cpu.edx = 5477088 /*0x5392e0*/;
    // 0044ce04  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044ce07  e8345cffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ce0c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044ce0e  7404                   -je 0x44ce14
    if (cpu.flags.zf)
    {
        goto L_0x0044ce14;
    }
    // 0044ce10  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044ce14:
    // 0044ce14  baf4925300             -mov edx, 0x5392f4
    cpu.edx = 5477108 /*0x5392f4*/;
    // 0044ce19  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044ce1c  e81f5cffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ce21  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044ce23  7404                   -je 0x44ce29
    if (cpu.flags.zf)
    {
        goto L_0x0044ce29;
    }
    // 0044ce25  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044ce29:
    // 0044ce29  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044ce30  7541                   -jne 0x44ce73
    if (!cpu.flags.zf)
    {
        goto L_0x0044ce73;
    }
    // 0044ce32  ba00935300             -mov edx, 0x539300
    cpu.edx = 5477120 /*0x539300*/;
    // 0044ce37  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044ce3a  e8015cffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ce3f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044ce41  7404                   -je 0x44ce47
    if (cpu.flags.zf)
    {
        goto L_0x0044ce47;
    }
    // 0044ce43  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044ce47:
    // 0044ce47  ba04935300             -mov edx, 0x539304
    cpu.edx = 5477124 /*0x539304*/;
    // 0044ce4c  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044ce4f  e8ec5bffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ce54  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044ce56  7404                   -je 0x44ce5c
    if (cpu.flags.zf)
    {
        goto L_0x0044ce5c;
    }
    // 0044ce58  806005ef               -and byte ptr [eax + 5], 0xef
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) &= x86::reg8(x86::sreg8(239 /*0xef*/));
L_0x0044ce5c:
    // 0044ce5c  ba0c935300             -mov edx, 0x53930c
    cpu.edx = 5477132 /*0x53930c*/;
    // 0044ce61  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044ce64  e8d75bffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ce69  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044ce6b  7445                   -je 0x44ceb2
    if (cpu.flags.zf)
    {
        goto L_0x0044ceb2;
    }
    // 0044ce6d  806005ef               +and byte ptr [eax + 5], 0xef
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) &= x86::reg8(x86::sreg8(239 /*0xef*/))));
    // 0044ce71  eb3f                   -jmp 0x44ceb2
    goto L_0x0044ceb2;
L_0x0044ce73:
    // 0044ce73  ba00935300             -mov edx, 0x539300
    cpu.edx = 5477120 /*0x539300*/;
    // 0044ce78  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044ce7b  e8c05bffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ce80  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044ce82  7404                   -je 0x44ce88
    if (cpu.flags.zf)
    {
        goto L_0x0044ce88;
    }
    // 0044ce84  806005ef               -and byte ptr [eax + 5], 0xef
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) &= x86::reg8(x86::sreg8(239 /*0xef*/));
L_0x0044ce88:
    // 0044ce88  ba04935300             -mov edx, 0x539304
    cpu.edx = 5477124 /*0x539304*/;
    // 0044ce8d  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044ce90  e8ab5bffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ce95  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044ce97  7404                   -je 0x44ce9d
    if (cpu.flags.zf)
    {
        goto L_0x0044ce9d;
    }
    // 0044ce99  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044ce9d:
    // 0044ce9d  ba0c935300             -mov edx, 0x53930c
    cpu.edx = 5477132 /*0x53930c*/;
    // 0044cea2  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044cea5  e8965bffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ceaa  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044ceac  7404                   -je 0x44ceb2
    if (cpu.flags.zf)
    {
        goto L_0x0044ceb2;
    }
    // 0044ceae  80480510               -or byte ptr [eax + 5], 0x10
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0044ceb2:
    // 0044ceb2  ba14935300             -mov edx, 0x539314
    cpu.edx = 5477140 /*0x539314*/;
    // 0044ceb7  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044ceba  e8815bffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cebf  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044cec1  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0044cec3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044cec5  7433                   -je 0x44cefa
    if (cpu.flags.zf)
    {
        goto L_0x0044cefa;
    }
    // 0044cec7  8d45f4                 -lea eax, [ebp - 0xc]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0044ceca  8d4df8                 -lea ecx, [ebp - 8]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0044cecd  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0044ced0  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0044ced1  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0044ced4  b8c0da7c00             -mov eax, 0x7cdac0
    cpu.eax = 8182464 /*0x7cdac0*/;
    // 0044ced9  e8421c0700             -call 0x4beb20
    cpu.esp -= 4;
    sub_4beb20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cede  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044cee5  750f                   -jne 0x44cef6
    if (!cpu.flags.zf)
    {
        goto L_0x0044cef6;
    }
    // 0044cee7  817df880020000         +cmp dword ptr [ebp - 8], 0x280
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(640 /*0x280*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ceee  7d06                   -jge 0x44cef6
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0044cef6;
    }
    // 0044cef0  804e0401               +or byte ptr [esi + 4], 1
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.esi + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 0044cef4  eb04                   -jmp 0x44cefa
    goto L_0x0044cefa;
L_0x0044cef6:
    // 0044cef6  806704fe               -and byte ptr [edi + 4], 0xfe
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(4) /* 0x4 */) &= x86::reg8(x86::sreg8(254 /*0xfe*/));
L_0x0044cefa:
    // 0044cefa  ba20935300             -mov edx, 0x539320
    cpu.edx = 5477152 /*0x539320*/;
    // 0044ceff  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044cf02  e8395bffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cf07  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044cf09  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044cf0b  743a                   -je 0x44cf47
    if (cpu.flags.zf)
    {
        goto L_0x0044cf47;
    }
    // 0044cf0d  8b35b0d36f00           -mov esi, dword ptr [0x6fd3b0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0044cf13  c7406410ca4400         -mov dword ptr [eax + 0x64], 0x44ca10
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4508176 /*0x44ca10*/;
    // 0044cf1a  83fe01                 +cmp esi, 1
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
    // 0044cf1d  7c19                   -jl 0x44cf38
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0044cf38;
    }
    // 0044cf1f  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0044cf27  7409                   -je 0x44cf32
    if (cpu.flags.zf)
    {
        goto L_0x0044cf32;
    }
    // 0044cf29  833de4227a0002         +cmp dword ptr [0x7a22e4], 2
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
    // 0044cf30  7506                   -jne 0x44cf38
    if (!cpu.flags.zf)
    {
        goto L_0x0044cf38;
    }
L_0x0044cf32:
    // 0044cf32  668148040110           -or word ptr [eax + 4], 0x1001
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg16(x86::sreg16(4097 /*0x1001*/));
L_0x0044cf38:
    // 0044cf38  833da0d36f0002         +cmp dword ptr [0x6fd3a0], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044cf3f  7c06                   -jl 0x44cf47
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0044cf47;
    }
    // 0044cf41  66816004feef           -and word ptr [eax + 4], 0xeffe
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */) &= x86::reg16(x86::sreg16(61438 /*0xeffe*/));
L_0x0044cf47:
    // 0044cf47  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044cf4a  e881f3ffff             -call 0x44c2d0
    cpu.esp -= 4;
    sub_44c2d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cf4f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044cf51  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0044cf53  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cf54  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cf55  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cf56  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cf57  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cf58  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cf59  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_44cf60(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044cf60  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044cf61  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044cf63  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044cf65  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cf66  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_44cf70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044cf70  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044cf71  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044cf72  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044cf73  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044cf75  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044cf77  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 0044cf7c  e8dfd6ffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cf81  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044cf83  740f                   -je 0x44cf94
    if (cpu.flags.zf)
    {
        goto L_0x0044cf94;
    }
    // 0044cf85  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044cf87  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044cf89  e822fa0100             -call 0x46c9b0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cf8e  d91d38bc6f00           -fstp dword ptr [0x6fbc38]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = cpu.fpu.st(0);
    app->getMemory<float>(x86::reg32(7322680) /* 0x6fbc38 */) = float(fpu1);
    cpu.fpu.count -= 1;
    }
L_0x0044cf94:
    // 0044cf94  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cf95  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cf96  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cf97  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_44cfa0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044cfa0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044cfa1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044cfa2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044cfa3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044cfa5  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044cfa7  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 0044cfac  e8afd6ffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cfb1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044cfb3  7419                   -je 0x44cfce
    if (cpu.flags.zf)
    {
        goto L_0x0044cfce;
    }
    // 0044cfb5  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044cfb7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044cfb9  e8f2f90100             -call 0x46c9b0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cfbe  d80d2c935300           -fmul dword ptr [0x53932c]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = cpu.fpu.st(0);
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5477164) /* 0x53932c */)));
    // 0044cfc4  d9e8                   -fld1 
    x86::Float fpu3 = 1.0;
    // 0044cfc6  dec1                   -faddp st(1)
    x86::Float fpu4 = cpu.fpu.add(fpu2, fpu3);
    // 0044cfc8  d91d04be6f00           -fstp dword ptr [0x6fbe04]
    app->getMemory<float>(x86::reg32(7323140) /* 0x6fbe04 */) = float(fpu4);
    cpu.fpu.count -= 1;
    }
L_0x0044cfce:
    // 0044cfce  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cfcf  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cfd0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044cfd1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_44cfe0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044cfe0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044cfe1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044cfe2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044cfe3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044cfe5  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044cfe7  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 0044cfec  e86fd6ffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cff1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044cff3  7419                   -je 0x44d00e
    if (cpu.flags.zf)
    {
        goto L_0x0044d00e;
    }
    // 0044cff5  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044cff7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044cff9  e8b2f90100             -call 0x46c9b0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044cffe  d80d30935300           -fmul dword ptr [0x539330]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = cpu.fpu.st(0);
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5477168) /* 0x539330 */)));
    // 0044d004  d9e8                   -fld1 
    x86::Float fpu3 = 1.0;
    // 0044d006  dec1                   -faddp st(1)
    x86::Float fpu4 = cpu.fpu.add(fpu2, fpu3);
    // 0044d008  d91d60bc6f00           -fstp dword ptr [0x6fbc60]
    app->getMemory<float>(x86::reg32(7322720) /* 0x6fbc60 */) = float(fpu4);
    cpu.fpu.count -= 1;
    }
L_0x0044d00e:
    // 0044d00e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d00f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d010  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d011  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_44d020(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d020  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d021  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d022  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d023  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d025  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044d027  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 0044d02c  e82fd6ffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d031  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d033  7419                   -je 0x44d04e
    if (cpu.flags.zf)
    {
        goto L_0x0044d04e;
    }
    // 0044d035  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d037  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044d039  e872f90100             -call 0x46c9b0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d03e  d80d34935300           -fmul dword ptr [0x539334]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = cpu.fpu.st(0);
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5477172) /* 0x539334 */)));
    // 0044d044  d9e8                   -fld1 
    x86::Float fpu3 = 1.0;
    // 0044d046  dec1                   -faddp st(1)
    x86::Float fpu4 = cpu.fpu.add(fpu2, fpu3);
    // 0044d048  d91d4cc16f00           -fstp dword ptr [0x6fc14c]
    app->getMemory<float>(x86::reg32(7323980) /* 0x6fc14c */) = float(fpu4);
    cpu.fpu.count -= 1;
    }
L_0x0044d04e:
    // 0044d04e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d04f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d050  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d051  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_44d060(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d060  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d061  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d062  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d063  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d065  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044d067  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 0044d06c  e8efd5ffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d071  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d073  7419                   -je 0x44d08e
    if (cpu.flags.zf)
    {
        goto L_0x0044d08e;
    }
    // 0044d075  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d077  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044d079  e832f90100             -call 0x46c9b0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d07e  d80d38935300           -fmul dword ptr [0x539338]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = cpu.fpu.st(0);
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5477176) /* 0x539338 */)));
    // 0044d084  d9e8                   -fld1 
    x86::Float fpu3 = 1.0;
    // 0044d086  dec1                   -faddp st(1)
    x86::Float fpu4 = cpu.fpu.add(fpu2, fpu3);
    // 0044d088  d91da8bf6f00           -fstp dword ptr [0x6fbfa8]
    app->getMemory<float>(x86::reg32(7323560) /* 0x6fbfa8 */) = float(fpu4);
    cpu.fpu.count -= 1;
    }
L_0x0044d08e:
    // 0044d08e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d08f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d090  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d091  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_44d0a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d0a0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d0a1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d0a2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d0a3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d0a5  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044d0a7  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 0044d0ac  e8afd5ffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d0b1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d0b3  7419                   -je 0x44d0ce
    if (cpu.flags.zf)
    {
        goto L_0x0044d0ce;
    }
    // 0044d0b5  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d0b7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044d0b9  e8f2f80100             -call 0x46c9b0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d0be  d80d3c935300           -fmul dword ptr [0x53933c]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = cpu.fpu.st(0);
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5477180) /* 0x53933c */)));
    // 0044d0c4  d9e8                   -fld1 
    x86::Float fpu3 = 1.0;
    // 0044d0c6  dec1                   -faddp st(1)
    x86::Float fpu4 = cpu.fpu.add(fpu2, fpu3);
    // 0044d0c8  d91d94c46f00           -fstp dword ptr [0x6fc494]
    app->getMemory<float>(x86::reg32(7324820) /* 0x6fc494 */) = float(fpu4);
    cpu.fpu.count -= 1;
    }
L_0x0044d0ce:
    // 0044d0ce  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d0cf  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d0d0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d0d1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_44d0e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d0e0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d0e1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d0e2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d0e3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d0e5  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044d0e7  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 0044d0ec  e86fd5ffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d0f1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d0f3  7419                   -je 0x44d10e
    if (cpu.flags.zf)
    {
        goto L_0x0044d10e;
    }
    // 0044d0f5  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d0f7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044d0f9  e8b2f80100             -call 0x46c9b0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d0fe  d80d40935300           -fmul dword ptr [0x539340]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = cpu.fpu.st(0);
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5477184) /* 0x539340 */)));
    // 0044d104  d9e8                   -fld1 
    x86::Float fpu3 = 1.0;
    // 0044d106  dec1                   -faddp st(1)
    x86::Float fpu4 = cpu.fpu.add(fpu2, fpu3);
    // 0044d108  d91df0c26f00           -fstp dword ptr [0x6fc2f0]
    app->getMemory<float>(x86::reg32(7324400) /* 0x6fc2f0 */) = float(fpu4);
    cpu.fpu.count -= 1;
    }
L_0x0044d10e:
    // 0044d10e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d10f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d110  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d111  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_44d120(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d120  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d121  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d122  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d123  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d125  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044d127  689a99193f             -push 0x3f19999a
    app->getMemory<x86::reg32>(cpu.esp-4) = 1058642330 /*0x3f19999a*/;
    cpu.esp -= 4;
    // 0044d12c  e8ff430000             -call 0x451530
    cpu.esp -= 4;
    nfs3hp::port::text::dimBackground(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d131  ba44935300             -mov edx, 0x539344
    cpu.edx = 5477188 /*0x539344*/;
    // 0044d136  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d138  e80359ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d13d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d13f  740d                   -je 0x44d14e
    if (cpu.flags.zf)
    {
        goto L_0x0044d14e;
    }
    // 0044d141  833d54bb6f0000         +cmp dword ptr [0x6fbb54], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7322452) /* 0x6fbb54 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044d148  7404                   -je 0x44d14e
    if (cpu.flags.zf)
    {
        goto L_0x0044d14e;
    }
    // 0044d14a  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044d14e:
    // 0044d14e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d150  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d151  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d152  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d153  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_44d160(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d160  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d161  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d163  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d165  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d166  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_44d170(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d170  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d171  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d172  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d174  6683fb0d               +cmp bx, 0xd
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(13 /*0xd*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0044d178  7508                   -jne 0x44d182
    if (!cpu.flags.zf)
    {
        goto L_0x0044d182;
    }
    // 0044d17a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044d17c  890d0c446600           -mov dword ptr [0x66440c], ecx
    app->getMemory<x86::reg32>(x86::reg32(6702092) /* 0x66440c */) = cpu.ecx;
L_0x0044d182:
    // 0044d182  0fbfdb                 -movsx ebx, bx
    cpu.ebx = x86::reg32(static_cast<x86::sreg16>(cpu.bx));
    // 0044d185  e8e6400100             -call 0x461270
    cpu.esp -= 4;
    nfs3hp::port::buttons::buttonKey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d18a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d18b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d18c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_44d190(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d190  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044d191  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d192  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d193  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d194  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d196  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044d198  689a99193f             -push 0x3f19999a
    app->getMemory<x86::reg32>(cpu.esp-4) = 1058642330 /*0x3f19999a*/;
    cpu.esp -= 4;
    // 0044d19d  e88e430000             -call 0x451530
    cpu.esp -= 4;
    nfs3hp::port::text::dimBackground(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d1a2  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044d1a7  bb68000000             -mov ebx, 0x68
    cpu.ebx = 104 /*0x68*/;
    // 0044d1ac  a1b0d36f00             -mov eax, dword ptr [0x6fd3b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0044d1b1  89150c446600           -mov dword ptr [0x66440c], edx
    app->getMemory<x86::reg32>(x86::reg32(6702092) /* 0x66440c */) = cpu.edx;
    // 0044d1b7  bad03c5f00             -mov edx, 0x5f3cd0
    cpu.edx = 6241488 /*0x5f3cd0*/;
    // 0044d1bc  e80f330400             -call 0x4904d0
    cpu.esp -= 4;
    nfs3hp::controls::controlsFor(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d1c1  e82ad30900             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d1c6  ba4c935300             -mov edx, 0x53934c
    cpu.edx = 5477196 /*0x53934c*/;
    // 0044d1cb  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d1cd  e86e58ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d1d2  f60570c96f0008         +test byte ptr [0x6fc970], 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(7326064) /* 0x6fc970 */) & 8 /*0x8*/));
    // 0044d1d9  7508                   -jne 0x44d1e3
    if (!cpu.flags.zf)
    {
        goto L_0x0044d1e3;
    }
    // 0044d1db  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d1dd  7404                   -je 0x44d1e3
    if (cpu.flags.zf)
    {
        goto L_0x0044d1e3;
    }
    // 0044d1df  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044d1e3:
    // 0044d1e3  ba5c935300             -mov edx, 0x53935c
    cpu.edx = 5477212 /*0x53935c*/;
    // 0044d1e8  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d1ea  e85158ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d1ef  833d70c96f0000         +cmp dword ptr [0x6fc970], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7326064) /* 0x6fc970 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044d1f6  7508                   -jne 0x44d200
    if (!cpu.flags.zf)
    {
        goto L_0x0044d200;
    }
    // 0044d1f8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d1fa  7404                   -je 0x44d200
    if (cpu.flags.zf)
    {
        goto L_0x0044d200;
    }
    // 0044d1fc  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044d200:
    // 0044d200  ba6c935300             -mov edx, 0x53936c
    cpu.edx = 5477228 /*0x53936c*/;
    // 0044d205  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d207  e83458ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d20c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d20e  7407                   -je 0x44d217
    if (cpu.flags.zf)
    {
        goto L_0x0044d217;
    }
    // 0044d210  c7403070d14400         -mov dword ptr [eax + 0x30], 0x44d170
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */) = 4510064 /*0x44d170*/;
L_0x0044d217:
    // 0044d217  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d219  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d21a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d21b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d21c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d21d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_44d220(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d220  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044d221  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d222  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d223  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d224  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d226  a1b0d36f00             -mov eax, dword ptr [0x6fd3b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0044d22b  e8a0320400             -call 0x4904d0
    cpu.esp -= 4;
    nfs3hp::controls::controlsFor(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d230  833d0c44660000         +cmp dword ptr [0x66440c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6702092) /* 0x66440c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044d237  7523                   -jne 0x44d25c
    if (!cpu.flags.zf)
    {
        goto L_0x0044d25c;
    }
    // 0044d239  bb68000000             -mov ebx, 0x68
    cpu.ebx = 104 /*0x68*/;
    // 0044d23e  8b0db0d36f00           -mov ecx, dword ptr [0x6fd3b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0044d244  83f901                 +cmp ecx, 1
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
    // 0044d247  7405                   -je 0x44d24e
    if (cpu.flags.zf)
    {
        goto L_0x0044d24e;
    }
    // 0044d249  bb34000000             -mov ebx, 0x34
    cpu.ebx = 52 /*0x34*/;
L_0x0044d24e:
    // 0044d24e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044d250  b8d03c5f00             -mov eax, 0x5f3cd0
    cpu.eax = 6241488 /*0x5f3cd0*/;
    // 0044d255  e896d20900             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d25a  eb05                   -jmp 0x44d261
    goto L_0x0044d261;
L_0x0044d25c:
    // 0044d25c  e88f3e0200             -call 0x4710f0
    cpu.esp -= 4;
    nfs3hp::controls::forcePlayerCar(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044d261:
    // 0044d261  e88a1f0400             -call 0x48f1f0
    cpu.esp -= 4;
    nfs3hp::controls::bindPlayersControls(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d266  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d268  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d269  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d26a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d26b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d26c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_44d270(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d270  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d271  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d273  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d275  e85659fdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d27a  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044d281  7407                   -je 0x44d28a
    if (cpu.flags.zf)
    {
        goto L_0x0044d28a;
    }
    // 0044d283  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044d288  eb02                   -jmp 0x44d28c
    goto L_0x0044d28c;
L_0x0044d28a:
    // 0044d28a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044d28c:
    // 0044d28c  e84f130700             -call 0x4be5e0
    cpu.esp -= 4;
    sub_4be5e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d291  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044d296  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d297  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_44d2a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d2a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044d2a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d2a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d2a3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044d2a4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044d2a5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d2a6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d2a8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0044d2aa  b96c170000             -mov ecx, 0x176c
    cpu.ecx = 5996 /*0x176c*/;
    // 0044d2af  be4cbb6f00             -mov esi, 0x6fbb4c
    cpu.esi = 7322444 /*0x6fbb4c*/;
    // 0044d2b4  bfa02c6600             -mov edi, 0x662ca0
    cpu.edi = 6696096 /*0x662ca0*/;
    // 0044d2b9  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044d2ba  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d2bc  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044d2bf  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044d2c1  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044d2c3  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044d2c6  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044d2c8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d2c9  689a99193f             -push 0x3f19999a
    app->getMemory<x86::reg32>(cpu.esp-4) = 1058642330 /*0x3f19999a*/;
    cpu.esp -= 4;
    // 0044d2ce  e85d420000             -call 0x451530
    cpu.esp -= 4;
    nfs3hp::port::text::dimBackground(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d2d3  f605583a7a0006         +test byte ptr [0x7a3a58], 6
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 6 /*0x6*/));
    // 0044d2da  753e                   -jne 0x44d31a
    if (!cpu.flags.zf)
    {
        goto L_0x0044d31a;
    }
    // 0044d2dc  ba74935300             -mov edx, 0x539374
    cpu.edx = 5477236 /*0x539374*/;
    // 0044d2e1  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d2e3  e85857ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d2e8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d2ea  7404                   -je 0x44d2f0
    if (cpu.flags.zf)
    {
        goto L_0x0044d2f0;
    }
    // 0044d2ec  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044d2f0:
    // 0044d2f0  ba80935300             -mov edx, 0x539380
    cpu.edx = 5477248 /*0x539380*/;
    // 0044d2f5  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d2f7  e84457ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d2fc  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d2fe  7406                   -je 0x44d306
    if (cpu.flags.zf)
    {
        goto L_0x0044d306;
    }
    // 0044d300  668148040110           -or word ptr [eax + 4], 0x1001
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg16(x86::sreg16(4097 /*0x1001*/));
L_0x0044d306:
    // 0044d306  ba9c935300             -mov edx, 0x53939c
    cpu.edx = 5477276 /*0x53939c*/;
    // 0044d30b  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d30d  e82e57ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d312  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d314  7404                   -je 0x44d31a
    if (cpu.flags.zf)
    {
        goto L_0x0044d31a;
    }
    // 0044d316  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044d31a:
    // 0044d31a  bab0935300             -mov edx, 0x5393b0
    cpu.edx = 5477296 /*0x5393b0*/;
    // 0044d31f  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d321  e81a57ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d326  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d328  7416                   -je 0x44d340
    if (cpu.flags.zf)
    {
        goto L_0x0044d340;
    }
    // 0044d32a  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044d331  7506                   -jne 0x44d339
    if (!cpu.flags.zf)
    {
        goto L_0x0044d339;
    }
    // 0044d333  80480401               +or byte ptr [eax + 4], 1
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 0044d337  eb07                   -jmp 0x44d340
    goto L_0x0044d340;
L_0x0044d339:
    // 0044d339  c7406470d24400         -mov dword ptr [eax + 0x64], 0x44d270
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4510320 /*0x44d270*/;
L_0x0044d340:
    // 0044d340  bac4935300             -mov edx, 0x5393c4
    cpu.edx = 5477316 /*0x5393c4*/;
    // 0044d345  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d347  e8f456ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d34c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d34e  740d                   -je 0x44d35d
    if (cpu.flags.zf)
    {
        goto L_0x0044d35d;
    }
    // 0044d350  f605583a7a0006         +test byte ptr [0x7a3a58], 6
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 6 /*0x6*/));
    // 0044d357  7404                   -je 0x44d35d
    if (cpu.flags.zf)
    {
        goto L_0x0044d35d;
    }
    // 0044d359  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044d35d:
    // 0044d35d  bad0935300             -mov edx, 0x5393d0
    cpu.edx = 5477328 /*0x5393d0*/;
    // 0044d362  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d364  e8d756ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d369  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d36b  7422                   -je 0x44d38f
    if (cpu.flags.zf)
    {
        goto L_0x0044d38f;
    }
    // 0044d36d  f605583a7a0040         +test byte ptr [0x7a3a58], 0x40
    cpu.clear_co();
    // Port: alpha intensity capability reported present.
    cpu.set_szp(static_cast<x86::reg8>(64 /*0x40, port: was the 0x7a3a58 probe*/));
    // 0044d374  7506                   -jne 0x44d37c
    if (!cpu.flags.zf)
    {
        goto L_0x0044d37c;
    }
    // 0044d376  80480401               +or byte ptr [eax + 4], 1
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 0044d37a  eb13                   -jmp 0x44d38f
    goto L_0x0044d38f;
L_0x0044d37c:
    // 0044d37c  d90538bc6f00           -fld dword ptr [0x6fbc38]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(x86::reg32(7322680) /* 0x6fbc38 */));
    // 0044d382  ba70cf4400             -mov edx, 0x44cf70
    cpu.edx = 4509552 /*0x44cf70*/;
    // 0044d387  d95854                 -fstp dword ptr [eax + 0x54]
    app->getMemory<float>(cpu.eax + x86::reg32(84) /* 0x54 */) = float(fpu1);
    }
    // 0044d38a  e831f70100             -call 0x46cac0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderSetOnMove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044d38f:
    // 0044d38f  bae0935300             -mov edx, 0x5393e0
    cpu.edx = 5477344 /*0x5393e0*/;
    // 0044d394  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d396  e8a556ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d39b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d39d  7413                   -je 0x44d3b2
    if (cpu.flags.zf)
    {
        goto L_0x0044d3b2;
    }
    // 0044d39f  833d683a7a0000         +cmp dword ptr [0x7a3a68], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010344) /* 0x7a3a68 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044d3a6  7406                   -je 0x44d3ae
    if (cpu.flags.zf)
    {
        goto L_0x0044d3ae;
    }
    // 0044d3a8  806004fe               +and byte ptr [eax + 4], 0xfe
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) &= x86::reg8(x86::sreg8(254 /*0xfe*/))));
    // 0044d3ac  eb04                   -jmp 0x44d3b2
    goto L_0x0044d3b2;
L_0x0044d3ae:
    // 0044d3ae  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044d3b2:
    // 0044d3b2  baec935300             -mov edx, 0x5393ec
    cpu.edx = 5477356 /*0x5393ec*/;
    // 0044d3b7  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d3b9  e88256ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d3be  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d3c0  7413                   -je 0x44d3d5
    if (cpu.flags.zf)
    {
        goto L_0x0044d3d5;
    }
    // 0044d3c2  833d683a7a0000         +cmp dword ptr [0x7a3a68], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010344) /* 0x7a3a68 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044d3c9  7406                   -je 0x44d3d1
    if (cpu.flags.zf)
    {
        goto L_0x0044d3d1;
    }
    // 0044d3cb  806004fe               +and byte ptr [eax + 4], 0xfe
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) &= x86::reg8(x86::sreg8(254 /*0xfe*/))));
    // 0044d3cf  eb04                   -jmp 0x44d3d5
    goto L_0x0044d3d5;
L_0x0044d3d1:
    // 0044d3d1  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044d3d5:
    // 0044d3d5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d3d7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d3d8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d3d9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d3da  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d3db  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d3dc  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d3dd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_44d3e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d3e0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d3e1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d3e2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d3e4  baec935300             -mov edx, 0x5393ec
    cpu.edx = 5477356 /*0x5393ec*/;
    // 0044d3e9  e85256ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d3ee  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044d3f0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d3f2  741f                   -je 0x44d413
    if (cpu.flags.zf)
    {
        goto L_0x0044d413;
    }
    // 0044d3f4  833d683a7a0000         +cmp dword ptr [0x7a3a68], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010344) /* 0x7a3a68 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044d3fb  7412                   -je 0x44d40f
    if (cpu.flags.zf)
    {
        goto L_0x0044d40f;
    }
    // 0044d3fd  833d34bc6f0000         +cmp dword ptr [0x6fbc34], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7322676) /* 0x6fbc34 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044d404  7409                   -je 0x44d40f
    if (cpu.flags.zf)
    {
        goto L_0x0044d40f;
    }
    // 0044d406  806004fe               -and byte ptr [eax + 4], 0xfe
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) &= x86::reg8(x86::sreg8(254 /*0xfe*/));
    // 0044d40a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d40c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d40d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d40e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044d40f:
    // 0044d40f  804a0401               -or byte ptr [edx + 4], 1
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044d413:
    // 0044d413  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d415  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d416  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d417  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_44d420(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d420  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044d421  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d422  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d423  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044d424  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044d425  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d426  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d428  8b5018                 -mov edx, dword ptr [eax + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0044d42b  8b4016                 -mov eax, dword ptr [eax + 0x16]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(22) /* 0x16 */);
    // 0044d42e  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0044d431  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0044d434  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0044d436  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0044d43d  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044d43f  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0044d442  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044d444  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0044d447  05e04e6000             -add eax, 0x604ee0
    (cpu.eax) += x86::reg32(x86::sreg32(6311648 /*0x604ee0*/));
    // 0044d44c  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 0044d44e  30db                   -xor bl, bl
    cpu.bl ^= x86::reg8(x86::sreg8(cpu.bl));
    // 0044d450  83fa05                 +cmp edx, 5
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
    // 0044d453  7562                   -jne 0x44d4b7
    if (!cpu.flags.zf)
    {
        goto L_0x0044d4b7;
    }
    // 0044d455  baf8935300             -mov edx, 0x5393f8
    cpu.edx = 5477368 /*0x5393f8*/;
    // 0044d45a  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0044d45d  e85e55ffff             -call 0x4429c0
    cpu.esp -= 4;
    nfs3hp::port::menus::codelinkMatches(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d462  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d464  7451                   -je 0x44d4b7
    if (cpu.flags.zf)
    {
        goto L_0x0044d4b7;
    }
    // 0044d466  8b0da02d6600           -mov ecx, dword ptr [0x662da0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(6696352) /* 0x662da0 */);
    // 0044d46c  3b0d4cbc6f00           +cmp ecx, dword ptr [0x6fbc4c]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7322700) /* 0x6fbc4c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044d472  7402                   -je 0x44d476
    if (cpu.flags.zf)
    {
        goto L_0x0044d476;
    }
    // 0044d474  b301                   -mov bl, 1
    cpu.bl = 1 /*0x1*/;
L_0x0044d476:
    // 0044d476  b96c170000             -mov ecx, 0x176c
    cpu.ecx = 5996 /*0x176c*/;
    // 0044d47b  bea02c6600             -mov esi, 0x662ca0
    cpu.esi = 6696096 /*0x662ca0*/;
    // 0044d480  bf4cbb6f00             -mov edi, 0x6fbb4c
    cpu.edi = 7322444 /*0x6fbb4c*/;
    // 0044d485  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044d486  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d488  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044d48b  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044d48d  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044d48f  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044d492  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044d494  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d495  84db                   +test bl, bl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bl & cpu.bl));
    // 0044d497  741e                   -je 0x44d4b7
    if (cpu.flags.zf)
    {
        goto L_0x0044d4b7;
    }
    // 0044d499  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d49b  e83057fdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d4a0  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044d4a7  7407                   -je 0x44d4b0
    if (cpu.flags.zf)
    {
        goto L_0x0044d4b0;
    }
    // 0044d4a9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044d4ae  eb02                   -jmp 0x44d4b2
    goto L_0x0044d4b2;
L_0x0044d4b0:
    // 0044d4b0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044d4b2:
    // 0044d4b2  e829110700             -call 0x4be5e0
    cpu.esp -= 4;
    sub_4be5e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044d4b7:
    // 0044d4b7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d4b9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d4ba  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d4bb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d4bc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d4bd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d4be  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d4bf  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_44d4c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d4c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044d4c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d4c2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d4c3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044d4c4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044d4c5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d4c6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d4c8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0044d4ca  b96c170000             -mov ecx, 0x176c
    cpu.ecx = 5996 /*0x176c*/;
    // 0044d4cf  be4cbb6f00             -mov esi, 0x6fbb4c
    cpu.esi = 7322444 /*0x6fbb4c*/;
    // 0044d4d4  bfa02c6600             -mov edi, 0x662ca0
    cpu.edi = 6696096 /*0x662ca0*/;
    // 0044d4d9  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044d4de  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044d4df  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d4e1  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044d4e4  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044d4e6  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044d4e8  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044d4eb  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044d4ed  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d4ee  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d4f0  89151c1b7900           -mov dword ptr [0x791b1c], edx
    app->getMemory<x86::reg32>(x86::reg32(7936796) /* 0x791b1c */) = cpu.edx;
    // 0044d4f6  e8d556fdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d4fb  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044d502  7407                   -je 0x44d50b
    if (cpu.flags.zf)
    {
        goto L_0x0044d50b;
    }
    // 0044d504  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044d509  eb02                   -jmp 0x44d50d
    goto L_0x0044d50d;
L_0x0044d50b:
    // 0044d50b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044d50d:
    // 0044d50d  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044d50f  69f0a4010000           -imul esi, eax, 0x1a4
    cpu.esi = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d515  8bb670bc6f00           -mov esi, dword ptr [esi + 0x6fbc70]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(7322736) /* 0x6fbc70 */);
    // 0044d51b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d51d  7507                   -jne 0x44d526
    if (!cpu.flags.zf)
    {
        goto L_0x0044d526;
    }
    // 0044d51f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044d524  eb02                   -jmp 0x44d528
    goto L_0x0044d528;
L_0x0044d526:
    // 0044d526  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044d528:
    // 0044d528  689a99193f             -push 0x3f19999a
    app->getMemory<x86::reg32>(cpu.esp-4) = 1058642330 /*0x3f19999a*/;
    cpu.esp -= 4;
    // 0044d52d  ba08945300             -mov edx, 0x539408
    cpu.edx = 5477384 /*0x539408*/;
    // 0044d532  a3f07d6700             -mov dword ptr [0x677df0], eax
    app->getMemory<x86::reg32>(x86::reg32(6782448) /* 0x677df0 */) = cpu.eax;
    // 0044d537  e8f43f0000             -call 0x451530
    cpu.esp -= 4;
    nfs3hp::port::text::dimBackground(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d53c  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d53e  e8fd54ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d543  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d545  743c                   -je 0x44d583
    if (cpu.flags.zf)
    {
        goto L_0x0044d583;
    }
    // 0044d547  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044d549  7419                   -je 0x44d564
    if (cpu.flags.zf)
    {
        goto L_0x0044d564;
    }
    // 0044d54b  d90504be6f00           +fld dword ptr [0x6fbe04]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(x86::reg32(7323140) /* 0x6fbe04 */));
    // 0044d551  d80534945300           +fadd dword ptr [0x539434]
    x86::Float fpu2 = cpu.fpu.add(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5477428) /* 0x539434 */)));
    // 0044d557  d80d38945300           +fmul dword ptr [0x539438]
    x86::Float fpu3 = cpu.fpu.mul(fpu2, x86::Float(app->getMemory<float>(x86::reg32(5477432) /* 0x539438 */)));
    // 0044d55d  baa0cf4400             -mov edx, 0x44cfa0
    cpu.edx = 4509600 /*0x44cfa0*/;
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu3;
    }
    // 0044d562  eb17                   -jmp 0x44d57b
    goto L_0x0044d57b;
L_0x0044d564:
    // 0044d564  d90560bc6f00           -fld dword ptr [0x6fbc60]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu4 = x86::Float(app->getMemory<float>(x86::reg32(7322720) /* 0x6fbc60 */));
    // 0044d56a  d80534945300           -fadd dword ptr [0x539434]
    x86::Float fpu5 = cpu.fpu.add(fpu4, x86::Float(app->getMemory<float>(x86::reg32(5477428) /* 0x539434 */)));
    // 0044d570  d80d38945300           -fmul dword ptr [0x539438]
    x86::Float fpu6 = cpu.fpu.mul(fpu5, x86::Float(app->getMemory<float>(x86::reg32(5477432) /* 0x539438 */)));
    // 0044d576  bae0cf4400             -mov edx, 0x44cfe0
    cpu.edx = 4509664 /*0x44cfe0*/;
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu6;
    }
L_0x0044d57b:
    // 0044d57b  d95854                 -fstp dword ptr [eax + 0x54]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu7 = cpu.fpu.st(0);
    app->getMemory<float>(cpu.eax + x86::reg32(84) /* 0x54 */) = float(fpu7);
    cpu.fpu.count -= 1;
    }
    // 0044d57e  e83df50100             -call 0x46cac0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderSetOnMove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044d583:
    // 0044d583  ba14945300             -mov edx, 0x539414
    cpu.edx = 5477396 /*0x539414*/;
    // 0044d588  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d58a  e8b154ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d58f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d591  7414                   -je 0x44d5a7
    if (cpu.flags.zf)
    {
        goto L_0x0044d5a7;
    }
    // 0044d593  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044d595  7409                   -je 0x44d5a0
    if (cpu.flags.zf)
    {
        goto L_0x0044d5a0;
    }
    // 0044d597  c7404020945300         -mov dword ptr [eax + 0x40], 0x539420
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 5477408 /*0x539420*/;
    // 0044d59e  eb07                   -jmp 0x44d5a7
    goto L_0x0044d5a7;
L_0x0044d5a0:
    // 0044d5a0  c7404028945300         -mov dword ptr [eax + 0x40], 0x539428
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 5477416 /*0x539428*/;
L_0x0044d5a7:
    // 0044d5a7  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d5a9  e822edffff             -call 0x44c2d0
    cpu.esp -= 4;
    sub_44c2d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d5ae  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044d5b0  7409                   -je 0x44d5bb
    if (cpu.flags.zf)
    {
        goto L_0x0044d5bb;
    }
    // 0044d5b2  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d5b4  e8a7edffff             -call 0x44c360
    cpu.esp -= 4;
    sub_44c360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d5b9  eb07                   -jmp 0x44d5c2
    goto L_0x0044d5c2;
L_0x0044d5bb:
    // 0044d5bb  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d5bd  e81eefffff             -call 0x44c4e0
    cpu.esp -= 4;
    sub_44c4e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044d5c2:
    // 0044d5c2  69d9a4010000           -imul ebx, ecx, 0x1a4
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d5c8  8bbb68bc6f00           -mov edi, dword ptr [ebx + 0x6fbc68]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(7322728) /* 0x6fbc68 */);
    // 0044d5ce  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0044d5d5  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0044d5d7  740b                   -je 0x44d5e4
    if (cpu.flags.zf)
    {
        goto L_0x0044d5e4;
    }
    // 0044d5d9  8d5fff                 -lea ebx, [edi - 1]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */);
    // 0044d5dc  899840886700           -mov dword ptr [eax + 0x678840], ebx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785088) /* 0x678840 */) = cpu.ebx;
    // 0044d5e2  eb0a                   -jmp 0x44d5ee
    goto L_0x0044d5ee;
L_0x0044d5e4:
    // 0044d5e4  c7804088670004000000   -mov dword ptr [eax + 0x678840], 4
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785088) /* 0x678840 */) = 4 /*0x4*/;
L_0x0044d5ee:
    // 0044d5ee  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d5f4  8bb86cbc6f00           -mov edi, dword ptr [eax + 0x6fbc6c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7322732) /* 0x6fbc6c */);
    // 0044d5fa  8d1c8d00000000         -lea ebx, [ecx*4]
    cpu.ebx = x86::reg32(cpu.ecx * 4);
    // 0044d601  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0044d603  740b                   -je 0x44d610
    if (cpu.flags.zf)
    {
        goto L_0x0044d610;
    }
    // 0044d605  8d47ff                 -lea eax, [edi - 1]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */);
    // 0044d608  898310886700           -mov dword ptr [ebx + 0x678810], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(6785040) /* 0x678810 */) = cpu.eax;
    // 0044d60e  eb0a                   -jmp 0x44d61a
    goto L_0x0044d61a;
L_0x0044d610:
    // 0044d610  c7831088670003000000   -mov dword ptr [ebx + 0x678810], 3
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(6785040) /* 0x678810 */) = 3 /*0x3*/;
L_0x0044d61a:
    // 0044d61a  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0044d621  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044d623  7409                   -je 0x44d62e
    if (cpu.flags.zf)
    {
        goto L_0x0044d62e;
    }
    // 0044d625  4e                     +dec esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044d626  89b0f8876700           -mov dword ptr [eax + 0x6787f8], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785016) /* 0x6787f8 */) = cpu.esi;
    // 0044d62c  eb0a                   -jmp 0x44d638
    goto L_0x0044d638;
L_0x0044d62e:
    // 0044d62e  c780f887670002000000   -mov dword ptr [eax + 0x6787f8], 2
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785016) /* 0x6787f8 */) = 2 /*0x2*/;
L_0x0044d638:
    // 0044d638  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d63e  8b9864bc6f00           -mov ebx, dword ptr [eax + 0x6fbc64]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7322724) /* 0x6fbc64 */);
    // 0044d644  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 0044d647  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0044d649  740b                   -je 0x44d656
    if (cpu.flags.zf)
    {
        goto L_0x0044d656;
    }
    // 0044d64b  8d43ff                 -lea eax, [ebx - 1]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(-1) /* -0x1 */);
    // 0044d64e  898128886700           -mov dword ptr [ecx + 0x678828], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6785064) /* 0x678828 */) = cpu.eax;
    // 0044d654  eb0a                   -jmp 0x44d660
    goto L_0x0044d660;
L_0x0044d656:
    // 0044d656  c7812888670005000000   -mov dword ptr [ecx + 0x678828], 5
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6785064) /* 0x678828 */) = 5 /*0x5*/;
L_0x0044d660:
    // 0044d660  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d662  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d663  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d664  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d665  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d666  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d667  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d668  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_44d670(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d670  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044d671  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d672  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d673  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044d674  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044d675  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d676  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d678  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0044d67a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d67c  e84f55fdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d681  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044d688  7407                   -je 0x44d691
    if (cpu.flags.zf)
    {
        goto L_0x0044d691;
    }
    // 0044d68a  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044d68f  eb02                   -jmp 0x44d693
    goto L_0x0044d693;
L_0x0044d691:
    // 0044d691  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044d693:
    // 0044d693  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0044d695  69caa4010000           -imul ecx, edx, 0x1a4
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d69b  be50bc6f00             -mov esi, 0x6fbc50
    cpu.esi = 7322704 /*0x6fbc50*/;
    // 0044d6a0  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 0044d6a3  01ce                   -add esi, ecx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0044d6a5  8bba40886700           -mov edi, dword ptr [edx + 0x678840]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785088) /* 0x678840 */);
    // 0044d6ab  83c620                 -add esi, 0x20
    (cpu.esi) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 0044d6ae  83ff04                 +cmp edi, 4
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044d6b1  7405                   -je 0x44d6b8
    if (cpu.flags.zf)
    {
        goto L_0x0044d6b8;
    }
    // 0044d6b3  8d5701                 -lea edx, [edi + 1]
    cpu.edx = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 0044d6b6  eb02                   -jmp 0x44d6ba
    goto L_0x0044d6ba;
L_0x0044d6b8:
    // 0044d6b8  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044d6ba:
    // 0044d6ba  899168bc6f00           -mov dword ptr [ecx + 0x6fbc68], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322728) /* 0x6fbc68 */) = cpu.edx;
    // 0044d6c0  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0044d6c7  69c8a4010000           -imul ecx, eax, 0x1a4
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d6cd  8bba10886700           -mov edi, dword ptr [edx + 0x678810]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785040) /* 0x678810 */);
    // 0044d6d3  83ff03                 +cmp edi, 3
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
    // 0044d6d6  7405                   -je 0x44d6dd
    if (cpu.flags.zf)
    {
        goto L_0x0044d6dd;
    }
    // 0044d6d8  8d5701                 -lea edx, [edi + 1]
    cpu.edx = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 0044d6db  eb02                   -jmp 0x44d6df
    goto L_0x0044d6df;
L_0x0044d6dd:
    // 0044d6dd  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044d6df:
    // 0044d6df  89916cbc6f00           -mov dword ptr [ecx + 0x6fbc6c], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322732) /* 0x6fbc6c */) = cpu.edx;
    // 0044d6e5  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0044d6ec  8b8af8876700           -mov ecx, dword ptr [edx + 0x6787f8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785016) /* 0x6787f8 */);
    // 0044d6f2  83f902                 +cmp ecx, 2
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
    // 0044d6f5  7407                   -je 0x44d6fe
    if (cpu.flags.zf)
    {
        goto L_0x0044d6fe;
    }
    // 0044d6f7  8d5101                 -lea edx, [ecx + 1]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(1) /* 0x1 */);
    // 0044d6fa  8916                   -mov dword ptr [esi], edx
    app->getMemory<x86::reg32>(cpu.esi) = cpu.edx;
    // 0044d6fc  eb06                   -jmp 0x44d704
    goto L_0x0044d704;
L_0x0044d6fe:
    // 0044d6fe  c70600000000           -mov dword ptr [esi], 0
    app->getMemory<x86::reg32>(cpu.esi) = 0 /*0x0*/;
L_0x0044d704:
    // 0044d704  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0044d70b  69c0a4010000           -imul eax, eax, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d711  8bb228886700           -mov esi, dword ptr [edx + 0x678828]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785064) /* 0x678828 */);
    // 0044d717  83fe05                 +cmp esi, 5
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
    // 0044d71a  740b                   -je 0x44d727
    if (cpu.flags.zf)
    {
        goto L_0x0044d727;
    }
    // 0044d71c  8d5601                 -lea edx, [esi + 1]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044d71f  899064bc6f00           -mov dword ptr [eax + 0x6fbc64], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7322724) /* 0x6fbc64 */) = cpu.edx;
    // 0044d725  eb08                   -jmp 0x44d72f
    goto L_0x0044d72f;
L_0x0044d727:
    // 0044d727  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044d729  89b864bc6f00           -mov dword ptr [eax + 0x6fbc64], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7322724) /* 0x6fbc64 */) = cpu.edi;
L_0x0044d72f:
    // 0044d72f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d731  e88a6d0300             -call 0x4844c0
    cpu.esp -= 4;
    nfs3hp::port::hud::hudPaneSet(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d736  8b5318                 -mov edx, dword ptr [ebx + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */);
    // 0044d739  8b4316                 -mov eax, dword ptr [ebx + 0x16]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(22) /* 0x16 */);
    // 0044d73c  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0044d73f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0044d742  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0044d744  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0044d74b  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044d74d  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0044d750  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044d752  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0044d755  05e04e6000             -add eax, 0x604ee0
    (cpu.eax) += x86::reg32(x86::sreg32(6311648 /*0x604ee0*/));
    // 0044d75a  833805                 +cmp dword ptr [eax], 5
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044d75d  7530                   -jne 0x44d78f
    if (!cpu.flags.zf)
    {
        goto L_0x0044d78f;
    }
    // 0044d75f  ba3c945300             -mov edx, 0x53943c
    cpu.edx = 5477436 /*0x53943c*/;
    // 0044d764  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0044d767  e85452ffff             -call 0x4429c0
    cpu.esp -= 4;
    nfs3hp::port::menus::codelinkMatches(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d76c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d76e  741f                   -je 0x44d78f
    if (cpu.flags.zf)
    {
        goto L_0x0044d78f;
    }
    // 0044d770  b96c170000             -mov ecx, 0x176c
    cpu.ecx = 5996 /*0x176c*/;
    // 0044d775  bea02c6600             -mov esi, 0x662ca0
    cpu.esi = 6696096 /*0x662ca0*/;
    // 0044d77a  bf4cbb6f00             -mov edi, 0x6fbb4c
    cpu.edi = 7322444 /*0x6fbb4c*/;
    // 0044d77f  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044d780  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d782  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044d785  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044d787  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044d789  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044d78c  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044d78e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0044d78f:
    // 0044d78f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d791  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d792  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d793  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d794  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d795  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d796  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d797  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_44d7a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d7a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044d7a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d7a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d7a3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044d7a4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044d7a5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d7a6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d7a8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0044d7aa  b96c170000             -mov ecx, 0x176c
    cpu.ecx = 5996 /*0x176c*/;
    // 0044d7af  be4cbb6f00             -mov esi, 0x6fbb4c
    cpu.esi = 7322444 /*0x6fbb4c*/;
    // 0044d7b4  bfa02c6600             -mov edi, 0x662ca0
    cpu.edi = 6696096 /*0x662ca0*/;
    // 0044d7b9  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044d7be  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044d7bf  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044d7c1  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044d7c4  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044d7c6  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044d7c8  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044d7cb  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044d7cd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d7ce  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d7d0  89151c1b7900           -mov dword ptr [0x791b1c], edx
    app->getMemory<x86::reg32>(x86::reg32(7936796) /* 0x791b1c */) = cpu.edx;
    // 0044d7d6  e8f553fdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d7db  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044d7e2  7407                   -je 0x44d7eb
    if (cpu.flags.zf)
    {
        goto L_0x0044d7eb;
    }
    // 0044d7e4  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0044d7e9  eb02                   -jmp 0x44d7ed
    goto L_0x0044d7ed;
L_0x0044d7eb:
    // 0044d7eb  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0044d7ed:
    // 0044d7ed  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044d7f2  e8d953fdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d7f7  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044d7fe  7407                   -je 0x44d807
    if (cpu.flags.zf)
    {
        goto L_0x0044d807;
    }
    // 0044d800  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 0044d805  eb02                   -jmp 0x44d809
    goto L_0x0044d809;
L_0x0044d807:
    // 0044d807  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x0044d809:
    // 0044d809  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d80f  8bb8b8bf6f00           -mov edi, dword ptr [eax + 0x6fbfb8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7323576) /* 0x6fbfb8 */);
    // 0044d815  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044d817  7507                   -jne 0x44d820
    if (!cpu.flags.zf)
    {
        goto L_0x0044d820;
    }
    // 0044d819  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044d81e  eb02                   -jmp 0x44d822
    goto L_0x0044d822;
L_0x0044d820:
    // 0044d820  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044d822:
    // 0044d822  a3f07d6700             -mov dword ptr [0x677df0], eax
    app->getMemory<x86::reg32>(x86::reg32(6782448) /* 0x677df0 */) = cpu.eax;
    // 0044d827  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044d829  7507                   -jne 0x44d832
    if (!cpu.flags.zf)
    {
        goto L_0x0044d832;
    }
    // 0044d82b  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044d830  eb02                   -jmp 0x44d834
    goto L_0x0044d834;
L_0x0044d832:
    // 0044d832  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044d834:
    // 0044d834  689a99193f             -push 0x3f19999a
    app->getMemory<x86::reg32>(cpu.esp-4) = 1058642330 /*0x3f19999a*/;
    cpu.esp -= 4;
    // 0044d839  ba08945300             -mov edx, 0x539408
    cpu.edx = 5477384 /*0x539408*/;
    // 0044d83e  a3f47d6700             -mov dword ptr [0x677df4], eax
    app->getMemory<x86::reg32>(x86::reg32(6782452) /* 0x677df4 */) = cpu.eax;
    // 0044d843  e8e83c0000             -call 0x451530
    cpu.esp -= 4;
    nfs3hp::port::text::dimBackground(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d848  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d84a  e8f151ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d84f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d851  743c                   -je 0x44d88f
    if (cpu.flags.zf)
    {
        goto L_0x0044d88f;
    }
    // 0044d853  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044d855  7419                   -je 0x44d870
    if (cpu.flags.zf)
    {
        goto L_0x0044d870;
    }
    // 0044d857  d9054cc16f00           +fld dword ptr [0x6fc14c]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(x86::reg32(7323980) /* 0x6fc14c */));
    // 0044d85d  d80574945300           +fadd dword ptr [0x539474]
    x86::Float fpu2 = cpu.fpu.add(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5477492) /* 0x539474 */)));
    // 0044d863  d80d78945300           +fmul dword ptr [0x539478]
    x86::Float fpu3 = cpu.fpu.mul(fpu2, x86::Float(app->getMemory<float>(x86::reg32(5477496) /* 0x539478 */)));
    // 0044d869  ba20d04400             -mov edx, 0x44d020
    cpu.edx = 4509728 /*0x44d020*/;
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu3;
    }
    // 0044d86e  eb17                   -jmp 0x44d887
    goto L_0x0044d887;
L_0x0044d870:
    // 0044d870  d905a8bf6f00           -fld dword ptr [0x6fbfa8]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu4 = x86::Float(app->getMemory<float>(x86::reg32(7323560) /* 0x6fbfa8 */));
    // 0044d876  d80574945300           -fadd dword ptr [0x539474]
    x86::Float fpu5 = cpu.fpu.add(fpu4, x86::Float(app->getMemory<float>(x86::reg32(5477492) /* 0x539474 */)));
    // 0044d87c  d80d78945300           -fmul dword ptr [0x539478]
    x86::Float fpu6 = cpu.fpu.mul(fpu5, x86::Float(app->getMemory<float>(x86::reg32(5477496) /* 0x539478 */)));
    // 0044d882  ba60d04400             -mov edx, 0x44d060
    cpu.edx = 4509792 /*0x44d060*/;
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu6;
    }
L_0x0044d887:
    // 0044d887  d95854                 -fstp dword ptr [eax + 0x54]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu7 = cpu.fpu.st(0);
    app->getMemory<float>(cpu.eax + x86::reg32(84) /* 0x54 */) = float(fpu7);
    cpu.fpu.count -= 1;
    }
    // 0044d88a  e831f20100             -call 0x46cac0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderSetOnMove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044d88f:
    // 0044d88f  ba14945300             -mov edx, 0x539414
    cpu.edx = 5477396 /*0x539414*/;
    // 0044d894  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d896  e8a551ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d89b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044d89d  742e                   -je 0x44d8cd
    if (cpu.flags.zf)
    {
        goto L_0x0044d8cd;
    }
    // 0044d89f  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044d8a1  7416                   -je 0x44d8b9
    if (cpu.flags.zf)
    {
        goto L_0x0044d8b9;
    }
    // 0044d8a3  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044d8a5  7409                   -je 0x44d8b0
    if (cpu.flags.zf)
    {
        goto L_0x0044d8b0;
    }
    // 0044d8a7  c740404c945300         -mov dword ptr [eax + 0x40], 0x53944c
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 5477452 /*0x53944c*/;
    // 0044d8ae  eb1d                   -jmp 0x44d8cd
    goto L_0x0044d8cd;
L_0x0044d8b0:
    // 0044d8b0  c7404054945300         -mov dword ptr [eax + 0x40], 0x539454
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 5477460 /*0x539454*/;
    // 0044d8b7  eb14                   -jmp 0x44d8cd
    goto L_0x0044d8cd;
L_0x0044d8b9:
    // 0044d8b9  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044d8bb  7409                   -je 0x44d8c6
    if (cpu.flags.zf)
    {
        goto L_0x0044d8c6;
    }
    // 0044d8bd  c7404060945300         -mov dword ptr [eax + 0x40], 0x539460
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 5477472 /*0x539460*/;
    // 0044d8c4  eb07                   -jmp 0x44d8cd
    goto L_0x0044d8cd;
L_0x0044d8c6:
    // 0044d8c6  c740406c945300         -mov dword ptr [eax + 0x40], 0x53946c
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 5477484 /*0x53946c*/;
L_0x0044d8cd:
    // 0044d8cd  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d8cf  e8fce9ffff             -call 0x44c2d0
    cpu.esp -= 4;
    sub_44c2d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d8d4  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044d8d6  7409                   -je 0x44d8e1
    if (cpu.flags.zf)
    {
        goto L_0x0044d8e1;
    }
    // 0044d8d8  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d8da  e881eaffff             -call 0x44c360
    cpu.esp -= 4;
    sub_44c360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d8df  eb07                   -jmp 0x44d8e8
    goto L_0x0044d8e8;
L_0x0044d8e1:
    // 0044d8e1  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044d8e3  e8f8ebffff             -call 0x44c4e0
    cpu.esp -= 4;
    sub_44c4e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044d8e8:
    // 0044d8e8  69d9a4010000           -imul ebx, ecx, 0x1a4
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d8ee  8bb3b0bf6f00           -mov esi, dword ptr [ebx + 0x6fbfb0]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(7323568) /* 0x6fbfb0 */);
    // 0044d8f4  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0044d8fb  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044d8fd  740b                   -je 0x44d90a
    if (cpu.flags.zf)
    {
        goto L_0x0044d90a;
    }
    // 0044d8ff  8d5eff                 -lea ebx, [esi - 1]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(-1) /* -0x1 */);
    // 0044d902  899848886700           -mov dword ptr [eax + 0x678848], ebx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785096) /* 0x678848 */) = cpu.ebx;
    // 0044d908  eb0a                   -jmp 0x44d914
    goto L_0x0044d914;
L_0x0044d90a:
    // 0044d90a  c7804888670004000000   -mov dword ptr [eax + 0x678848], 4
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785096) /* 0x678848 */) = 4 /*0x4*/;
L_0x0044d914:
    // 0044d914  69d9a4010000           -imul ebx, ecx, 0x1a4
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d91a  8bb3b4bf6f00           -mov esi, dword ptr [ebx + 0x6fbfb4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(7323572) /* 0x6fbfb4 */);
    // 0044d920  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0044d927  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044d929  740b                   -je 0x44d936
    if (cpu.flags.zf)
    {
        goto L_0x0044d936;
    }
    // 0044d92b  8d5eff                 -lea ebx, [esi - 1]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(-1) /* -0x1 */);
    // 0044d92e  899818886700           -mov dword ptr [eax + 0x678818], ebx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785048) /* 0x678818 */) = cpu.ebx;
    // 0044d934  eb0a                   -jmp 0x44d940
    goto L_0x0044d940;
L_0x0044d936:
    // 0044d936  c7801888670003000000   -mov dword ptr [eax + 0x678818], 3
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785048) /* 0x678818 */) = 3 /*0x3*/;
L_0x0044d940:
    // 0044d940  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0044d947  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0044d949  7409                   -je 0x44d954
    if (cpu.flags.zf)
    {
        goto L_0x0044d954;
    }
    // 0044d94b  4f                     +dec edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044d94c  89b800886700           -mov dword ptr [eax + 0x678800], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785024) /* 0x678800 */) = cpu.edi;
    // 0044d952  eb0a                   -jmp 0x44d95e
    goto L_0x0044d95e;
L_0x0044d954:
    // 0044d954  c7800088670002000000   -mov dword ptr [eax + 0x678800], 2
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785024) /* 0x678800 */) = 2 /*0x2*/;
L_0x0044d95e:
    // 0044d95e  69d9a4010000           -imul ebx, ecx, 0x1a4
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d964  8bb3acbf6f00           -mov esi, dword ptr [ebx + 0x6fbfac]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(7323564) /* 0x6fbfac */);
    // 0044d96a  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0044d971  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044d973  740b                   -je 0x44d980
    if (cpu.flags.zf)
    {
        goto L_0x0044d980;
    }
    // 0044d975  8d4eff                 -lea ecx, [esi - 1]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(-1) /* -0x1 */);
    // 0044d978  898830886700           -mov dword ptr [eax + 0x678830], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785072) /* 0x678830 */) = cpu.ecx;
    // 0044d97e  eb0a                   -jmp 0x44d98a
    goto L_0x0044d98a;
L_0x0044d980:
    // 0044d980  c7803088670005000000   -mov dword ptr [eax + 0x678830], 5
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785072) /* 0x678830 */) = 5 /*0x5*/;
L_0x0044d98a:
    // 0044d98a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d98c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d98d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d98e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d98f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d990  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d991  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044d992  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_44d9a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044d9a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044d9a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044d9a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044d9a3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044d9a4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044d9a5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044d9a6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044d9a8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0044d9aa  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044d9ac  e81f52fdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044d9b1  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044d9b8  7407                   -je 0x44d9c1
    if (cpu.flags.zf)
    {
        goto L_0x0044d9c1;
    }
    // 0044d9ba  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044d9bf  eb02                   -jmp 0x44d9c3
    goto L_0x0044d9c3;
L_0x0044d9c1:
    // 0044d9c1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044d9c3:
    // 0044d9c3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0044d9c5  69caa4010000           -imul ecx, edx, 0x1a4
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d9cb  be98bf6f00             -mov esi, 0x6fbf98
    cpu.esi = 7323544 /*0x6fbf98*/;
    // 0044d9d0  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 0044d9d3  01ce                   -add esi, ecx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0044d9d5  8bba48886700           -mov edi, dword ptr [edx + 0x678848]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785096) /* 0x678848 */);
    // 0044d9db  83c620                 -add esi, 0x20
    (cpu.esi) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 0044d9de  83ff04                 +cmp edi, 4
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044d9e1  7405                   -je 0x44d9e8
    if (cpu.flags.zf)
    {
        goto L_0x0044d9e8;
    }
    // 0044d9e3  8d5701                 -lea edx, [edi + 1]
    cpu.edx = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 0044d9e6  eb02                   -jmp 0x44d9ea
    goto L_0x0044d9ea;
L_0x0044d9e8:
    // 0044d9e8  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044d9ea:
    // 0044d9ea  8991b0bf6f00           -mov dword ptr [ecx + 0x6fbfb0], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7323568) /* 0x6fbfb0 */) = cpu.edx;
    // 0044d9f0  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0044d9f7  69c8a4010000           -imul ecx, eax, 0x1a4
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044d9fd  8bba18886700           -mov edi, dword ptr [edx + 0x678818]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785048) /* 0x678818 */);
    // 0044da03  83ff03                 +cmp edi, 3
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
    // 0044da06  7405                   -je 0x44da0d
    if (cpu.flags.zf)
    {
        goto L_0x0044da0d;
    }
    // 0044da08  8d5701                 -lea edx, [edi + 1]
    cpu.edx = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 0044da0b  eb02                   -jmp 0x44da0f
    goto L_0x0044da0f;
L_0x0044da0d:
    // 0044da0d  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044da0f:
    // 0044da0f  8991b4bf6f00           -mov dword ptr [ecx + 0x6fbfb4], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7323572) /* 0x6fbfb4 */) = cpu.edx;
    // 0044da15  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0044da1c  8b8a00886700           -mov ecx, dword ptr [edx + 0x678800]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785024) /* 0x678800 */);
    // 0044da22  83f902                 +cmp ecx, 2
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
    // 0044da25  7407                   -je 0x44da2e
    if (cpu.flags.zf)
    {
        goto L_0x0044da2e;
    }
    // 0044da27  8d5101                 -lea edx, [ecx + 1]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(1) /* 0x1 */);
    // 0044da2a  8916                   -mov dword ptr [esi], edx
    app->getMemory<x86::reg32>(cpu.esi) = cpu.edx;
    // 0044da2c  eb06                   -jmp 0x44da34
    goto L_0x0044da34;
L_0x0044da2e:
    // 0044da2e  c70600000000           -mov dword ptr [esi], 0
    app->getMemory<x86::reg32>(cpu.esi) = 0 /*0x0*/;
L_0x0044da34:
    // 0044da34  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0044da3b  69c0a4010000           -imul eax, eax, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044da41  8bb230886700           -mov esi, dword ptr [edx + 0x678830]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785072) /* 0x678830 */);
    // 0044da47  83fe05                 +cmp esi, 5
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
    // 0044da4a  740b                   -je 0x44da57
    if (cpu.flags.zf)
    {
        goto L_0x0044da57;
    }
    // 0044da4c  8d5601                 -lea edx, [esi + 1]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044da4f  8990acbf6f00           -mov dword ptr [eax + 0x6fbfac], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7323564) /* 0x6fbfac */) = cpu.edx;
    // 0044da55  eb08                   -jmp 0x44da5f
    goto L_0x0044da5f;
L_0x0044da57:
    // 0044da57  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044da59  89b8acbf6f00           -mov dword ptr [eax + 0x6fbfac], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7323564) /* 0x6fbfac */) = cpu.edi;
L_0x0044da5f:
    // 0044da5f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044da61  e85a6a0300             -call 0x4844c0
    cpu.esp -= 4;
    nfs3hp::port::hud::hudPaneSet(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044da66  8b5318                 -mov edx, dword ptr [ebx + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */);
    // 0044da69  8b4316                 -mov eax, dword ptr [ebx + 0x16]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(22) /* 0x16 */);
    // 0044da6c  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0044da6f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0044da72  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0044da74  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0044da7b  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044da7d  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0044da80  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044da82  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0044da85  05e04e6000             -add eax, 0x604ee0
    (cpu.eax) += x86::reg32(x86::sreg32(6311648 /*0x604ee0*/));
    // 0044da8a  833805                 +cmp dword ptr [eax], 5
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044da8d  7530                   -jne 0x44dabf
    if (!cpu.flags.zf)
    {
        goto L_0x0044dabf;
    }
    // 0044da8f  ba3c945300             -mov edx, 0x53943c
    cpu.edx = 5477436 /*0x53943c*/;
    // 0044da94  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0044da97  e8244fffff             -call 0x4429c0
    cpu.esp -= 4;
    nfs3hp::port::menus::codelinkMatches(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044da9c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044da9e  741f                   -je 0x44dabf
    if (cpu.flags.zf)
    {
        goto L_0x0044dabf;
    }
    // 0044daa0  b96c170000             -mov ecx, 0x176c
    cpu.ecx = 5996 /*0x176c*/;
    // 0044daa5  bea02c6600             -mov esi, 0x662ca0
    cpu.esi = 6696096 /*0x662ca0*/;
    // 0044daaa  bf4cbb6f00             -mov edi, 0x6fbb4c
    cpu.edi = 7322444 /*0x6fbb4c*/;
    // 0044daaf  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044dab0  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044dab2  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044dab5  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044dab7  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044dab9  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044dabc  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044dabe  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0044dabf:
    // 0044dabf  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044dac1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dac2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dac3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dac4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dac5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dac6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dac7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_44dad0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044dad0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044dad1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044dad2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044dad3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044dad4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044dad5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044dad6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044dad8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0044dada  b96c170000             -mov ecx, 0x176c
    cpu.ecx = 5996 /*0x176c*/;
    // 0044dadf  be4cbb6f00             -mov esi, 0x6fbb4c
    cpu.esi = 7322444 /*0x6fbb4c*/;
    // 0044dae4  bfa02c6600             -mov edi, 0x662ca0
    cpu.edi = 6696096 /*0x662ca0*/;
    // 0044dae9  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044daee  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044daef  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044daf1  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044daf4  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044daf6  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044daf8  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044dafb  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044dafd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dafe  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044db00  89151c1b7900           -mov dword ptr [0x791b1c], edx
    app->getMemory<x86::reg32>(x86::reg32(7936796) /* 0x791b1c */) = cpu.edx;
    // 0044db06  e8c550fdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044db0b  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044db12  7407                   -je 0x44db1b
    if (cpu.flags.zf)
    {
        goto L_0x0044db1b;
    }
    // 0044db14  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044db19  eb02                   -jmp 0x44db1d
    goto L_0x0044db1d;
L_0x0044db1b:
    // 0044db1b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044db1d:
    // 0044db1d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044db22  e8a950fdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044db27  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0044db29  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044db30  7407                   -je 0x44db39
    if (cpu.flags.zf)
    {
        goto L_0x0044db39;
    }
    // 0044db32  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044db37  eb02                   -jmp 0x44db3b
    goto L_0x0044db3b;
L_0x0044db39:
    // 0044db39  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044db3b:
    // 0044db3b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044db3d  69c0a4010000           -imul eax, eax, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044db43  8bb000c36f00           -mov esi, dword ptr [eax + 0x6fc300]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7324416) /* 0x6fc300 */);
    // 0044db49  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0044db4b  7507                   -jne 0x44db54
    if (!cpu.flags.zf)
    {
        goto L_0x0044db54;
    }
    // 0044db4d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044db52  eb02                   -jmp 0x44db56
    goto L_0x0044db56;
L_0x0044db54:
    // 0044db54  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044db56:
    // 0044db56  a3f07d6700             -mov dword ptr [0x677df0], eax
    app->getMemory<x86::reg32>(x86::reg32(6782448) /* 0x677df0 */) = cpu.eax;
    // 0044db5b  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044db5d  7507                   -jne 0x44db66
    if (!cpu.flags.zf)
    {
        goto L_0x0044db66;
    }
    // 0044db5f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044db64  eb02                   -jmp 0x44db68
    goto L_0x0044db68;
L_0x0044db66:
    // 0044db66  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044db68:
    // 0044db68  689a99193f             -push 0x3f19999a
    app->getMemory<x86::reg32>(cpu.esp-4) = 1058642330 /*0x3f19999a*/;
    cpu.esp -= 4;
    // 0044db6d  ba08945300             -mov edx, 0x539408
    cpu.edx = 5477384 /*0x539408*/;
    // 0044db72  a3f47d6700             -mov dword ptr [0x677df4], eax
    app->getMemory<x86::reg32>(x86::reg32(6782452) /* 0x677df4 */) = cpu.eax;
    // 0044db77  e8b4390000             -call 0x451530
    cpu.esp -= 4;
    nfs3hp::port::text::dimBackground(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044db7c  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044db7e  e8bd4effff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044db83  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044db85  743c                   -je 0x44dbc3
    if (cpu.flags.zf)
    {
        goto L_0x0044dbc3;
    }
    // 0044db87  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044db89  7419                   -je 0x44dba4
    if (cpu.flags.zf)
    {
        goto L_0x0044dba4;
    }
    // 0044db8b  d90594c46f00           +fld dword ptr [0x6fc494]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(x86::reg32(7324820) /* 0x6fc494 */));
    // 0044db91  d8057c945300           +fadd dword ptr [0x53947c]
    x86::Float fpu2 = cpu.fpu.add(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5477500) /* 0x53947c */)));
    // 0044db97  d80d80945300           +fmul dword ptr [0x539480]
    x86::Float fpu3 = cpu.fpu.mul(fpu2, x86::Float(app->getMemory<float>(x86::reg32(5477504) /* 0x539480 */)));
    // 0044db9d  baa0d04400             -mov edx, 0x44d0a0
    cpu.edx = 4509856 /*0x44d0a0*/;
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu3;
    }
    // 0044dba2  eb17                   -jmp 0x44dbbb
    goto L_0x0044dbbb;
L_0x0044dba4:
    // 0044dba4  d905f0c26f00           -fld dword ptr [0x6fc2f0]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu4 = x86::Float(app->getMemory<float>(x86::reg32(7324400) /* 0x6fc2f0 */));
    // 0044dbaa  d8057c945300           -fadd dword ptr [0x53947c]
    x86::Float fpu5 = cpu.fpu.add(fpu4, x86::Float(app->getMemory<float>(x86::reg32(5477500) /* 0x53947c */)));
    // 0044dbb0  d80d80945300           -fmul dword ptr [0x539480]
    x86::Float fpu6 = cpu.fpu.mul(fpu5, x86::Float(app->getMemory<float>(x86::reg32(5477504) /* 0x539480 */)));
    // 0044dbb6  bae0d04400             -mov edx, 0x44d0e0
    cpu.edx = 4509920 /*0x44d0e0*/;
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu6;
    }
L_0x0044dbbb:
    // 0044dbbb  d95854                 -fstp dword ptr [eax + 0x54]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu7 = cpu.fpu.st(0);
    app->getMemory<float>(cpu.eax + x86::reg32(84) /* 0x54 */) = float(fpu7);
    cpu.fpu.count -= 1;
    }
    // 0044dbbe  e8fdee0100             -call 0x46cac0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderSetOnMove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044dbc3:
    // 0044dbc3  ba14945300             -mov edx, 0x539414
    cpu.edx = 5477396 /*0x539414*/;
    // 0044dbc8  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044dbca  e8714effff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044dbcf  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044dbd1  742e                   -je 0x44dc01
    if (cpu.flags.zf)
    {
        goto L_0x0044dc01;
    }
    // 0044dbd3  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0044dbd5  7416                   -je 0x44dbed
    if (cpu.flags.zf)
    {
        goto L_0x0044dbed;
    }
    // 0044dbd7  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044dbd9  7409                   -je 0x44dbe4
    if (cpu.flags.zf)
    {
        goto L_0x0044dbe4;
    }
    // 0044dbdb  c740404c945300         -mov dword ptr [eax + 0x40], 0x53944c
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 5477452 /*0x53944c*/;
    // 0044dbe2  eb1d                   -jmp 0x44dc01
    goto L_0x0044dc01;
L_0x0044dbe4:
    // 0044dbe4  c7404054945300         -mov dword ptr [eax + 0x40], 0x539454
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 5477460 /*0x539454*/;
    // 0044dbeb  eb14                   -jmp 0x44dc01
    goto L_0x0044dc01;
L_0x0044dbed:
    // 0044dbed  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044dbef  7409                   -je 0x44dbfa
    if (cpu.flags.zf)
    {
        goto L_0x0044dbfa;
    }
    // 0044dbf1  c7404060945300         -mov dword ptr [eax + 0x40], 0x539460
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 5477472 /*0x539460*/;
    // 0044dbf8  eb07                   -jmp 0x44dc01
    goto L_0x0044dc01;
L_0x0044dbfa:
    // 0044dbfa  c740406c945300         -mov dword ptr [eax + 0x40], 0x53946c
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 5477484 /*0x53946c*/;
L_0x0044dc01:
    // 0044dc01  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044dc03  e8c8e6ffff             -call 0x44c2d0
    cpu.esp -= 4;
    sub_44c2d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044dc08  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044dc0a  7409                   -je 0x44dc15
    if (cpu.flags.zf)
    {
        goto L_0x0044dc15;
    }
    // 0044dc0c  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044dc0e  e84de7ffff             -call 0x44c360
    cpu.esp -= 4;
    sub_44c360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044dc13  eb07                   -jmp 0x44dc1c
    goto L_0x0044dc1c;
L_0x0044dc15:
    // 0044dc15  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044dc17  e8c4e8ffff             -call 0x44c4e0
    cpu.esp -= 4;
    sub_44c4e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044dc1c:
    // 0044dc1c  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044dc22  8b98f8c26f00           -mov ebx, dword ptr [eax + 0x6fc2f8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7324408) /* 0x6fc2f8 */);
    // 0044dc28  8d148d00000000         -lea edx, [ecx*4]
    cpu.edx = x86::reg32(cpu.ecx * 4);
    // 0044dc2f  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0044dc31  740b                   -je 0x44dc3e
    if (cpu.flags.zf)
    {
        goto L_0x0044dc3e;
    }
    // 0044dc33  8d43ff                 -lea eax, [ebx - 1]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(-1) /* -0x1 */);
    // 0044dc36  898250886700           -mov dword ptr [edx + 0x678850], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785104) /* 0x678850 */) = cpu.eax;
    // 0044dc3c  eb0a                   -jmp 0x44dc48
    goto L_0x0044dc48;
L_0x0044dc3e:
    // 0044dc3e  c7825088670004000000   -mov dword ptr [edx + 0x678850], 4
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785104) /* 0x678850 */) = 4 /*0x4*/;
L_0x0044dc48:
    // 0044dc48  69c1a4010000           -imul eax, ecx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044dc4e  8b98fcc26f00           -mov ebx, dword ptr [eax + 0x6fc2fc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7324412) /* 0x6fc2fc */);
    // 0044dc54  8d148d00000000         -lea edx, [ecx*4]
    cpu.edx = x86::reg32(cpu.ecx * 4);
    // 0044dc5b  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0044dc5d  740b                   -je 0x44dc6a
    if (cpu.flags.zf)
    {
        goto L_0x0044dc6a;
    }
    // 0044dc5f  8d43ff                 -lea eax, [ebx - 1]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(-1) /* -0x1 */);
    // 0044dc62  898220886700           -mov dword ptr [edx + 0x678820], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785056) /* 0x678820 */) = cpu.eax;
    // 0044dc68  eb0a                   -jmp 0x44dc74
    goto L_0x0044dc74;
L_0x0044dc6a:
    // 0044dc6a  c7822088670003000000   -mov dword ptr [edx + 0x678820], 3
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785056) /* 0x678820 */) = 3 /*0x3*/;
L_0x0044dc74:
    // 0044dc74  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0044dc7b  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044dc7d  7409                   -je 0x44dc88
    if (cpu.flags.zf)
    {
        goto L_0x0044dc88;
    }
    // 0044dc7f  4e                     +dec esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044dc80  89b008886700           -mov dword ptr [eax + 0x678808], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785032) /* 0x678808 */) = cpu.esi;
    // 0044dc86  eb0a                   -jmp 0x44dc92
    goto L_0x0044dc92;
L_0x0044dc88:
    // 0044dc88  c7800888670002000000   -mov dword ptr [eax + 0x678808], 2
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785032) /* 0x678808 */) = 2 /*0x2*/;
L_0x0044dc92:
    // 0044dc92  69d1a4010000           -imul edx, ecx, 0x1a4
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044dc98  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0044dc9f  83baf4c26f0000         +cmp dword ptr [edx + 0x6fc2f4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7324404) /* 0x6fc2f4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044dca6  740f                   -je 0x44dcb7
    if (cpu.flags.zf)
    {
        goto L_0x0044dcb7;
    }
    // 0044dca8  8b92acbf6f00           -mov edx, dword ptr [edx + 0x6fbfac]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7323564) /* 0x6fbfac */);
    // 0044dcae  4a                     +dec edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0044dcaf  899038886700           -mov dword ptr [eax + 0x678838], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785080) /* 0x678838 */) = cpu.edx;
    // 0044dcb5  eb0a                   -jmp 0x44dcc1
    goto L_0x0044dcc1;
L_0x0044dcb7:
    // 0044dcb7  c7803888670005000000   -mov dword ptr [eax + 0x678838], 5
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785080) /* 0x678838 */) = 5 /*0x5*/;
L_0x0044dcc1:
    // 0044dcc1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044dcc3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dcc4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dcc5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dcc6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dcc7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dcc8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dcc9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_44dcd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044dcd0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044dcd1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044dcd2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044dcd3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044dcd4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044dcd5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044dcd6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044dcd8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0044dcda  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044dcdf  e8ec4efdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044dce4  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044dceb  7407                   -je 0x44dcf4
    if (cpu.flags.zf)
    {
        goto L_0x0044dcf4;
    }
    // 0044dced  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044dcf2  eb02                   -jmp 0x44dcf6
    goto L_0x0044dcf6;
L_0x0044dcf4:
    // 0044dcf4  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044dcf6:
    // 0044dcf6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0044dcf8  69caa4010000           -imul ecx, edx, 0x1a4
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044dcfe  bee0c26f00             -mov esi, 0x6fc2e0
    cpu.esi = 7324384 /*0x6fc2e0*/;
    // 0044dd03  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 0044dd06  01ce                   -add esi, ecx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0044dd08  8bba50886700           -mov edi, dword ptr [edx + 0x678850]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785104) /* 0x678850 */);
    // 0044dd0e  83c620                 -add esi, 0x20
    (cpu.esi) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 0044dd11  83ff04                 +cmp edi, 4
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044dd14  7405                   -je 0x44dd1b
    if (cpu.flags.zf)
    {
        goto L_0x0044dd1b;
    }
    // 0044dd16  8d5701                 -lea edx, [edi + 1]
    cpu.edx = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 0044dd19  eb02                   -jmp 0x44dd1d
    goto L_0x0044dd1d;
L_0x0044dd1b:
    // 0044dd1b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044dd1d:
    // 0044dd1d  8991f8c26f00           -mov dword ptr [ecx + 0x6fc2f8], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7324408) /* 0x6fc2f8 */) = cpu.edx;
    // 0044dd23  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0044dd2a  69c8a4010000           -imul ecx, eax, 0x1a4
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044dd30  8bba20886700           -mov edi, dword ptr [edx + 0x678820]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785056) /* 0x678820 */);
    // 0044dd36  83ff03                 +cmp edi, 3
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
    // 0044dd39  7405                   -je 0x44dd40
    if (cpu.flags.zf)
    {
        goto L_0x0044dd40;
    }
    // 0044dd3b  8d5701                 -lea edx, [edi + 1]
    cpu.edx = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 0044dd3e  eb02                   -jmp 0x44dd42
    goto L_0x0044dd42;
L_0x0044dd40:
    // 0044dd40  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044dd42:
    // 0044dd42  8991fcc26f00           -mov dword ptr [ecx + 0x6fc2fc], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7324412) /* 0x6fc2fc */) = cpu.edx;
    // 0044dd48  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0044dd4f  8b8a08886700           -mov ecx, dword ptr [edx + 0x678808]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785032) /* 0x678808 */);
    // 0044dd55  83f902                 +cmp ecx, 2
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
    // 0044dd58  7407                   -je 0x44dd61
    if (cpu.flags.zf)
    {
        goto L_0x0044dd61;
    }
    // 0044dd5a  8d5101                 -lea edx, [ecx + 1]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(1) /* 0x1 */);
    // 0044dd5d  8916                   -mov dword ptr [esi], edx
    app->getMemory<x86::reg32>(cpu.esi) = cpu.edx;
    // 0044dd5f  eb06                   -jmp 0x44dd67
    goto L_0x0044dd67;
L_0x0044dd61:
    // 0044dd61  c70600000000           -mov dword ptr [esi], 0
    app->getMemory<x86::reg32>(cpu.esi) = 0 /*0x0*/;
L_0x0044dd67:
    // 0044dd67  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0044dd6e  69c0a4010000           -imul eax, eax, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044dd74  8bb238886700           -mov esi, dword ptr [edx + 0x678838]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6785080) /* 0x678838 */);
    // 0044dd7a  83fe05                 +cmp esi, 5
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
    // 0044dd7d  740b                   -je 0x44dd8a
    if (cpu.flags.zf)
    {
        goto L_0x0044dd8a;
    }
    // 0044dd7f  8d5601                 -lea edx, [esi + 1]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0044dd82  8990f4c26f00           -mov dword ptr [eax + 0x6fc2f4], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7324404) /* 0x6fc2f4 */) = cpu.edx;
    // 0044dd88  eb08                   -jmp 0x44dd92
    goto L_0x0044dd92;
L_0x0044dd8a:
    // 0044dd8a  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044dd8c  89b8f4c26f00           -mov dword ptr [eax + 0x6fc2f4], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7324404) /* 0x6fc2f4 */) = cpu.edi;
L_0x0044dd92:
    // 0044dd92  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044dd97  e824670300             -call 0x4844c0
    cpu.esp -= 4;
    nfs3hp::port::hud::hudPaneSet(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044dd9c  8b5318                 -mov edx, dword ptr [ebx + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */);
    // 0044dd9f  8b4316                 -mov eax, dword ptr [ebx + 0x16]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(22) /* 0x16 */);
    // 0044dda2  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0044dda5  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0044dda8  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0044ddaa  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0044ddb1  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044ddb3  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0044ddb6  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044ddb8  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0044ddbb  05e04e6000             -add eax, 0x604ee0
    (cpu.eax) += x86::reg32(x86::sreg32(6311648 /*0x604ee0*/));
    // 0044ddc0  833805                 +cmp dword ptr [eax], 5
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ddc3  7530                   -jne 0x44ddf5
    if (!cpu.flags.zf)
    {
        goto L_0x0044ddf5;
    }
    // 0044ddc5  ba3c945300             -mov edx, 0x53943c
    cpu.edx = 5477436 /*0x53943c*/;
    // 0044ddca  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0044ddcd  e8ee4bffff             -call 0x4429c0
    cpu.esp -= 4;
    nfs3hp::port::menus::codelinkMatches(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ddd2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044ddd4  741f                   -je 0x44ddf5
    if (cpu.flags.zf)
    {
        goto L_0x0044ddf5;
    }
    // 0044ddd6  b96c170000             -mov ecx, 0x176c
    cpu.ecx = 5996 /*0x176c*/;
    // 0044dddb  bea02c6600             -mov esi, 0x662ca0
    cpu.esi = 6696096 /*0x662ca0*/;
    // 0044dde0  bf4cbb6f00             -mov edi, 0x6fbb4c
    cpu.edi = 7322444 /*0x6fbb4c*/;
    // 0044dde5  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044dde6  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044dde8  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044ddeb  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044dded  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044ddef  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044ddf2  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044ddf4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0044ddf5:
    // 0044ddf5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044ddf7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ddf8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ddf9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ddfa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ddfb  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ddfc  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ddfd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_44de00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044de00  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044de01  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044de03  6683fb0d               +cmp bx, 0xd
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(13 /*0xd*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0044de07  750a                   -jne 0x44de13
    if (!cpu.flags.zf)
    {
        goto L_0x0044de13;
    }
    // 0044de09  c7050c44660001000000   -mov dword ptr [0x66440c], 1
    app->getMemory<x86::reg32>(x86::reg32(6702092) /* 0x66440c */) = 1 /*0x1*/;
L_0x0044de13:
    // 0044de13  0fbfdb                 -movsx ebx, bx
    cpu.ebx = x86::reg32(static_cast<x86::sreg16>(cpu.bx));
    // 0044de16  e855340100             -call 0x461270
    cpu.esp -= 4;
    nfs3hp::port::buttons::buttonKey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044de1b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044de1c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_44de20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044de20  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044de21  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044de23  6683fb0d               +cmp bx, 0xd
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(13 /*0xd*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0044de27  754c                   -jne 0x44de75
    if (!cpu.flags.zf)
    {
        goto L_0x0044de75;
    }
    // 0044de29  e852780300             -call 0x485680
    cpu.esp -= 4;
    sub_485680(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044de2e  668b5a18               -mov bx, word ptr [edx + 0x18]
    cpu.bx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(24) /* 0x18 */);
L_0x0044de32:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044de32  0fbfd3                 -movsx edx, bx
    cpu.edx = x86::reg32(static_cast<x86::sreg16>(cpu.bx));
    // 0044de35  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0044de3c  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044de3e  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0044de41  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0044de43  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0044de46  83b8e04e600000         +cmp dword ptr [eax + 0x604ee0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6311648) /* 0x604ee0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044de4d  741f                   -je 0x44de6e
    if (cpu.flags.zf)
    {
        goto L_0x0044de6e;
    }
    // 0044de4f  bae04e6000             -mov edx, 0x604ee0
    cpu.edx = 6311648 /*0x604ee0*/;
    // 0044de54  01c2                   +add edx, eax
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
    // 0044de56  7413                   -je 0x44de6b
    if (cpu.flags.zf)
    {
        goto L_0x0044de6b;
    }
    // 0044de58  833a1f                 +cmp dword ptr [edx], 0x1f
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(31 /*0x1f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044de5b  750e                   -jne 0x44de6b
    if (!cpu.flags.zf)
    {
        goto L_0x0044de6b;
    }
    // 0044de5d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0044de5f  e82cec0000             -call 0x45ca90
    cpu.esp -= 4;
    sub_45ca90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044de64  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0044de66  e865eb0000             -call 0x45c9d0
    cpu.esp -= 4;
    sub_45c9d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044de6b:
    // 0044de6b  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044de6c  ebc4                   -jmp 0x44de32
    goto L_0x0044de32;
L_0x0044de6e:
    // 0044de6e  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044de73  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044de74  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044de75:
    // 0044de75  0fbfdb                 -movsx ebx, bx
    cpu.ebx = x86::reg32(static_cast<x86::sreg16>(cpu.bx));
    // 0044de78  e8f3330100             -call 0x461270
    cpu.esp -= 4;
    nfs3hp::port::buttons::buttonKey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044de7d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044de7e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_44de80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044de80  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044de81  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044de82  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044de83  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044de84  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044de85  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044de86  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044de88  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0044de8b  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0044de8d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044de8f  e83c4dfdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044de94  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044de9b  7407                   -je 0x44dea4
    if (cpu.flags.zf)
    {
        goto L_0x0044dea4;
    }
    // 0044de9d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044dea2  eb02                   -jmp 0x44dea6
    goto L_0x0044dea6;
L_0x0044dea4:
    // 0044dea4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044dea6:
    // 0044dea6  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044dea8  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044dead  e81e4dfdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044deb2  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044deb9  7407                   -je 0x44dec2
    if (cpu.flags.zf)
    {
        goto L_0x0044dec2;
    }
    // 0044debb  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044dec0  eb02                   -jmp 0x44dec4
    goto L_0x0044dec4;
L_0x0044dec2:
    // 0044dec2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044dec4:
    // 0044dec4  689a99193f             -push 0x3f19999a
    app->getMemory<x86::reg32>(cpu.esp-4) = 1058642330 /*0x3f19999a*/;
    cpu.esp -= 4;
    // 0044dec9  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044dece  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0044ded1  e85a360000             -call 0x451530
    cpu.esp -= 4;
    nfs3hp::port::text::dimBackground(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ded6  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044ded8  891564925500           -mov dword ptr [0x559264], edx
    app->getMemory<x86::reg32>(x86::reg32(5608036) /* 0x559264 */) = cpu.edx;
    // 0044dede  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044dee0  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 0044dee5  890d0c446600           -mov dword ptr [0x66440c], ecx
    app->getMemory<x86::reg32>(x86::reg32(6702092) /* 0x66440c */) = cpu.ecx;
    // 0044deeb  e8705afdff             -call 0x423960
    cpu.esp -= 4;
    nfs3hp::carstate::setOverrideMode(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044def0  8b3db0d36f00           -mov edi, dword ptr [0x6fd3b0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0044def6  83ff01                 +cmp edi, 1
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
    // 0044def9  750c                   -jne 0x44df07
    if (!cpu.flags.zf)
    {
        goto L_0x0044df07;
    }
    // 0044defb  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 0044df00  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0044df02  e8595afdff             -call 0x423960
    cpu.esp -= 4;
    nfs3hp::carstate::setOverrideMode(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044df07:
    // 0044df07  69f6a4010000           -imul esi, esi, 0x1a4
    cpu.esi = x86::reg32(x86::sreg64(x86::sreg32(cpu.esi)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044df0d  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044df14  7540                   -jne 0x44df56
    if (!cpu.flags.zf)
    {
        goto L_0x0044df56;
    }
    // 0044df16  b970010000             -mov ecx, 0x170
    cpu.ecx = 368 /*0x170*/;
    // 0044df1b  81c698bf6f00           -add esi, 0x6fbf98
    (cpu.esi) += x86::reg32(x86::sreg32(7323544 /*0x6fbf98*/));
    // 0044df21  bf90296600             -mov edi, 0x662990
    cpu.edi = 6695312 /*0x662990*/;
    // 0044df26  83c624                 -add esi, 0x24
    (cpu.esi) += x86::reg32(x86::sreg32(36 /*0x24*/));
    // 0044df29  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044df2c  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044df2d  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044df2f  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044df32  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044df34  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044df36  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044df39  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044df3b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044df3c  69c2a4010000           -imul eax, edx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044df42  b970010000             -mov ecx, 0x170
    cpu.ecx = 368 /*0x170*/;
    // 0044df47  05e0c26f00             +add eax, 0x6fc2e0
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7324384 /*0x6fc2e0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0044df4c  bf002b6600             -mov edi, 0x662b00
    cpu.edi = 6695680 /*0x662b00*/;
    // 0044df51  8d7024                 -lea esi, [eax + 0x24]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(36) /* 0x24 */);
    // 0044df54  eb13                   -jmp 0x44df69
    goto L_0x0044df69;
L_0x0044df56:
    // 0044df56  b970010000             -mov ecx, 0x170
    cpu.ecx = 368 /*0x170*/;
    // 0044df5b  81c650bc6f00           -add esi, 0x6fbc50
    (cpu.esi) += x86::reg32(x86::sreg32(7322704 /*0x6fbc50*/));
    // 0044df61  bf90296600             -mov edi, 0x662990
    cpu.edi = 6695312 /*0x662990*/;
    // 0044df66  83c624                 -add esi, 0x24
    (cpu.esi) += x86::reg32(x86::sreg32(36 /*0x24*/));
L_0x0044df69:
    // 0044df69  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044df6a  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044df6c  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044df6f  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044df71  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044df73  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044df76  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044df78  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044df79  ba84945300             -mov edx, 0x539484
    cpu.edx = 5477508 /*0x539484*/;
    // 0044df7e  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044df80  e8bb4affff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044df85  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044df87  7407                   -je 0x44df90
    if (cpu.flags.zf)
    {
        goto L_0x0044df90;
    }
    // 0044df89  c7403000de4400         -mov dword ptr [eax + 0x30], 0x44de00
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */) = 4513280 /*0x44de00*/;
L_0x0044df90:
    // 0044df90  ba8c945300             -mov edx, 0x53948c
    cpu.edx = 5477516 /*0x53948c*/;
    // 0044df95  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044df97  e8a44affff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044df9c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044df9e  7407                   -je 0x44dfa7
    if (cpu.flags.zf)
    {
        goto L_0x0044dfa7;
    }
    // 0044dfa0  c7403020de4400         -mov dword ptr [eax + 0x30], 0x44de20
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */) = 4513312 /*0x44de20*/;
L_0x0044dfa7:
    // 0044dfa7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044dfa9  e8f2200300             -call 0x4800a0
    cpu.esp -= 4;
    nfs3hp::render::hudBand(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044dfae  8b0db0d36f00           -mov ecx, dword ptr [0x6fd3b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0044dfb4  83f901                 +cmp ecx, 1
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
    // 0044dfb7  7507                   -jne 0x44dfc0
    if (!cpu.flags.zf)
    {
        goto L_0x0044dfc0;
    }
    // 0044dfb9  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044dfbb  e8e0200300             -call 0x4800a0
    cpu.esp -= 4;
    nfs3hp::render::hudBand(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044dfc0:
    // 0044dfc0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044dfc2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0044dfc4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dfc5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dfc6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dfc7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dfc8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dfc9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044dfca  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_44dfd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044dfd0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044dfd1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044dfd2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044dfd3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044dfd4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044dfd5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044dfd6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044dfd8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044dfda  e8f14bfdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044dfdf  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044dfe6  7407                   -je 0x44dfef
    if (cpu.flags.zf)
    {
        goto L_0x0044dfef;
    }
    // 0044dfe8  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044dfed  eb02                   -jmp 0x44dff1
    goto L_0x0044dff1;
L_0x0044dfef:
    // 0044dfef  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044dff1:
    // 0044dff1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044dff3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044dff8  e8d34bfdff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044dffd  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044e004  7407                   -je 0x44e00d
    if (cpu.flags.zf)
    {
        goto L_0x0044e00d;
    }
    // 0044e006  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0044e00b  eb02                   -jmp 0x44e00f
    goto L_0x0044e00f;
L_0x0044e00d:
    // 0044e00d  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x0044e00f:
    // 0044e00f  833d0c44660000         +cmp dword ptr [0x66440c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6702092) /* 0x66440c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e016  0f85a8000000           -jne 0x44e0c4
    if (!cpu.flags.zf)
    {
        goto L_0x0044e0c4;
    }
    // 0044e01c  69c9a4010000           -imul ecx, ecx, 0x1a4
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044e022  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044e029  753b                   -jne 0x44e066
    if (!cpu.flags.zf)
    {
        goto L_0x0044e066;
    }
    // 0044e02b  b898bf6f00             -mov eax, 0x6fbf98
    cpu.eax = 7323544 /*0x6fbf98*/;
    // 0044e030  be90296600             -mov esi, 0x662990
    cpu.esi = 6695312 /*0x662990*/;
    // 0044e035  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0044e037  b970010000             -mov ecx, 0x170
    cpu.ecx = 368 /*0x170*/;
    // 0044e03c  8d7824                 -lea edi, [eax + 0x24]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(36) /* 0x24 */);
    // 0044e03f  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044e040  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044e042  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044e045  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044e047  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044e049  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044e04c  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044e04e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e04f  69c3a4010000           -imul eax, ebx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0044e055  b970010000             -mov ecx, 0x170
    cpu.ecx = 368 /*0x170*/;
    // 0044e05a  05e0c26f00             +add eax, 0x6fc2e0
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7324384 /*0x6fc2e0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0044e05f  be002b6600             -mov esi, 0x662b00
    cpu.esi = 6695680 /*0x662b00*/;
    // 0044e064  eb11                   -jmp 0x44e077
    goto L_0x0044e077;
L_0x0044e066:
    // 0044e066  b850bc6f00             -mov eax, 0x6fbc50
    cpu.eax = 7322704 /*0x6fbc50*/;
    // 0044e06b  be90296600             -mov esi, 0x662990
    cpu.esi = 6695312 /*0x662990*/;
    // 0044e070  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0044e072  b970010000             -mov ecx, 0x170
    cpu.ecx = 368 /*0x170*/;
L_0x0044e077:
    // 0044e077  8d7824                 -lea edi, [eax + 0x24]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(36) /* 0x24 */);
    // 0044e07a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044e07b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044e07d  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0044e080  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044e082  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0044e084  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0044e087  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 0044e089  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e08a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0044e08c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044e08c  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044e093  7507                   -jne 0x44e09c
    if (!cpu.flags.zf)
    {
        goto L_0x0044e09c;
    }
    // 0044e095  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e09a  eb05                   -jmp 0x44e0a1
    goto L_0x0044e0a1;
L_0x0044e09c:
    // 0044e09c  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0044e0a1:
    // 0044e0a1  39c1                   +cmp ecx, eax
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
    // 0044e0a3  7d1f                   -jge 0x44e0c4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0044e0c4;
    }
    // 0044e0a5  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0044e0a7  eb05                   -jmp 0x44e0ae
    goto L_0x0044e0ae;
L_0x0044e0a9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044e0a9  83fb17                 +cmp ebx, 0x17
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(23 /*0x17*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e0ac  7d0c                   -jge 0x44e0ba
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0044e0ba;
    }
L_0x0044e0ae:
    // 0044e0ae  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0044e0b0  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044e0b2  e809730300             -call 0x4853c0
    cpu.esp -= 4;
    sub_4853c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e0b7  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044e0b8  ebef                   -jmp 0x44e0a9
    goto L_0x0044e0a9;
L_0x0044e0ba:
    // 0044e0ba  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044e0bc  e8df1f0300             -call 0x4800a0
    cpu.esp -= 4;
    nfs3hp::render::hudBand(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e0c1  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044e0c2  ebc8                   -jmp 0x44e08c
    goto L_0x0044e08c;
L_0x0044e0c4:
    // 0044e0c4  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0044e0c9  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e0cb  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044e0cd  e85e3f0300             -call 0x482030
    cpu.esp -= 4;
    sub_482030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e0d2  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044e0d7  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0044e0dc  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e0de  e84d3f0300             -call 0x482030
    cpu.esp -= 4;
    sub_482030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e0e3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e0e5  ba17000000             -mov edx, 0x17
    cpu.edx = 23 /*0x17*/;
    // 0044e0ea  a364925500             -mov dword ptr [0x559264], eax
    app->getMemory<x86::reg32>(x86::reg32(5608036) /* 0x559264 */) = cpu.eax;
    // 0044e0ef  e86c58fdff             -call 0x423960
    cpu.esp -= 4;
    nfs3hp::carstate::setOverrideMode(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e0f4  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044e0fb  750f                   -jne 0x44e10c
    if (!cpu.flags.zf)
    {
        goto L_0x0044e10c;
    }
    // 0044e0fd  ba17000000             -mov edx, 0x17
    cpu.edx = 23 /*0x17*/;
    // 0044e102  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044e107  e85458fdff             -call 0x423960
    cpu.esp -= 4;
    nfs3hp::carstate::setOverrideMode(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044e10c:
    // 0044e10c  8b0db0d36f00           -mov ecx, dword ptr [0x6fd3b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0044e112  83f901                 +cmp ecx, 1
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
    // 0044e115  750e                   -jne 0x44e125
    if (!cpu.flags.zf)
    {
        goto L_0x0044e125;
    }
    // 0044e117  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e119  e8a2630300             -call 0x4844c0
    cpu.esp -= 4;
    nfs3hp::port::hud::hudPaneSet(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e11e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044e120  e89b630300             -call 0x4844c0
    cpu.esp -= 4;
    nfs3hp::port::hud::hudPaneSet(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044e125:
    // 0044e125  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e127  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e128  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e129  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e12a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e12b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e12c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e12d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_44e130(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e130  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e131  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e133  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e135  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e136  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_44e140(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e140  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e141  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e143  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e145  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e146  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
/* data blob: 000066006700680069006a006b006c006d006e0000008d80000000008d542200 */
void Application::sub_44e170(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e170  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044e171  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044e172  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044e173  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e174  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e176  e8d5360800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e17b  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044e17d  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0044e182  bb00040000             -mov ebx, 0x400
    cpu.ebx = 1024 /*0x400*/;
    // 0044e187  baa8446600             -mov edx, 0x6644a8
    cpu.edx = 6702248 /*0x6644a8*/;
    // 0044e18c  a3a8446600             -mov dword ptr [0x6644a8], eax
    app->getMemory<x86::reg32>(x86::reg32(6702248) /* 0x6644a8 */) = cpu.eax;
    // 0044e191  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044e193  e8e868ffff             -call 0x444a80
    cpu.esp -= 4;
    nfs3hp::frontend::dialogOpenMessage(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e198  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e199  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e19a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e19b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e19c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_44e1a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e1a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044e1a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044e1a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044e1a3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044e1a4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044e1a5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e1a6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e1a8  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0044e1ab  8b15b0d36f00           -mov edx, dword ptr [0x6fd3b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0044e1b1  83fa01                 +cmp edx, 1
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
    // 0044e1b4  7504                   -jne 0x44e1ba
    if (!cpu.flags.zf)
    {
        goto L_0x0044e1ba;
    }
    // 0044e1b6  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0044e1b8  eb02                   -jmp 0x44e1bc
    goto L_0x0044e1bc;
L_0x0044e1ba:
    // 0044e1ba  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x0044e1bc:
    // 0044e1bc  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e1be  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044e1c0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e1c2  8975f8                 -mov dword ptr [ebp - 8], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.esi;
    // 0044e1c5  e8467a0000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e1ca  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044e1cc  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x0044e1ce:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044e1ce  3b7df8                 +cmp edi, dword ptr [ebp - 8]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e1d1  0f8f71020000           -jg 0x44e448
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0044e448;
    }
    // 0044e1d7  8b1df4d46f00           -mov ebx, dword ptr [0x6fd4f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 0044e1dd  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044e1e2  01fb                   -add ebx, edi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edi));
    // 0044e1e4  83f802                 +cmp eax, 2
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
    // 0044e1e7  7210                   -jb 0x44e1f9
    if (cpu.flags.cf)
    {
        goto L_0x0044e1f9;
    }
    // 0044e1e9  761c                   -jbe 0x44e207
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044e207;
    }
    // 0044e1eb  83f803                 +cmp eax, 3
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
    // 0044e1ee  0f845d010000           -je 0x44e351
    if (cpu.flags.zf)
    {
        goto L_0x0044e351;
    }
    // 0044e1f4  e949020000             -jmp 0x44e442
    goto L_0x0044e442;
L_0x0044e1f9:
    // 0044e1f9  83f801                 +cmp eax, 1
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
    // 0044e1fc  0f8488000000           -je 0x44e28a
    if (cpu.flags.zf)
    {
        goto L_0x0044e28a;
    }
    // 0044e202  e93b020000             -jmp 0x44e442
    goto L_0x0044e442;
L_0x0044e207:
    // 0044e207  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0044e209  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e20b  e8d0720000             -call 0x4554e0
    cpu.esp -= 4;
    sub_4554e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e210  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044e212  7503                   -jne 0x44e217
    if (!cpu.flags.zf)
    {
        goto L_0x0044e217;
    }
    // 0044e214  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044e215  ebb7                   -jmp 0x44e1ce
    goto L_0x0044e1ce;
L_0x0044e217:
    // 0044e217  8b1dacd46f00           -mov ebx, dword ptr [0x6fd4ac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 0044e21d  83fb08                 +cmp ebx, 8
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
    // 0044e220  0f851c020000           -jne 0x44e442
    if (!cpu.flags.zf)
    {
        goto L_0x0044e442;
    }
    // 0044e226  83b8c402000002         +cmp dword ptr [eax + 0x2c4], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e22d  0f840f020000           -je 0x44e442
    if (cpu.flags.zf)
    {
        goto L_0x0044e442;
    }
    // 0044e233  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044e235  e82649ffff             -call 0x442b60
    cpu.esp -= 4;
    sub_442b60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e23a  833dacd26f0000         +cmp dword ptr [0x6fd2ac], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328428) /* 0x6fd2ac */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e241  7505                   -jne 0x44e248
    if (!cpu.flags.zf)
    {
        goto L_0x0044e248;
    }
    // 0044e243  beb7010000             -mov esi, 0x1b7
    cpu.esi = 439 /*0x1b7*/;
L_0x0044e248:
    // 0044e248  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0044e24d  a170d46f00             -mov eax, dword ptr [0x6fd470]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328880) /* 0x6fd470 */);
    // 0044e252  891dacd26f00           -mov dword ptr [0x6fd2ac], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328428) /* 0x6fd2ac */) = cpu.ebx;
    // 0044e258  39d8                   +cmp eax, ebx
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
    // 0044e25a  0f85e2010000           -jne 0x44e442
    if (!cpu.flags.zf)
    {
        goto L_0x0044e442;
    }
    // 0044e260  f605a8d26f0001         +test byte ptr [0x6fd2a8], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(7328424) /* 0x6fd2a8 */) & 1 /*0x1*/));
    // 0044e267  7514                   -jne 0x44e27d
    if (!cpu.flags.zf)
    {
        goto L_0x0044e27d;
    }
    // 0044e269  81feb7010000           +cmp esi, 0x1b7
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(439 /*0x1b7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e26f  7507                   -jne 0x44e278
    if (!cpu.flags.zf)
    {
        goto L_0x0044e278;
    }
    // 0044e271  beb5010000             -mov esi, 0x1b5
    cpu.esi = 437 /*0x1b5*/;
    // 0044e276  eb05                   -jmp 0x44e27d
    goto L_0x0044e27d;
L_0x0044e278:
    // 0044e278  beb6010000             -mov esi, 0x1b6
    cpu.esi = 438 /*0x1b6*/;
L_0x0044e27d:
    // 0044e27d  800da8d26f0001         +or byte ptr [0x6fd2a8], 1
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(x86::reg32(7328424) /* 0x6fd2a8 */) |= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 0044e284  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044e285  e944ffffff             -jmp 0x44e1ce
    goto L_0x0044e1ce;
L_0x0044e28a:
    // 0044e28a  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0044e28c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e28e  e84d720000             -call 0x4554e0
    cpu.esp -= 4;
    sub_4554e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e293  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0044e296  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044e298  7506                   -jne 0x44e2a0
    if (!cpu.flags.zf)
    {
        goto L_0x0044e2a0;
    }
    // 0044e29a  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044e29b  e92effffff             -jmp 0x44e1ce
    goto L_0x0044e1ce;
L_0x0044e2a0:
    // 0044e2a0  8b0d20d05500           -mov ecx, dword ptr [0x55d020]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5623840) /* 0x55d020 */);
    // 0044e2a6  3b0dacd46f00           +cmp ecx, dword ptr [0x6fd4ac]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e2ac  0f8590010000           -jne 0x44e442
    if (!cpu.flags.zf)
    {
        goto L_0x0044e442;
    }
    // 0044e2b2  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044e2b4  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0044e2b9  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 0044e2be  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044e2c0  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044e2c2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e2c4  e8077b0000             -call 0x455dd0
    cpu.esp -= 4;
    sub_455dd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e2c9  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e2cb  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0044e2ce  895dec                 -mov dword ptr [ebp - 0x14], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.ebx;
    // 0044e2d1  895df0                 -mov dword ptr [ebp - 0x10], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ebx;
    // 0044e2d4  895df4                 -mov dword ptr [ebp - 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;
    // 0044e2d7  8d5dec                 -lea ebx, [ebp - 0x14]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0044e2da  e851720000             -call 0x455530
    cpu.esp -= 4;
    sub_455530(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e2df  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0044e2e2  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0044e2e5  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044e2e8  e843720000             -call 0x455530
    cpu.esp -= 4;
    sub_455530(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e2ed  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0044e2f0  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0044e2f3  39d8                   +cmp eax, ebx
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
    // 0044e2f5  0f8c47010000           -jl 0x44e442
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0044e442;
    }
    // 0044e2fb  8a35a8d26f00           -mov dh, byte ptr [0x6fd2a8]
    cpu.dh = app->getMemory<x86::reg8>(x86::reg32(7328424) /* 0x6fd2a8 */);
    // 0044e301  f6c604                 +test dh, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dh & 4 /*0x4*/));
    // 0044e304  7510                   -jne 0x44e316
    if (!cpu.flags.zf)
    {
        goto L_0x0044e316;
    }
    // 0044e306  88f3                   -mov bl, dh
    cpu.bl = cpu.dh;
    // 0044e308  80cb04                 -or bl, 4
    cpu.bl |= x86::reg8(x86::sreg8(4 /*0x4*/));
    // 0044e30b  beb4010000             -mov esi, 0x1b4
    cpu.esi = 436 /*0x1b4*/;
    // 0044e310  881da8d26f00           -mov byte ptr [0x6fd2a8], bl
    app->getMemory<x86::reg8>(x86::reg32(7328424) /* 0x6fd2a8 */) = cpu.bl;
L_0x0044e316:
    // 0044e316  833d70d46f0001         +cmp dword ptr [0x6fd470], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328880) /* 0x6fd470 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e31d  0f851f010000           -jne 0x44e442
    if (!cpu.flags.zf)
    {
        goto L_0x0044e442;
    }
    // 0044e323  f605a8d26f0002         +test byte ptr [0x6fd2a8], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(7328424) /* 0x6fd2a8 */) & 2 /*0x2*/));
    // 0044e32a  0f8512010000           -jne 0x44e442
    if (!cpu.flags.zf)
    {
        goto L_0x0044e442;
    }
    // 0044e330  81feb4010000           +cmp esi, 0x1b4
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(436 /*0x1b4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e336  7507                   -jne 0x44e33f
    if (!cpu.flags.zf)
    {
        goto L_0x0044e33f;
    }
    // 0044e338  beb2010000             -mov esi, 0x1b2
    cpu.esi = 434 /*0x1b2*/;
    // 0044e33d  eb05                   -jmp 0x44e344
    goto L_0x0044e344;
L_0x0044e33f:
    // 0044e33f  beb3010000             -mov esi, 0x1b3
    cpu.esi = 435 /*0x1b3*/;
L_0x0044e344:
    // 0044e344  800da8d26f0002         +or byte ptr [0x6fd2a8], 2
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(x86::reg32(7328424) /* 0x6fd2a8 */) |= x86::reg8(x86::sreg8(2 /*0x2*/))));
    // 0044e34b  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044e34c  e97dfeffff             -jmp 0x44e1ce
    goto L_0x0044e1ce;
L_0x0044e351:
    // 0044e351  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044e356  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0044e358  e883710000             -call 0x4554e0
    cpu.esp -= 4;
    sub_4554e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e35d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044e35f  745d                   -je 0x44e3be
    if (cpu.flags.zf)
    {
        goto L_0x0044e3be;
    }
    // 0044e361  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044e368  7454                   -je 0x44e3be
    if (cpu.flags.zf)
    {
        goto L_0x0044e3be;
    }
    // 0044e36a  8b15b8d46f00           -mov edx, dword ptr [0x6fd4b8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */);
    // 0044e370  3b90cc020000           +cmp edx, dword ptr [eax + 0x2cc]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(716) /* 0x2cc */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e376  7f46                   -jg 0x44e3be
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0044e3be;
    }
    // 0044e378  833dacd46f0008         +cmp dword ptr [0x6fd4ac], 8
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e37f  7f1d                   -jg 0x44e39e
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0044e39e;
    }
    // 0044e381  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044e386  8a0dacd46f00           -mov cl, byte ptr [0x6fd4ac]
    cpu.cl = app->getMemory<x86::reg8>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 0044e38c  668b15b0d26f00         -mov dx, word ptr [0x6fd2b0]
    cpu.dx = app->getMemory<x86::reg16>(x86::reg32(7328432) /* 0x6fd2b0 */);
    // 0044e393  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 0044e395  09c2                   -or edx, eax
    cpu.edx |= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e397  668915b0d26f00         -mov word ptr [0x6fd2b0], dx
    app->getMemory<x86::reg16>(x86::reg32(7328432) /* 0x6fd2b0 */) = cpu.dx;
L_0x0044e39e:
    // 0044e39e  66813db0d26f00ff01     +cmp word ptr [0x6fd2b0], 0x1ff
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328432) /* 0x6fd2b0 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(511 /*0x1ff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0044e3a7  7515                   -jne 0x44e3be
    if (!cpu.flags.zf)
    {
        goto L_0x0044e3be;
    }
    // 0044e3a9  f605a8d26f0008         +test byte ptr [0x6fd2a8], 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(7328424) /* 0x6fd2a8 */) & 8 /*0x8*/));
    // 0044e3b0  7505                   -jne 0x44e3b7
    if (!cpu.flags.zf)
    {
        goto L_0x0044e3b7;
    }
    // 0044e3b2  beb8010000             -mov esi, 0x1b8
    cpu.esi = 440 /*0x1b8*/;
L_0x0044e3b7:
    // 0044e3b7  800da8d26f0008         -or byte ptr [0x6fd2a8], 8
    app->getMemory<x86::reg8>(x86::reg32(7328424) /* 0x6fd2a8 */) |= x86::reg8(x86::sreg8(8 /*0x8*/));
L_0x0044e3be:
    // 0044e3be  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0044e3c0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e3c2  e819710000             -call 0x4554e0
    cpu.esp -= 4;
    sub_4554e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e3c7  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044e3c9  0f8473000000           -je 0x44e442
    if (cpu.flags.zf)
    {
        goto L_0x0044e442;
    }
    // 0044e3cf  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044e3d6  7406                   -je 0x44e3de
    if (cpu.flags.zf)
    {
        goto L_0x0044e3de;
    }
    // 0044e3d8  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044e3d9  e9f0fdffff             -jmp 0x44e1ce
    goto L_0x0044e1ce;
L_0x0044e3de:
    // 0044e3de  83b8bc02000001         +cmp dword ptr [eax + 0x2bc], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(700) /* 0x2bc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e3e5  7406                   -je 0x44e3ed
    if (cpu.flags.zf)
    {
        goto L_0x0044e3ed;
    }
    // 0044e3e7  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044e3e8  e9e1fdffff             -jmp 0x44e1ce
    goto L_0x0044e1ce;
L_0x0044e3ed:
    // 0044e3ed  83b8c402000004         +cmp dword ptr [eax + 0x2c4], 4
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
    // 0044e3f4  7506                   -jne 0x44e3fc
    if (!cpu.flags.zf)
    {
        goto L_0x0044e3fc;
    }
    // 0044e3f6  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044e3f7  e9d2fdffff             -jmp 0x44e1ce
    goto L_0x0044e1ce;
L_0x0044e3fc:
    // 0044e3fc  833dacd46f0008         +cmp dword ptr [0x6fd4ac], 8
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e403  7f1d                   -jg 0x44e422
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0044e422;
    }
    // 0044e405  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044e40a  8a0dacd46f00           -mov cl, byte ptr [0x6fd4ac]
    cpu.cl = app->getMemory<x86::reg8>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 0044e410  668b15b2d26f00         -mov dx, word ptr [0x6fd2b2]
    cpu.dx = app->getMemory<x86::reg16>(x86::reg32(7328434) /* 0x6fd2b2 */);
    // 0044e417  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 0044e419  09c2                   -or edx, eax
    cpu.edx |= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e41b  668915b2d26f00         -mov word ptr [0x6fd2b2], dx
    app->getMemory<x86::reg16>(x86::reg32(7328434) /* 0x6fd2b2 */) = cpu.dx;
L_0x0044e422:
    // 0044e422  66813db2d26f00ff01     +cmp word ptr [0x6fd2b2], 0x1ff
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328434) /* 0x6fd2b2 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(511 /*0x1ff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0044e42b  7515                   -jne 0x44e442
    if (!cpu.flags.zf)
    {
        goto L_0x0044e442;
    }
    // 0044e42d  f605a8d26f0010         +test byte ptr [0x6fd2a8], 0x10
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(7328424) /* 0x6fd2a8 */) & 16 /*0x10*/));
    // 0044e434  7505                   -jne 0x44e43b
    if (!cpu.flags.zf)
    {
        goto L_0x0044e43b;
    }
    // 0044e436  beb9010000             -mov esi, 0x1b9
    cpu.esi = 441 /*0x1b9*/;
L_0x0044e43b:
    // 0044e43b  800da8d26f0010         +or byte ptr [0x6fd2a8], 0x10
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(x86::reg32(7328424) /* 0x6fd2a8 */) |= x86::reg8(x86::sreg8(16 /*0x10*/))));
L_0x0044e442:
    // 0044e442  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044e443  e986fdffff             -jmp 0x44e1ce
    goto L_0x0044e1ce;
L_0x0044e448:
    // 0044e448  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044e44a  7407                   -je 0x44e453
    if (cpu.flags.zf)
    {
        goto L_0x0044e453;
    }
    // 0044e44c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0044e44e  e81dfdffff             -call 0x44e170
    cpu.esp -= 4;
    sub_44e170(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044e453:
    // 0044e453  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0044e455  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e456  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e457  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e458  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e459  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e45a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e45b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_44e460(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e460  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e461  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e463  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e465  e8c695ffff             -call 0x447a30
    cpu.esp -= 4;
    sub_447a30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e46a  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e46f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e470  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_44e480(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e480  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e481  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e483  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044e488  e8a395ffff             -call 0x447a30
    cpu.esp -= 4;
    sub_447a30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e48d  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e492  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e493  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_44e4a0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e4a0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e4a1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e4a3  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 0044e4a8  e88395ffff             -call 0x447a30
    cpu.esp -= 4;
    sub_447a30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e4ad  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e4b2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e4b3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_44e4c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e4c0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044e4c1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e4c2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e4c4  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044e4cb  7e34                   -jle 0x44e501
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0044e501;
    }
    // 0044e4cd  833de0227a0000         +cmp dword ptr [0x7a22e0], 0
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
    // 0044e4d4  742b                   -je 0x44e501
    if (cpu.flags.zf)
    {
        goto L_0x0044e501;
    }
    // 0044e4d6  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 0044e4de  7521                   -jne 0x44e501
    if (!cpu.flags.zf)
    {
        goto L_0x0044e501;
    }
    // 0044e4e0  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0044e4e8  7412                   -je 0x44e4fc
    if (cpu.flags.zf)
    {
        goto L_0x0044e4fc;
    }
    // 0044e4ea  ba21000000             -mov edx, 0x21
    cpu.edx = 33 /*0x21*/;
    // 0044e4ef  b828000000             -mov eax, 0x28
    cpu.eax = 40 /*0x28*/;
    // 0044e4f4  e817850500             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e4f9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e4fa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e4fb  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044e4fc:
    // 0044e4fc  e8df800500             -call 0x4a65e0
    cpu.esp -= 4;
    nfs3hp::ea::closeNetwork(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044e501:
    // 0044e501  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e502  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e503  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_44e510(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e510  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e511  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e513  c60200                 -mov byte ptr [edx], 0
    app->getMemory<x86::reg8>(cpu.edx) = 0 /*0x0*/;
    // 0044e516  83f803                 +cmp eax, 3
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
    // 0044e519  7507                   -jne 0x44e522
    if (!cpu.flags.zf)
    {
        goto L_0x0044e522;
    }
    // 0044e51b  c60201                 -mov byte ptr [edx], 1
    app->getMemory<x86::reg8>(cpu.edx) = 1 /*0x1*/;
    // 0044e51e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e520  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e521  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044e522:
    // 0044e522  83f801                 +cmp eax, 1
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
    // 0044e525  7520                   -jne 0x44e547
    if (!cpu.flags.zf)
    {
        goto L_0x0044e547;
    }
    // 0044e527  e89461ffff             -call 0x4446c0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogFocusedButton(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e52c  83f801                 +cmp eax, 1
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
    // 0044e52f  7507                   -jne 0x44e538
    if (!cpu.flags.zf)
    {
        goto L_0x0044e538;
    }
    // 0044e531  c60201                 -mov byte ptr [edx], 1
    app->getMemory<x86::reg8>(cpu.edx) = 1 /*0x1*/;
    // 0044e534  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e536  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e537  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044e538:
    // 0044e538  c60201                 -mov byte ptr [edx], 1
    app->getMemory<x86::reg8>(cpu.edx) = 1 /*0x1*/;
    // 0044e53b  e880ffffff             -call 0x44e4c0
    cpu.esp -= 4;
    sub_44e4c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e540  b80b000000             -mov eax, 0xb
    cpu.eax = 11 /*0xb*/;
    // 0044e545  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e546  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044e547:
    // 0044e547  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e549  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e54a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_44e550(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e550  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044e551  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044e552  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044e553  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e554  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e556  e8f5320800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e55b  a3b4446600             -mov dword ptr [0x6644b4], eax
    app->getMemory<x86::reg32>(x86::reg32(6702260) /* 0x6644b4 */) = cpu.eax;
    // 0044e560  b8f9000000             -mov eax, 0xf9
    cpu.eax = 249 /*0xf9*/;
    // 0044e565  e8e6320800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e56a  a3ac446600             -mov dword ptr [0x6644ac], eax
    app->getMemory<x86::reg32>(x86::reg32(6702252) /* 0x6644ac */) = cpu.eax;
    // 0044e56f  b8c6000000             -mov eax, 0xc6
    cpu.eax = 198 /*0xc6*/;
    // 0044e574  e8d7320800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e579  6810e54400             -push 0x44e510
    app->getMemory<x86::reg32>(cpu.esp-4) = 4515088 /*0x44e510*/;
    cpu.esp -= 4;
    // 0044e57e  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044e580  b9ac446600             -mov ecx, 0x6644ac
    cpu.ecx = 6702252 /*0x6644ac*/;
    // 0044e585  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0044e587  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 0044e58c  bab4446600             -mov edx, 0x6644b4
    cpu.edx = 6702260 /*0x6644b4*/;
    // 0044e591  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044e593  a3b0446600             -mov dword ptr [0x6644b0], eax
    app->getMemory<x86::reg32>(x86::reg32(6702256) /* 0x6644b0 */) = cpu.eax;
    // 0044e598  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044e59d  e82e61ffff             -call 0x4446d0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogOpenQuestion(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e5a2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e5a3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e5a4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e5a5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e5a6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_44e5b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e5b0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e5b1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e5b3  e8d80a0800             -call 0x4cf090
    cpu.esp -= 4;
    sub_4cf090(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e5b8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044e5ba  7435                   -je 0x44e5f1
    if (cpu.flags.zf)
    {
        goto L_0x0044e5f1;
    }
    // 0044e5bc  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044e5c1  83f801                 +cmp eax, 1
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
    // 0044e5c4  722b                   -jb 0x44e5f1
    if (cpu.flags.cf)
    {
        goto L_0x0044e5f1;
    }
    // 0044e5c6  7607                   -jbe 0x44e5cf
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044e5cf;
    }
    // 0044e5c8  83f802                 +cmp eax, 2
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
    // 0044e5cb  7413                   -je 0x44e5e0
    if (cpu.flags.zf)
    {
        goto L_0x0044e5e0;
    }
    // 0044e5cd  eb22                   -jmp 0x44e5f1
    goto L_0x0044e5f1;
L_0x0044e5cf:
    // 0044e5cf  b8fe000000             -mov eax, 0xfe
    cpu.eax = 254 /*0xfe*/;
    // 0044e5d4  e877ffffff             -call 0x44e550
    cpu.esp -= 4;
    sub_44e550(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e5d9  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e5de  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e5df  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044e5e0:
    // 0044e5e0  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
    // 0044e5e5  e866ffffff             -call 0x44e550
    cpu.esp -= 4;
    sub_44e550(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e5ea  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e5ef  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e5f0  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044e5f1:
    // 0044e5f1  b80b000000             -mov eax, 0xb
    cpu.eax = 11 /*0xb*/;
    // 0044e5f6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e5f7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_44e600(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e600  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e601  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e603  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0044e60b  751a                   -jne 0x44e627
    if (!cpu.flags.zf)
    {
        goto L_0x0044e627;
    }
    // 0044e60d  833de0227a0000         +cmp dword ptr [0x7a22e0], 0
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
    // 0044e614  741f                   -je 0x44e635
    if (cpu.flags.zf)
    {
        goto L_0x0044e635;
    }
    // 0044e616  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 0044e61e  7515                   -jne 0x44e635
    if (!cpu.flags.zf)
    {
        goto L_0x0044e635;
    }
    // 0044e620  b801010000             -mov eax, 0x101
    cpu.eax = 257 /*0x101*/;
    // 0044e625  eb4f                   -jmp 0x44e676
    goto L_0x0044e676;
L_0x0044e627:
    // 0044e627  e8640a0800             -call 0x4cf090
    cpu.esp -= 4;
    sub_4cf090(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e62c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044e62e  750c                   -jne 0x44e63c
    if (!cpu.flags.zf)
    {
        goto L_0x0044e63c;
    }
    // 0044e630  e88bfeffff             -call 0x44e4c0
    cpu.esp -= 4;
    sub_44e4c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044e635:
    // 0044e635  b80b000000             -mov eax, 0xb
    cpu.eax = 11 /*0xb*/;
    // 0044e63a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e63b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044e63c:
    // 0044e63c  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044e641  83f801                 +cmp eax, 1
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
    // 0044e644  722b                   -jb 0x44e671
    if (cpu.flags.cf)
    {
        goto L_0x0044e671;
    }
    // 0044e646  7607                   -jbe 0x44e64f
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044e64f;
    }
    // 0044e648  83f802                 +cmp eax, 2
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
    // 0044e64b  7413                   -je 0x44e660
    if (cpu.flags.zf)
    {
        goto L_0x0044e660;
    }
    // 0044e64d  eb22                   -jmp 0x44e671
    goto L_0x0044e671;
L_0x0044e64f:
    // 0044e64f  b8fe000000             -mov eax, 0xfe
    cpu.eax = 254 /*0xfe*/;
    // 0044e654  e8f7feffff             -call 0x44e550
    cpu.esp -= 4;
    sub_44e550(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e659  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e65e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e65f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044e660:
    // 0044e660  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
    // 0044e665  e8e6feffff             -call 0x44e550
    cpu.esp -= 4;
    sub_44e550(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e66a  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e66f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e670  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044e671:
    // 0044e671  b8fd000000             -mov eax, 0xfd
    cpu.eax = 253 /*0xfd*/;
L_0x0044e676:
    // 0044e676  e8d5feffff             -call 0x44e550
    cpu.esp -= 4;
    sub_44e550(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e67b  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e680  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e681  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_44e690(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e690  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e691  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e693  b80a000000             -mov eax, 0xa
    cpu.eax = 10 /*0xa*/;
    // 0044e698  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e699  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_44e6a0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e6a0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e6a1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e6a3  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e6a8  e88393ffff             -call 0x447a30
    cpu.esp -= 4;
    sub_447a30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e6ad  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e6b2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e6b3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_44e6c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e6c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044e6c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044e6c2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044e6c3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e6c4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e6c6  b8cd010000             -mov eax, 0x1cd
    cpu.eax = 461 /*0x1cd*/;
    // 0044e6cb  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0044e6d0  bb00040000             -mov ebx, 0x400
    cpu.ebx = 1024 /*0x400*/;
    // 0044e6d5  e876310800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e6da  a3b8446600             -mov dword ptr [0x6644b8], eax
    app->getMemory<x86::reg32>(x86::reg32(6702264) /* 0x6644b8 */) = cpu.eax;
    // 0044e6df  b8ce010000             -mov eax, 0x1ce
    cpu.eax = 462 /*0x1ce*/;
    // 0044e6e4  bab8446600             -mov edx, 0x6644b8
    cpu.edx = 6702264 /*0x6644b8*/;
    // 0044e6e9  e862310800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e6ee  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044e6f0  a3bc446600             -mov dword ptr [0x6644bc], eax
    app->getMemory<x86::reg32>(x86::reg32(6702268) /* 0x6644bc */) = cpu.eax;
    // 0044e6f5  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e6fa  e88163ffff             -call 0x444a80
    cpu.esp -= 4;
    nfs3hp::frontend::dialogOpenMessage(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e6ff  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e704  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e705  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e706  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e707  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e708  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_44e710(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e710  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044e711  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044e712  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044e713  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044e714  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e715  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e717  81ecb8000000           -sub esp, 0xb8
    (cpu.esp) -= x86::reg32(x86::sreg32(184 /*0xb8*/));
    // 0044e71d  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 0044e723  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 0044e728  8d7d2e                 -lea edi, [ebp + 0x2e]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(46) /* 0x2e */);
    // 0044e72b  bebcd36f00             -mov esi, 0x6fd3bc
    cpu.esi = 7328700 /*0x6fd3bc*/;
    // 0044e730  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 0044e735  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044e737  a050e14400             -mov al, byte ptr [0x44e150]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(4514128) /* 0x44e150 */);
    // 0044e73c  b918000000             -mov ecx, 0x18
    cpu.ecx = 24 /*0x18*/;
    // 0044e741  8d7dca                 -lea edi, [ebp - 0x36]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-54) /* -0x36 */);
    // 0044e744  be0cd46f00             -mov esi, 0x6fd40c
    cpu.esi = 7328780 /*0x6fd40c*/;
    // 0044e749  88457e                 -mov byte ptr [ebp + 0x7e], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.al;
    // 0044e74c  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0044e74f  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044e751  66a5                   -movsw word ptr es:[edi], word ptr [esi]
    app->getMemory<x86::reg16>(cpu.ees + cpu.edi) = app->getMemory<x86::reg16>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 2;
        cpu.esi -= 2;
    }
    else
    {
        cpu.edi += 2;
        cpu.esi += 2;
    }
    // 0044e753  e83882ffff             -call 0x446990
    cpu.esp -= 4;
    nfs3hp::load::loadKeptReplay(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e758  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044e75a  7407                   -je 0x44e763
    if (cpu.flags.zf)
    {
        goto L_0x0044e763;
    }
    // 0044e75c  ba07000000             -mov edx, 7
    cpu.edx = 7 /*0x7*/;
    // 0044e761  eb0a                   -jmp 0x44e76d
    goto L_0x0044e76d;
L_0x0044e763:
    // 0044e763  b841000000             -mov eax, 0x41
    cpu.eax = 65 /*0x41*/;
    // 0044e768  e87364ffff             -call 0x444be0
    cpu.esp -= 4;
    nfs3hp::frontend::dialogMessageById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044e76d:
    // 0044e76d  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 0044e772  8d752e                 -lea esi, [ebp + 0x2e]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(46) /* 0x2e */);
    // 0044e775  bfbcd36f00             -mov edi, 0x6fd3bc
    cpu.edi = 7328700 /*0x6fd3bc*/;
    // 0044e77a  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044e77c  b918000000             -mov ecx, 0x18
    cpu.ecx = 24 /*0x18*/;
    // 0044e781  8d75ca                 -lea esi, [ebp - 0x36]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-54) /* -0x36 */);
    // 0044e784  bf0cd46f00             -mov edi, 0x6fd40c
    cpu.edi = 7328780 /*0x6fd40c*/;
    // 0044e789  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0044e78b  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044e78d  66a5                   -movsw word ptr es:[edi], word ptr [esi]
    app->getMemory<x86::reg16>(cpu.ees + cpu.edi) = app->getMemory<x86::reg16>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 2;
        cpu.esi -= 2;
    }
    else
    {
        cpu.edi += 2;
        cpu.esi += 2;
    }
    // 0044e78f  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 0044e795  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e796  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e797  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e798  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e799  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e79a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_44e7a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e7a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044e7a1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044e7a2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e7a3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e7a5  8b156c935500           -mov edx, dword ptr [0x55936c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5608300) /* 0x55936c */);
    // 0044e7ab  668b1c5572935500       -mov bx, word ptr [edx*2 + 0x559372]
    cpu.bx = app->getMemory<x86::reg16>(x86::reg32(5608306) /* 0x559372 */ + cpu.edx * 2);
    // 0044e7b3  42                     -inc edx
    (cpu.edx)++;
    // 0044e7b4  6685db                 +test bx, bx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.bx & cpu.bx));
    // 0044e7b7  7502                   -jne 0x44e7bb
    if (!cpu.flags.zf)
    {
        goto L_0x0044e7bb;
    }
    // 0044e7b9  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0044e7bb:
    // 0044e7bb  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044e7c0  89156c935500           -mov dword ptr [0x55936c], edx
    app->getMemory<x86::reg32>(x86::reg32(5608300) /* 0x55936c */) = cpu.edx;
    // 0044e7c6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e7c7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e7c8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e7c9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_44e7d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e7d0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044e7d1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e7d2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e7d4  ba68000000             -mov edx, 0x68
    cpu.edx = 104 /*0x68*/;
    // 0044e7d9  b840446600             -mov eax, 0x664440
    cpu.eax = 6702144 /*0x664440*/;
    // 0044e7de  e8291f0900             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e7e3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e7e4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e7e5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_44e7f0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044e7f0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e7f1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e7f3  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 0044e7f6  c784c24044660001000000 -mov dword ptr [edx + eax*8 + 0x664440], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6702144) /* 0x664440 */ + cpu.eax * 8) = 1 /*0x1*/;
    // 0044e801  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e802  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_44e820(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0044e820  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044e821  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044e822  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044e823  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044e824  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044e825  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044e826  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044e828  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044e82a  8b0df4d46f00           -mov ecx, dword ptr [0x6fd4f4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 0044e830  83f801                 +cmp eax, 1
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
    // 0044e833  7502                   -jne 0x44e837
    if (!cpu.flags.zf)
    {
        goto L_0x0044e837;
    }
    // 0044e835  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x0044e837:
    // 0044e837  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0044e839  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e83b  e8a06c0000             -call 0x4554e0
    cpu.esp -= 4;
    sub_4554e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e840  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0044e842  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044e844  7516                   -jne 0x44e85c
    if (!cpu.flags.zf)
    {
        goto L_0x0044e85c;
    }
    // 0044e846  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044e84b  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0044e84d  e88e6c0000             -call 0x4554e0
    cpu.esp -= 4;
    sub_4554e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e852  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0044e854  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044e856  0f84e8010000           -je 0x44ea44
    if (cpu.flags.zf)
    {
        goto L_0x0044ea44;
    }
L_0x0044e85c:
    // 0044e85c  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044e861  83f803                 +cmp eax, 3
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
    // 0044e864  0f87da010000           -ja 0x44ea44
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0044ea44;
    }
    // 0044e86a  ff248504e84400         -jmp dword ptr [eax*4 + 0x44e804]
    cpu.ip = app->getMemory<x86::reg32>(4515844 + cpu.eax * 4); goto dynamic_jump;
  case 0x0044e871:
    // 0044e871  f6870002000020         +test byte ptr [edi + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.edi + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 0044e878  7423                   -je 0x44e89d
    if (cpu.flags.zf)
    {
        goto L_0x0044e89d;
    }
    // 0044e87a  8b15b8d46f00           -mov edx, dword ptr [0x6fd4b8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */);
    // 0044e880  3b97cc020000           +cmp edx, dword ptr [edi + 0x2cc]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi + x86::reg32(716) /* 0x2cc */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e886  0f8fb8010000           -jg 0x44ea44
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0044ea44;
    }
    // 0044e88c  b80b000000             -mov eax, 0xb
    cpu.eax = 11 /*0xb*/;
    // 0044e891  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0044e893  e858ffffff             -call 0x44e7f0
    cpu.esp -= 4;
    sub_44e7f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e898  e9a7010000             -jmp 0x44ea44
    goto L_0x0044ea44;
L_0x0044e89d:
    // 0044e89d  83bfc402000004         +cmp dword ptr [edi + 0x2c4], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e8a4  0f849a010000           -je 0x44ea44
    if (cpu.flags.zf)
    {
        goto L_0x0044ea44;
    }
    // 0044e8aa  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044e8af  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e8b1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e8b3  e858730000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e8b8  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044e8ba  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e8bc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044e8be  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e8c0  e80b740000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e8c5  39c7                   +cmp edi, eax
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
    // 0044e8c7  0f8577010000           -jne 0x44ea44
    if (!cpu.flags.zf)
    {
        goto L_0x0044ea44;
    }
    // 0044e8cd  b80c000000             -mov eax, 0xc
    cpu.eax = 12 /*0xc*/;
    // 0044e8d2  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0044e8d4  e817ffffff             -call 0x44e7f0
    cpu.esp -= 4;
    sub_44e7f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e8d9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e8da  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e8db  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e8dc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e8dd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e8de  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e8df  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0044e8e0:
    // 0044e8e0  833d307d670001         +cmp dword ptr [0x677d30], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e8e7  0f8e57010000           -jle 0x44ea44
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0044ea44;
    }
    // 0044e8ed  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044e8f2  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e8f4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e8f6  e815730000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e8fb  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044e8fd  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e8ff  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044e901  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e903  e8c8730000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e908  39c7                   +cmp edi, eax
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
    // 0044e90a  0f8534010000           -jne 0x44ea44
    if (!cpu.flags.zf)
    {
        goto L_0x0044ea44;
    }
    // 0044e910  b80a000000             -mov eax, 0xa
    cpu.eax = 10 /*0xa*/;
    // 0044e915  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0044e917  e8d4feffff             -call 0x44e7f0
    cpu.esp -= 4;
    sub_44e7f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e91c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e91d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e91e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e91f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e920  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e921  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e922  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0044e923:
    // 0044e923  8b1520d05500           -mov edx, dword ptr [0x55d020]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5623840) /* 0x55d020 */);
    // 0044e929  3b15acd46f00           +cmp edx, dword ptr [0x6fd4ac]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e92f  0f850f010000           -jne 0x44ea44
    if (!cpu.flags.zf)
    {
        goto L_0x0044ea44;
    }
    // 0044e935  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0044e93a  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 0044e93f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e941  e8ca720000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e946  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044e948  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e94a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044e94c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e94e  e87d730000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e953  39c7                   +cmp edi, eax
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
    // 0044e955  7507                   -jne 0x44e95e
    if (!cpu.flags.zf)
    {
        goto L_0x0044e95e;
    }
    // 0044e957  b807000000             -mov eax, 7
    cpu.eax = 7 /*0x7*/;
    // 0044e95c  eb44                   -jmp 0x44e9a2
    goto L_0x0044e9a2;
L_0x0044e95e:
    // 0044e95e  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044e963  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044e965  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e967  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e969  e862730000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e96e  39c7                   +cmp edi, eax
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
    // 0044e970  7513                   -jne 0x44e985
    if (!cpu.flags.zf)
    {
        goto L_0x0044e985;
    }
    // 0044e972  b808000000             -mov eax, 8
    cpu.eax = 8 /*0x8*/;
    // 0044e977  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0044e979  e872feffff             -call 0x44e7f0
    cpu.esp -= 4;
    sub_44e7f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e97e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e97f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e980  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e981  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e982  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e983  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e984  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044e985:
    // 0044e985  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 0044e98a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044e98c  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e98e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e990  e83b730000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e995  39c7                   +cmp edi, eax
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
    // 0044e997  0f85a7000000           -jne 0x44ea44
    if (!cpu.flags.zf)
    {
        goto L_0x0044ea44;
    }
    // 0044e99d  b809000000             -mov eax, 9
    cpu.eax = 9 /*0x9*/;
L_0x0044e9a2:
    // 0044e9a2  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0044e9a4  e847feffff             -call 0x44e7f0
    cpu.esp -= 4;
    sub_44e7f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e9a9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e9aa  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e9ab  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e9ac  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e9ad  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e9ae  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044e9af  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0044e9b0:
    // 0044e9b0  8b8720020000           -mov eax, dword ptr [edi + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(544) /* 0x220 */);
    // 0044e9b6  8b0d30dc6f00           -mov ecx, dword ptr [0x6fdc30]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7330864) /* 0x6fdc30 */);
    // 0044e9bc  3b4814                 +cmp ecx, dword ptr [eax + 0x14]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044e9bf  0f847f000000           -je 0x44ea44
    if (cpu.flags.zf)
    {
        goto L_0x0044ea44;
    }
    // 0044e9c5  8b1dacd46f00           -mov ebx, dword ptr [0x6fd4ac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 0044e9cb  83fb08                 +cmp ebx, 8
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
    // 0044e9ce  7539                   -jne 0x44ea09
    if (!cpu.flags.zf)
    {
        goto L_0x0044ea09;
    }
    // 0044e9d0  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0044e9d5  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e9d7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e9d9  e832720000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e9de  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044e9e0  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044e9e2  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044e9e4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044e9e6  e8e5720000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044e9eb  39c7                   +cmp edi, eax
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
    // 0044e9ed  7507                   -jne 0x44e9f6
    if (!cpu.flags.zf)
    {
        goto L_0x0044e9f6;
    }
    // 0044e9ef  b806000000             -mov eax, 6
    cpu.eax = 6 /*0x6*/;
    // 0044e9f4  eb05                   -jmp 0x44e9fb
    goto L_0x0044e9fb;
L_0x0044e9f6:
    // 0044e9f6  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
L_0x0044e9fb:
    // 0044e9fb  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0044e9fd  e8eefdffff             -call 0x44e7f0
    cpu.esp -= 4;
    sub_44e7f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ea02  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea03  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea04  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea05  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea06  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea07  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea08  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044ea09:
    // 0044ea09  83bfc402000002         +cmp dword ptr [edi + 0x2c4], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ea10  7507                   -jne 0x44ea19
    if (!cpu.flags.zf)
    {
        goto L_0x0044ea19;
    }
    // 0044ea12  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 0044ea17  eb24                   -jmp 0x44ea3d
    goto L_0x0044ea3d;
L_0x0044ea19:
    // 0044ea19  a106d46f00             -mov eax, dword ptr [0x6fd406]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328774) /* 0x6fd406 */);
    // 0044ea1e  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0044ea21  39d8                   +cmp eax, ebx
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
    // 0044ea23  7513                   -jne 0x44ea38
    if (!cpu.flags.zf)
    {
        goto L_0x0044ea38;
    }
    // 0044ea25  b805000000             -mov eax, 5
    cpu.eax = 5 /*0x5*/;
    // 0044ea2a  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0044ea2c  e8bffdffff             -call 0x44e7f0
    cpu.esp -= 4;
    sub_44e7f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ea31  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea32  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea33  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea34  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea35  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea36  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea37  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044ea38:
    // 0044ea38  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
L_0x0044ea3d:
    // 0044ea3d  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0044ea3f  e8acfdffff             -call 0x44e7f0
    cpu.esp -= 4;
    sub_44e7f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044ea44:
    // 0044ea44  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea45  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea46  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea47  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea48  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea49  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ea4a  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x90 */
void Application::sub_44ea60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0044ea60  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044ea61  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044ea62  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044ea63  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044ea64  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044ea65  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044ea67  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044ea69  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044ea70  7507                   -jne 0x44ea79
    if (!cpu.flags.zf)
    {
        goto L_0x0044ea79;
    }
    // 0044ea72  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0044ea77  eb05                   -jmp 0x44ea7e
    goto L_0x0044ea7e;
L_0x0044ea79:
    // 0044ea79  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
L_0x0044ea7e:
    // 0044ea7e  e86daf0700             -call 0x4c99f0
    cpu.esp -= 4;
    nfs3hp::port::spokenStatus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ea83  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0044ea85  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044ea87  0f8484020000           -je 0x44ed11
    if (cpu.flags.zf)
    {
        goto L_0x0044ed11;
    }
    // 0044ea8d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0044ea8f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0044ea8f  39d8                   +cmp eax, ebx
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
    // 0044ea91  0f8d7a020000           -jge 0x44ed11
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0044ed11;
    }
    // 0044ea97  83fa04                 +cmp edx, 4
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
    // 0044ea9a  0f876b020000           -ja 0x44ed0b
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0044ed0b;
    }
    // 0044eaa0  ff24954cea4400         -jmp dword ptr [edx*4 + 0x44ea4c]
    cpu.ip = app->getMemory<x86::reg32>(4516428 + cpu.edx * 4); goto dynamic_jump;
  case 0x0044eaa7:
    // 0044eaa7  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 0044eaae  83b94844660000         +cmp dword ptr [ecx + 0x664448], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702152) /* 0x664448 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044eab5  7428                   -je 0x44eadf
    if (cpu.flags.zf)
    {
        goto L_0x0044eadf;
    }
    // 0044eab7  83b94044660000         +cmp dword ptr [ecx + 0x664440], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702144) /* 0x664440 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044eabe  741f                   -je 0x44eadf
    if (cpu.flags.zf)
    {
        goto L_0x0044eadf;
    }
    // 0044eac0  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044eac2  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 0044eac7  e8c4ae0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044eacc  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0044eace  898148446600           -mov dword ptr [ecx + 0x664448], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702152) /* 0x664448 */) = cpu.eax;
    // 0044ead4  898140446600           -mov dword ptr [ecx + 0x664440], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702144) /* 0x664440 */) = cpu.eax;
    // 0044eada  e932020000             -jmp 0x44ed11
    goto L_0x0044ed11;
L_0x0044eadf:
    // 0044eadf  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 0044eae6  83b95044660000         +cmp dword ptr [ecx + 0x664450], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702160) /* 0x664450 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044eaed  7429                   -je 0x44eb18
    if (cpu.flags.zf)
    {
        goto L_0x0044eb18;
    }
    // 0044eaef  83b94044660000         +cmp dword ptr [ecx + 0x664440], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702144) /* 0x664440 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044eaf6  7420                   -je 0x44eb18
    if (cpu.flags.zf)
    {
        goto L_0x0044eb18;
    }
    // 0044eaf8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044eafa  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 0044eaff  e88cae0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044eb04  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044eb06  898150446600           -mov dword ptr [ecx + 0x664450], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702160) /* 0x664450 */) = cpu.eax;
    // 0044eb0c  898140446600           -mov dword ptr [ecx + 0x664440], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702144) /* 0x664440 */) = cpu.eax;
    // 0044eb12  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb13  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb14  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb15  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb16  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb17  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044eb18:
    // 0044eb18  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 0044eb1f  83b95044660000         +cmp dword ptr [ecx + 0x664450], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702160) /* 0x664450 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044eb26  741a                   -je 0x44eb42
    if (cpu.flags.zf)
    {
        goto L_0x0044eb42;
    }
    // 0044eb28  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044eb2a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044eb2f  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044eb31  e85aae0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044eb36  89b950446600           -mov dword ptr [ecx + 0x664450], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702160) /* 0x664450 */) = cpu.edi;
    // 0044eb3c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb3d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb3e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb3f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb40  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb41  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044eb42:
    // 0044eb42  83b94844660000         +cmp dword ptr [ecx + 0x664448], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702152) /* 0x664448 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044eb49  7417                   -je 0x44eb62
    if (cpu.flags.zf)
    {
        goto L_0x0044eb62;
    }
    // 0044eb4b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044eb4d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044eb4f  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044eb51  e83aae0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044eb56  89b948446600           -mov dword ptr [ecx + 0x664448], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702152) /* 0x664448 */) = cpu.edi;
    // 0044eb5c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb5d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb5e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb5f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb60  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb61  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044eb62:
    // 0044eb62  83b94044660000         +cmp dword ptr [ecx + 0x664440], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702144) /* 0x664440 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044eb69  0f849c010000           -je 0x44ed0b
    if (cpu.flags.zf)
    {
        goto L_0x0044ed0b;
    }
    // 0044eb6f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044eb71  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044eb76  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044eb78  e813ae0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044eb7d  89b940446600           -mov dword ptr [ecx + 0x664440], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702144) /* 0x664440 */) = cpu.edi;
    // 0044eb83  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb84  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb85  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb86  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb87  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044eb88  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0044eb89:
    // 0044eb89  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 0044eb90  83b99044660000         +cmp dword ptr [ecx + 0x664490], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702224) /* 0x664490 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044eb97  741a                   -je 0x44ebb3
    if (cpu.flags.zf)
    {
        goto L_0x0044ebb3;
    }
    // 0044eb99  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044eb9b  b80a000000             -mov eax, 0xa
    cpu.eax = 10 /*0xa*/;
    // 0044eba0  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044eba2  e8e9ad0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044eba7  89b990446600           -mov dword ptr [ecx + 0x664490], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702224) /* 0x664490 */) = cpu.edi;
    // 0044ebad  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ebae  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ebaf  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ebb0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ebb1  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ebb2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044ebb3:
    // 0044ebb3  83b9a044660000         +cmp dword ptr [ecx + 0x6644a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702240) /* 0x6644a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ebba  0f844b010000           -je 0x44ed0b
    if (cpu.flags.zf)
    {
        goto L_0x0044ed0b;
    }
    // 0044ebc0  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044ebc2  b809000000             -mov eax, 9
    cpu.eax = 9 /*0x9*/;
    // 0044ebc7  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044ebc9  e8c2ad0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ebce  89b9a0446600           -mov dword ptr [ecx + 0x6644a0], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702240) /* 0x6644a0 */) = cpu.edi;
    // 0044ebd4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ebd5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ebd6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ebd7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ebd8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ebd9  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0044ebda:
    // 0044ebda  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 0044ebe1  83b97844660000         +cmp dword ptr [ecx + 0x664478], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702200) /* 0x664478 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ebe8  741a                   -je 0x44ec04
    if (cpu.flags.zf)
    {
        goto L_0x0044ec04;
    }
    // 0044ebea  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044ebec  b805000000             -mov eax, 5
    cpu.eax = 5 /*0x5*/;
    // 0044ebf1  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044ebf3  e898ad0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ebf8  89b978446600           -mov dword ptr [ecx + 0x664478], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702200) /* 0x664478 */) = cpu.edi;
    // 0044ebfe  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ebff  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec00  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec01  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec02  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec03  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044ec04:
    // 0044ec04  83b98044660000         +cmp dword ptr [ecx + 0x664480], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702208) /* 0x664480 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ec0b  741a                   -je 0x44ec27
    if (cpu.flags.zf)
    {
        goto L_0x0044ec27;
    }
    // 0044ec0d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044ec0f  b806000000             -mov eax, 6
    cpu.eax = 6 /*0x6*/;
    // 0044ec14  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044ec16  e875ad0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ec1b  89b980446600           -mov dword ptr [ecx + 0x664480], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702208) /* 0x664480 */) = cpu.edi;
    // 0044ec21  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec22  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec23  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec24  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec25  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec26  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044ec27:
    // 0044ec27  83b98844660000         +cmp dword ptr [ecx + 0x664488], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702216) /* 0x664488 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ec2e  0f84d7000000           -je 0x44ed0b
    if (cpu.flags.zf)
    {
        goto L_0x0044ed0b;
    }
    // 0044ec34  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044ec36  b807000000             -mov eax, 7
    cpu.eax = 7 /*0x7*/;
    // 0044ec3b  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044ec3d  e84ead0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ec42  89b988446600           -mov dword ptr [ecx + 0x664488], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702216) /* 0x664488 */) = cpu.edi;
    // 0044ec48  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec49  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec4a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec4b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec4c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec4d  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0044ec4e:
    // 0044ec4e  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 0044ec55  83b95844660000         +cmp dword ptr [ecx + 0x664458], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702168) /* 0x664458 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ec5c  741a                   -je 0x44ec78
    if (cpu.flags.zf)
    {
        goto L_0x0044ec78;
    }
    // 0044ec5e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044ec60  b80f000000             -mov eax, 0xf
    cpu.eax = 15 /*0xf*/;
    // 0044ec65  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044ec67  e824ad0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ec6c  89b958446600           -mov dword ptr [ecx + 0x664458], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702168) /* 0x664458 */) = cpu.edi;
    // 0044ec72  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec73  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec74  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec75  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec76  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec77  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044ec78:
    // 0044ec78  83b97044660000         +cmp dword ptr [ecx + 0x664470], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702192) /* 0x664470 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ec7f  741a                   -je 0x44ec9b
    if (cpu.flags.zf)
    {
        goto L_0x0044ec9b;
    }
    // 0044ec81  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044ec83  b80e000000             -mov eax, 0xe
    cpu.eax = 14 /*0xe*/;
    // 0044ec88  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044ec8a  e801ad0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ec8f  89b970446600           -mov dword ptr [ecx + 0x664470], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702192) /* 0x664470 */) = cpu.edi;
    // 0044ec95  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec96  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec97  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec98  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec99  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ec9a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044ec9b:
    // 0044ec9b  83b96044660000         +cmp dword ptr [ecx + 0x664460], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702176) /* 0x664460 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044eca2  741a                   -je 0x44ecbe
    if (cpu.flags.zf)
    {
        goto L_0x0044ecbe;
    }
    // 0044eca4  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044eca6  b80c000000             -mov eax, 0xc
    cpu.eax = 12 /*0xc*/;
    // 0044ecab  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044ecad  e8deac0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ecb2  89b960446600           -mov dword ptr [ecx + 0x664460], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702176) /* 0x664460 */) = cpu.edi;
    // 0044ecb8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ecb9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ecba  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ecbb  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ecbc  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ecbd  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044ecbe:
    // 0044ecbe  83b96844660000         +cmp dword ptr [ecx + 0x664468], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702184) /* 0x664468 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ecc5  7444                   -je 0x44ed0b
    if (cpu.flags.zf)
    {
        goto L_0x0044ed0b;
    }
    // 0044ecc7  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044ecc9  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 0044ecce  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0044ecd0  e8bbac0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ecd5  89b968446600           -mov dword ptr [ecx + 0x664468], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702184) /* 0x664468 */) = cpu.edi;
    // 0044ecdb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ecdc  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ecdd  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ecde  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ecdf  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ece0  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0044ece1:
    // 0044ece1  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 0044ece8  83b99844660000         +cmp dword ptr [ecx + 0x664498], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702232) /* 0x664498 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ecef  741a                   -je 0x44ed0b
    if (cpu.flags.zf)
    {
        goto L_0x0044ed0b;
    }
    // 0044ecf1  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044ecf3  b808000000             -mov eax, 8
    cpu.eax = 8 /*0x8*/;
    // 0044ecf8  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0044ecfa  e891ac0700             -call 0x4c9990
    cpu.esp -= 4;
    nfs3hp::port::carLineSpeak(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ecff  89b998446600           -mov dword ptr [ecx + 0x664498], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6702232) /* 0x664498 */) = cpu.edi;
    // 0044ed05  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed06  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed07  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed08  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed09  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed0a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044ed0b:
    // 0044ed0b  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0044ed0c  e97efdffff             -jmp 0x44ea8f
    goto L_0x0044ea8f;
L_0x0044ed11:
    // 0044ed11  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed12  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed13  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed14  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed15  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed16  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_44ed30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0044ed30  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044ed31  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044ed33  a16c935500             -mov eax, dword ptr [0x55936c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608300) /* 0x55936c */);
    // 0044ed38  83f803                 +cmp eax, 3
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
    // 0044ed3b  774f                   -ja 0x44ed8c
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0044ed8c;
    }
    // 0044ed3d  ff248518ed4400         -jmp dword ptr [eax*4 + 0x44ed18]
    cpu.ip = app->getMemory<x86::reg32>(4517144 + cpu.eax * 4); goto dynamic_jump;
  case 0x0044ed44:
    // 0044ed44  833db8d36f0002         +cmp dword ptr [0x6fd3b8], 2
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
    // 0044ed4b  7507                   -jne 0x44ed54
    if (!cpu.flags.zf)
    {
        goto L_0x0044ed54;
    }
    // 0044ed4d  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 0044ed52  eb05                   -jmp 0x44ed59
    goto L_0x0044ed59;
L_0x0044ed54:
    // 0044ed54  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0044ed59:
    // 0044ed59  e802fdffff             -call 0x44ea60
    cpu.esp -= 4;
    sub_44ea60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ed5e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044ed60  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed61  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0044ed62:
    // 0044ed62  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0044ed64  eb21                   -jmp 0x44ed87
    goto L_0x0044ed87;
  case 0x0044ed66:
    // 0044ed66  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044ed6b  83f801                 +cmp eax, 1
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
    // 0044ed6e  721c                   -jb 0x44ed8c
    if (cpu.flags.cf)
    {
        goto L_0x0044ed8c;
    }
    // 0044ed70  7609                   -jbe 0x44ed7b
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044ed7b;
    }
    // 0044ed72  83f803                 +cmp eax, 3
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
    // 0044ed75  740b                   -je 0x44ed82
    if (cpu.flags.zf)
    {
        goto L_0x0044ed82;
    }
    // 0044ed77  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0044ed79  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed7a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044ed7b:
    // 0044ed7b  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0044ed80  eb05                   -jmp 0x44ed87
    goto L_0x0044ed87;
L_0x0044ed82:
    // 0044ed82  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
L_0x0044ed87:
    // 0044ed87  e8d4fcffff             -call 0x44ea60
    cpu.esp -= 4;
    sub_44ea60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
  [[fallthrough]];
  case 0x0044ed8c:
L_0x0044ed8c:
    // 0044ed8c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044ed8e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ed8f  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip  */
void Application::sub_44eda0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0044eda0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044eda1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044eda2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044eda3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044eda4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044eda5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044eda6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044eda8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0044edaa  e8b1aa0700             -call 0x4c9860
    cpu.esp -= 4;
    nfs3hp::port::descriptionService(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044edaf  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044edb6  7e48                   -jle 0x44ee00
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0044ee00;
    }
    // 0044edb8  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 0044edc0  743e                   -je 0x44ee00
    if (cpu.flags.zf)
    {
        goto L_0x0044ee00;
    }
    // 0044edc2  833de0227a0000         +cmp dword ptr [0x7a22e0], 0
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
    // 0044edc9  7435                   -je 0x44ee00
    if (cpu.flags.zf)
    {
        goto L_0x0044ee00;
    }
    // 0044edcb  b814010000             -mov eax, 0x114
    cpu.eax = 276 /*0x114*/;
    // 0044edd0  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0044edd5  bb00020000             -mov ebx, 0x200
    cpu.ebx = 512 /*0x200*/;
    // 0044edda  bac0446600             -mov edx, 0x6644c0
    cpu.edx = 6702272 /*0x6644c0*/;
    // 0044eddf  e86c2a0800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ede4  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0044ede6  a3c0446600             -mov dword ptr [0x6644c0], eax
    app->getMemory<x86::reg32>(x86::reg32(6702272) /* 0x6644c0 */) = cpu.eax;
    // 0044edeb  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0044eded  e88e5cffff             -call 0x444a80
    cpu.esp -= 4;
    nfs3hp::frontend::dialogOpenMessage(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044edf2  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0044edf4  66891dde227a00         -mov word ptr [0x7a22de], bx
    app->getMemory<x86::reg16>(x86::reg32(8004318) /* 0x7a22de */) = cpu.bx;
    // 0044edfb  e8e0770500             -call 0x4a65e0
    cpu.esp -= 4;
    nfs3hp::ea::closeNetwork(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044ee00:
    // 0044ee00  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044ee07  7e1d                   -jle 0x44ee26
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0044ee26;
    }
    // 0044ee09  833de0227a0000         +cmp dword ptr [0x7a22e0], 0
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
    // 0044ee10  7514                   -jne 0x44ee26
    if (!cpu.flags.zf)
    {
        goto L_0x0044ee26;
    }
    // 0044ee12  ba94945300             -mov edx, 0x539494
    cpu.edx = 5477524 /*0x539494*/;
    // 0044ee17  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0044ee19  e8223cffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044ee1e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044ee20  7404                   -je 0x44ee26
    if (cpu.flags.zf)
    {
        goto L_0x0044ee26;
    }
    // 0044ee22  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044ee26:
    // 0044ee26  833df093550000         +cmp dword ptr [0x5593f0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5608432) /* 0x5593f0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ee2d  0f848c010000           -je 0x44efbf
    if (cpu.flags.zf)
    {
        goto L_0x0044efbf;
    }
    // 0044ee33  833df493550000         +cmp dword ptr [0x5593f4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5608436) /* 0x5593f4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ee3a  0f847f010000           -je 0x44efbf
    if (cpu.flags.zf)
    {
        goto L_0x0044efbf;
    }
    // 0044ee40  833df893550000         +cmp dword ptr [0x5593f8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5608440) /* 0x5593f8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ee47  0f8472010000           -je 0x44efbf
    if (cpu.flags.zf)
    {
        goto L_0x0044efbf;
    }
    // 0044ee4d  833dfc93550000         +cmp dword ptr [0x5593fc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5608444) /* 0x5593fc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ee54  0f8465010000           -je 0x44efbf
    if (cpu.flags.zf)
    {
        goto L_0x0044efbf;
    }
    // 0044ee5a  833d0094550000         +cmp dword ptr [0x559400], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5608448) /* 0x559400 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044ee61  0f8458010000           -je 0x44efbf
    if (cpu.flags.zf)
    {
        goto L_0x0044efbf;
    }
    // 0044ee67  8b3d04945500           -mov edi, dword ptr [0x559404]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5608452) /* 0x559404 */);
    // 0044ee6d  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0044ee6f  0f844a010000           -je 0x44efbf
    if (cpu.flags.zf)
    {
        goto L_0x0044efbf;
    }
    // 0044ee75  668b4f6c               -mov cx, word ptr [edi + 0x6c]
    cpu.cx = app->getMemory<x86::reg16>(cpu.edi + x86::reg32(108) /* 0x6c */);
    // 0044ee79  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0044ee7b  6683f903               +cmp cx, 3
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(3 /*0x3*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0044ee7f  7406                   -je 0x44ee87
    if (cpu.flags.zf)
    {
        goto L_0x0044ee87;
    }
    // 0044ee81  66c7476c0200           -mov word ptr [edi + 0x6c], 2
    app->getMemory<x86::reg16>(cpu.edi + x86::reg32(108) /* 0x6c */) = 2 /*0x2*/;
L_0x0044ee87:
    // 0044ee87  a1f8935500             -mov eax, dword ptr [0x5593f8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608440) /* 0x5593f8 */);
    // 0044ee8c  6683b8b800000003       +cmp word ptr [eax + 0xb8], 3
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(184) /* 0xb8 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(3 /*0x3*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0044ee94  7409                   -je 0x44ee9f
    if (cpu.flags.zf)
    {
        goto L_0x0044ee9f;
    }
    // 0044ee96  66c780b80000000200     -mov word ptr [eax + 0xb8], 2
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(184) /* 0xb8 */) = 2 /*0x2*/;
L_0x0044ee9f:
    // 0044ee9f  a1f0935500             -mov eax, dword ptr [0x5593f0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608432) /* 0x5593f0 */);
    // 0044eea4  6683b88800000003       +cmp word ptr [eax + 0x88], 3
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
    // 0044eeac  7409                   -je 0x44eeb7
    if (cpu.flags.zf)
    {
        goto L_0x0044eeb7;
    }
    // 0044eeae  66c780880000000200     -mov word ptr [eax + 0x88], 2
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(136) /* 0x88 */) = 2 /*0x2*/;
L_0x0044eeb7:
    // 0044eeb7  a1fc935500             -mov eax, dword ptr [0x5593fc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608444) /* 0x5593fc */);
    // 0044eebc  6683787e03             +cmp word ptr [eax + 0x7e], 3
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(3 /*0x3*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0044eec1  7406                   -je 0x44eec9
    if (cpu.flags.zf)
    {
        goto L_0x0044eec9;
    }
    // 0044eec3  66c7407e0200           -mov word ptr [eax + 0x7e], 2
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */) = 2 /*0x2*/;
L_0x0044eec9:
    // 0044eec9  a100945500             -mov eax, dword ptr [0x559400]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608448) /* 0x559400 */);
    // 0044eece  6683787e03             +cmp word ptr [eax + 0x7e], 3
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(3 /*0x3*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0044eed3  7406                   -je 0x44eedb
    if (cpu.flags.zf)
    {
        goto L_0x0044eedb;
    }
    // 0044eed5  66c7407e0200           -mov word ptr [eax + 0x7e], 2
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */) = 2 /*0x2*/;
L_0x0044eedb:
    // 0044eedb  8b1df8935500           -mov ebx, dword ptr [0x5593f8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5608440) /* 0x5593f8 */);
    // 0044eee1  8a7304                 -mov dh, byte ptr [ebx + 4]
    cpu.dh = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0044eee4  80ce01                 -or dh, 1
    cpu.dh |= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 0044eee7  a104945500             -mov eax, dword ptr [0x559404]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608452) /* 0x559404 */);
    // 0044eeec  887304                 -mov byte ptr [ebx + 4], dh
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(4) /* 0x4 */) = cpu.dh;
    // 0044eeef  80480401               -or byte ptr [eax + 4], 1
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 0044eef3  a1f0935500             -mov eax, dword ptr [0x5593f0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608432) /* 0x5593f0 */);
    // 0044eef8  8a6804                 -mov ch, byte ptr [eax + 4]
    cpu.ch = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0044eefb  80cd01                 -or ch, 1
    cpu.ch |= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 0044eefe  8b15fc935500           -mov edx, dword ptr [0x5593fc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5608444) /* 0x5593fc */);
    // 0044ef04  886804                 -mov byte ptr [eax + 4], ch
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.ch;
    // 0044ef07  804a0401               -or byte ptr [edx + 4], 1
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 0044ef0b  8b0d00945500           -mov ecx, dword ptr [0x559400]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5608448) /* 0x559400 */);
    // 0044ef11  8b356c935500           -mov esi, dword ptr [0x55936c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5608300) /* 0x55936c */);
    // 0044ef17  80490401               -or byte ptr [ecx + 4], 1
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 0044ef1b  83fe03                 +cmp esi, 3
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
    // 0044ef1e  0f879b000000           -ja 0x44efbf
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0044efbf;
    }
    // 0044ef24  ff24b590ed4400         -jmp dword ptr [esi*4 + 0x44ed90]
    cpu.ip = app->getMemory<x86::reg32>(4517264 + cpu.esi * 4); goto dynamic_jump;
  case 0x0044ef2b:
    // 0044ef2b  a104945500             -mov eax, dword ptr [0x559404]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608452) /* 0x559404 */);
    // 0044ef30  8a6804                 -mov ch, byte ptr [eax + 4]
    cpu.ch = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0044ef33  66c7406c0100           -mov word ptr [eax + 0x6c], 1
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(108) /* 0x6c */) = 1 /*0x1*/;
    // 0044ef39  80e5fe                 -and ch, 0xfe
    cpu.ch &= x86::reg8(x86::sreg8(254 /*0xfe*/));
    // 0044ef3c  886804                 -mov byte ptr [eax + 4], ch
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.ch;
    // 0044ef3f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044ef41  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef42  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef43  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef44  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef45  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef46  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef47  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0044ef48:
    // 0044ef48  8a5004                 -mov dl, byte ptr [eax + 4]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0044ef4b  66c780880000000100     -mov word ptr [eax + 0x88], 1
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(136) /* 0x88 */) = 1 /*0x1*/;
    // 0044ef54  80e2fe                 -and dl, 0xfe
    cpu.dl &= x86::reg8(x86::sreg8(254 /*0xfe*/));
    // 0044ef57  885004                 -mov byte ptr [eax + 4], dl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.dl;
    // 0044ef5a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044ef5c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef5d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef5e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef5f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef60  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef61  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef62  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0044ef63:
    // 0044ef63  8a7304                 -mov dh, byte ptr [ebx + 4]
    cpu.dh = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0044ef66  66c783b80000000100     -mov word ptr [ebx + 0xb8], 1
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(184) /* 0xb8 */) = 1 /*0x1*/;
    // 0044ef6f  80e6fe                 -and dh, 0xfe
    cpu.dh &= x86::reg8(x86::sreg8(254 /*0xfe*/));
    // 0044ef72  887304                 -mov byte ptr [ebx + 4], dh
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(4) /* 0x4 */) = cpu.dh;
    // 0044ef75  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044ef77  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef78  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef79  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef7a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef7b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef7c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef7d  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0044ef7e:
    // 0044ef7e  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044ef83  83f801                 +cmp eax, 1
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
    // 0044ef86  7237                   -jb 0x44efbf
    if (cpu.flags.cf)
    {
        goto L_0x0044efbf;
    }
    // 0044ef88  760e                   -jbe 0x44ef98
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044ef98;
    }
    // 0044ef8a  83f803                 +cmp eax, 3
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
    // 0044ef8d  7421                   -je 0x44efb0
    if (cpu.flags.zf)
    {
        goto L_0x0044efb0;
    }
    // 0044ef8f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044ef91  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef92  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef93  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef94  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef95  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef96  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044ef97  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044ef98:
    // 0044ef98  8a5a04                 -mov bl, byte ptr [edx + 4]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 0044ef9b  66c7427e0100           -mov word ptr [edx + 0x7e], 1
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(126) /* 0x7e */) = 1 /*0x1*/;
    // 0044efa1  80e3fe                 -and bl, 0xfe
    cpu.bl &= x86::reg8(x86::sreg8(254 /*0xfe*/));
    // 0044efa4  885a04                 -mov byte ptr [edx + 4], bl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */) = cpu.bl;
    // 0044efa7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044efa9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efaa  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efab  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efac  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efad  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efae  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efaf  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044efb0:
    // 0044efb0  8a7904                 -mov bh, byte ptr [ecx + 4]
    cpu.bh = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0044efb3  66c7417e0100           -mov word ptr [ecx + 0x7e], 1
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(126) /* 0x7e */) = 1 /*0x1*/;
    // 0044efb9  80e7fe                 -and bh, 0xfe
    cpu.bh &= x86::reg8(x86::sreg8(254 /*0xfe*/));
    // 0044efbc  887904                 -mov byte ptr [ecx + 4], bh
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(4) /* 0x4 */) = cpu.bh;
L_0x0044efbf:
    // 0044efbf  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044efc1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efc2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efc3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efc4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efc5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efc6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044efc7  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_44efd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044efd0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0044efd1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044efd2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0044efd3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0044efd4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0044efd5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044efd6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044efd8  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0044efdb  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0044efde  e89dff0700             -call 0x4cef80
    cpu.esp -= 4;
    sub_4cef80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044efe3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044efe5  7505                   -jne 0x44efec
    if (!cpu.flags.zf)
    {
        goto L_0x0044efec;
    }
    // 0044efe7  e8b4f1ffff             -call 0x44e1a0
    cpu.esp -= 4;
    sub_44e1a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0044efec:
    // 0044efec  ba9c945300             -mov edx, 0x53949c
    cpu.edx = 5477532 /*0x53949c*/;
    // 0044eff1  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044eff4  e8473affff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044eff9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044effb  7407                   -je 0x44f004
    if (cpu.flags.zf)
    {
        goto L_0x0044f004;
    }
    // 0044effd  c7402c30ed4400         -mov dword ptr [eax + 0x2c], 0x44ed30
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(44) /* 0x2c */) = 4517168 /*0x44ed30*/;
L_0x0044f004:
    // 0044f004  baa8945300             -mov edx, 0x5394a8
    cpu.edx = 5477544 /*0x5394a8*/;
    // 0044f009  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f00c  e82f3affff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f011  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044f013  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f015  744a                   -je 0x44f061
    if (cpu.flags.zf)
    {
        goto L_0x0044f061;
    }
    // 0044f017  8d7de8                 -lea edi, [ebp - 0x18]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0044f01a  be52e14400             -mov esi, 0x44e152
    cpu.esi = 4514130 /*0x44e152*/;
    // 0044f01f  a1acd46f00             -mov eax, dword ptr [0x6fd4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 0044f024  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044f025  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044f026  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044f027  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044f028  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0044f029  8b4445e6               -mov eax, dword ptr [ebp + eax*2 - 0x1a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-26) /* -0x1a */ + cpu.eax * 2);
    // 0044f02d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0044f030  e81b280800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f035  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0044f036  b805000000             -mov eax, 5
    cpu.eax = 5 /*0x5*/;
    // 0044f03b  e810280800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f040  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0044f041  8b0db4d46f00           -mov ecx, dword ptr [0x6fd4b4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */);
    // 0044f047  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0044f048  68b8945300             -push 0x5394b8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5477560 /*0x5394b8*/;
    cpu.esp -= 4;
    // 0044f04d  68c4446600             -push 0x6644c4
    app->getMemory<x86::reg32>(cpu.esp-4) = 6702276 /*0x6644c4*/;
    cpu.esp -= 4;
    // 0044f052  e839060900             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f057  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0044f05a  c7423cc4446600         -mov dword ptr [edx + 0x3c], 0x6644c4
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */) = 6702276 /*0x6644c4*/;
L_0x0044f061:
    // 0044f061  bac4945300             -mov edx, 0x5394c4
    cpu.edx = 5477572 /*0x5394c4*/;
    // 0044f066  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f069  e8d239ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f06e  bad4945300             -mov edx, 0x5394d4
    cpu.edx = 5477588 /*0x5394d4*/;
    // 0044f073  a3f8935500             -mov dword ptr [0x5593f8], eax
    app->getMemory<x86::reg32>(x86::reg32(5608440) /* 0x5593f8 */) = cpu.eax;
    // 0044f078  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f07b  e8c039ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f080  bae8945300             -mov edx, 0x5394e8
    cpu.edx = 5477608 /*0x5394e8*/;
    // 0044f085  a3f0935500             -mov dword ptr [0x5593f0], eax
    app->getMemory<x86::reg32>(x86::reg32(5608432) /* 0x5593f0 */) = cpu.eax;
    // 0044f08a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f08d  e8ae39ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f092  bafc945300             -mov edx, 0x5394fc
    cpu.edx = 5477628 /*0x5394fc*/;
    // 0044f097  a3fc935500             -mov dword ptr [0x5593fc], eax
    app->getMemory<x86::reg32>(x86::reg32(5608444) /* 0x5593fc */) = cpu.eax;
    // 0044f09c  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f09f  e89c39ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f0a4  ba08955300             -mov edx, 0x539508
    cpu.edx = 5477640 /*0x539508*/;
    // 0044f0a9  a300945500             -mov dword ptr [0x559400], eax
    app->getMemory<x86::reg32>(x86::reg32(5608448) /* 0x559400 */) = cpu.eax;
    // 0044f0ae  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f0b1  e88a39ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f0b6  ba18955300             -mov edx, 0x539518
    cpu.edx = 5477656 /*0x539518*/;
    // 0044f0bb  a3f4935500             -mov dword ptr [0x5593f4], eax
    app->getMemory<x86::reg32>(x86::reg32(5608436) /* 0x5593f4 */) = cpu.eax;
    // 0044f0c0  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f0c3  e87839ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f0c8  8b1df0935500           -mov ebx, dword ptr [0x5593f0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5608432) /* 0x5593f0 */);
    // 0044f0ce  a304945500             -mov dword ptr [0x559404], eax
    app->getMemory<x86::reg32>(x86::reg32(5608452) /* 0x559404 */) = cpu.eax;
    // 0044f0d3  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0044f0d5  0f840c040000           -je 0x44f4e7
    if (cpu.flags.zf)
    {
        goto L_0x0044f4e7;
    }
    // 0044f0db  8b35f4935500           -mov esi, dword ptr [0x5593f4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5608436) /* 0x5593f4 */);
    // 0044f0e1  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044f0e3  0f84fe030000           -je 0x44f4e7
    if (cpu.flags.zf)
    {
        goto L_0x0044f4e7;
    }
    // 0044f0e9  8b3df8935500           -mov edi, dword ptr [0x5593f8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5608440) /* 0x5593f8 */);
    // 0044f0ef  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0044f0f1  0f84f0030000           -je 0x44f4e7
    if (cpu.flags.zf)
    {
        goto L_0x0044f4e7;
    }
    // 0044f0f7  8b15fc935500           -mov edx, dword ptr [0x5593fc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5608444) /* 0x5593fc */);
    // 0044f0fd  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0044f0ff  0f84e2030000           -je 0x44f4e7
    if (cpu.flags.zf)
    {
        goto L_0x0044f4e7;
    }
    // 0044f105  8b0d00945500           -mov ecx, dword ptr [0x559400]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5608448) /* 0x559400 */);
    // 0044f10b  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0044f10d  0f84d4030000           -je 0x44f4e7
    if (cpu.flags.zf)
    {
        goto L_0x0044f4e7;
    }
    // 0044f113  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f115  0f84cc030000           -je 0x44f4e7
    if (cpu.flags.zf)
    {
        goto L_0x0044f4e7;
    }
    // 0044f11b  c74664a0e74400         -mov dword ptr [esi + 0x64], 0x44e7a0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(100) /* 0x64 */) = 4515744 /*0x44e7a0*/;
    // 0044f122  66c7406c0300           -mov word ptr [eax + 0x6c], 3
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(108) /* 0x6c */) = 3 /*0x3*/;
    // 0044f128  66c7406ea802           -mov word ptr [eax + 0x6e], 0x2a8
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(110) /* 0x6e */) = 680 /*0x2a8*/;
    // 0044f12e  66c787b80000000300     -mov word ptr [edi + 0xb8], 3
    app->getMemory<x86::reg16>(cpu.edi + x86::reg32(184) /* 0xb8 */) = 3 /*0x3*/;
    // 0044f137  66c787ba000000a802     -mov word ptr [edi + 0xba], 0x2a8
    app->getMemory<x86::reg16>(cpu.edi + x86::reg32(186) /* 0xba */) = 680 /*0x2a8*/;
    // 0044f140  66c783880000000300     -mov word ptr [ebx + 0x88], 3
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(136) /* 0x88 */) = 3 /*0x3*/;
    // 0044f149  66c7838a000000a802     -mov word ptr [ebx + 0x8a], 0x2a8
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(138) /* 0x8a */) = 680 /*0x2a8*/;
    // 0044f152  66c7427e0300           -mov word ptr [edx + 0x7e], 3
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(126) /* 0x7e */) = 3 /*0x3*/;
    // 0044f158  66c78280000000a802     -mov word ptr [edx + 0x80], 0x2a8
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(128) /* 0x80 */) = 680 /*0x2a8*/;
    // 0044f161  66c7417e0300           -mov word ptr [ecx + 0x7e], 3
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(126) /* 0x7e */) = 3 /*0x3*/;
    // 0044f167  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044f16c  66c78180000000a802     -mov word ptr [ecx + 0x80], 0x2a8
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */) = 680 /*0x2a8*/;
    // 0044f175  83f801                 +cmp eax, 1
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
    // 0044f178  0f82a4000000           -jb 0x44f222
    if (cpu.flags.cf)
    {
        goto L_0x0044f222;
    }
    // 0044f17e  760a                   -jbe 0x44f18a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0044f18a;
    }
    // 0044f180  83f803                 +cmp eax, 3
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
    // 0044f183  7428                   -je 0x44f1ad
    if (cpu.flags.zf)
    {
        goto L_0x0044f1ad;
    }
    // 0044f185  e998000000             -jmp 0x44f222
    goto L_0x0044f222;
L_0x0044f18a:
    // 0044f18a  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 0044f18f  ba70935500             -mov edx, 0x559370
    cpu.edx = 5608304 /*0x559370*/;
    // 0044f194  b890935500             -mov eax, 0x559390
    cpu.eax = 5608336 /*0x559390*/;
    // 0044f199  e852b30900             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f19e  c7056c93550003000000   -mov dword ptr [0x55936c], 3
    app->getMemory<x86::reg32>(x86::reg32(5608300) /* 0x55936c */) = 3 /*0x3*/;
    // 0044f1a8  e993000000             -jmp 0x44f240
    goto L_0x0044f240;
L_0x0044f1ad:
    // 0044f1ad  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0044f1b2  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0044f1b4  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0044f1b6  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0044f1b8  e8136b0000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f1bd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f1bf  7521                   -jne 0x44f1e2
    if (!cpu.flags.zf)
    {
        goto L_0x0044f1e2;
    }
    // 0044f1c1  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 0044f1c6  ba70935500             -mov edx, 0x559370
    cpu.edx = 5608304 /*0x559370*/;
    // 0044f1cb  b8b0935500             -mov eax, 0x5593b0
    cpu.eax = 5608368 /*0x5593b0*/;
    // 0044f1d0  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 0044f1d5  e816b30900             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f1da  89356c935500           -mov dword ptr [0x55936c], esi
    app->getMemory<x86::reg32>(x86::reg32(5608300) /* 0x55936c */) = cpu.esi;
    // 0044f1e0  eb5e                   -jmp 0x44f240
    goto L_0x0044f240;
L_0x0044f1e2:
    // 0044f1e2  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 0044f1e7  ba70935500             -mov edx, 0x559370
    cpu.edx = 5608304 /*0x559370*/;
    // 0044f1ec  b8d0935500             -mov eax, 0x5593d0
    cpu.eax = 5608400 /*0x5593d0*/;
    // 0044f1f1  e8fab20900             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f1f6  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044f1fb  8b15f4d46f00           -mov edx, dword ptr [0x6fd4f4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 0044f201  e8da620000             -call 0x4554e0
    cpu.esp -= 4;
    sub_4554e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f206  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f208  740c                   -je 0x44f216
    if (cpu.flags.zf)
    {
        goto L_0x0044f216;
    }
    // 0044f20a  c7056c93550003000000   -mov dword ptr [0x55936c], 3
    app->getMemory<x86::reg32>(x86::reg32(5608300) /* 0x55936c */) = 3 /*0x3*/;
    // 0044f214  eb2a                   -jmp 0x44f240
    goto L_0x0044f240;
L_0x0044f216:
    // 0044f216  c7056c93550002000000   -mov dword ptr [0x55936c], 2
    app->getMemory<x86::reg32>(x86::reg32(5608300) /* 0x55936c */) = 2 /*0x2*/;
    // 0044f220  eb1e                   -jmp 0x44f240
    goto L_0x0044f240;
L_0x0044f222:
    // 0044f222  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 0044f227  ba70935500             -mov edx, 0x559370
    cpu.edx = 5608304 /*0x559370*/;
    // 0044f22c  b8b0935500             -mov eax, 0x5593b0
    cpu.eax = 5608368 /*0x5593b0*/;
    // 0044f231  e8bab20900             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f236  c7056c93550002000000   -mov dword ptr [0x55936c], 2
    app->getMemory<x86::reg32>(x86::reg32(5608300) /* 0x55936c */) = 2 /*0x2*/;
L_0x0044f240:
    // 0044f240  ba2c955300             -mov edx, 0x53952c
    cpu.edx = 5477676 /*0x53952c*/;
    // 0044f245  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f248  e8f337ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f24d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044f24f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f251  741f                   -je 0x44f272
    if (cpu.flags.zf)
    {
        goto L_0x0044f272;
    }
    // 0044f253  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044f25a  7f09                   -jg 0x44f265
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0044f265;
    }
    // 0044f25c  833dacd36f0000         +cmp dword ptr [0x6fd3ac], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328684) /* 0x6fd3ac */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044f263  7506                   -jne 0x44f26b
    if (!cpu.flags.zf)
    {
        goto L_0x0044f26b;
    }
L_0x0044f265:
    // 0044f265  804a0401               +or byte ptr [edx + 4], 1
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 0044f269  eb07                   -jmp 0x44f272
    goto L_0x0044f272;
L_0x0044f26b:
    // 0044f26b  c7406410e74400         -mov dword ptr [eax + 0x64], 0x44e710
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4515600 /*0x44e710*/;
L_0x0044f272:
    // 0044f272  ba38955300             -mov edx, 0x539538
    cpu.edx = 5477688 /*0x539538*/;
    // 0044f277  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f27a  e8c137ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f27f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044f281  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044f283  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f285  7440                   -je 0x44f2c7
    if (cpu.flags.zf)
    {
        goto L_0x0044f2c7;
    }
    // 0044f287  833dacd36f0000         +cmp dword ptr [0x6fd3ac], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328684) /* 0x6fd3ac */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044f28e  7506                   -jne 0x44f296
    if (!cpu.flags.zf)
    {
        goto L_0x0044f296;
    }
    // 0044f290  80480401               +or byte ptr [eax + 4], 1
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 0044f294  eb31                   -jmp 0x44f2c7
    goto L_0x0044f2c7;
L_0x0044f296:
    // 0044f296  e8a5fd0700             -call 0x4cf040
    cpu.esp -= 4;
    sub_4cf040(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f29b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f29d  7409                   -je 0x44f2a8
    if (cpu.flags.zf)
    {
        goto L_0x0044f2a8;
    }
    // 0044f29f  c74264c0e64400         -mov dword ptr [edx + 0x64], 0x44e6c0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(100) /* 0x64 */) = 4515520 /*0x44e6c0*/;
    // 0044f2a6  eb1f                   -jmp 0x44f2c7
    goto L_0x0044f2c7;
L_0x0044f2a8:
    // 0044f2a8  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044f2af  7e0f                   -jle 0x44f2c0
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0044f2c0;
    }
    // 0044f2b1  833d0cd56f0000         +cmp dword ptr [0x6fd50c], 0
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
    // 0044f2b8  7406                   -je 0x44f2c0
    if (cpu.flags.zf)
    {
        goto L_0x0044f2c0;
    }
    // 0044f2ba  804a0401               +or byte ptr [edx + 4], 1
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 0044f2be  eb07                   -jmp 0x44f2c7
    goto L_0x0044f2c7;
L_0x0044f2c0:
    // 0044f2c0  c74164a0e64400         -mov dword ptr [ecx + 0x64], 0x44e6a0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(100) /* 0x64 */) = 4515488 /*0x44e6a0*/;
L_0x0044f2c7:
    // 0044f2c7  ba94945300             -mov edx, 0x539494
    cpu.edx = 5477524 /*0x539494*/;
    // 0044f2cc  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f2cf  e86c37ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f2d4  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044f2d6  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044f2d8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f2da  7448                   -je 0x44f324
    if (cpu.flags.zf)
    {
        goto L_0x0044f324;
    }
    // 0044f2dc  e8affd0700             -call 0x4cf090
    cpu.esp -= 4;
    sub_4cf090(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f2e1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f2e3  7504                   -jne 0x44f2e9
    if (!cpu.flags.zf)
    {
        goto L_0x0044f2e9;
    }
    // 0044f2e5  804a0401               -or byte ptr [edx + 4], 1
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044f2e9:
    // 0044f2e9  b8f2000000             -mov eax, 0xf2
    cpu.eax = 242 /*0xf2*/;
    // 0044f2ee  e85d250800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f2f3  8b15b8d36f00           -mov edx, dword ptr [0x6fd3b8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044f2f9  89413c                 -mov dword ptr [ecx + 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */) = cpu.eax;
    // 0044f2fc  83fa02                 +cmp edx, 2
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
    // 0044f2ff  750d                   -jne 0x44f30e
    if (!cpu.flags.zf)
    {
        goto L_0x0044f30e;
    }
    // 0044f301  b8f1000000             -mov eax, 0xf1
    cpu.eax = 241 /*0xf1*/;
    // 0044f306  e845250800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f30b  89413c                 -mov dword ptr [ecx + 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */) = cpu.eax;
L_0x0044f30e:
    // 0044f30e  833db8d36f0001         +cmp dword ptr [0x6fd3b8], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044f315  750d                   -jne 0x44f324
    if (!cpu.flags.zf)
    {
        goto L_0x0044f324;
    }
    // 0044f317  b8f1000000             -mov eax, 0xf1
    cpu.eax = 241 /*0xf1*/;
    // 0044f31c  e82f250800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f321  89413c                 -mov dword ptr [ecx + 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */) = cpu.eax;
L_0x0044f324:
    // 0044f324  ba44955300             -mov edx, 0x539544
    cpu.edx = 5477700 /*0x539544*/;
    // 0044f329  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f32c  e80f37ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f331  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0044f333  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0044f335  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f337  7453                   -je 0x44f38c
    if (cpu.flags.zf)
    {
        goto L_0x0044f38c;
    }
    // 0044f339  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044f33e  8b15f4d46f00           -mov edx, dword ptr [0x6fd4f4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 0044f344  e897610000             -call 0x4554e0
    cpu.esp -= 4;
    sub_4554e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f349  8b35acd36f00           -mov esi, dword ptr [0x6fd3ac]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328684) /* 0x6fd3ac */);
    // 0044f34f  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0044f351  7506                   -jne 0x44f359
    if (!cpu.flags.zf)
    {
        goto L_0x0044f359;
    }
    // 0044f353  80490401               +or byte ptr [ecx + 4], 1
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 0044f357  eb33                   -jmp 0x44f38c
    goto L_0x0044f38c;
L_0x0044f359:
    // 0044f359  833db0d36f0000         +cmp dword ptr [0x6fd3b0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044f360  7526                   -jne 0x44f388
    if (!cpu.flags.zf)
    {
        goto L_0x0044f388;
    }
    // 0044f362  8b15b8d36f00           -mov edx, dword ptr [0x6fd3b8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044f368  83fa01                 +cmp edx, 1
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
    // 0044f36b  741b                   -je 0x44f388
    if (cpu.flags.zf)
    {
        goto L_0x0044f388;
    }
    // 0044f36d  83fa02                 +cmp edx, 2
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
    // 0044f370  7416                   -je 0x44f388
    if (cpu.flags.zf)
    {
        goto L_0x0044f388;
    }
    // 0044f372  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f374  7512                   -jne 0x44f388
    if (!cpu.flags.zf)
    {
        goto L_0x0044f388;
    }
    // 0044f376  8a5104                 -mov dl, byte ptr [ecx + 4]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0044f379  c74164a0e44400         -mov dword ptr [ecx + 0x64], 0x44e4a0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(100) /* 0x64 */) = 4514976 /*0x44e4a0*/;
    // 0044f380  80e2fe                 +and dl, 0xfe
    cpu.clear_co();
    cpu.set_szp((cpu.dl &= x86::reg8(x86::sreg8(254 /*0xfe*/))));
    // 0044f383  885104                 -mov byte ptr [ecx + 4], dl
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(4) /* 0x4 */) = cpu.dl;
    // 0044f386  eb04                   -jmp 0x44f38c
    goto L_0x0044f38c;
L_0x0044f388:
    // 0044f388  804b0401               -or byte ptr [ebx + 4], 1
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0044f38c:
    // 0044f38c  ba50955300             -mov edx, 0x539550
    cpu.edx = 5477712 /*0x539550*/;
    // 0044f391  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f394  e8a736ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f399  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044f39b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f39d  743d                   -je 0x44f3dc
    if (cpu.flags.zf)
    {
        goto L_0x0044f3dc;
    }
    // 0044f39f  8b3db8d36f00           -mov edi, dword ptr [0x6fd3b8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0044f3a5  83ff01                 +cmp edi, 1
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
    // 0044f3a8  752c                   -jne 0x44f3d6
    if (!cpu.flags.zf)
    {
        goto L_0x0044f3d6;
    }
    // 0044f3aa  3b3db0d36f00           +cmp edi, dword ptr [0x6fd3b0]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044f3b0  7c24                   -jl 0x44f3d6
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0044f3d6;
    }
    // 0044f3b2  668b5804               -mov bx, word ptr [eax + 4]
    cpu.bx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0044f3b6  c7406480e44400         -mov dword ptr [eax + 0x64], 0x44e480
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4514944 /*0x44e480*/;
    // 0044f3bd  81e3feefffff           -and ebx, 0xffffeffe
    cpu.ebx &= x86::reg32(x86::sreg32(4294963198 /*0xffffeffe*/));
    // 0044f3c3  66895804               -mov word ptr [eax + 4], bx
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.bx;
    // 0044f3c7  e8c4fc0700             -call 0x4cf090
    cpu.esp -= 4;
    sub_4cf090(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f3cc  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f3ce  750c                   -jne 0x44f3dc
    if (!cpu.flags.zf)
    {
        goto L_0x0044f3dc;
    }
    // 0044f3d0  804a0401               +or byte ptr [edx + 4], 1
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 0044f3d4  eb06                   -jmp 0x44f3dc
    goto L_0x0044f3dc;
L_0x0044f3d6:
    // 0044f3d6  668148040110           -or word ptr [eax + 4], 0x1001
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg16(x86::sreg16(4097 /*0x1001*/));
L_0x0044f3dc:
    // 0044f3dc  ba5c955300             -mov edx, 0x53955c
    cpu.edx = 5477724 /*0x53955c*/;
    // 0044f3e1  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f3e4  e85736ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f3e9  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044f3eb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f3ed  743c                   -je 0x44f42b
    if (cpu.flags.zf)
    {
        goto L_0x0044f42b;
    }
    // 0044f3ef  833db8d36f0002         +cmp dword ptr [0x6fd3b8], 2
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
    // 0044f3f6  752d                   -jne 0x44f425
    if (!cpu.flags.zf)
    {
        goto L_0x0044f425;
    }
    // 0044f3f8  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044f3ff  7f24                   -jg 0x44f425
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0044f425;
    }
    // 0044f401  668b7004               -mov si, word ptr [eax + 4]
    cpu.si = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0044f405  c7406460e44400         -mov dword ptr [eax + 0x64], 0x44e460
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4514912 /*0x44e460*/;
    // 0044f40c  81e6feefffff           -and esi, 0xffffeffe
    cpu.esi &= x86::reg32(x86::sreg32(4294963198 /*0xffffeffe*/));
    // 0044f412  66897004               -mov word ptr [eax + 4], si
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.si;
    // 0044f416  e875fc0700             -call 0x4cf090
    cpu.esp -= 4;
    sub_4cf090(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f41b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f41d  750c                   -jne 0x44f42b
    if (!cpu.flags.zf)
    {
        goto L_0x0044f42b;
    }
    // 0044f41f  804a0401               +or byte ptr [edx + 4], 1
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 0044f423  eb06                   -jmp 0x44f42b
    goto L_0x0044f42b;
L_0x0044f425:
    // 0044f425  668148040110           -or word ptr [eax + 4], 0x1001
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */) |= x86::reg16(x86::sreg16(4097 /*0x1001*/));
L_0x0044f42b:
    // 0044f42b  ba64955300             -mov edx, 0x539564
    cpu.edx = 5477732 /*0x539564*/;
    // 0044f430  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f433  e80836ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f438  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0044f43a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f43c  7440                   -je 0x44f47e
    if (cpu.flags.zf)
    {
        goto L_0x0044f47e;
    }
    // 0044f43e  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044f445  7e09                   -jle 0x44f450
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0044f450;
    }
    // 0044f447  c7406400e64400         -mov dword ptr [eax + 0x64], 0x44e600
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4515328 /*0x44e600*/;
    // 0044f44e  eb07                   -jmp 0x44f457
    goto L_0x0044f457;
L_0x0044f450:
    // 0044f450  c74064b0e54400         -mov dword ptr [eax + 0x64], 0x44e5b0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4515248 /*0x44e5b0*/;
L_0x0044f457:
    // 0044f457  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 0044f45e  7e11                   -jle 0x44f471
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0044f471;
    }
    // 0044f460  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 0044f468  7507                   -jne 0x44f471
    if (!cpu.flags.zf)
    {
        goto L_0x0044f471;
    }
    // 0044f46a  b8a9010000             -mov eax, 0x1a9
    cpu.eax = 425 /*0x1a9*/;
    // 0044f46f  eb05                   -jmp 0x44f476
    goto L_0x0044f476;
L_0x0044f471:
    // 0044f471  b805010000             -mov eax, 0x105
    cpu.eax = 261 /*0x105*/;
L_0x0044f476:
    // 0044f476  e8d5230800             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f47b  89423c                 -mov dword ptr [edx + 0x3c], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */) = cpu.eax;
L_0x0044f47e:
    // 0044f47e  ba70955300             -mov edx, 0x539570
    cpu.edx = 5477744 /*0x539570*/;
    // 0044f483  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0044f486  e8b535ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f48b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f48d  7407                   -je 0x44f496
    if (cpu.flags.zf)
    {
        goto L_0x0044f496;
    }
    // 0044f48f  c7406490e64400         -mov dword ptr [eax + 0x64], 0x44e690
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4515472 /*0x44e690*/;
L_0x0044f496:
    // 0044f496  bad4945300             -mov edx, 0x5394d4
    cpu.edx = 5477588 /*0x5394d4*/;
    // 0044f49b  e810baffff             -call 0x44aeb0
    cpu.esp -= 4;
    nfs3hp::port::loop::currentMenu(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f4a0  e89b35ffff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f4a5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0044f4a7  7406                   -je 0x44f4af
    if (cpu.flags.zf)
    {
        goto L_0x0044f4af;
    }
    // 0044f4a9  66816004feef           -and word ptr [eax + 4], 0xeffe
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */) &= x86::reg16(x86::sreg16(61438 /*0xeffe*/));
L_0x0044f4af:
    // 0044f4af  8b154cbb6f00           -mov edx, dword ptr [0x6fbb4c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7322444) /* 0x6fbb4c */);
    // 0044f4b5  83fa01                 +cmp edx, 1
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
    // 0044f4b8  7405                   -je 0x44f4bf
    if (cpu.flags.zf)
    {
        goto L_0x0044f4bf;
    }
    // 0044f4ba  83fa05                 +cmp edx, 5
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
    // 0044f4bd  7528                   -jne 0x44f4e7
    if (!cpu.flags.zf)
    {
        goto L_0x0044f4e7;
    }
L_0x0044f4bf:
    // 0044f4bf  a1f8935500             -mov eax, dword ptr [0x5593f8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608440) /* 0x5593f8 */);
    // 0044f4c4  66c740424800           -mov word ptr [eax + 0x42], 0x48
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(66) /* 0x42 */) = 72 /*0x48*/;
    // 0044f4ca  66c740445900           -mov word ptr [eax + 0x44], 0x59
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(68) /* 0x44 */) = 89 /*0x59*/;
    // 0044f4d0  66c740462a00           -mov word ptr [eax + 0x46], 0x2a
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(70) /* 0x46 */) = 42 /*0x2a*/;
    // 0044f4d6  a100945500             -mov eax, dword ptr [0x559400]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608448) /* 0x559400 */);
    // 0044f4db  66c7403ca000           -mov word ptr [eax + 0x3c], 0xa0
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(60) /* 0x3c */) = 160 /*0xa0*/;
    // 0044f4e1  66c7403e8c00           -mov word ptr [eax + 0x3e], 0x8c
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(62) /* 0x3e */) = 140 /*0x8c*/;
L_0x0044f4e7:
    // 0044f4e7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044f4e9  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0044f4eb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044f4ec  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044f4ed  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044f4ee  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044f4ef  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044f4f0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044f4f1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_44f500(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044f500  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044f501  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044f503  e8c8a40700             -call 0x4c99d0
    cpu.esp -= 4;
    nfs3hp::port::spokenStop(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0044f508  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044f50a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044f50b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_44f510: native, nfs3hp::load::loadRecords */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_44f5d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0044f5d0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0044f5d1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0044f5d3  833d0894550000         +cmp dword ptr [0x559408], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5608456) /* 0x559408 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0044f5da  7504                   -jne 0x44f5e0
    if (!cpu.flags.zf)
    {
        goto L_0x0044f5e0;
    }
    // 0044f5dc  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0044f5de  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044f5df  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0044f5e0:
    // 0044f5e0  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0044f5e5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0044f5e6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_44f5f0: native, nfs3hp::load::saveRecords */

}
