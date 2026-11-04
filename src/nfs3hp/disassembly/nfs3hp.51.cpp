#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_5261cc: native, nfs3hp::dead::stub_0x5261cc */

/* align: skip 0x8b 0xc0 */
void Application::sub_5261fc(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005261fc  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005261fd  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 005261fe  803d24b1560000         +cmp byte ptr [0x56b124], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5681444) /* 0x56b124 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00526205  741a                   -je 0x526221
    if (cpu.flags.zf)
    {
        goto L_0x00526221;
    }
    // 00526207  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00526209  682c615200             -push 0x52612c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5398828 /*0x52612c*/;
    cpu.esp -= 4;
    // 0052620e  2eff15cc455300         -call dword ptr cs:[0x5345cc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457356) /* 0x5345cc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526215  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00526217  7408                   -je 0x526221
    if (cpu.flags.zf)
    {
        goto L_0x00526221;
    }
    // 00526219  30d2                   -xor dl, dl
    cpu.dl ^= x86::reg8(x86::sreg8(cpu.dl));
    // 0052621b  881524b15600           -mov byte ptr [0x56b124], dl
    app->getMemory<x86::reg8>(x86::reg32(5681444) /* 0x56b124 */) = cpu.dl;
L_0x00526221:
    // 00526221  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00526223  a024b15600             -mov al, byte ptr [0x56b124]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5681444) /* 0x56b124 */);
    // 00526228  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052622a  0f94c0                 -sete al
    cpu.al = cpu.flags.zf;
    // 0052622d  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00526232  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526233  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526234  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_526238: native, nfs3hp::dead::stub_0x526238 */

/* align: skip 0x8b 0xc0 */
void Application::sub_526244(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00526244  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00526245  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00526246  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00526247  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00526249  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0052624e  e875feffff             -call 0x5260c8
    cpu.esp -= 4;
    sub_5260c8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526253  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00526255  83f801                 +cmp eax, 1
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
    // 00526258  7425                   -je 0x52627f
    if (cpu.flags.zf)
    {
        goto L_0x0052627f;
    }
    // 0052625a  83f802                 +cmp eax, 2
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
    // 0052625d  7420                   -je 0x52627f
    if (cpu.flags.zf)
    {
        goto L_0x0052627f;
    }
    // 0052625f  83f803                 +cmp eax, 3
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
    // 00526262  741b                   -je 0x52627f
    if (cpu.flags.zf)
    {
        goto L_0x0052627f;
    }
    // 00526264  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 00526269  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0052626b  e810feffff             -call 0x526080
    cpu.esp -= 4;
    sub_526080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526270  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00526275  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00526277  ffd1                   -call ecx
    cpu.ip = cpu.ecx;
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526279  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052627b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052627c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052627d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052627e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052627f:
    // 0052627f  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 00526284  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526285  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526286  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526287  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_526288: native, nfs3hp::dead::stub_0x526288 */

/* align: skip 0x90 */
void Application::sub_526338(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00526338  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00526339  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052633a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052633b  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0052633d  e886fdffff             -call 0x5260c8
    cpu.esp -= 4;
    sub_5260c8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526342  8d53ff                 -lea edx, [ebx - 1]
    cpu.edx = x86::reg32(cpu.ebx + x86::reg32(-1) /* -0x1 */);
    // 00526345  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00526347  83fa0b                 +cmp edx, 0xb
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(11 /*0xb*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052634a  774d                   -ja 0x526399
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00526399;
    }
    // 0052634c  2eff249508635200       -jmp dword ptr cs:[edx*4 + 0x526308]
    cpu.ip = app->getMemory<x86::reg32>(5399304 + cpu.edx * 4); goto dynamic_jump;
  case 0x00526354:
    // 00526354  b88c000000             -mov eax, 0x8c
    cpu.eax = 140 /*0x8c*/;
    // 00526359  e8e6feffff             -call 0x526244
    cpu.esp -= 4;
    sub_526244(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052635e  eb42                   -jmp 0x5263a2
    goto L_0x005263a2;
  case 0x00526360:
    // 00526360  83f802                 +cmp eax, 2
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
    // 00526363  7505                   -jne 0x52636a
    if (!cpu.flags.zf)
    {
        goto L_0x0052636a;
    }
    // 00526365  e80e650000             -call 0x52c878
    cpu.esp -= 4;
    sub_52c878(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
  [[fallthrough]];
  case 0x0052636a:
L_0x0052636a:
    // 0052636a  83f901                 +cmp ecx, 1
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
    // 0052636d  741a                   -je 0x526389
    if (cpu.flags.zf)
    {
        goto L_0x00526389;
    }
    // 0052636f  83f902                 +cmp ecx, 2
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
    // 00526372  7415                   -je 0x526389
    if (cpu.flags.zf)
    {
        goto L_0x00526389;
    }
    // 00526374  83f903                 +cmp ecx, 3
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
    // 00526377  7410                   -je 0x526389
    if (cpu.flags.zf)
    {
        goto L_0x00526389;
    }
    // 00526379  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 0052637e  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00526380  e8fbfcffff             -call 0x526080
    cpu.esp -= 4;
    sub_526080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526385  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00526387  ffd1                   -call ecx
    cpu.ip = cpu.ecx;
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00526389:
    // 00526389  e806feffff             -call 0x526194
    cpu.esp -= 4;
    sub_526194(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052638e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00526390  7510                   -jne 0x5263a2
    if (!cpu.flags.zf)
    {
        goto L_0x005263a2;
    }
    // 00526392  e865feffff             -call 0x5261fc
    cpu.esp -= 4;
    sub_5261fc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526397  eb09                   -jmp 0x5263a2
    goto L_0x005263a2;
L_0x00526399:
    // 00526399  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052639e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052639f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005263a0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005263a1  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005263a2:
    // 005263a2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005263a4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005263a5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005263a6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005263a7  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip  */
void Application::sub_5263a8(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005263a8  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 005263a9  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 005263aa  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005263ab  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005263ac  06                     -push es
    app->getMemory<x86::reg16>(cpu.esp-4) = cpu.es;
    cpu.esp -= 4;
    // 005263ad  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
L_0x005263b2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005263b2  ff1564775600           -call dword ptr [0x567764]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666660) /* 0x567764 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005263b8  8d3410                 -lea esi, [eax + edx]
    cpu.esi = x86::reg32(cpu.eax + cpu.edx * 1);
    // 005263bb  8cd8                   -mov eax, ds
    cpu.eax = cpu.ds;
    // 005263bd  8ec0                   -mov es, eax
    cpu.es = cpu.eax;
    // 005263bf  8d7e58                 -lea edi, [esi + 0x58]
    cpu.edi = x86::reg32(cpu.esi + x86::reg32(88) /* 0x58 */);
    // 005263c2  8db2bcb05600           -lea esi, [edx + 0x56b0bc]
    cpu.esi = x86::reg32(cpu.edx + x86::reg32(5681340) /* 0x56b0bc */);
    // 005263c8  83c208                 -add edx, 8
    (cpu.edx) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 005263cb  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005263cc  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005263cd  83fa68                 +cmp edx, 0x68
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(104 /*0x68*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005263d0  75e0                   -jne 0x5263b2
    if (!cpu.flags.zf)
    {
        goto L_0x005263b2;
    }
    // 005263d2  ba10615200             -mov edx, 0x526110
    cpu.edx = 5398800 /*0x526110*/;
    // 005263d7  bb38635200             -mov ebx, 0x526338
    cpu.ebx = 5399352 /*0x526338*/;
    // 005263dc  8915d4ac5600           -mov dword ptr [0x56acd4], edx
    app->getMemory<x86::reg32>(x86::reg32(5680340) /* 0x56acd4 */) = cpu.edx;
    // 005263e2  891dd8ac5600           -mov dword ptr [0x56acd8], ebx
    app->getMemory<x86::reg32>(x86::reg32(5680344) /* 0x56acd8 */) = cpu.ebx;
    // 005263e8  07                     -pop es
    cpu.es = app->getMemory<x86::reg16>(cpu.esp);
    cpu.esp += 4;
    // 005263e9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005263ea  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005263eb  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005263ec  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005263ed  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_5263f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005263f0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 005263f1  e89efdffff             -call 0x526194
    cpu.esp -= 4;
    sub_526194(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005263f6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005263f8  7423                   -je 0x52641d
    if (cpu.flags.zf)
    {
        goto L_0x0052641d;
    }
    // 005263fa  e8fdfdffff             -call 0x5261fc
    cpu.esp -= 4;
    sub_5261fc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005263ff  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 00526404  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 00526409  e872fcffff             -call 0x526080
    cpu.esp -= 4;
    sub_526080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052640e  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 00526413  b807000000             -mov eax, 7
    cpu.eax = 7 /*0x7*/;
    // 00526418  e863fcffff             -call 0x526080
    cpu.esp -= 4;
    sub_526080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052641d:
    // 0052641d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052641e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_526420(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00526420  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00526421  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00526422  baa8635200             -mov edx, 0x5263a8
    cpu.edx = 5399464 /*0x5263a8*/;
    // 00526427  bbf0635200             -mov ebx, 0x5263f0
    cpu.ebx = 5399536 /*0x5263f0*/;
    // 0052642c  8915a4775600           -mov dword ptr [0x5677a4], edx
    app->getMemory<x86::reg32>(x86::reg32(5666724) /* 0x5677a4 */) = cpu.edx;
    // 00526432  891da8775600           -mov dword ptr [0x5677a8], ebx
    app->getMemory<x86::reg32>(x86::reg32(5666728) /* 0x5677a8 */) = cpu.ebx;
    // 00526438  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526439  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052643a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 */
void Application::sub_52643c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fpa1 = cpu.fpu.st(0);
  x86::Float fp0;
    // 0052643c  09db                   +or ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0052643e  750b                   -jne 0x52644b
    if (!cpu.flags.zf)
    {
        goto L_0x0052644b;
    }
    // 00526440  01c9                   +add ecx, ecx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526442  7505                   -jne 0x526449
    if (!cpu.flags.zf)
    {
        goto L_0x00526449;
    }
    // 00526444  e952640000             -jmp 0x52c89b
    cpu.fpu.st(0) = fpa1;
    return sub_52c89b(app, cpu.handOver());
L_0x00526449:
    // 00526449  d1d9                   -rcr ecx, 1
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
L_0x0052644b:
    // 0052644b  09c0                   +or eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax |= x86::reg32(x86::sreg32(cpu.eax))));
    // 0052644d  7507                   -jne 0x526456
    if (!cpu.flags.zf)
    {
        goto L_0x00526456;
    }
    // 0052644f  01d2                   +add edx, edx
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
    // 00526451  7501                   -jne 0x526454
    if (!cpu.flags.zf)
    {
        goto L_0x00526454;
    }
    // 00526453  c3                     -ret 
    cpu.fpu.st(0) = fpa1;
    cpu.esp += 4;
    return;
L_0x00526454:
    // 00526454  d1da                   -rcr edx, 1
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
L_0x00526456:
    // 00526456  803dc144560000         +cmp byte ptr [0x5644c1], 0
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
    // 0052645d  7430                   -je 0x52648f
    if (cpu.flags.zf)
    {
        goto L_0x0052648f;
    }
    // 0052645f  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00526460  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00526461  dd0424                 -fld qword ptr [esp]
    fp0 = x86::Float(app->getMemory<double>(cpu.esp));
    // 00526464  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00526465  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00526466  f6055878560001         +test byte ptr [0x567858], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5666904) /* 0x567858 */) & 1 /*0x1*/));
    // 0052646d  7407                   -je 0x526476
    if (cpu.flags.zf)
    {
        goto L_0x00526476;
    }
    // 0052646f  e8980b0000             -call 0x52700c
    cpu.fpu.push(fpa1);
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    sub_52700c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    fpa1 = cpu.fpu.take();
    // 00526474  eb06                   -jmp 0x52647c
    goto L_0x0052647c;
L_0x00526476:
    // 00526476  dc3424                 -fdiv qword ptr [esp]
    fp0 = fp::div(fp0, x86::Float(app->getMemory<double>(cpu.esp)));
    // 00526479  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
L_0x0052647c:
    // 0052647c  dd1c24                 -fstp qword ptr [esp]
    app->getMemory<double>(cpu.esp) = double(fp0);
    // 0052647f  9b                     -wait 
    /*nothing*/;
    // 00526480  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526481  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526482  81fa00000080           +cmp edx, 0x80000000
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00526488  7504                   -jne 0x52648e
    if (!cpu.flags.zf)
    {
        goto L_0x0052648e;
    }
    // 0052648a  29d2                   -sub edx, edx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0052648c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x0052648e:
    // 0052648e  c3                     -ret 
    cpu.fpu.st(0) = fpa1;
    cpu.esp += 4;
    return;
L_0x0052648f:
    // 0052648f  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00526490  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00526492  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00526493  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00526494  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00526496  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 00526498  c1ff14                 -sar edi, 0x14
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (20 /*0x14*/ % 32));
    // 0052649b  c1f914                 -sar ecx, 0x14
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (20 /*0x14*/ % 32));
    // 0052649e  81e7ff070080           -and edi, 0x800007ff
    cpu.edi &= x86::reg32(x86::sreg32(2147485695 /*0x800007ff*/));
    // 005264a4  81e1ff070080           -and ecx, 0x800007ff
    cpu.ecx &= x86::reg32(x86::sreg32(2147485695 /*0x800007ff*/));
    // 005264aa  c1c710                 -rol edi, 0x10
    {
        x86::reg32& op = cpu.edi;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005264ad  c1c110                 -rol ecx, 0x10
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005264b0  6601cf                 -add di, cx
    (cpu.di) += x86::reg16(x86::sreg16(cpu.cx));
    // 005264b3  c1c710                 -rol edi, 0x10
    {
        x86::reg32& op = cpu.edi;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005264b6  c1c110                 -rol ecx, 0x10
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005264b9  81e2ffff0f00           -and edx, 0xfffff
    cpu.edx &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 005264bf  81e6ffff0f00           -and esi, 0xfffff
    cpu.esi &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 005264c5  6609ff                 +or di, di
    cpu.clear_co();
    cpu.set_szp((cpu.di |= x86::reg16(x86::sreg16(cpu.di))));
    // 005264c8  7408                   -je 0x5264d2
    if (cpu.flags.zf)
    {
        goto L_0x005264d2;
    }
    // 005264ca  81ca00001000           +or edx, 0x100000
    cpu.clear_co();
    cpu.set_szp((cpu.edx |= x86::reg32(x86::sreg32(1048576 /*0x100000*/))));
    // 005264d0  eb0e                   -jmp 0x5264e0
    goto L_0x005264e0;
L_0x005264d2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005264d2  01c0                   +add eax, eax
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
    // 005264d4  11d2                   -adc edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 005264d6  664f                   -dec di
    (cpu.di)--;
    // 005264d8  f7c200001000           +test edx, 0x100000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 1048576 /*0x100000*/));
    // 005264de  74f2                   -je 0x5264d2
    if (cpu.flags.zf)
    {
        goto L_0x005264d2;
    }
L_0x005264e0:
    // 005264e0  6609c9                 +or cx, cx
    cpu.clear_co();
    cpu.set_szp((cpu.cx |= x86::reg16(x86::sreg16(cpu.cx))));
    // 005264e3  7408                   -je 0x5264ed
    if (cpu.flags.zf)
    {
        goto L_0x005264ed;
    }
    // 005264e5  81ce00001000           +or esi, 0x100000
    cpu.clear_co();
    cpu.set_szp((cpu.esi |= x86::reg32(x86::sreg32(1048576 /*0x100000*/))));
    // 005264eb  eb0e                   -jmp 0x5264fb
    goto L_0x005264fb;
L_0x005264ed:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005264ed  01db                   +add ebx, ebx
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005264ef  11f6                   -adc esi, esi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.esi) + cpu.flags.cf);
    // 005264f1  6649                   -dec cx
    (cpu.cx)--;
    // 005264f3  f7c600001000           +test esi, 0x100000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & 1048576 /*0x100000*/));
    // 005264f9  74f2                   -je 0x5264ed
    if (cpu.flags.zf)
    {
        goto L_0x005264ed;
    }
L_0x005264fb:
    // 005264fb  6629cf                 -sub di, cx
    (cpu.di) -= x86::reg16(x86::sreg16(cpu.cx));
    // 005264fe  6681c7ff03             +add di, 0x3ff
    {
        x86::reg16& tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1023 /*0x3ff*/));
        x86::reg16 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) == (1 & (tmp2 >> 15));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526503  7811                   -js 0x526516
    if (cpu.flags.sf)
    {
        goto L_0x00526516;
    }
    // 00526505  6681ffff07             +cmp di, 0x7ff
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2047 /*0x7ff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0052650a  720a                   -jb 0x526516
    if (cpu.flags.cf)
    {
        goto L_0x00526516;
    }
    // 0052650c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052650e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052650f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526510  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526511  e991630000             -jmp 0x52c8a7
    cpu.fpu.st(0) = fpa1;
    return sub_52c8a7(app, cpu.handOver());
L_0x00526516:
    // 00526516  6683ffcc               +cmp di, -0x34
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(-52 /*-0x34*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0052651a  7d08                   -jge 0x526524
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00526524;
    }
    // 0052651c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052651d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052651e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052651f  e966630000             -jmp 0x52c88a
    cpu.fpu.st(0) = fpa1;
    return sub_52c88a(app, cpu.handOver());
L_0x00526524:
    // 00526524  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00526525  b10b                   -mov cl, 0xb
    cpu.cl = 11 /*0xb*/;
    // 00526527  0fa5c2                 -shld edx, eax, cl
    {
        x86::reg32& destination = cpu.edx;
        destination <<= (cpu.cl % 32);
        destination |= cpu.eax >> (32 - (cpu.cl % 32));
    }
    // 0052652a  0fa5e8                 -shld eax, ebp, cl
    {
        x86::reg32& destination = cpu.eax;
        destination <<= (cpu.cl % 32);
        destination |= cpu.ebp >> (32 - (cpu.cl % 32));
    }
    // 0052652d  2500f8ffff             -and eax, 0xfffff800
    cpu.eax &= x86::reg32(x86::sreg32(4294965248 /*0xfffff800*/));
    // 00526532  0fa5de                 -shld esi, ebx, cl
    {
        x86::reg32& destination = cpu.esi;
        destination <<= (cpu.cl % 32);
        destination |= cpu.ebx >> (32 - (cpu.cl % 32));
    }
    // 00526535  0fa5eb                 -shld ebx, ebp, cl
    {
        x86::reg32& destination = cpu.ebx;
        destination <<= (cpu.cl % 32);
        destination |= cpu.ebp >> (32 - (cpu.cl % 32));
    }
    // 00526538  81e300f8ffff           -and ebx, 0xfffff800
    cpu.ebx &= x86::reg32(x86::sreg32(4294965248 /*0xfffff800*/));
    // 0052653e  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052653f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00526540  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 00526542  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00526544  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00526546  29c0                   -sub eax, eax
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00526548  39d1                   +cmp ecx, edx
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
    // 0052654a  7703                   -ja 0x52654f
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052654f;
    }
    // 0052654c  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0052654e  40                     -inc eax
    (cpu.eax)++;
L_0x0052654f:
    // 0052654f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00526550  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00526552  f7f1                   -div ecx
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = cpu.ecx;
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 00526554  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00526555  93                     -xchg ebx, eax
    {
        x86::reg32 tmp = cpu.ebx;
        cpu.ebx = cpu.eax;
        cpu.eax = tmp;
    }
    // 00526556  f7e3                   -mul ebx
    cpu.edx_eax = x86::reg64(cpu.eax) * x86::reg64(cpu.ebx);
    // 00526558  91                     -xchg ecx, eax
    {
        x86::reg32 tmp = cpu.ecx;
        cpu.ecx = cpu.eax;
        cpu.eax = tmp;
    }
    // 00526559  87d3                   -xchg ebx, edx
    {
        x86::reg32 tmp = cpu.ebx;
        cpu.ebx = cpu.edx;
        cpu.edx = tmp;
    }
    // 0052655b  f7e2                   -mul edx
    cpu.edx_eax = x86::reg64(cpu.eax) * x86::reg64(cpu.edx);
    // 0052655d  01d8                   +add eax, ebx
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
    // 0052655f  83d200                 -adc edx, 0
    (cpu.edx) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 00526562  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00526565  f645e801               +test byte ptr [ebp - 0x18], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-24) /* -0x18 */) & 1 /*0x1*/));
    // 00526569  7405                   -je 0x526570
    if (cpu.flags.zf)
    {
        goto L_0x00526570;
    }
    // 0052656b  01d8                   +add eax, ebx
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
    // 0052656d  1355f0                 -adc edx, dword ptr [ebp - 0x10]
    (cpu.edx) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */)) + cpu.flags.cf);
L_0x00526570:
    // 00526570  f7d9                   +neg ecx
    {
        x86::reg32 tmp1 = 0;
        x86::reg32& tmp2 = cpu.ecx;
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp2 = result;
        cpu.set_szp(tmp2);
    }
    // 00526572  19c6                   +sbb esi, eax
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 - tmp2 - carry;
        cpu.flags.cf = tmp1 < tmp2 || (carry && tmp1 == tmp2);
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526574  19d7                   +sbb edi, edx
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 - tmp2 - carry;
        cpu.flags.cf = tmp1 < tmp2 || (carry && tmp1 == tmp2);
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526576  7412                   -je 0x52658a
    if (cpu.flags.zf)
    {
        goto L_0x0052658a;
    }
L_0x00526578:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00526578  836de401               +sub dword ptr [ebp - 0x1c], 1
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052657c  835de800               -sbb dword ptr [ebp - 0x18], 0
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */)) -= x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 00526580  01d9                   +add ecx, ebx
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
    // 00526582  1375f0                 +adc esi, dword ptr [ebp - 0x10]
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526585  83d700                 +adc edi, 0
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526588  75ee                   -jne 0x526578
    if (!cpu.flags.zf)
    {
        goto L_0x00526578;
    }
L_0x0052658a:
    // 0052658a  89f7                   -mov edi, esi
    cpu.edi = cpu.esi;
    // 0052658c  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 0052658e  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00526591  39f9                   +cmp ecx, edi
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
    // 00526593  770a                   -ja 0x52659f
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052659f;
    }
    // 00526595  29cf                   -sub edi, ecx
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00526597  8345e401               +add dword ptr [ebp - 0x1c], 1
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052659b  8355e800               -adc dword ptr [ebp - 0x18], 0
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */)) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
L_0x0052659f:
    // 0052659f  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 005265a1  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 005265a3  f7f1                   -div ecx
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = cpu.ecx;
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 005265a5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 005265a6  09c0                   +or eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax |= x86::reg32(x86::sreg32(cpu.eax))));
    // 005265a8  742c                   -je 0x5265d6
    if (cpu.flags.zf)
    {
        goto L_0x005265d6;
    }
    // 005265aa  93                     -xchg ebx, eax
    {
        x86::reg32 tmp = cpu.ebx;
        cpu.ebx = cpu.eax;
        cpu.eax = tmp;
    }
    // 005265ab  f7e3                   -mul ebx
    cpu.edx_eax = x86::reg64(cpu.eax) * x86::reg64(cpu.ebx);
    // 005265ad  91                     -xchg ecx, eax
    {
        x86::reg32 tmp = cpu.ecx;
        cpu.ecx = cpu.eax;
        cpu.eax = tmp;
    }
    // 005265ae  87d3                   -xchg ebx, edx
    {
        x86::reg32 tmp = cpu.ebx;
        cpu.ebx = cpu.edx;
        cpu.edx = tmp;
    }
    // 005265b0  f7e2                   -mul edx
    cpu.edx_eax = x86::reg64(cpu.eax) * x86::reg64(cpu.edx);
    // 005265b2  01d8                   +add eax, ebx
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
    // 005265b4  83d200                 -adc edx, 0
    (cpu.edx) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 005265b7  f7d9                   +neg ecx
    {
        x86::reg32 tmp1 = 0;
        x86::reg32& tmp2 = cpu.ecx;
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp2 = result;
        cpu.set_szp(tmp2);
    }
    // 005265b9  19c6                   +sbb esi, eax
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 - tmp2 - carry;
        cpu.flags.cf = tmp1 < tmp2 || (carry && tmp1 == tmp2);
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005265bb  19d7                   +sbb edi, edx
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 - tmp2 - carry;
        cpu.flags.cf = tmp1 < tmp2 || (carry && tmp1 == tmp2);
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005265bd  7417                   -je 0x5265d6
    if (cpu.flags.zf)
    {
        goto L_0x005265d6;
    }
L_0x005265bf:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005265bf  836de001               +sub dword ptr [ebp - 0x20], 1
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005265c3  835de400               +sbb dword ptr [ebp - 0x1c], 0
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 - tmp2 - carry;
        cpu.flags.cf = tmp1 < tmp2 || (carry && tmp1 == tmp2);
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005265c7  835de800               -sbb dword ptr [ebp - 0x18], 0
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */)) -= x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 005265cb  034dec                 +add ecx, dword ptr [ebp - 0x14]
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */)));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005265ce  1375f0                 +adc esi, dword ptr [ebp - 0x10]
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005265d1  83d700                 +adc edi, 0
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005265d4  75e9                   -jne 0x5265bf
    if (!cpu.flags.zf)
    {
        goto L_0x005265bf;
    }
L_0x005265d6:
    // 005265d6  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005265d7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005265d8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005265d9  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 005265dc  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005265dd  664f                   -dec di
    (cpu.di)--;
    // 005265df  d1eb                   +shr ebx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 005265e1  7305                   -jae 0x5265e8
    if (!cpu.flags.cf)
    {
        goto L_0x005265e8;
    }
    // 005265e3  d1da                   +rcr edx, 1
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005265e5  d1d8                   -rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005265e7  47                     -inc edi
    (cpu.edi)++;
L_0x005265e8:
    // 005265e8  29f6                   -sub esi, esi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.esi));
    // 005265ea  b10b                   -mov cl, 0xb
    cpu.cl = 11 /*0xb*/;
    // 005265ec  0fadd0                 +shrd eax, edx, cl
    {
        x86::reg32& destination = cpu.eax;
        cpu.flags.cf = 1 & (destination >> (cpu.cl - 1));
        cpu.flags.of = 1 & (destination >> (32 - 1));
        destination >>= (cpu.cl % 32);
        destination |= cpu.edx  << (32 - (cpu.cl % 32));
        cpu.flags.of ^= 1 & (destination >> (32 - 1));
        cpu.set_szp(destination);
    }
    // 005265ef  d1de                   -rcr esi, 1
    {
        x86::reg32& op = cpu.esi;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005265f1  0fadf2                 -shrd edx, esi, cl
    {
        x86::reg32& destination = cpu.edx;
        destination >>= (cpu.cl % 32);
        destination |= cpu.esi  << (32 - (cpu.cl % 32));
    }
    // 005265f4  81ca0000f0ff           -or edx, 0xfff00000
    cpu.edx |= x86::reg32(x86::sreg32(4293918720 /*0xfff00000*/));
    // 005265fa  01f6                   +add esi, esi
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005265fc  83d000                 +adc eax, 0
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005265ff  83d200                 +adc edx, 0
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526602  83d700                 -adc edi, 0
    (cpu.edi) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 00526605  6609ff                 +or di, di
    cpu.clear_co();
    cpu.set_szp((cpu.di |= x86::reg16(x86::sreg16(cpu.di))));
    // 00526608  7f1d                   -jg 0x526627
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00526627;
    }
    // 0052660a  7504                   -jne 0x526610
    if (!cpu.flags.zf)
    {
        goto L_0x00526610;
    }
    // 0052660c  b101                   -mov cl, 1
    cpu.cl = 1 /*0x1*/;
    // 0052660e  eb06                   -jmp 0x526616
    goto L_0x00526616;
L_0x00526610:
    // 00526610  66f7df                 -neg di
    cpu.di = ~cpu.di + 1;
    // 00526613  6689f9                 -mov cx, di
    cpu.cx = cpu.di;
L_0x00526616:
    // 00526616  81e2ffff1f00           -and edx, 0x1fffff
    cpu.edx &= x86::reg32(x86::sreg32(2097151 /*0x1fffff*/));
    // 0052661c  29db                   -sub ebx, ebx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 0052661e  0fadd0                 -shrd eax, edx, cl
    {
        x86::reg32& destination = cpu.eax;
        destination >>= (cpu.cl % 32);
        destination |= cpu.edx  << (32 - (cpu.cl % 32));
    }
    // 00526621  0fadda                 -shrd edx, ebx, cl
    {
        x86::reg32& destination = cpu.edx;
        destination >>= (cpu.cl % 32);
        destination |= cpu.ebx  << (32 - (cpu.cl % 32));
    }
    // 00526624  6629ff                 -sub di, di
    (cpu.di) -= x86::reg16(x86::sreg16(cpu.di));
L_0x00526627:
    // 00526627  81e2ffff0f00           -and edx, 0xfffff
    cpu.edx &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 0052662d  89fe                   -mov esi, edi
    cpu.esi = cpu.edi;
    // 0052662f  c1cf0b                 -ror edi, 0xb
    {
        x86::reg32& op = cpu.edi;
        x86::reg32 shift = 11 /*0xb*/ % 32;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | cf << 31;
            shift--;
        }
    }
    // 00526632  01f6                   +add esi, esi
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526634  d1df                   -rcr edi, 1
    {
        x86::reg32& op = cpu.edi;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 00526636  81e70000f0ff           -and edi, 0xfff00000
    cpu.edi &= x86::reg32(x86::sreg32(4293918720 /*0xfff00000*/));
    // 0052663c  09fa                   -or edx, edi
    cpu.edx |= x86::reg32(x86::sreg32(cpu.edi));
    // 0052663e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052663f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526640  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526641  c3                     -ret 
    cpu.fpu.st(0) = fpa1;
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_526642(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 00526642  81f100000080           -xor ecx, 0x80000000
    cpu.ecx ^= x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
    // 00526648  09db                   +or ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0052664a  7506                   -jne 0x526652
    if (!cpu.flags.zf)
    {
        goto L_0x00526652;
    }
    // 0052664c  01c9                   +add ecx, ecx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052664e  740e                   -je 0x52665e
    if (cpu.flags.zf)
    {
        goto L_0x0052665e;
    }
    // 00526650  d1d9                   -rcr ecx, 1
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
L_0x00526652:
    // 00526652  09c0                   +or eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax |= x86::reg32(x86::sreg32(cpu.eax))));
    // 00526654  750b                   -jne 0x526661
    if (!cpu.flags.zf)
    {
        goto L_0x00526661;
    }
    // 00526656  01d2                   +add edx, edx
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
    // 00526658  7505                   -jne 0x52665f
    if (!cpu.flags.zf)
    {
        goto L_0x0052665f;
    }
    // 0052665a  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0052665c  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
L_0x0052665e:
    // 0052665e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052665f:
    // 0052665f  d1da                   -rcr edx, 1
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
L_0x00526661:
    // 00526661  803dc144560000         +cmp byte ptr [0x5644c1], 0
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
    // 00526668  7421                   -je 0x52668b
    if (cpu.flags.zf)
    {
        goto L_0x0052668b;
    }
    // 0052666a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052666b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052666c  dd0424                 -fld qword ptr [esp]
    fp0 = x86::Float(app->getMemory<double>(cpu.esp));
    // 0052666f  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00526670  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00526671  dc0424                 -fadd qword ptr [esp]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(cpu.esp)));
    // 00526674  dd5c2408               -fstp qword ptr [esp + 8]
    app->getMemory<double>(cpu.esp + x86::reg32(8) /* 0x8 */) = double(fp0);
    // 00526678  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0052667b  9b                     -wait 
    /*nothing*/;
    // 0052667c  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052667d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052667e  81fa00000080           +cmp edx, 0x80000000
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00526684  7504                   -jne 0x52668a
    if (!cpu.flags.zf)
    {
        goto L_0x0052668a;
    }
    // 00526686  29d2                   -sub edx, edx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00526688  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x0052668a:
    // 0052668a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052668b:
    // 0052668b  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052668c  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052668d  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052668e  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00526690  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 00526692  c1ff14                 -sar edi, 0x14
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (20 /*0x14*/ % 32));
    // 00526695  c1f914                 -sar ecx, 0x14
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (20 /*0x14*/ % 32));
    // 00526698  81e7ff070080           -and edi, 0x800007ff
    cpu.edi &= x86::reg32(x86::sreg32(2147485695 /*0x800007ff*/));
    // 0052669e  81e1ff070080           -and ecx, 0x800007ff
    cpu.ecx &= x86::reg32(x86::sreg32(2147485695 /*0x800007ff*/));
    // 005266a4  89cd                   -mov ebp, ecx
    cpu.ebp = cpu.ecx;
    // 005266a6  c1c710                 -rol edi, 0x10
    {
        x86::reg32& op = cpu.edi;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005266a9  c1c110                 -rol ecx, 0x10
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005266ac  6601f9                 -add cx, di
    (cpu.cx) += x86::reg16(x86::sreg16(cpu.di));
    // 005266af  c1c710                 -rol edi, 0x10
    {
        x86::reg32& op = cpu.edi;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005266b2  c1c110                 -rol ecx, 0x10
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005266b5  81e2ffff0f00           -and edx, 0xfffff
    cpu.edx &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 005266bb  81e6ffff0f00           -and esi, 0xfffff
    cpu.esi &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 005266c1  6609ff                 +or di, di
    cpu.clear_co();
    cpu.set_szp((cpu.di |= x86::reg16(x86::sreg16(cpu.di))));
    // 005266c4  7406                   -je 0x5266cc
    if (cpu.flags.zf)
    {
        goto L_0x005266cc;
    }
    // 005266c6  81ca00001000           -or edx, 0x100000
    cpu.edx |= x86::reg32(x86::sreg32(1048576 /*0x100000*/));
L_0x005266cc:
    // 005266cc  6609c9                 +or cx, cx
    cpu.clear_co();
    cpu.set_szp((cpu.cx |= x86::reg16(x86::sreg16(cpu.cx))));
    // 005266cf  7406                   -je 0x5266d7
    if (cpu.flags.zf)
    {
        goto L_0x005266d7;
    }
    // 005266d1  81ce00001000           -or esi, 0x100000
    cpu.esi |= x86::reg32(x86::sreg32(1048576 /*0x100000*/));
L_0x005266d7:
    // 005266d7  01c0                   +add eax, eax
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
    // 005266d9  11d2                   -adc edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 005266db  01db                   +add ebx, ebx
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005266dd  11f6                   -adc esi, esi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.esi) + cpu.flags.cf);
    // 005266df  6629f9                 +sub cx, di
    {
        x86::reg16& tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(cpu.di));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005266e2  742f                   -je 0x526713
    if (cpu.flags.zf)
    {
        goto L_0x00526713;
    }
    // 005266e4  7308                   -jae 0x5266ee
    if (!cpu.flags.cf)
    {
        goto L_0x005266ee;
    }
    // 005266e6  89fd                   -mov ebp, edi
    cpu.ebp = cpu.edi;
    // 005266e8  66f7d9                 -neg cx
    cpu.cx = ~cpu.cx + 1;
    // 005266eb  93                     -xchg ebx, eax
    {
        x86::reg32 tmp = cpu.ebx;
        cpu.ebx = cpu.eax;
        cpu.eax = tmp;
    }
    // 005266ec  87f2                   -xchg edx, esi
    {
        x86::reg32 tmp = cpu.edx;
        cpu.edx = cpu.esi;
        cpu.esi = tmp;
    }
L_0x005266ee:
    // 005266ee  6683f936               +cmp cx, 0x36
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(54 /*0x36*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 005266f2  761f                   -jbe 0x526713
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00526713;
    }
    // 005266f4  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 005266f6  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 005266f8  01ed                   +add ebp, ebp
    {
        x86::reg32& tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005266fa  d1da                   +rcr edx, 1
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005266fc  d1d8                   -rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005266fe  81e2ffff0f80           -and edx, 0x800fffff
    cpu.edx &= x86::reg32(x86::sreg32(2148532223 /*0x800fffff*/));
    // 00526704  c1cd0d                 -ror ebp, 0xd
    {
        x86::reg32& op = cpu.ebp;
        x86::reg32 shift = 13 /*0xd*/ % 32;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | cf << 31;
            shift--;
        }
    }
    // 00526707  81e50000f07f           -and ebp, 0x7ff00000
    cpu.ebp &= x86::reg32(x86::sreg32(2146435072 /*0x7ff00000*/));
    // 0052670d  09ea                   -or edx, ebp
    cpu.edx |= x86::reg32(x86::sreg32(cpu.ebp));
    // 0052670f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526710  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526711  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526712  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00526713:
    // 00526713  09c9                   +or ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx |= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00526715  790d                   -jns 0x526724
    if (!cpu.flags.sf)
    {
        goto L_0x00526724;
    }
    // 00526717  f7de                   -neg esi
    cpu.esi = ~cpu.esi + 1;
    // 00526719  f7db                   +neg ebx
    {
        x86::reg32 tmp1 = 0;
        x86::reg32& tmp2 = cpu.ebx;
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp2 = result;
        cpu.set_szp(tmp2);
    }
    // 0052671b  83de00                 -sbb esi, 0
    (cpu.esi) -= x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 0052671e  81f500000080           -xor ebp, 0x80000000
    cpu.ebp ^= x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
L_0x00526724:
    // 00526724  29ff                   -sub edi, edi
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00526726  80f900                 +cmp cl, 0
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00526729  7423                   -je 0x52674e
    if (cpu.flags.zf)
    {
        goto L_0x0052674e;
    }
    // 0052672b  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052672c  29db                   -sub ebx, ebx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 0052672e  80f920                 +cmp cl, 0x20
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(32 /*0x20*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00526731  720d                   -jb 0x526740
    if (cpu.flags.cf)
    {
        goto L_0x00526740;
    }
    // 00526733  09c0                   +or eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax |= x86::reg32(x86::sreg32(cpu.eax))));
    // 00526735  0f95c3                 -setne bl
    cpu.bl = !cpu.flags.zf;
    // 00526738  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0052673a  29db                   -sub ebx, ebx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 0052673c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0052673e  29d2                   -sub edx, edx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edx));
L_0x00526740:
    // 00526740  0fadc3                 -shrd ebx, eax, cl
    {
        x86::reg32& destination = cpu.ebx;
        destination >>= (cpu.cl % 32);
        destination |= cpu.eax  << (32 - (cpu.cl % 32));
    }
    // 00526743  09df                   -or edi, ebx
    cpu.edi |= x86::reg32(x86::sreg32(cpu.ebx));
    // 00526745  29db                   -sub ebx, ebx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00526747  0fadd0                 -shrd eax, edx, cl
    {
        x86::reg32& destination = cpu.eax;
        destination >>= (cpu.cl % 32);
        destination |= cpu.edx  << (32 - (cpu.cl % 32));
    }
    // 0052674a  0fadda                 -shrd edx, ebx, cl
    {
        x86::reg32& destination = cpu.edx;
        destination >>= (cpu.cl % 32);
        destination |= cpu.ebx  << (32 - (cpu.cl % 32));
    }
    // 0052674d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0052674e:
    // 0052674e  01d8                   +add eax, ebx
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
    // 00526750  11f2                   +adc edx, esi
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526752  7923                   -jns 0x526777
    if (!cpu.flags.sf)
    {
        goto L_0x00526777;
    }
    // 00526754  80f935                 +cmp cl, 0x35
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(53 /*0x35*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00526757  7211                   -jb 0x52676a
    if (cpu.flags.cf)
    {
        goto L_0x0052676a;
    }
    // 00526759  f7c7ffffff7f           +test edi, 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & 2147483647 /*0x7fffffff*/));
    // 0052675f  0f95c3                 -setne bl
    cpu.bl = !cpu.flags.zf;
    // 00526762  d1eb                   +shr ebx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 00526764  83d000                 +adc eax, 0
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526767  83d200                 -adc edx, 0
    (cpu.edx) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
L_0x0052676a:
    // 0052676a  f7da                   -neg edx
    cpu.edx = ~cpu.edx + 1;
    // 0052676c  f7d8                   +neg eax
    {
        x86::reg32 tmp1 = 0;
        x86::reg32& tmp2 = cpu.eax;
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp2 = result;
        cpu.set_szp(tmp2);
    }
    // 0052676e  83da00                 -sbb edx, 0
    (cpu.edx) -= x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 00526771  81f500000080           -xor ebp, 0x80000000
    cpu.ebp ^= x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
L_0x00526777:
    // 00526777  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00526779  09d3                   +or ebx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(cpu.edx))));
    // 0052677b  746a                   -je 0x5267e7
    if (cpu.flags.zf)
    {
        goto L_0x005267e7;
    }
    // 0052677d  6609ed                 +or bp, bp
    cpu.clear_co();
    cpu.set_szp((cpu.bp |= x86::reg16(x86::sreg16(cpu.bp))));
    // 00526780  7469                   -je 0x5267eb
    if (cpu.flags.zf)
    {
        goto L_0x005267eb;
    }
L_0x00526782:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00526782  f7c20000e07f           +test edx, 0x7fe00000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 2145386496 /*0x7fe00000*/));
    // 00526788  750a                   -jne 0x526794
    if (!cpu.flags.zf)
    {
        goto L_0x00526794;
    }
    // 0052678a  664d                   +dec bp
    {
        x86::reg16& tmp = cpu.bp;
        cpu.flags.of = 1 & (tmp >> 15);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 15));
        cpu.set_szp(tmp);
    }
    // 0052678c  745d                   -je 0x5267eb
    if (cpu.flags.zf)
    {
        goto L_0x005267eb;
    }
    // 0052678e  01c0                   +add eax, eax
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
    // 00526790  11d2                   +adc edx, edx
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526792  ebee                   -jmp 0x526782
    goto L_0x00526782;
L_0x00526794:
    // 00526794  f7c200004000           +test edx, 0x400000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 4194304 /*0x400000*/));
    // 0052679a  7410                   -je 0x5267ac
    if (cpu.flags.zf)
    {
        goto L_0x005267ac;
    }
    // 0052679c  d1ea                   +shr edx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 0052679e  d1d8                   +rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267a0  83d700                 -adc edi, 0
    (cpu.edi) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 005267a3  6645                   -inc bp
    (cpu.bp)++;
    // 005267a5  6681fdff07             +cmp bp, 0x7ff
    {
        x86::reg16 tmp1 = cpu.bp;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2047 /*0x7ff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 005267aa  7449                   -je 0x5267f5
    if (cpu.flags.zf)
    {
        goto L_0x005267f5;
    }
L_0x005267ac:
    // 005267ac  d1ea                   +shr edx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 005267ae  d1d8                   +rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267b0  7324                   -jae 0x5267d6
    if (!cpu.flags.cf)
    {
        goto L_0x005267d6;
    }
    // 005267b2  09ff                   +or edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi |= x86::reg32(x86::sreg32(cpu.edi))));
    // 005267b4  0f95c3                 -setne bl
    cpu.bl = !cpu.flags.zf;
    // 005267b7  09c3                   -or ebx, eax
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.eax));
    // 005267b9  d1eb                   +shr ebx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 005267bb  83d000                 +adc eax, 0
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005267be  83d200                 -adc edx, 0
    (cpu.edx) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 005267c1  f7c200002000           +test edx, 0x200000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 2097152 /*0x200000*/));
    // 005267c7  740d                   -je 0x5267d6
    if (cpu.flags.zf)
    {
        goto L_0x005267d6;
    }
    // 005267c9  d1ea                   +shr edx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 005267cb  d1d8                   -rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267cd  6645                   -inc bp
    (cpu.bp)++;
    // 005267cf  6681fdff07             +cmp bp, 0x7ff
    {
        x86::reg16 tmp1 = cpu.bp;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2047 /*0x7ff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 005267d4  741f                   -je 0x5267f5
    if (cpu.flags.zf)
    {
        goto L_0x005267f5;
    }
L_0x005267d6:
    // 005267d6  81e2ffff0f00           -and edx, 0xfffff
    cpu.edx &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 005267dc  89e9                   -mov ecx, ebp
    cpu.ecx = cpu.ebp;
    // 005267de  c1e515                 -shl ebp, 0x15
    cpu.ebp <<= 21 /*0x15*/ % 32;
    // 005267e1  01c9                   +add ecx, ecx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005267e3  d1dd                   -rcr ebp, 1
    {
        x86::reg32& op = cpu.ebp;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267e5  09ea                   -or edx, ebp
    cpu.edx |= x86::reg32(x86::sreg32(cpu.ebp));
L_0x005267e7:
    // 005267e7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267e8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267e9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267ea  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005267eb:
    // 005267eb  01ed                   +add ebp, ebp
    {
        x86::reg32& tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005267ed  d1da                   +rcr edx, 1
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267ef  d1d8                   +rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267f1  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267f2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267f3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267f4  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005267f5:
    // 005267f5  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 005267f7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267f8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267f9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267fa  e9a8600000             -jmp 0x52c8a7
    return sub_52c8a7(app, cpu.handOver());
}

/* align: skip  */
void Application::sub_526674(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fpa1 = cpu.fpu.st(0);
    goto L_entry_0x00526674;
    // 00526642  81f100000080           -xor ecx, 0x80000000
    cpu.ecx ^= x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
    // 00526648  09db                   +or ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0052664a  7506                   -jne 0x526652
    if (!cpu.flags.zf)
    {
        goto L_0x00526652;
    }
    // 0052664c  01c9                   +add ecx, ecx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052664e  740e                   -je 0x52665e
    if (cpu.flags.zf)
    {
        goto L_0x0052665e;
    }
    // 00526650  d1d9                   -rcr ecx, 1
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
L_0x00526652:
    // 00526652  09c0                   +or eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax |= x86::reg32(x86::sreg32(cpu.eax))));
    // 00526654  750b                   -jne 0x526661
    if (!cpu.flags.zf)
    {
        goto L_0x00526661;
    }
    // 00526656  01d2                   +add edx, edx
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
    // 00526658  7505                   -jne 0x52665f
    if (!cpu.flags.zf)
    {
        goto L_0x0052665f;
    }
    // 0052665a  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0052665c  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
L_0x0052665e:
    // 0052665e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052665f:
    // 0052665f  d1da                   -rcr edx, 1
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
L_0x00526661:
    // 00526661  803dc144560000         +cmp byte ptr [0x5644c1], 0
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
    // 00526668  7421                   -je 0x52668b
    if (cpu.flags.zf)
    {
        goto L_0x0052668b;
    }
    // 0052666a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052666b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052666c  dd0424                 -fld qword ptr [esp]
    cpu.fpu.push(x86::Float(app->getMemory<double>(cpu.esp)));
    // 0052666f  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00526670  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00526671  dc0424                 -fadd qword ptr [esp]
    cpu.fpu.st(0) = fp::add(cpu.fpu.st(0), x86::Float(app->getMemory<double>(cpu.esp)));
L_entry_0x00526674:
    // 00526674  dd5c2408               -fstp qword ptr [esp + 8]
    app->getMemory<double>(cpu.esp + x86::reg32(8) /* 0x8 */) = double(fpa1);
    // 00526678  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0052667b  9b                     -wait 
    /*nothing*/;
    // 0052667c  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052667d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052667e  81fa00000080           +cmp edx, 0x80000000
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00526684  7504                   -jne 0x52668a
    if (!cpu.flags.zf)
    {
        goto L_0x0052668a;
    }
    // 00526686  29d2                   -sub edx, edx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00526688  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x0052668a:
    // 0052668a  c3                     -ret 
    cpu.fpu.count -= 1;
    cpu.esp += 4;
    return;
L_0x0052668b:
    // 0052668b  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052668c  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052668d  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052668e  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00526690  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 00526692  c1ff14                 -sar edi, 0x14
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (20 /*0x14*/ % 32));
    // 00526695  c1f914                 -sar ecx, 0x14
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (20 /*0x14*/ % 32));
    // 00526698  81e7ff070080           -and edi, 0x800007ff
    cpu.edi &= x86::reg32(x86::sreg32(2147485695 /*0x800007ff*/));
    // 0052669e  81e1ff070080           -and ecx, 0x800007ff
    cpu.ecx &= x86::reg32(x86::sreg32(2147485695 /*0x800007ff*/));
    // 005266a4  89cd                   -mov ebp, ecx
    cpu.ebp = cpu.ecx;
    // 005266a6  c1c710                 -rol edi, 0x10
    {
        x86::reg32& op = cpu.edi;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005266a9  c1c110                 -rol ecx, 0x10
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005266ac  6601f9                 -add cx, di
    (cpu.cx) += x86::reg16(x86::sreg16(cpu.di));
    // 005266af  c1c710                 -rol edi, 0x10
    {
        x86::reg32& op = cpu.edi;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005266b2  c1c110                 -rol ecx, 0x10
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005266b5  81e2ffff0f00           -and edx, 0xfffff
    cpu.edx &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 005266bb  81e6ffff0f00           -and esi, 0xfffff
    cpu.esi &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 005266c1  6609ff                 +or di, di
    cpu.clear_co();
    cpu.set_szp((cpu.di |= x86::reg16(x86::sreg16(cpu.di))));
    // 005266c4  7406                   -je 0x5266cc
    if (cpu.flags.zf)
    {
        goto L_0x005266cc;
    }
    // 005266c6  81ca00001000           -or edx, 0x100000
    cpu.edx |= x86::reg32(x86::sreg32(1048576 /*0x100000*/));
L_0x005266cc:
    // 005266cc  6609c9                 +or cx, cx
    cpu.clear_co();
    cpu.set_szp((cpu.cx |= x86::reg16(x86::sreg16(cpu.cx))));
    // 005266cf  7406                   -je 0x5266d7
    if (cpu.flags.zf)
    {
        goto L_0x005266d7;
    }
    // 005266d1  81ce00001000           -or esi, 0x100000
    cpu.esi |= x86::reg32(x86::sreg32(1048576 /*0x100000*/));
L_0x005266d7:
    // 005266d7  01c0                   +add eax, eax
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
    // 005266d9  11d2                   -adc edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 005266db  01db                   +add ebx, ebx
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005266dd  11f6                   -adc esi, esi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.esi) + cpu.flags.cf);
    // 005266df  6629f9                 +sub cx, di
    {
        x86::reg16& tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(cpu.di));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005266e2  742f                   -je 0x526713
    if (cpu.flags.zf)
    {
        goto L_0x00526713;
    }
    // 005266e4  7308                   -jae 0x5266ee
    if (!cpu.flags.cf)
    {
        goto L_0x005266ee;
    }
    // 005266e6  89fd                   -mov ebp, edi
    cpu.ebp = cpu.edi;
    // 005266e8  66f7d9                 -neg cx
    cpu.cx = ~cpu.cx + 1;
    // 005266eb  93                     -xchg ebx, eax
    {
        x86::reg32 tmp = cpu.ebx;
        cpu.ebx = cpu.eax;
        cpu.eax = tmp;
    }
    // 005266ec  87f2                   -xchg edx, esi
    {
        x86::reg32 tmp = cpu.edx;
        cpu.edx = cpu.esi;
        cpu.esi = tmp;
    }
L_0x005266ee:
    // 005266ee  6683f936               +cmp cx, 0x36
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(54 /*0x36*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 005266f2  761f                   -jbe 0x526713
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00526713;
    }
    // 005266f4  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 005266f6  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 005266f8  01ed                   +add ebp, ebp
    {
        x86::reg32& tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005266fa  d1da                   +rcr edx, 1
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005266fc  d1d8                   -rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005266fe  81e2ffff0f80           -and edx, 0x800fffff
    cpu.edx &= x86::reg32(x86::sreg32(2148532223 /*0x800fffff*/));
    // 00526704  c1cd0d                 -ror ebp, 0xd
    {
        x86::reg32& op = cpu.ebp;
        x86::reg32 shift = 13 /*0xd*/ % 32;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | cf << 31;
            shift--;
        }
    }
    // 00526707  81e50000f07f           -and ebp, 0x7ff00000
    cpu.ebp &= x86::reg32(x86::sreg32(2146435072 /*0x7ff00000*/));
    // 0052670d  09ea                   -or edx, ebp
    cpu.edx |= x86::reg32(x86::sreg32(cpu.ebp));
    // 0052670f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526710  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526711  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526712  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00526713:
    // 00526713  09c9                   +or ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx |= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00526715  790d                   -jns 0x526724
    if (!cpu.flags.sf)
    {
        goto L_0x00526724;
    }
    // 00526717  f7de                   -neg esi
    cpu.esi = ~cpu.esi + 1;
    // 00526719  f7db                   +neg ebx
    {
        x86::reg32 tmp1 = 0;
        x86::reg32& tmp2 = cpu.ebx;
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp2 = result;
        cpu.set_szp(tmp2);
    }
    // 0052671b  83de00                 -sbb esi, 0
    (cpu.esi) -= x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 0052671e  81f500000080           -xor ebp, 0x80000000
    cpu.ebp ^= x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
L_0x00526724:
    // 00526724  29ff                   -sub edi, edi
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00526726  80f900                 +cmp cl, 0
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00526729  7423                   -je 0x52674e
    if (cpu.flags.zf)
    {
        goto L_0x0052674e;
    }
    // 0052672b  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052672c  29db                   -sub ebx, ebx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 0052672e  80f920                 +cmp cl, 0x20
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(32 /*0x20*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00526731  720d                   -jb 0x526740
    if (cpu.flags.cf)
    {
        goto L_0x00526740;
    }
    // 00526733  09c0                   +or eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax |= x86::reg32(x86::sreg32(cpu.eax))));
    // 00526735  0f95c3                 -setne bl
    cpu.bl = !cpu.flags.zf;
    // 00526738  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0052673a  29db                   -sub ebx, ebx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 0052673c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0052673e  29d2                   -sub edx, edx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edx));
L_0x00526740:
    // 00526740  0fadc3                 -shrd ebx, eax, cl
    {
        x86::reg32& destination = cpu.ebx;
        destination >>= (cpu.cl % 32);
        destination |= cpu.eax  << (32 - (cpu.cl % 32));
    }
    // 00526743  09df                   -or edi, ebx
    cpu.edi |= x86::reg32(x86::sreg32(cpu.ebx));
    // 00526745  29db                   -sub ebx, ebx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00526747  0fadd0                 -shrd eax, edx, cl
    {
        x86::reg32& destination = cpu.eax;
        destination >>= (cpu.cl % 32);
        destination |= cpu.edx  << (32 - (cpu.cl % 32));
    }
    // 0052674a  0fadda                 -shrd edx, ebx, cl
    {
        x86::reg32& destination = cpu.edx;
        destination >>= (cpu.cl % 32);
        destination |= cpu.ebx  << (32 - (cpu.cl % 32));
    }
    // 0052674d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0052674e:
    // 0052674e  01d8                   +add eax, ebx
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
    // 00526750  11f2                   +adc edx, esi
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526752  7923                   -jns 0x526777
    if (!cpu.flags.sf)
    {
        goto L_0x00526777;
    }
    // 00526754  80f935                 +cmp cl, 0x35
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(53 /*0x35*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00526757  7211                   -jb 0x52676a
    if (cpu.flags.cf)
    {
        goto L_0x0052676a;
    }
    // 00526759  f7c7ffffff7f           +test edi, 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & 2147483647 /*0x7fffffff*/));
    // 0052675f  0f95c3                 -setne bl
    cpu.bl = !cpu.flags.zf;
    // 00526762  d1eb                   +shr ebx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 00526764  83d000                 +adc eax, 0
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526767  83d200                 -adc edx, 0
    (cpu.edx) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
L_0x0052676a:
    // 0052676a  f7da                   -neg edx
    cpu.edx = ~cpu.edx + 1;
    // 0052676c  f7d8                   +neg eax
    {
        x86::reg32 tmp1 = 0;
        x86::reg32& tmp2 = cpu.eax;
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp2 = result;
        cpu.set_szp(tmp2);
    }
    // 0052676e  83da00                 -sbb edx, 0
    (cpu.edx) -= x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 00526771  81f500000080           -xor ebp, 0x80000000
    cpu.ebp ^= x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
L_0x00526777:
    // 00526777  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00526779  09d3                   +or ebx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(cpu.edx))));
    // 0052677b  746a                   -je 0x5267e7
    if (cpu.flags.zf)
    {
        goto L_0x005267e7;
    }
    // 0052677d  6609ed                 +or bp, bp
    cpu.clear_co();
    cpu.set_szp((cpu.bp |= x86::reg16(x86::sreg16(cpu.bp))));
    // 00526780  7469                   -je 0x5267eb
    if (cpu.flags.zf)
    {
        goto L_0x005267eb;
    }
L_0x00526782:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00526782  f7c20000e07f           +test edx, 0x7fe00000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 2145386496 /*0x7fe00000*/));
    // 00526788  750a                   -jne 0x526794
    if (!cpu.flags.zf)
    {
        goto L_0x00526794;
    }
    // 0052678a  664d                   +dec bp
    {
        x86::reg16& tmp = cpu.bp;
        cpu.flags.of = 1 & (tmp >> 15);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 15));
        cpu.set_szp(tmp);
    }
    // 0052678c  745d                   -je 0x5267eb
    if (cpu.flags.zf)
    {
        goto L_0x005267eb;
    }
    // 0052678e  01c0                   +add eax, eax
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
    // 00526790  11d2                   +adc edx, edx
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526792  ebee                   -jmp 0x526782
    goto L_0x00526782;
L_0x00526794:
    // 00526794  f7c200004000           +test edx, 0x400000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 4194304 /*0x400000*/));
    // 0052679a  7410                   -je 0x5267ac
    if (cpu.flags.zf)
    {
        goto L_0x005267ac;
    }
    // 0052679c  d1ea                   +shr edx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 0052679e  d1d8                   +rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267a0  83d700                 -adc edi, 0
    (cpu.edi) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 005267a3  6645                   -inc bp
    (cpu.bp)++;
    // 005267a5  6681fdff07             +cmp bp, 0x7ff
    {
        x86::reg16 tmp1 = cpu.bp;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2047 /*0x7ff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 005267aa  7449                   -je 0x5267f5
    if (cpu.flags.zf)
    {
        goto L_0x005267f5;
    }
L_0x005267ac:
    // 005267ac  d1ea                   +shr edx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 005267ae  d1d8                   +rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267b0  7324                   -jae 0x5267d6
    if (!cpu.flags.cf)
    {
        goto L_0x005267d6;
    }
    // 005267b2  09ff                   +or edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi |= x86::reg32(x86::sreg32(cpu.edi))));
    // 005267b4  0f95c3                 -setne bl
    cpu.bl = !cpu.flags.zf;
    // 005267b7  09c3                   -or ebx, eax
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.eax));
    // 005267b9  d1eb                   +shr ebx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 005267bb  83d000                 +adc eax, 0
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005267be  83d200                 -adc edx, 0
    (cpu.edx) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 005267c1  f7c200002000           +test edx, 0x200000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 2097152 /*0x200000*/));
    // 005267c7  740d                   -je 0x5267d6
    if (cpu.flags.zf)
    {
        goto L_0x005267d6;
    }
    // 005267c9  d1ea                   +shr edx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 005267cb  d1d8                   -rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267cd  6645                   -inc bp
    (cpu.bp)++;
    // 005267cf  6681fdff07             +cmp bp, 0x7ff
    {
        x86::reg16 tmp1 = cpu.bp;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2047 /*0x7ff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 005267d4  741f                   -je 0x5267f5
    if (cpu.flags.zf)
    {
        goto L_0x005267f5;
    }
L_0x005267d6:
    // 005267d6  81e2ffff0f00           -and edx, 0xfffff
    cpu.edx &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 005267dc  89e9                   -mov ecx, ebp
    cpu.ecx = cpu.ebp;
    // 005267de  c1e515                 -shl ebp, 0x15
    cpu.ebp <<= 21 /*0x15*/ % 32;
    // 005267e1  01c9                   +add ecx, ecx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005267e3  d1dd                   -rcr ebp, 1
    {
        x86::reg32& op = cpu.ebp;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267e5  09ea                   -or edx, ebp
    cpu.edx |= x86::reg32(x86::sreg32(cpu.ebp));
L_0x005267e7:
    // 005267e7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267e8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267e9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267ea  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005267eb:
    // 005267eb  01ed                   +add ebp, ebp
    {
        x86::reg32& tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005267ed  d1da                   +rcr edx, 1
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267ef  d1d8                   +rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005267f1  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267f2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267f3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267f4  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005267f5:
    // 005267f5  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 005267f7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267f8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267f9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005267fa  e9a8600000             -jmp 0x52c8a7
    return sub_52c8a7(app, cpu.handOver());
}

/* align: skip  */
void Application::sub_5267ff(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 005267ff  09c0                   +or eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax |= x86::reg32(x86::sreg32(cpu.eax))));
    // 00526801  7507                   -jne 0x52680a
    if (!cpu.flags.zf)
    {
        goto L_0x0052680a;
    }
    // 00526803  01d2                   +add edx, edx
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
    // 00526805  7501                   -jne 0x526808
    if (!cpu.flags.zf)
    {
        goto L_0x00526808;
    }
    // 00526807  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00526808:
    // 00526808  d1da                   -rcr edx, 1
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
L_0x0052680a:
    // 0052680a  09db                   +or ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0052680c  750b                   -jne 0x526819
    if (!cpu.flags.zf)
    {
        goto L_0x00526819;
    }
    // 0052680e  01c9                   +add ecx, ecx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526810  7505                   -jne 0x526817
    if (!cpu.flags.zf)
    {
        goto L_0x00526817;
    }
    // 00526812  29c0                   -sub eax, eax
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00526814  29d2                   +sub edx, edx
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526816  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00526817:
    // 00526817  d1d9                   -rcr ecx, 1
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
L_0x00526819:
    // 00526819  803dc144560000         +cmp byte ptr [0x5644c1], 0
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
    // 00526820  740f                   -je 0x526831
    if (cpu.flags.zf)
    {
        goto L_0x00526831;
    }
    // 00526822  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00526823  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00526824  dd0424                 +fld qword ptr [esp]
    fp0 = x86::Float(app->getMemory<double>(cpu.esp));
    // 00526827  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00526828  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00526829  dc0c24                 +fmul qword ptr [esp]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(cpu.esp)));
    // 0052682c  e943feffff             -jmp 0x526674
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fp0;
    return sub_526674(app, cpu.handOver());
L_0x00526831:
    // 00526831  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00526832  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00526833  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00526834  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00526836  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 00526838  c1ff14                 -sar edi, 0x14
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (20 /*0x14*/ % 32));
    // 0052683b  c1f914                 -sar ecx, 0x14
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (20 /*0x14*/ % 32));
    // 0052683e  81e7ff070080           -and edi, 0x800007ff
    cpu.edi &= x86::reg32(x86::sreg32(2147485695 /*0x800007ff*/));
    // 00526844  81e1ff070080           -and ecx, 0x800007ff
    cpu.ecx &= x86::reg32(x86::sreg32(2147485695 /*0x800007ff*/));
    // 0052684a  c1c710                 -rol edi, 0x10
    {
        x86::reg32& op = cpu.edi;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 0052684d  c1c110                 -rol ecx, 0x10
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 00526850  6601f9                 -add cx, di
    (cpu.cx) += x86::reg16(x86::sreg16(cpu.di));
    // 00526853  c1c710                 -rol edi, 0x10
    {
        x86::reg32& op = cpu.edi;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 00526856  c1c110                 -rol ecx, 0x10
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 16 /*0x10*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 00526859  81e2ffff0f00           -and edx, 0xfffff
    cpu.edx &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 0052685f  81e6ffff0f00           -and esi, 0xfffff
    cpu.esi &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 00526865  6609ff                 +or di, di
    cpu.clear_co();
    cpu.set_szp((cpu.di |= x86::reg16(x86::sreg16(cpu.di))));
    // 00526868  7510                   -jne 0x52687a
    if (!cpu.flags.zf)
    {
        goto L_0x0052687a;
    }
    // 0052686a  6647                   -inc di
    (cpu.di)++;
L_0x0052686c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052686c  664f                   -dec di
    (cpu.di)--;
    // 0052686e  01c0                   +add eax, eax
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
    // 00526870  11d2                   -adc edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 00526872  f7c200001000           +test edx, 0x100000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 1048576 /*0x100000*/));
    // 00526878  74f2                   -je 0x52686c
    if (cpu.flags.zf)
    {
        goto L_0x0052686c;
    }
L_0x0052687a:
    // 0052687a  81ca00001000           -or edx, 0x100000
    cpu.edx |= x86::reg32(x86::sreg32(1048576 /*0x100000*/));
    // 00526880  6609c9                 +or cx, cx
    cpu.clear_co();
    cpu.set_szp((cpu.cx |= x86::reg16(x86::sreg16(cpu.cx))));
    // 00526883  7510                   -jne 0x526895
    if (!cpu.flags.zf)
    {
        goto L_0x00526895;
    }
    // 00526885  6641                   -inc cx
    (cpu.cx)++;
L_0x00526887:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00526887  6649                   -dec cx
    (cpu.cx)--;
    // 00526889  01db                   +add ebx, ebx
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052688b  11f6                   -adc esi, esi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.esi) + cpu.flags.cf);
    // 0052688d  f7c600001000           +test esi, 0x100000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & 1048576 /*0x100000*/));
    // 00526893  74f2                   -je 0x526887
    if (cpu.flags.zf)
    {
        goto L_0x00526887;
    }
L_0x00526895:
    // 00526895  81ce00001000           -or esi, 0x100000
    cpu.esi |= x86::reg32(x86::sreg32(1048576 /*0x100000*/));
    // 0052689b  6601f9                 -add cx, di
    (cpu.cx) += x86::reg16(x86::sreg16(cpu.di));
    // 0052689e  6681e9ff03             +sub cx, 0x3ff
    {
        x86::reg16& tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1023 /*0x3ff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005268a3  7811                   -js 0x5268b6
    if (cpu.flags.sf)
    {
        goto L_0x005268b6;
    }
    // 005268a5  6681f9ff07             +cmp cx, 0x7ff
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2047 /*0x7ff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 005268aa  720a                   -jb 0x5268b6
    if (cpu.flags.cf)
    {
        goto L_0x005268b6;
    }
    // 005268ac  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 005268ae  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005268af  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005268b0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005268b1  e9f15f0000             -jmp 0x52c8a7
    return sub_52c8a7(app, cpu.handOver());
L_0x005268b6:
    // 005268b6  6683f9cb               +cmp cx, -0x35
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(-53 /*-0x35*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 005268ba  7d08                   -jge 0x5268c4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x005268c4;
    }
    // 005268bc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005268bd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005268be  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005268bf  e9c65f0000             -jmp 0x52c88a
    return sub_52c88a(app, cpu.handOver());
L_0x005268c4:
    // 005268c4  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005268c5  b10b                   -mov cl, 0xb
    cpu.cl = 11 /*0xb*/;
    // 005268c7  0fa5c2                 -shld edx, eax, cl
    {
        x86::reg32& destination = cpu.edx;
        destination <<= (cpu.cl % 32);
        destination |= cpu.eax >> (32 - (cpu.cl % 32));
    }
    // 005268ca  0fa5e8                 -shld eax, ebp, cl
    {
        x86::reg32& destination = cpu.eax;
        destination <<= (cpu.cl % 32);
        destination |= cpu.ebp >> (32 - (cpu.cl % 32));
    }
    // 005268cd  2500f8ffff             -and eax, 0xfffff800
    cpu.eax &= x86::reg32(x86::sreg32(4294965248 /*0xfffff800*/));
    // 005268d2  0fa5de                 -shld esi, ebx, cl
    {
        x86::reg32& destination = cpu.esi;
        destination <<= (cpu.cl % 32);
        destination |= cpu.ebx >> (32 - (cpu.cl % 32));
    }
    // 005268d5  0fa5eb                 -shld ebx, ebp, cl
    {
        x86::reg32& destination = cpu.ebx;
        destination <<= (cpu.cl % 32);
        destination |= cpu.ebp >> (32 - (cpu.cl % 32));
    }
    // 005268d8  81e300f8ffff           -and ebx, 0xfffff800
    cpu.ebx &= x86::reg32(x86::sreg32(4294965248 /*0xfffff800*/));
    // 005268de  29ed                   -sub ebp, ebp
    (cpu.ebp) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 005268e0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005268e1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 005268e2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 005268e3  f7e3                   -mul ebx
    cpu.edx_eax = x86::reg64(cpu.eax) * x86::reg64(cpu.ebx);
    // 005268e5  96                     -xchg esi, eax
    {
        x86::reg32 tmp = cpu.esi;
        cpu.esi = cpu.eax;
        cpu.eax = tmp;
    }
    // 005268e6  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 005268e8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005268e9  f7e2                   -mul edx
    cpu.edx_eax = x86::reg64(cpu.eax) * x86::reg64(cpu.edx);
    // 005268eb  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 005268ed  01c1                   +add ecx, eax
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
    // 005268ef  11ef                   +adc edi, ebp
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005268f1  11ed                   -adc ebp, ebp
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.ebp) + cpu.flags.cf);
    // 005268f3  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005268f4  93                     -xchg ebx, eax
    {
        x86::reg32 tmp = cpu.ebx;
        cpu.ebx = cpu.eax;
        cpu.eax = tmp;
    }
    // 005268f5  f7e3                   -mul ebx
    cpu.edx_eax = x86::reg64(cpu.eax) * x86::reg64(cpu.ebx);
    // 005268f7  01c1                   +add ecx, eax
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
    // 005268f9  11d7                   +adc edi, edx
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005268fb  83d500                 -adc ebp, 0
    (cpu.ebp) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 005268fe  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00526900  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526901  f7e2                   -mul edx
    cpu.edx_eax = x86::reg64(cpu.eax) * x86::reg64(cpu.edx);
    // 00526903  01f8                   +add eax, edi
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526905  11ea                   -adc edx, ebp
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebp) + cpu.flags.cf);
    // 00526907  29db                   -sub ebx, ebx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00526909  b10a                   -mov cl, 0xa
    cpu.cl = 10 /*0xa*/;
    // 0052690b  0fadc3                 -shrd ebx, eax, cl
    {
        x86::reg32& destination = cpu.ebx;
        destination >>= (cpu.cl % 32);
        destination |= cpu.eax  << (32 - (cpu.cl % 32));
    }
    // 0052690e  0fadd0                 -shrd eax, edx, cl
    {
        x86::reg32& destination = cpu.eax;
        destination >>= (cpu.cl % 32);
        destination |= cpu.edx  << (32 - (cpu.cl % 32));
    }
    // 00526911  0fadda                 -shrd edx, ebx, cl
    {
        x86::reg32& destination = cpu.edx;
        destination >>= (cpu.cl % 32);
        destination |= cpu.ebx  << (32 - (cpu.cl % 32));
    }
    // 00526914  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x00526915:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00526915  f7c200002000           +test edx, 0x200000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 2097152 /*0x200000*/));
    // 0052691b  7411                   -je 0x52692e
    if (cpu.flags.zf)
    {
        goto L_0x0052692e;
    }
    // 0052691d  d1ea                   +shr edx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 0052691f  d1d8                   +rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 00526921  d1db                   -rcr ebx, 1
    {
        x86::reg32& op = cpu.ebx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 00526923  6641                   -inc cx
    (cpu.cx)++;
    // 00526925  6681f9ff07             +cmp cx, 0x7ff
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2047 /*0x7ff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0052692a  7466                   -je 0x526992
    if (cpu.flags.zf)
    {
        goto L_0x00526992;
    }
    // 0052692c  ebe7                   -jmp 0x526915
    goto L_0x00526915;
L_0x0052692e:
    // 0052692e  01db                   +add ebx, ebx
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526930  732a                   -jae 0x52695c
    if (!cpu.flags.cf)
    {
        goto L_0x0052695c;
    }
    // 00526932  750d                   -jne 0x526941
    if (!cpu.flags.zf)
    {
        goto L_0x00526941;
    }
    // 00526934  09f6                   +or esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi |= x86::reg32(x86::sreg32(cpu.esi))));
    // 00526936  0f95c3                 -setne bl
    cpu.bl = !cpu.flags.zf;
    // 00526939  d1eb                   +shr ebx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 0052693b  7204                   -jb 0x526941
    if (cpu.flags.cf)
    {
        goto L_0x00526941;
    }
    // 0052693d  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0052693f  d1ee                   +shr esi, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.esi;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
L_0x00526941:
    // 00526941  83d000                 +adc eax, 0
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        const bool carry = cpu.flags.cf;
        x86::reg32 result = tmp1 + tmp2 + carry;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1 || (carry && result == tmp1);
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526944  83d200                 -adc edx, 0
    (cpu.edx) += x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 00526947  f7c200002000           +test edx, 0x200000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 2097152 /*0x200000*/));
    // 0052694d  740d                   -je 0x52695c
    if (cpu.flags.zf)
    {
        goto L_0x0052695c;
    }
    // 0052694f  d1ea                   +shr edx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 00526951  d1d8                   -rcr eax, 1
    {
        x86::reg32& op = cpu.eax;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 00526953  6641                   -inc cx
    (cpu.cx)++;
    // 00526955  6681f9ff07             +cmp cx, 0x7ff
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2047 /*0x7ff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0052695a  7436                   -je 0x526992
    if (cpu.flags.zf)
    {
        goto L_0x00526992;
    }
L_0x0052695c:
    // 0052695c  6609c9                 +or cx, cx
    cpu.clear_co();
    cpu.set_szp((cpu.cx |= x86::reg16(x86::sreg16(cpu.cx))));
    // 0052695f  7f16                   -jg 0x526977
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00526977;
    }
    // 00526961  7504                   -jne 0x526967
    if (!cpu.flags.zf)
    {
        goto L_0x00526967;
    }
    // 00526963  b101                   -mov cl, 1
    cpu.cl = 1 /*0x1*/;
    // 00526965  eb05                   -jmp 0x52696c
    goto L_0x0052696c;
L_0x00526967:
    // 00526967  66f7d9                 -neg cx
    cpu.cx = ~cpu.cx + 1;
    // 0052696a  6649                   -dec cx
    (cpu.cx)--;
L_0x0052696c:
    // 0052696c  29db                   -sub ebx, ebx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 0052696e  0fadd0                 -shrd eax, edx, cl
    {
        x86::reg32& destination = cpu.eax;
        destination >>= (cpu.cl % 32);
        destination |= cpu.edx  << (32 - (cpu.cl % 32));
    }
    // 00526971  0fadda                 -shrd edx, ebx, cl
    {
        x86::reg32& destination = cpu.edx;
        destination >>= (cpu.cl % 32);
        destination |= cpu.ebx  << (32 - (cpu.cl % 32));
    }
    // 00526974  6629c9                 -sub cx, cx
    (cpu.cx) -= x86::reg16(x86::sreg16(cpu.cx));
L_0x00526977:
    // 00526977  81e2ffff0f00           -and edx, 0xfffff
    cpu.edx &= x86::reg32(x86::sreg32(1048575 /*0xfffff*/));
    // 0052697d  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 0052697f  c1c90b                 -ror ecx, 0xb
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 11 /*0xb*/ % 32;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | cf << 31;
            shift--;
        }
    }
    // 00526982  01f6                   +add esi, esi
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00526984  d1d9                   -rcr ecx, 1
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 00526986  81e10000f0ff           -and ecx, 0xfff00000
    cpu.ecx &= x86::reg32(x86::sreg32(4293918720 /*0xfff00000*/));
    // 0052698c  09ca                   +or edx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.edx |= x86::reg32(x86::sreg32(cpu.ecx))));
    // 0052698e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052698f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526990  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526991  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00526992:
    // 00526992  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00526994  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526995  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526996  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526997  e90b5f0000             -jmp 0x52c8a7
    return sub_52c8a7(app, cpu.handOver());
}

/* align: skip  */
void Application::sub_52699c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052699c  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052699d  f7c20000f07f           +test edx, 0x7ff00000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 2146435072 /*0x7ff00000*/));
    // 005269a3  7502                   -jne 0x5269a7
    if (!cpu.flags.zf)
    {
        goto L_0x005269a7;
    }
    // 005269a5  29d2                   -sub edx, edx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edx));
L_0x005269a7:
    // 005269a7  f7c10000f07f           +test ecx, 0x7ff00000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & 2146435072 /*0x7ff00000*/));
    // 005269ad  7502                   -jne 0x5269b1
    if (!cpu.flags.zf)
    {
        goto L_0x005269b1;
    }
    // 005269af  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
L_0x005269b1:
    // 005269b1  89cd                   -mov ebp, ecx
    cpu.ebp = cpu.ecx;
    // 005269b3  31d5                   +xor ebp, edx
    cpu.clear_co();
    cpu.set_szp((cpu.ebp ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 005269b5  bd00000000             -mov ebp, 0
    cpu.ebp = 0 /*0x0*/;
    // 005269ba  780c                   -js 0x5269c8
    if (cpu.flags.sf)
    {
        goto L_0x005269c8;
    }
    // 005269bc  39ca                   +cmp edx, ecx
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
    // 005269be  7502                   -jne 0x5269c2
    if (!cpu.flags.zf)
    {
        goto L_0x005269c2;
    }
    // 005269c0  39d8                   +cmp eax, ebx
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
L_0x005269c2:
    // 005269c2  740c                   -je 0x5269d0
    if (cpu.flags.zf)
    {
        goto L_0x005269d0;
    }
    // 005269c4  d1d9                   -rcr ecx, 1
    {
        x86::reg32& op = cpu.ecx;
        x86::reg32 shift = 1 /*0x1*/ % 32;
        cpu.flags.of = (1 & (op >> 31)) ^ cpu.flags.cf;
        while (shift)
        {
            x86::reg32 cf = op & 1;
            op = op >> 1 | x86::reg32(cpu.flags.cf) << 31;
            cpu.flags.cf = cf;
            shift--;
        }
    }
    // 005269c6  31ca                   -xor edx, ecx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x005269c8:
    // 005269c8  01d2                   +add edx, edx
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
    // 005269ca  83dd00                 -sbb ebp, 0
    (cpu.ebp) -= x86::reg32(x86::sreg32(0 /*0x0*/) + cpu.flags.cf);
    // 005269cd  01ed                   -add ebp, ebp
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.ebp));
    // 005269cf  45                     -inc ebp
    (cpu.ebp)++;
L_0x005269d0:
    // 005269d0  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 005269d2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005269d3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_5269d4: native, nfs3hp::dead::stub_0x5269d4 */

/* sub_526aeb: native, nfs3hp::dead::stub_0x526aeb */

/* align: skip  */
void Application::sub_526f87(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fpa1 = cpu.fpu.st(0);
  x86::Float fpa2 = cpu.fpu.st(1);
    // 00526f87  83ec2c                 -sub esp, 0x2c
    (cpu.esp) -= x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 00526f8a  db3c24                 -fstp xword ptr [esp]
    app->getMemory<x86::IEEEf80>(cpu.esp) = x86::Float(fpa1);
    // 00526f8d  db7c240c               -fstp xword ptr [esp + 0xc]
    app->getMemory<x86::IEEEf80>(cpu.esp + x86::reg32(12) /* 0xc */) = x86::Float(fpa2);
    // 00526f91  e83efaffff             -call 0x5269d4
    cpu.esp -= 4;
    nfs3hp::dead::stub_0x5269d4(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fpa2 = cpu.fpu.take();
    // 00526f96  83c42c                 -add esp, 0x2c
    (cpu.esp) += x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 00526f99  c3                     -ret 
    cpu.fpu.count -= 1;
    cpu.fpu.st(0) = fpa2;
    cpu.esp += 4;
    return;
}

/* sub_526f9a: native, nfs3hp::dead::stub_0x526f9a */

/* sub_526fad: native, nfs3hp::dead::stub_0x526fad */

/* sub_526fc0: native, nfs3hp::dead::stub_0x526fc0 */

/* align: skip  */
void Application::sub_52700c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fpa1 = cpu.fpu.st(0);
  x86::Float fpa2 = cpu.fpu.st(1);
  x86::Float fp0;
    // 0052700c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052700d  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 00527011  250000f07f             -and eax, 0x7ff00000
    cpu.eax &= x86::reg32(x86::sreg32(2146435072 /*0x7ff00000*/));
    // 00527016  3d0000f07f             +cmp eax, 0x7ff00000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2146435072 /*0x7ff00000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052701b  7433                   -je 0x527050
    if (cpu.flags.zf)
    {
        goto L_0x00527050;
    }
    // 0052701d  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 0052701f  2500380000             +and eax, 0x3800
    cpu.clear_co();
    cpu.set_szp((cpu.eax &= x86::reg32(x86::sreg32(14336 /*0x3800*/))));
    // 00527024  740d                   -je 0x527033
    if (cpu.flags.zf)
    {
        goto L_0x00527033;
    }
    // 00527026  dd442408               -fld qword ptr [esp + 8]
    fp0 = x86::Float(app->getMemory<double>(cpu.esp + x86::reg32(8) /* 0x8 */));
    // 0052702a  e858ffffff             -call 0x526f87
    cpu.fpu.push(fpa1);
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    sub_526f87(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fpa1 = cpu.fpu.take();
    // 0052702f  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00527030  c20800                 -ret 8
    cpu.fpu.st(0) = fpa1;
    cpu.fpu.st(1) = fpa2;
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x00527033:
    // 00527033  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fpa1;
        fpa1 = fpa2;
        fpa2 = tmp;
    }
    // 00527035  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00527038  db3c24                 -fstp xword ptr [esp]
    app->getMemory<x86::IEEEf80>(cpu.esp) = x86::Float(fpa1);
    // 0052703b  dd442414               -fld qword ptr [esp + 0x14]
    fpa1 = x86::Float(app->getMemory<double>(cpu.esp + x86::reg32(20) /* 0x14 */));
    // 0052703f  e843ffffff             -call 0x526f87
    cpu.fpu.push(fpa2);
    cpu.fpu.push(fpa1);
    cpu.esp -= 4;
    sub_526f87(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fpa2 = cpu.fpu.take();
    // 00527044  db2c24                 -fld xword ptr [esp]
    fpa1 = x86::Float(app->getMemory<x86::IEEEf80>(cpu.esp));
    // 00527047  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fpa1;
        fpa1 = fpa2;
        fpa2 = tmp;
    }
    // 00527049  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052704c  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052704d  c20800                 -ret 8
    cpu.fpu.st(0) = fpa1;
    cpu.fpu.st(1) = fpa2;
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x00527050:
    // 00527050  dc742408               -fdiv qword ptr [esp + 8]
    fpa1 = fp::div(fpa1, x86::Float(app->getMemory<double>(cpu.esp + x86::reg32(8) /* 0x8 */)));
    // 00527054  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00527055  c20800                 -ret 8
    cpu.fpu.st(0) = fpa1;
    cpu.fpu.st(1) = fpa2;
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* sub_527058: native, nfs3hp::dead::stub_0x527058 */

/* sub_5270a4: native, nfs3hp::dead::stub_0x5270a4 */

/* sub_5270f0: native, nfs3hp::mix::interpolateMono8 */

/* sub_527170: native, nfs3hp::mix::interpolateStereo8 */

/* sub_527210: native, nfs3hp::mix::interpolateMono16 */

/* sub_527290: native, nfs3hp::mix::interpolateStereo16 */

/* sub_527330: native, nfs3hp::snd::dead_0x527330 */

/* sub_5273c0: native, nfs3hp::snd::dead_0x5273c0 */

/* sub_5274b0: native, nfs3hp::port::pcmOnceMono */

/* sub_5274d0: native, nfs3hp::port::pcmLoopedMono */

/* sub_527500: native, nfs3hp::port::pcmStreamedMono */

/* sub_527520: native, nfs3hp::port::pcmFileMono */

/* sub_527540: native, nfs3hp::port::pcmOnceStereo */

/* sub_527560: native, nfs3hp::port::pcmLoopedStereo */

/* sub_527590: native, nfs3hp::port::pcmStreamedStereo */

/* sub_5275b0: native, nfs3hp::port::pcmFileStereo */

/* sub_5275d0: native, nfs3hp::snd::dead_0x5275d0 */

/* sub_5275f0: native, nfs3hp::snd::dead_0x5275f0 */

/* sub_527620: native, nfs3hp::snd::dead_0x527620 */

/* sub_527640: native, nfs3hp::snd::dead_0x527640 */

/* sub_527660: native, nfs3hp::snd::dead_0x527660 */

/* sub_527680: native, nfs3hp::snd::dead_0x527680 */

/* sub_5276b0: native, nfs3hp::snd::dead_0x5276b0 */

/* sub_5276d0: native, nfs3hp::snd::dead_0x5276d0 */

/* sub_5276f0: native, nfs3hp::port::pcmOnceStereo2 */

/* sub_527710: native, nfs3hp::port::pcmLoopedStereo2 */

/* sub_527740: native, nfs3hp::port::pcmStreamedStereo2 */

/* sub_527760: native, nfs3hp::port::pcmFileStereo2 */

/* sub_527780: native, nfs3hp::port::pcmOnceQuad */

/* sub_5277a0: native, nfs3hp::port::pcmLoopedQuad */

/* sub_5277d0: native, nfs3hp::port::pcmStreamedQuad */

/* sub_5277f0: native, nfs3hp::port::pcmFileQuad */

/* sub_527810: native, nfs3hp::snd::dead_0x527810 */

/* sub_527840: native, nfs3hp::snd::dead_0x527840 */

/* sub_527870: native, nfs3hp::snd::dead_0x527870 */

/* sub_527890: native, nfs3hp::snd::dead_0x527890 */

/* sub_5278b0: native, nfs3hp::snd::dead_0x5278b0 */

/* sub_5278e0: native, nfs3hp::snd::dead_0x5278e0 */

/* sub_527910: native, nfs3hp::snd::dead_0x527910 */

/* sub_527930: native, nfs3hp::snd::dead_0x527930 */

/* sub_527950: native, nfs3hp::snd::dead_0x527950 */

/* sub_527954: native, nfs3hp::snd::dead_0x527954 */

/* sub_527ae8: native, nfs3hp::snd::dead_0x527ae8 */

/* sub_527b30: native, nfs3hp::snd::dead_0x527b30 */

/* sub_527b8c: native, nfs3hp::snd::dead_0x527b8c */

/* sub_527d54: native, nfs3hp::snd::dead_0x527d54 */

/* sub_527f20: native, nfs3hp::snd::dead_0x527f20 */

/* sub_527f80: native, nfs3hp::snd::dead_0x527f80 */

/* sub_52813c: native, nfs3hp::snd::dead_0x52813c */

/* sub_528180: native, nfs3hp::snd::dead_0x528180 */

/* sub_5283a4: native, nfs3hp::snd::dead_0x5283a4 */

/* sub_528400: native, nfs3hp::snd::dead_0x528400 */

/* sub_528404: native, nfs3hp::snd::dead_0x528404 */

/* sub_5285b8: native, nfs3hp::snd::dead_0x5285b8 */

/* sub_528610: native, nfs3hp::snd::dead_0x528610 */

/* sub_528680: native, nfs3hp::snd::dead_0x528680 */

/* sub_5288b0: native, nfs3hp::snd::dead_0x5288b0 */

/* sub_528a98: native, nfs3hp::snd::dead_0x528a98 */

/* sub_528b00: native, nfs3hp::snd::dead_0x528b00 */

/* sub_528cac: native, nfs3hp::snd::dead_0x528cac */

/* sub_528d00: native, nfs3hp::snd::dead_0x528d00 */

/* sub_528f0c: native, nfs3hp::snd::dead_0x528f0c */

/* sub_528f70: native, nfs3hp::port::xaPlayed */

/* sub_528f74: native, nfs3hp::port::xaOnceDecode */

/* sub_529104: native, nfs3hp::port::xaOnceSetup */

/* sub_529140: native, nfs3hp::port::xaKeptCopy */

/* sub_5291a8: native, nfs3hp::port::xaLoopDecode */

/* sub_529384: native, nfs3hp::port::xaIntroDecode */

/* sub_529534: native, nfs3hp::port::xaIntroSetup */

/* sub_529590: native, nfs3hp::port::xaStreamDecode */

/* sub_529778: native, nfs3hp::port::xaStreamSetup */

/* sub_5297c0: native, nfs3hp::port::xaPacketDecode */

/* sub_5299f4: native, nfs3hp::port::xaPacketSetup */

/* sub_529a50: native, nfs3hp::port::xaStereoPlayed */

/* sub_529a54: native, nfs3hp::port::xaStereoOnceDecode */

/* sub_529c2c: native, nfs3hp::port::xaStereoOnceSetup */

/* sub_529c80: native, nfs3hp::port::xaStereoKeptCopy */

/* sub_529cfc: native, nfs3hp::port::xaStereoLoopDecode */

/* sub_529f24: native, nfs3hp::port::xaStereoIntroDecode */

}
