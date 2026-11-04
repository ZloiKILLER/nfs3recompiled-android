#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_native_loading.py): defined in native_loading.cpp.
bool refpack(win32::WinApplication* app, x86::CPU& cpu);
// Port (tools/apply_native_vertices.py): defined in native_vertices.cpp.
bool commEventTrace(win32::WinApplication* app, x86::CPU& cpu);

/* sub_50e220: native, nfs3hp::ea::qposition */

/* align: skip 0x90 */
void Application::sub_50e2d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0050e2d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0050e2d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e2d2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050e2d3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0050e2d4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e2d5  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0050e2d7  8b5018                 -mov edx, dword ptr [eax + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0050e2da  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050e2db  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e2e1  8b4b18                 -mov ecx, dword ptr [ebx + 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */);
    // 0050e2e4  beffffffff             -mov esi, 0xffffffff
    cpu.esi = 4294967295 /*0xffffffff*/;
    // 0050e2e9  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0050e2eb  7418                   -je 0x50e305
    if (cpu.flags.zf)
    {
        goto L_0x0050e305;
    }
L_0x0050e2ed:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e2ed  837b0800               +cmp dword ptr [ebx + 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0050e2f1  7545                   -jne 0x50e338
    if (!cpu.flags.zf)
    {
        goto L_0x0050e338;
    }
    // 0050e2f3  8b4b18                 -mov ecx, dword ptr [ebx + 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */);
    // 0050e2f6  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e2f7  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e2fd  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0050e2ff  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e300  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e301  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e302  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e303  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e304  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0050e305:
    // 0050e305  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0050e306  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0050e307  bd50fc5400             -mov ebp, 0x54fc50
    cpu.ebp = 5569616 /*0x54fc50*/;
    // 0050e30c  bfb8fb5400             -mov edi, 0x54fbb8
    cpu.edi = 5569464 /*0x54fbb8*/;
    // 0050e311  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e312  b8b9020000             -mov eax, 0x2b9
    cpu.eax = 697 /*0x2b9*/;
    // 0050e317  893d90215500           -mov dword ptr [0x552190], edi
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edi;
    // 0050e31d  68ccfb5400             -push 0x54fbcc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5569484 /*0x54fbcc*/;
    cpu.esp -= 4;
    // 0050e322  892d94215500           -mov dword ptr [0x552194], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebp;
    // 0050e328  a398215500             -mov dword ptr [0x552198], eax
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.eax;
    // 0050e32d  e8de2cefff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e332  83c40c                 +add esp, 0xc
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
    // 0050e335  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e336  ebb5                   -jmp 0x50e2ed
    goto L_0x0050e2ed;
L_0x0050e338:
    // 0050e338  8b5314                 -mov edx, dword ptr [ebx + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */);
    // 0050e33b  8b4308                 -mov eax, dword ptr [ebx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 0050e33e  ff5310                 -call dword ptr [ebx + 0x10]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e341  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0050e343  8b4b18                 -mov ecx, dword ptr [ebx + 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */);
    // 0050e346  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e347  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e34d  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0050e34f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e350  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e351  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e352  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e353  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e354  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_50e360: native, nfs3hp::dead::stub_0x50e360 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_50e3f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0050e3f0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e3f1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0050e3f2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0050e3f3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e3f4  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 0050e3f6  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0050e3f8  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 0050e3fa  8b5018                 -mov edx, dword ptr [eax + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0050e3fd  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050e3fe  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e404  837d1800               +cmp dword ptr [ebp + 0x18], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0050e408  742c                   -je 0x50e436
    if (cpu.flags.zf)
    {
        goto L_0x0050e436;
    }
L_0x0050e40a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e40a  8b5d08                 -mov ebx, dword ptr [ebp + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 0050e40d  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0050e40f  7414                   -je 0x50e425
    if (cpu.flags.zf)
    {
        goto L_0x0050e425;
    }
L_0x0050e411:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e411  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0050e413  7410                   -je 0x50e425
    if (cpu.flags.zf)
    {
        goto L_0x0050e425;
    }
    // 0050e415  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0050e417  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e419  ffd7                   -call edi
    cpu.ip = cpu.edi;
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e41b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0050e41d  7506                   -jne 0x50e425
    if (!cpu.flags.zf)
    {
        goto L_0x0050e425;
    }
    // 0050e41f  8b1b                   -mov ebx, dword ptr [ebx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0050e421  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0050e423  75ec                   -jne 0x50e411
    if (!cpu.flags.zf)
    {
        goto L_0x0050e411;
    }
L_0x0050e425:
    // 0050e425  8b4d18                 -mov ecx, dword ptr [ebp + 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 0050e428  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e429  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e42f  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e431  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e432  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e433  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e434  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e435  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0050e436:
    // 0050e436  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e437  b870fc5400             -mov eax, 0x54fc70
    cpu.eax = 5569648 /*0x54fc70*/;
    // 0050e43c  bbb8fb5400             -mov ebx, 0x54fbb8
    cpu.ebx = 5569464 /*0x54fbb8*/;
    // 0050e441  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0050e442  ba03030000             -mov edx, 0x303
    cpu.edx = 771 /*0x303*/;
    // 0050e447  891d90215500           -mov dword ptr [0x552190], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebx;
    // 0050e44d  68ccfb5400             -push 0x54fbcc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5569484 /*0x54fbcc*/;
    cpu.esp -= 4;
    // 0050e452  a394215500             -mov dword ptr [0x552194], eax
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.eax;
    // 0050e457  891598215500           -mov dword ptr [0x552198], edx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edx;
    // 0050e45d  e8ae2befff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e462  83c40c                 +add esp, 0xc
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
    // 0050e465  eba3                   -jmp 0x50e40a
    goto L_0x0050e40a;
}

/* sub_50e470: native, nfs3hp::ea::qfindindex */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_50e500(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0050e500  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0050e501  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e502  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0050e503  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0050e504  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e505  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0050e507  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0050e509  8b5018                 -mov edx, dword ptr [eax + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0050e50c  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050e50d  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e513  837e1800               +cmp dword ptr [esi + 0x18], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0050e517  742f                   -je 0x50e548
    if (cpu.flags.zf)
    {
        goto L_0x0050e548;
    }
L_0x0050e519:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e519  8b5e08                 -mov ebx, dword ptr [esi + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 0050e51c  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0050e51e  7412                   -je 0x50e532
    if (cpu.flags.zf)
    {
        goto L_0x0050e532;
    }
L_0x0050e520:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e520  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e522  8b5614                 -mov edx, dword ptr [esi + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 0050e525  ff5610                 -call dword ptr [esi + 0x10]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e528  39f8                   +cmp eax, edi
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
    // 0050e52a  7306                   -jae 0x50e532
    if (!cpu.flags.cf)
    {
        goto L_0x0050e532;
    }
    // 0050e52c  8b1b                   -mov ebx, dword ptr [ebx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0050e52e  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0050e530  75ee                   -jne 0x50e520
    if (!cpu.flags.zf)
    {
        goto L_0x0050e520;
    }
L_0x0050e532:
    // 0050e532  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0050e534  7543                   -jne 0x50e579
    if (!cpu.flags.zf)
    {
        goto L_0x0050e579;
    }
L_0x0050e536:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e536  8b5618                 -mov edx, dword ptr [esi + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0050e539  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050e53a  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e540  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e542  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e543  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e544  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e545  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e546  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e547  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0050e548:
    // 0050e548  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0050e549  bd84fc5400             -mov ebp, 0x54fc84
    cpu.ebp = 5569668 /*0x54fc84*/;
    // 0050e54e  bbb8fb5400             -mov ebx, 0x54fbb8
    cpu.ebx = 5569464 /*0x54fbb8*/;
    // 0050e553  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e554  b821030000             -mov eax, 0x321
    cpu.eax = 801 /*0x321*/;
    // 0050e559  891d90215500           -mov dword ptr [0x552190], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebx;
    // 0050e55f  68ccfb5400             -push 0x54fbcc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5569484 /*0x54fbcc*/;
    cpu.esp -= 4;
    // 0050e564  892d94215500           -mov dword ptr [0x552194], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebp;
    // 0050e56a  a398215500             -mov dword ptr [0x552198], eax
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.eax;
    // 0050e56f  e89c2aefff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e574  83c40c                 +add esp, 0xc
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
    // 0050e577  eba0                   -jmp 0x50e519
    goto L_0x0050e519;
L_0x0050e579:
    // 0050e579  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e57b  8b5614                 -mov edx, dword ptr [esi + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 0050e57e  ff5610                 -call dword ptr [esi + 0x10]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e581  39f8                   +cmp eax, edi
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
    // 0050e583  74b1                   -je 0x50e536
    if (cpu.flags.zf)
    {
        goto L_0x0050e536;
    }
    // 0050e585  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0050e587  8b5618                 -mov edx, dword ptr [esi + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0050e58a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050e58b  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e591  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e593  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e594  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e595  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e596  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e597  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e598  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_50e5a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0050e5a0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e5a1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0050e5a2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0050e5a3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e5a4  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 0050e5a6  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0050e5a8  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0050e5aa  8b5018                 -mov edx, dword ptr [eax + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0050e5ad  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050e5ae  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e5b4  837d1800               +cmp dword ptr [ebp + 0x18], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0050e5b8  7430                   -je 0x50e5ea
    if (cpu.flags.zf)
    {
        goto L_0x0050e5ea;
    }
L_0x0050e5ba:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e5ba  8b5d08                 -mov ebx, dword ptr [ebp + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 0050e5bd  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0050e5bf  7414                   -je 0x50e5d5
    if (cpu.flags.zf)
    {
        goto L_0x0050e5d5;
    }
L_0x0050e5c1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e5c1  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0050e5c3  7410                   -je 0x50e5d5
    if (cpu.flags.zf)
    {
        goto L_0x0050e5d5;
    }
    // 0050e5c5  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0050e5c7  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e5c9  ffd6                   -call esi
    cpu.ip = cpu.esi;
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e5cb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0050e5cd  7506                   -jne 0x50e5d5
    if (!cpu.flags.zf)
    {
        goto L_0x0050e5d5;
    }
    // 0050e5cf  8b1b                   -mov ebx, dword ptr [ebx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0050e5d1  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0050e5d3  75ec                   -jne 0x50e5c1
    if (!cpu.flags.zf)
    {
        goto L_0x0050e5c1;
    }
L_0x0050e5d5:
    // 0050e5d5  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0050e5d7  7542                   -jne 0x50e61b
    if (!cpu.flags.zf)
    {
        goto L_0x0050e61b;
    }
L_0x0050e5d9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e5d9  8b7d18                 -mov edi, dword ptr [ebp + 0x18]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 0050e5dc  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0050e5dd  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e5e3  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e5e5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e5e6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e5e7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e5e8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e5e9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0050e5ea:
    // 0050e5ea  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e5eb  b894fc5400             -mov eax, 0x54fc94
    cpu.eax = 5569684 /*0x54fc94*/;
    // 0050e5f0  bbb8fb5400             -mov ebx, 0x54fbb8
    cpu.ebx = 5569464 /*0x54fbb8*/;
    // 0050e5f5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0050e5f6  ba6a030000             -mov edx, 0x36a
    cpu.edx = 874 /*0x36a*/;
    // 0050e5fb  891d90215500           -mov dword ptr [0x552190], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebx;
    // 0050e601  68ccfb5400             -push 0x54fbcc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5569484 /*0x54fbcc*/;
    cpu.esp -= 4;
    // 0050e606  a394215500             -mov dword ptr [0x552194], eax
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.eax;
    // 0050e60b  891598215500           -mov dword ptr [0x552198], edx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edx;
    // 0050e611  e8fa29efff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e616  83c40c                 +add esp, 0xc
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
    // 0050e619  eb9f                   -jmp 0x50e5ba
    goto L_0x0050e5ba;
L_0x0050e61b:
    // 0050e61b  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0050e61d  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0050e61f  e8fcf5ffff             -call 0x50dc20
    cpu.esp -= 4;
    nfs3hp::ea::qunlink(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e624  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0050e626  75b1                   -jne 0x50e5d9
    if (!cpu.flags.zf)
    {
        goto L_0x0050e5d9;
    }
    // 0050e628  b9b8fb5400             -mov ecx, 0x54fbb8
    cpu.ecx = 5569464 /*0x54fbb8*/;
    // 0050e62d  bb94fc5400             -mov ebx, 0x54fc94
    cpu.ebx = 5569684 /*0x54fc94*/;
    // 0050e632  be73030000             -mov esi, 0x373
    cpu.esi = 883 /*0x373*/;
    // 0050e637  689cfc5400             -push 0x54fc9c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5569692 /*0x54fc9c*/;
    cpu.esp -= 4;
    // 0050e63c  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 0050e642  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 0050e648  893598215500           -mov dword ptr [0x552198], esi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.esi;
    // 0050e64e  e8bd29efff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e653  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0050e656  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0050e658  8b7d18                 -mov edi, dword ptr [ebp + 0x18]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 0050e65b  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0050e65c  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e662  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e664  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e665  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e666  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e667  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e668  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_50e670: native, nfs3hp::dead::stub_0x50e670 */

/* sub_50e740: native, nfs3hp::dead::stub_0x50e740 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_50e800(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0050e800  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0050e801  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e802  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0050e803  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0050e804  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e805  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0050e807  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0050e809  8b5018                 -mov edx, dword ptr [eax + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0050e80c  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050e80d  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e813  837e1800               +cmp dword ptr [esi + 0x18], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0050e817  7431                   -je 0x50e84a
    if (cpu.flags.zf)
    {
        goto L_0x0050e84a;
    }
L_0x0050e819:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e819  8b5e08                 -mov ebx, dword ptr [esi + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 0050e81c  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0050e81e  7412                   -je 0x50e832
    if (cpu.flags.zf)
    {
        goto L_0x0050e832;
    }
L_0x0050e820:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e820  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e822  8b5614                 -mov edx, dword ptr [esi + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 0050e825  ff5610                 -call dword ptr [esi + 0x10]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e828  39f8                   +cmp eax, edi
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
    // 0050e82a  7306                   -jae 0x50e832
    if (!cpu.flags.cf)
    {
        goto L_0x0050e832;
    }
    // 0050e82c  8b1b                   -mov ebx, dword ptr [ebx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0050e82e  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0050e830  75ee                   -jne 0x50e820
    if (!cpu.flags.zf)
    {
        goto L_0x0050e820;
    }
L_0x0050e832:
    // 0050e832  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0050e834  7545                   -jne 0x50e87b
    if (!cpu.flags.zf)
    {
        goto L_0x0050e87b;
    }
L_0x0050e836:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e836  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x0050e838:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e838  8b7e18                 -mov edi, dword ptr [esi + 0x18]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0050e83b  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0050e83c  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e842  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e844  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e845  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e846  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e847  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e848  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e849  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0050e84a:
    // 0050e84a  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0050e84b  bd58fd5400             -mov ebp, 0x54fd58
    cpu.ebp = 5569880 /*0x54fd58*/;
    // 0050e850  bbb8fb5400             -mov ebx, 0x54fbb8
    cpu.ebx = 5569464 /*0x54fbb8*/;
    // 0050e855  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e856  b8ac030000             -mov eax, 0x3ac
    cpu.eax = 940 /*0x3ac*/;
    // 0050e85b  891d90215500           -mov dword ptr [0x552190], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebx;
    // 0050e861  68ccfb5400             -push 0x54fbcc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5569484 /*0x54fbcc*/;
    cpu.esp -= 4;
    // 0050e866  892d94215500           -mov dword ptr [0x552194], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebp;
    // 0050e86c  a398215500             -mov dword ptr [0x552198], eax
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.eax;
    // 0050e871  e89a27efff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e876  83c40c                 +add esp, 0xc
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
    // 0050e879  eb9e                   -jmp 0x50e819
    goto L_0x0050e819;
L_0x0050e87b:
    // 0050e87b  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e87d  8b5614                 -mov edx, dword ptr [esi + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 0050e880  ff5610                 -call dword ptr [esi + 0x10]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e883  39f8                   +cmp eax, edi
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
    // 0050e885  75af                   -jne 0x50e836
    if (!cpu.flags.zf)
    {
        goto L_0x0050e836;
    }
    // 0050e887  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0050e889  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0050e88b  e890f3ffff             -call 0x50dc20
    cpu.esp -= 4;
    nfs3hp::ea::qunlink(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e890  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0050e892  75a4                   -jne 0x50e838
    if (!cpu.flags.zf)
    {
        goto L_0x0050e838;
    }
    // 0050e894  bab8fb5400             -mov edx, 0x54fbb8
    cpu.edx = 5569464 /*0x54fbb8*/;
    // 0050e899  b958fd5400             -mov ecx, 0x54fd58
    cpu.ecx = 5569880 /*0x54fd58*/;
    // 0050e89e  bbb5030000             -mov ebx, 0x3b5
    cpu.ebx = 949 /*0x3b5*/;
    // 0050e8a3  6868fd5400             -push 0x54fd68
    app->getMemory<x86::reg32>(cpu.esp-4) = 5569896 /*0x54fd68*/;
    cpu.esp -= 4;
    // 0050e8a8  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 0050e8ae  890d94215500           -mov dword ptr [0x552194], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ecx;
    // 0050e8b4  891d98215500           -mov dword ptr [0x552198], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebx;
    // 0050e8ba  e85127efff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e8bf  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0050e8c2  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0050e8c4  8b7e18                 -mov edi, dword ptr [esi + 0x18]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0050e8c7  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0050e8c8  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e8ce  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e8d0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e8d1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e8d2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e8d3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e8d4  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e8d5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_50e8e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0050e8e0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e8e1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0050e8e2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0050e8e3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e8e4  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0050e8e7  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0050e8e9  89d5                   -mov ebp, edx
    cpu.ebp = cpu.edx;
    // 0050e8eb  8b5018                 -mov edx, dword ptr [eax + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0050e8ee  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050e8ef  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e8f5  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0050e8f8  83e001                 -and eax, 1
    cpu.eax &= x86::reg32(x86::sreg32(1 /*0x1*/));
    // 0050e8fb  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0050e8fe  8a6604                 -mov ah, byte ptr [esi + 4]
    cpu.ah = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0050e901  8b4e18                 -mov ecx, dword ptr [esi + 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0050e904  80e4fe                 -and ah, 0xfe
    cpu.ah &= x86::reg8(x86::sreg8(254 /*0xfe*/));
    // 0050e907  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e908  886604                 -mov byte ptr [esi + 4], ah
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(4) /* 0x4 */) = cpu.ah;
    // 0050e90b  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e911  8b4618                 -mov eax, dword ptr [esi + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0050e914  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0050e916  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0050e918  7458                   -je 0x50e972
    if (cpu.flags.zf)
    {
        goto L_0x0050e972;
    }
L_0x0050e91a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e91a  8b4e08                 -mov ecx, dword ptr [esi + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 0050e91d  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0050e91f  7422                   -je 0x50e943
    if (cpu.flags.zf)
    {
        goto L_0x0050e943;
    }
L_0x0050e921:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e921  8a5604                 -mov dl, byte ptr [esi + 4]
    cpu.dl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0050e924  f6c201                 +test dl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 1 /*0x1*/));
    // 0050e927  751a                   -jne 0x50e943
    if (!cpu.flags.zf)
    {
        goto L_0x0050e943;
    }
    // 0050e929  f6c201                 +test dl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 1 /*0x1*/));
    // 0050e92c  750f                   -jne 0x50e93d
    if (!cpu.flags.zf)
    {
        goto L_0x0050e93d;
    }
    // 0050e92e  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0050e930  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0050e932  ffd5                   -call ebp
    cpu.ip = cpu.ebp;
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e934  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0050e936  7504                   -jne 0x50e93c
    if (!cpu.flags.zf)
    {
        goto L_0x0050e93c;
    }
    // 0050e938  804e0401               -or byte ptr [esi + 4], 1
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(4) /* 0x4 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0050e93c:
    // 0050e93c  47                     -inc edi
    (cpu.edi)++;
L_0x0050e93d:
    // 0050e93d  8b09                   -mov ecx, dword ptr [ecx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx);
    // 0050e93f  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0050e941  75de                   -jne 0x50e921
    if (!cpu.flags.zf)
    {
        goto L_0x0050e921;
    }
L_0x0050e943:
    // 0050e943  8b5618                 -mov edx, dword ptr [esi + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0050e946  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050e947  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e94d  f6460401               +test byte ptr [esi + 4], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.esi + x86::reg32(4) /* 0x4 */) & 1 /*0x1*/));
    // 0050e951  7453                   -je 0x50e9a6
    if (cpu.flags.zf)
    {
        goto L_0x0050e9a6;
    }
L_0x0050e953:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e953  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 0050e956  8b4e04                 -mov ecx, dword ptr [esi + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0050e959  8b5e18                 -mov ebx, dword ptr [esi + 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0050e95c  09c1                   -or ecx, eax
    cpu.ecx |= x86::reg32(x86::sreg32(cpu.eax));
    // 0050e95e  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0050e95f  894e04                 -mov dword ptr [esi + 4], ecx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = cpu.ecx;
    // 0050e962  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e968  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0050e96a  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0050e96d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e96e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e96f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e970  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e971  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0050e972:
    // 0050e972  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0050e973  b9a4fd5400             -mov ecx, 0x54fda4
    cpu.ecx = 5569956 /*0x54fda4*/;
    // 0050e978  bab8fb5400             -mov edx, 0x54fbb8
    cpu.edx = 5569464 /*0x54fbb8*/;
    // 0050e97d  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e97e  b8f6030000             -mov eax, 0x3f6
    cpu.eax = 1014 /*0x3f6*/;
    // 0050e983  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 0050e989  68ccfb5400             -push 0x54fbcc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5569484 /*0x54fbcc*/;
    cpu.esp -= 4;
    // 0050e98e  890d94215500           -mov dword ptr [0x552194], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ecx;
    // 0050e994  a398215500             -mov dword ptr [0x552198], eax
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.eax;
    // 0050e999  e87226efff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e99e  83c40c                 +add esp, 0xc
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
    // 0050e9a1  e974ffffff             -jmp 0x50e91a
    goto L_0x0050e91a;
L_0x0050e9a6:
    // 0050e9a6  bfffffffff             -mov edi, 0xffffffff
    cpu.edi = 4294967295 /*0xffffffff*/;
    // 0050e9ab  eba6                   -jmp 0x50e953
    goto L_0x0050e953;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_50e9b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0050e9b0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e9b1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0050e9b2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0050e9b3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0050e9b4  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0050e9b6  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0050e9b8  8b5018                 -mov edx, dword ptr [eax + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0050e9bb  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050e9bc  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e9c2  837e1800               +cmp dword ptr [esi + 0x18], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0050e9c6  741c                   -je 0x50e9e4
    if (cpu.flags.zf)
    {
        goto L_0x0050e9e4;
    }
L_0x0050e9c8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0050e9c8  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0050e9ca  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0050e9cc  e80fffffff             -call 0x50e8e0
    cpu.esp -= 4;
    sub_50e8e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e9d1  8b4e18                 -mov ecx, dword ptr [esi + 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0050e9d4  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050e9d5  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0050e9d7  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050e9dd  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0050e9df  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e9e0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e9e1  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e9e2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050e9e3  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0050e9e4:
    // 0050e9e4  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0050e9e5  b8b0fd5400             -mov eax, 0x54fdb0
    cpu.eax = 5569968 /*0x54fdb0*/;
    // 0050e9ea  bdb8fb5400             -mov ebp, 0x54fbb8
    cpu.ebp = 5569464 /*0x54fbb8*/;
    // 0050e9ef  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0050e9f0  ba14040000             -mov edx, 0x414
    cpu.edx = 1044 /*0x414*/;
    // 0050e9f5  892d90215500           -mov dword ptr [0x552190], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebp;
    // 0050e9fb  68ccfb5400             -push 0x54fbcc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5569484 /*0x54fbcc*/;
    cpu.esp -= 4;
    // 0050ea00  a394215500             -mov dword ptr [0x552194], eax
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.eax;
    // 0050ea05  891598215500           -mov dword ptr [0x552198], edx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edx;
    // 0050ea0b  e80026efff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050ea10  83c40c                 +add esp, 0xc
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
    // 0050ea13  ebb3                   -jmp 0x50e9c8
    goto L_0x0050e9c8;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_50ea20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0050ea20  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050ea21  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050ea22  8b5018                 -mov edx, dword ptr [eax + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0050ea25  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050ea26  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050ea2c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0050ea2e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050ea2f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050ea30  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_50ea40(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0050ea40  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0050ea41  8b5018                 -mov edx, dword ptr [eax + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0050ea44  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0050ea45  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0050ea4b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0050ea4c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_50ea4e: native, nfs3hp::watcom::modf */

/* sub_50ea68: native, nfs3hp::dead::stub_0x50ea68 */

/* sub_50ea85: native, nfs3hp::dead::stub_0x50ea85 */

/* sub_50eaa7: native, nfs3hp::dead::stub_0x50eaa7 */

/* sub_50ead0: native, nfs3hp::watcom::getenv */

/* sub_50eb30: native, nfs3hp::watcom::getctime */

/* sub_50eba0: native, nfs3hp::watcom::mktime */

/* sub_50ed50: native, nfs3hp::ea::huffUnpack */

/* sub_510110: native, nfs3hp::ea::runUnpack */

/* align: skip 0x00 0x00 0x00 */
void Application::sub_5102a4(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (refpack(app, cpu.sync())) /* port: native (tools/apply_native_loading.py) */
    {
        cpu.reload();
        return;
    }
    // 005102a4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005102a5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 005102a7  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 005102aa  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005102ab  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005102ac  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005102ad  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 005102af  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 005102b1  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 005102b3  c745fc00000000         -mov dword ptr [ebp - 4], 0
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = 0 /*0x0*/;
    // 005102ba  09db                   +or ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(cpu.ebx))));
    // 005102bc  0f84dc000000           -je 0x51039e
    if (cpu.flags.zf)
    {
    return sub_51039e(app, cpu.handOver());
    }
    // 005102c2  668b03                 -mov ax, word ptr [ebx]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ebx);
    // 005102c5  8d5b02                 -lea ebx, [ebx + 2]
    cpu.ebx = x86::reg32(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 005102c8  2401                   +and al, 1
    cpu.clear_co();
    cpu.set_szp((cpu.al &= x86::reg8(x86::sreg8(1 /*0x1*/))));
    // 005102ca  7403                   -je 0x5102cf
    if (cpu.flags.zf)
    {
        goto L_0x005102cf;
    }
    // 005102cc  8d5b03                 -lea ebx, [ebx + 3]
    cpu.ebx = x86::reg32(cpu.ebx + x86::reg32(3) /* 0x3 */);
L_0x005102cf:
    // 005102cf  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005102d1  8a03                   -mov al, byte ptr [ebx]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx);
    // 005102d3  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 005102d6  8a6301                 -mov ah, byte ptr [ebx + 1]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 005102d9  8a4302                 -mov al, byte ptr [ebx + 2]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 005102dc  8d5b03                 -lea ebx, [ebx + 3]
    cpu.ebx = x86::reg32(cpu.ebx + x86::reg32(3) /* 0x3 */);
    // 005102df  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 005102e2  83f900                 +cmp ecx, 0
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005102e5  0f84b3000000           -je 0x51039e
    if (cpu.flags.zf)
    {
    return sub_51039e(app, cpu.handOver());
    }
    // 005102eb  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 005102ed  eb48                   -jmp 0x510337
    return sub_510337(app, cpu.handOver());
}

/* align: skip 0x90 */
void Application::sub_5102f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
L_0x005102f0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005102f0  8d7302                 -lea esi, [ebx + 2]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 005102f3  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 005102f5  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 005102f7  88d5                   -mov ch, dl
    cpu.ch = cpu.dl;
    // 005102f9  88f1                   -mov cl, dh
    cpu.cl = cpu.dh;
    // 005102fb  83e21c                 -and edx, 0x1c
    cpu.edx &= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 005102fe  c0ed05                 -shr ch, 5
    cpu.ch >>= 5 /*0x5*/ % 32;
    // 00510301  c1ea02                 -shr edx, 2
    cpu.edx >>= 2 /*0x2*/ % 32;
    // 00510304  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00510306  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 0051030a  8d4a03                 -lea ecx, [edx + 3]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(3) /* 0x3 */);
    // 0051030d  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 0051030f  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510311  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 00510313  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 00510315  7828                   -js 0x51033f
    if (cpu.flags.sf)
    {
        goto L_0x0051033f;
    }
L_0x00510317:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00510317  83e103                 +and ecx, 3
    cpu.clear_co();
    cpu.set_szp((cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/))));
    // 0051031a  75d4                   -jne 0x5102f0
    if (!cpu.flags.zf)
    {
        goto L_0x005102f0;
    }
    // 0051031c  8d5b02                 -lea ebx, [ebx + 2]
    cpu.ebx = x86::reg32(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 0051031f  88d5                   -mov ch, dl
    cpu.ch = cpu.dl;
    // 00510321  88f1                   -mov cl, dh
    cpu.cl = cpu.dh;
    // 00510323  83e21c                 -and edx, 0x1c
    cpu.edx &= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 00510326  c0ed05                 -shr ch, 5
    cpu.ch >>= 5 /*0x5*/ % 32;
    // 00510329  c1ea02                 -shr edx, 2
    cpu.edx >>= 2 /*0x2*/ % 32;
    // 0051032c  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051032e  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 00510332  8d4a03                 -lea ecx, [edx + 3]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(3) /* 0x3 */);
    // 00510335  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 00510337  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510339  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 0051033b  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0051033d  79d8                   -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
L_0x0051033f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051033f  00c9                   +add cl, cl
    {
        x86::reg8& tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.cl));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00510341  7831                   -js 0x510374
    if (cpu.flags.sf)
    {
        goto L_0x00510374;
    }
    // 00510343  88f1                   -mov cl, dh
    cpu.cl = cpu.dh;
    // 00510345  8d7303                 -lea esi, [ebx + 3]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(3) /* 0x3 */);
    // 00510348  c1e906                 -shr ecx, 6
    cpu.ecx >>= 6 /*0x6*/ % 32;
    // 0051034b  83e103                 -and ecx, 3
    cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0051034e  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 00510350  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00510352  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00510354  c1e910                 -shr ecx, 0x10
    cpu.ecx >>= 16 /*0x10*/ % 32;
    // 00510357  88f5                   -mov ch, dh
    cpu.ch = cpu.dh;
    // 00510359  80e53f                 -and ch, 0x3f
    cpu.ch &= x86::reg8(x86::sreg8(63 /*0x3f*/));
    // 0051035c  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051035e  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 00510362  83e23f                 -and edx, 0x3f
    cpu.edx &= x86::reg32(x86::sreg32(63 /*0x3f*/));
    // 00510365  8d4a04                 -lea ecx, [edx + 4]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 00510368  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 0051036a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051036c  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 0051036e  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 00510370  79a5                   -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 00510372  ebcb                   -jmp 0x51033f
    goto L_0x0051033f;
L_0x00510374:
    // 00510374  00c9                   +add cl, cl
    {
        x86::reg8& tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.cl));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00510376  7930                   -jns 0x5103a8
    if (!cpu.flags.sf)
    {
        goto L_0x005103a8;
    }
    // 00510378  80fafc                 +cmp dl, 0xfc
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(252 /*0xfc*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051037b  7317                   -jae 0x510394
    if (!cpu.flags.cf)
    {
        goto L_0x00510394;
    }
    // 0051037d  83e21f                 -and edx, 0x1f
    cpu.edx &= x86::reg32(x86::sreg32(31 /*0x1f*/));
    // 00510380  8d7301                 -lea esi, [ebx + 1]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 00510383  8d4a01                 -lea ecx, [edx + 1]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 00510386  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00510388  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0051038a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051038c  0a0e                   +or cl, byte ptr [esi]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.esi)))));
    // 0051038e  8b16                   -mov edx, dword ptr [esi]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi);
    // 00510390  7985                   -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 00510392  ebab                   -jmp 0x51033f
    goto L_0x0051033f;
L_0x00510394:
    // 00510394  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00510396  8d7301                 -lea esi, [ebx + 1]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 00510399  83e103                 -and ecx, 3
    cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0051039c  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 0051039e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 005103a1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 005103a6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005103a8:
    // 005103a8  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 005103aa  8d7304                 -lea esi, [ebx + 4]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 005103ad  83e103                 -and ecx, 3
    cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 005103b0  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 005103b2  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 005103b4  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 005103b6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 005103b8  83e110                 -and ecx, 0x10
    cpu.ecx &= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 005103bb  c1e808                 -shr eax, 8
    cpu.eax >>= 8 /*0x8*/ % 32;
    // 005103be  c1e10c                 -shl ecx, 0xc
    cpu.ecx <<= 12 /*0xc*/ % 32;
    // 005103c1  88e1                   -mov cl, ah
    cpu.cl = cpu.ah;
    // 005103c3  88c5                   -mov ch, al
    cpu.ch = cpu.al;
    // 005103c5  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 005103c7  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 005103cb  c1c208                 -rol edx, 8
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 8 /*0x8*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005103ce  c0ee02                 -shr dh, 2
    cpu.dh >>= 2 /*0x2*/ % 32;
    // 005103d1  81e2ff030000           -and edx, 0x3ff
    cpu.edx &= x86::reg32(x86::sreg32(1023 /*0x3ff*/));
    // 005103d7  83f9fc                 +cmp ecx, -4
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-4 /*-0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005103da  7d24                   -jge 0x510400
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00510400;
    }
    // 005103dc  8d4a05                 -lea ecx, [edx + 5]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(5) /* 0x5 */);
    // 005103df  8d5205                 -lea edx, [edx + 5]
    cpu.edx = x86::reg32(cpu.edx + x86::reg32(5) /* 0x5 */);
    // 005103e2  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 005103e5  83e203                 -and edx, 3
    cpu.edx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 005103e8  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005103ea  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 005103ec  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 005103ee  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 005103f0  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 005103f2  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 005103f4  0f891dffffff           -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 005103fa  e940ffffff             -jmp 0x51033f
    goto L_0x0051033f;
    // 005103ff  90                     -nop 
    ;
L_0x00510400:
    // 00510400  8d4a05                 -lea ecx, [edx + 5]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(5) /* 0x5 */);
    // 00510403  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 00510405  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510407  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 00510409  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0051040b  0f8906ffffff           -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 00510411  e929ffffff             -jmp 0x51033f
    goto L_0x0051033f;
}

/* align: skip  */
void Application::sub_51039e(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x0051039e;
L_0x005102f0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005102f0  8d7302                 -lea esi, [ebx + 2]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 005102f3  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 005102f5  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 005102f7  88d5                   -mov ch, dl
    cpu.ch = cpu.dl;
    // 005102f9  88f1                   -mov cl, dh
    cpu.cl = cpu.dh;
    // 005102fb  83e21c                 -and edx, 0x1c
    cpu.edx &= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 005102fe  c0ed05                 -shr ch, 5
    cpu.ch >>= 5 /*0x5*/ % 32;
    // 00510301  c1ea02                 -shr edx, 2
    cpu.edx >>= 2 /*0x2*/ % 32;
    // 00510304  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00510306  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 0051030a  8d4a03                 -lea ecx, [edx + 3]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(3) /* 0x3 */);
    // 0051030d  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 0051030f  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510311  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 00510313  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 00510315  7828                   -js 0x51033f
    if (cpu.flags.sf)
    {
        goto L_0x0051033f;
    }
L_0x00510317:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00510317  83e103                 +and ecx, 3
    cpu.clear_co();
    cpu.set_szp((cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/))));
    // 0051031a  75d4                   -jne 0x5102f0
    if (!cpu.flags.zf)
    {
        goto L_0x005102f0;
    }
    // 0051031c  8d5b02                 -lea ebx, [ebx + 2]
    cpu.ebx = x86::reg32(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 0051031f  88d5                   -mov ch, dl
    cpu.ch = cpu.dl;
    // 00510321  88f1                   -mov cl, dh
    cpu.cl = cpu.dh;
    // 00510323  83e21c                 -and edx, 0x1c
    cpu.edx &= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 00510326  c0ed05                 -shr ch, 5
    cpu.ch >>= 5 /*0x5*/ % 32;
    // 00510329  c1ea02                 -shr edx, 2
    cpu.edx >>= 2 /*0x2*/ % 32;
    // 0051032c  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051032e  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 00510332  8d4a03                 -lea ecx, [edx + 3]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(3) /* 0x3 */);
    // 00510335  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 00510337  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510339  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 0051033b  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0051033d  79d8                   -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
L_0x0051033f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051033f  00c9                   +add cl, cl
    {
        x86::reg8& tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.cl));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00510341  7831                   -js 0x510374
    if (cpu.flags.sf)
    {
        goto L_0x00510374;
    }
    // 00510343  88f1                   -mov cl, dh
    cpu.cl = cpu.dh;
    // 00510345  8d7303                 -lea esi, [ebx + 3]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(3) /* 0x3 */);
    // 00510348  c1e906                 -shr ecx, 6
    cpu.ecx >>= 6 /*0x6*/ % 32;
    // 0051034b  83e103                 -and ecx, 3
    cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0051034e  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 00510350  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00510352  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00510354  c1e910                 -shr ecx, 0x10
    cpu.ecx >>= 16 /*0x10*/ % 32;
    // 00510357  88f5                   -mov ch, dh
    cpu.ch = cpu.dh;
    // 00510359  80e53f                 -and ch, 0x3f
    cpu.ch &= x86::reg8(x86::sreg8(63 /*0x3f*/));
    // 0051035c  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051035e  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 00510362  83e23f                 -and edx, 0x3f
    cpu.edx &= x86::reg32(x86::sreg32(63 /*0x3f*/));
    // 00510365  8d4a04                 -lea ecx, [edx + 4]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 00510368  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 0051036a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051036c  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 0051036e  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 00510370  79a5                   -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 00510372  ebcb                   -jmp 0x51033f
    goto L_0x0051033f;
L_0x00510374:
    // 00510374  00c9                   +add cl, cl
    {
        x86::reg8& tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.cl));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00510376  7930                   -jns 0x5103a8
    if (!cpu.flags.sf)
    {
        goto L_0x005103a8;
    }
    // 00510378  80fafc                 +cmp dl, 0xfc
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(252 /*0xfc*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051037b  7317                   -jae 0x510394
    if (!cpu.flags.cf)
    {
        goto L_0x00510394;
    }
    // 0051037d  83e21f                 -and edx, 0x1f
    cpu.edx &= x86::reg32(x86::sreg32(31 /*0x1f*/));
    // 00510380  8d7301                 -lea esi, [ebx + 1]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 00510383  8d4a01                 -lea ecx, [edx + 1]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 00510386  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00510388  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0051038a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051038c  0a0e                   +or cl, byte ptr [esi]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.esi)))));
    // 0051038e  8b16                   -mov edx, dword ptr [esi]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi);
    // 00510390  7985                   -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 00510392  ebab                   -jmp 0x51033f
    goto L_0x0051033f;
L_0x00510394:
    // 00510394  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00510396  8d7301                 -lea esi, [ebx + 1]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 00510399  83e103                 -and ecx, 3
    cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0051039c  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
L_entry_0x0051039e:
    // 0051039e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 005103a1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 005103a6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005103a8:
    // 005103a8  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 005103aa  8d7304                 -lea esi, [ebx + 4]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 005103ad  83e103                 -and ecx, 3
    cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 005103b0  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 005103b2  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 005103b4  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 005103b6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 005103b8  83e110                 -and ecx, 0x10
    cpu.ecx &= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 005103bb  c1e808                 -shr eax, 8
    cpu.eax >>= 8 /*0x8*/ % 32;
    // 005103be  c1e10c                 -shl ecx, 0xc
    cpu.ecx <<= 12 /*0xc*/ % 32;
    // 005103c1  88e1                   -mov cl, ah
    cpu.cl = cpu.ah;
    // 005103c3  88c5                   -mov ch, al
    cpu.ch = cpu.al;
    // 005103c5  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 005103c7  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 005103cb  c1c208                 -rol edx, 8
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 8 /*0x8*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005103ce  c0ee02                 -shr dh, 2
    cpu.dh >>= 2 /*0x2*/ % 32;
    // 005103d1  81e2ff030000           -and edx, 0x3ff
    cpu.edx &= x86::reg32(x86::sreg32(1023 /*0x3ff*/));
    // 005103d7  83f9fc                 +cmp ecx, -4
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-4 /*-0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005103da  7d24                   -jge 0x510400
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00510400;
    }
    // 005103dc  8d4a05                 -lea ecx, [edx + 5]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(5) /* 0x5 */);
    // 005103df  8d5205                 -lea edx, [edx + 5]
    cpu.edx = x86::reg32(cpu.edx + x86::reg32(5) /* 0x5 */);
    // 005103e2  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 005103e5  83e203                 -and edx, 3
    cpu.edx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 005103e8  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005103ea  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 005103ec  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 005103ee  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 005103f0  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 005103f2  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 005103f4  0f891dffffff           -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 005103fa  e940ffffff             -jmp 0x51033f
    goto L_0x0051033f;
    // 005103ff  90                     -nop 
    ;
L_0x00510400:
    // 00510400  8d4a05                 -lea ecx, [edx + 5]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(5) /* 0x5 */);
    // 00510403  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 00510405  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510407  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 00510409  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0051040b  0f8906ffffff           -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 00510411  e929ffffff             -jmp 0x51033f
    goto L_0x0051033f;
}

/* align: skip  */
void Application::sub_510337(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x00510337;
L_0x005102f0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005102f0  8d7302                 -lea esi, [ebx + 2]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 005102f3  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 005102f5  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 005102f7  88d5                   -mov ch, dl
    cpu.ch = cpu.dl;
    // 005102f9  88f1                   -mov cl, dh
    cpu.cl = cpu.dh;
    // 005102fb  83e21c                 -and edx, 0x1c
    cpu.edx &= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 005102fe  c0ed05                 -shr ch, 5
    cpu.ch >>= 5 /*0x5*/ % 32;
    // 00510301  c1ea02                 -shr edx, 2
    cpu.edx >>= 2 /*0x2*/ % 32;
    // 00510304  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00510306  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 0051030a  8d4a03                 -lea ecx, [edx + 3]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(3) /* 0x3 */);
    // 0051030d  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 0051030f  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510311  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 00510313  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 00510315  7828                   -js 0x51033f
    if (cpu.flags.sf)
    {
        goto L_0x0051033f;
    }
L_0x00510317:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00510317  83e103                 +and ecx, 3
    cpu.clear_co();
    cpu.set_szp((cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/))));
    // 0051031a  75d4                   -jne 0x5102f0
    if (!cpu.flags.zf)
    {
        goto L_0x005102f0;
    }
    // 0051031c  8d5b02                 -lea ebx, [ebx + 2]
    cpu.ebx = x86::reg32(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 0051031f  88d5                   -mov ch, dl
    cpu.ch = cpu.dl;
    // 00510321  88f1                   -mov cl, dh
    cpu.cl = cpu.dh;
    // 00510323  83e21c                 -and edx, 0x1c
    cpu.edx &= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 00510326  c0ed05                 -shr ch, 5
    cpu.ch >>= 5 /*0x5*/ % 32;
    // 00510329  c1ea02                 -shr edx, 2
    cpu.edx >>= 2 /*0x2*/ % 32;
    // 0051032c  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051032e  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 00510332  8d4a03                 -lea ecx, [edx + 3]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(3) /* 0x3 */);
    // 00510335  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
L_entry_0x00510337:
    // 00510337  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510339  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 0051033b  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0051033d  79d8                   -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
L_0x0051033f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051033f  00c9                   +add cl, cl
    {
        x86::reg8& tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.cl));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00510341  7831                   -js 0x510374
    if (cpu.flags.sf)
    {
        goto L_0x00510374;
    }
    // 00510343  88f1                   -mov cl, dh
    cpu.cl = cpu.dh;
    // 00510345  8d7303                 -lea esi, [ebx + 3]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(3) /* 0x3 */);
    // 00510348  c1e906                 -shr ecx, 6
    cpu.ecx >>= 6 /*0x6*/ % 32;
    // 0051034b  83e103                 -and ecx, 3
    cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0051034e  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 00510350  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00510352  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00510354  c1e910                 -shr ecx, 0x10
    cpu.ecx >>= 16 /*0x10*/ % 32;
    // 00510357  88f5                   -mov ch, dh
    cpu.ch = cpu.dh;
    // 00510359  80e53f                 -and ch, 0x3f
    cpu.ch &= x86::reg8(x86::sreg8(63 /*0x3f*/));
    // 0051035c  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051035e  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 00510362  83e23f                 -and edx, 0x3f
    cpu.edx &= x86::reg32(x86::sreg32(63 /*0x3f*/));
    // 00510365  8d4a04                 -lea ecx, [edx + 4]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 00510368  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 0051036a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051036c  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 0051036e  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 00510370  79a5                   -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 00510372  ebcb                   -jmp 0x51033f
    goto L_0x0051033f;
L_0x00510374:
    // 00510374  00c9                   +add cl, cl
    {
        x86::reg8& tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.cl));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00510376  7930                   -jns 0x5103a8
    if (!cpu.flags.sf)
    {
        goto L_0x005103a8;
    }
    // 00510378  80fafc                 +cmp dl, 0xfc
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(252 /*0xfc*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051037b  7317                   -jae 0x510394
    if (!cpu.flags.cf)
    {
        goto L_0x00510394;
    }
    // 0051037d  83e21f                 -and edx, 0x1f
    cpu.edx &= x86::reg32(x86::sreg32(31 /*0x1f*/));
    // 00510380  8d7301                 -lea esi, [ebx + 1]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 00510383  8d4a01                 -lea ecx, [edx + 1]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 00510386  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00510388  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0051038a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051038c  0a0e                   +or cl, byte ptr [esi]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.esi)))));
    // 0051038e  8b16                   -mov edx, dword ptr [esi]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi);
    // 00510390  7985                   -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 00510392  ebab                   -jmp 0x51033f
    goto L_0x0051033f;
L_0x00510394:
    // 00510394  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00510396  8d7301                 -lea esi, [ebx + 1]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 00510399  83e103                 -and ecx, 3
    cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0051039c  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 0051039e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 005103a1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 005103a6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005103a7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005103a8:
    // 005103a8  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 005103aa  8d7304                 -lea esi, [ebx + 4]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 005103ad  83e103                 -and ecx, 3
    cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 005103b0  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 005103b2  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 005103b4  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 005103b6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 005103b8  83e110                 -and ecx, 0x10
    cpu.ecx &= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 005103bb  c1e808                 -shr eax, 8
    cpu.eax >>= 8 /*0x8*/ % 32;
    // 005103be  c1e10c                 -shl ecx, 0xc
    cpu.ecx <<= 12 /*0xc*/ % 32;
    // 005103c1  88e1                   -mov cl, ah
    cpu.cl = cpu.ah;
    // 005103c3  88c5                   -mov ch, al
    cpu.ch = cpu.al;
    // 005103c5  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 005103c7  8d740fff               -lea esi, [edi + ecx - 1]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */ + cpu.ecx * 1);
    // 005103cb  c1c208                 -rol edx, 8
    {
        x86::reg32& op = cpu.edx;
        x86::reg32 shift = 8 /*0x8*/ % 32;
        while (shift)
        {
            x86::reg32 cf = (op & 0x80000000);
            op = op << 1 | cf >> 31;
            shift--;
        }
    }
    // 005103ce  c0ee02                 -shr dh, 2
    cpu.dh >>= 2 /*0x2*/ % 32;
    // 005103d1  81e2ff030000           -and edx, 0x3ff
    cpu.edx &= x86::reg32(x86::sreg32(1023 /*0x3ff*/));
    // 005103d7  83f9fc                 +cmp ecx, -4
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-4 /*-0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005103da  7d24                   -jge 0x510400
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00510400;
    }
    // 005103dc  8d4a05                 -lea ecx, [edx + 5]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(5) /* 0x5 */);
    // 005103df  8d5205                 -lea edx, [edx + 5]
    cpu.edx = x86::reg32(cpu.edx + x86::reg32(5) /* 0x5 */);
    // 005103e2  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 005103e5  83e203                 -and edx, 3
    cpu.edx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 005103e8  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005103ea  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 005103ec  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 005103ee  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 005103f0  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 005103f2  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 005103f4  0f891dffffff           -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 005103fa  e940ffffff             -jmp 0x51033f
    goto L_0x0051033f;
    // 005103ff  90                     -nop 
    ;
L_0x00510400:
    // 00510400  8d4a05                 -lea ecx, [edx + 5]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(5) /* 0x5 */);
    // 00510403  f3a4                   -rep movsb byte ptr es:[edi], byte ptr [esi]
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
    }
    // 00510405  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510407  0a0b                   +or cl, byte ptr [ebx]
    cpu.clear_co();
    cpu.set_szp((cpu.cl |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebx)))));
    // 00510409  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0051040b  0f8906ffffff           -jns 0x510317
    if (!cpu.flags.sf)
    {
        goto L_0x00510317;
    }
    // 00510411  e929ffffff             -jmp 0x51033f
    goto L_0x0051033f;
}

/* sub_510420: native, nfs3hp::ea::dxtBlock */

/* sub_510740: native, nfs3hp::dead::stub_0x510740 */

/* sub_510760: native, nfs3hp::ea::dxtUnpack */

/* sub_510820: native, nfs3hp::ea::pairExpand */

/* sub_510870: native, nfs3hp::ea::pairUnpack */

/* sub_510a10: native, nfs3hp::port::findRequest */

/* align: skip 0x00 0x00 */
void Application::sub_510a50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2;
    // 00510a50  8b5e14                 -mov ebx, dword ptr [esi + 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 00510a53  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 00510a56  8b4e04                 -mov ecx, dword ptr [esi + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00510a59  8b561c                 -mov edx, dword ptr [esi + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(28) /* 0x1c */);
    // 00510a5c  09d9                   -or ecx, ebx
    cpu.ecx |= x86::reg32(x86::sreg32(cpu.ebx));
    // 00510a5e  09c2                   -or edx, eax
    cpu.edx |= x86::reg32(x86::sreg32(cpu.eax));
    // 00510a60  8b7e08                 -mov edi, dword ptr [esi + 8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 00510a63  8b6e18                 -mov ebp, dword ptr [esi + 0x18]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 00510a66  09ca                   -or edx, ecx
    cpu.edx |= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510a68  09ef                   -or edi, ebp
    cpu.edi |= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510a6a  8b4e10                 -mov ecx, dword ptr [esi + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 00510a6d  09fa                   -or edx, edi
    cpu.edx |= x86::reg32(x86::sreg32(cpu.edi));
    // 00510a6f  09ca                   +or edx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.edx |= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00510a71  0f8410010000           -je 0x510b87
    if (cpu.flags.zf)
    {
        goto L_0x00510b87;
    }
    // 00510a77  8b4e04                 -mov ecx, dword ptr [esi + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00510a7a  8b561c                 -mov edx, dword ptr [esi + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(28) /* 0x1c */);
    // 00510a7d  8d3c03                 -lea edi, [ebx + eax]
    cpu.edi = x86::reg32(cpu.ebx + cpu.eax * 1);
    // 00510a80  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00510a82  8d0411                 -lea eax, [ecx + edx]
    cpu.eax = x86::reg32(cpu.ecx + cpu.edx * 1);
    // 00510a85  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00510a87  891de8cd5600           -mov dword ptr [0x56cde8], ebx
    app->getMemory<x86::reg32>(x86::reg32(5688808) /* 0x56cde8 */) = cpu.ebx;
    // 00510a8d  890deccd5600           -mov dword ptr [0x56cdec], ecx
    app->getMemory<x86::reg32>(x86::reg32(5688812) /* 0x56cdec */) = cpu.ecx;
    // 00510a93  8d2c38                 -lea ebp, [eax + edi]
    cpu.ebp = x86::reg32(cpu.eax + cpu.edi * 1);
    // 00510a96  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00510a98  db05e8cd5600           -fild dword ptr [0x56cde8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5688808) /* 0x56cde8 */)));
    // 00510a9e  db05eccd5600           -fild dword ptr [0x56cdec]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5688812) /* 0x56cdec */)));
    // 00510aa4  d9c0                   -fld st(0)
    fp2 = x86::Float(fp1);
    // 00510aa6  d80d00ce5600           -fmul dword ptr [0x56ce00]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(x86::reg32(5688832) /* 0x56ce00 */)));
    // 00510aac  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00510aae  dcc1                   -fadd st(1), st(0)
    fp1 = fp::add(fp1, x86::Float(fp2));
    // 00510ab0  d80dfccd5600           -fmul dword ptr [0x56cdfc]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(x86::reg32(5688828) /* 0x56cdfc */)));
    // 00510ab6  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00510ab8  d80d04ce5600           -fmul dword ptr [0x56ce04]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(x86::reg32(5688836) /* 0x56ce04 */)));
    // 00510abe  f72df8cd5600           -imul dword ptr [0x56cdf8]
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5688824) /* 0x56cdf8 */))));
    // 00510ac4  dcc1                   -fadd st(1), st(0)
    fp1 = fp::add(fp1, x86::Float(fp2));
    // 00510ac6  deea                   -fsubp st(2)
    fp0 = fp::sub(fp0, fp2);
    // 00510ac8  d80508ce5600           -fadd dword ptr [0x56ce08]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(x86::reg32(5688840) /* 0x56ce08 */)));
    // 00510ace  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00510ad0  d80508ce5600           -fadd dword ptr [0x56ce08]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(x86::reg32(5688840) /* 0x56ce08 */)));
    // 00510ad6  8b4608                 -mov eax, dword ptr [esi + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 00510ad9  8b5e18                 -mov ebx, dword ptr [esi + 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 00510adc  d1e2                   -shl edx, 1
    cpu.edx <<= 1 /*0x1*/ % 32;
    // 00510ade  dd1df0cd5600           -fstp qword ptr [0x56cdf0]
    app->getMemory<double>(x86::reg32(5688816) /* 0x56cdf0 */) = double(fp1);
    // 00510ae4  dd1de8cd5600           -fstp qword ptr [0x56cde8]
    app->getMemory<double>(x86::reg32(5688808) /* 0x56cde8 */) = double(fp0);
    // 00510aea  8b0de8cd5600           -mov ecx, dword ptr [0x56cde8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5688808) /* 0x56cde8 */);
    // 00510af0  8b3df0cd5600           -mov edi, dword ptr [0x56cdf0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5688816) /* 0x56cdf0 */);
    // 00510af6  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 00510af8  01fa                   -add edx, edi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edi));
    // 00510afa  01ef                   -add edi, ebp
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ebp));
    // 00510afc  890deccd5600           -mov dword ptr [0x56cdec], ecx
    app->getMemory<x86::reg32>(x86::reg32(5688812) /* 0x56cdec */) = cpu.ecx;
    // 00510b02  8915f0cd5600           -mov dword ptr [0x56cdf0], edx
    app->getMemory<x86::reg32>(x86::reg32(5688816) /* 0x56cdf0 */) = cpu.edx;
    // 00510b08  893df4cd5600           -mov dword ptr [0x56cdf4], edi
    app->getMemory<x86::reg32>(x86::reg32(5688820) /* 0x56cdf4 */) = cpu.edi;
    // 00510b0e  8b0e                   -mov ecx, dword ptr [esi]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi);
    // 00510b10  8b5610                 -mov edx, dword ptr [esi + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 00510b13  8d3418                 -lea esi, [eax + ebx]
    cpu.esi = x86::reg32(cpu.eax + cpu.ebx * 1);
    // 00510b16  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00510b18  8d1c11                 -lea ebx, [ecx + edx]
    cpu.ebx = x86::reg32(cpu.ecx + cpu.edx * 1);
    // 00510b1b  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00510b1d  f72df8cd5600           -imul dword ptr [0x56cdf8]
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5688824) /* 0x56cdf8 */))));
    // 00510b23  d1e2                   -shl edx, 1
    cpu.edx <<= 1 /*0x1*/ % 32;
    // 00510b25  8b3de4cd5600           -mov edi, dword ptr [0x56cde4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */);
    // 00510b2b  01d6                   -add esi, edx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edx));
    // 00510b2d  8b2deccd5600           -mov ebp, dword ptr [0x56cdec]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(5688812) /* 0x56cdec */);
    // 00510b33  8d0411                 -lea eax, [ecx + edx]
    cpu.eax = x86::reg32(cpu.ecx + cpu.edx * 1);
    // 00510b36  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00510b38  8d1433                 -lea edx, [ebx + esi]
    cpu.edx = x86::reg32(cpu.ebx + cpu.esi * 1);
    // 00510b3b  29f3                   -sub ebx, esi
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 00510b3d  8d3429                 -lea esi, [ecx + ebp]
    cpu.esi = x86::reg32(cpu.ecx + cpu.ebp * 1);
    // 00510b40  29e9                   -sub ecx, ebp
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510b42  8b2de8cd5600           -mov ebp, dword ptr [0x56cde8]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(5688808) /* 0x56cde8 */);
    // 00510b48  897748                 -mov dword ptr [edi + 0x48], esi
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(72) /* 0x48 */) = cpu.esi;
    // 00510b4b  8b35f0cd5600           -mov esi, dword ptr [0x56cdf0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5688816) /* 0x56cdf0 */);
    // 00510b51  898fb4000000           -mov dword ptr [edi + 0xb4], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(180) /* 0xb4 */) = cpu.ecx;
    // 00510b57  8d0c2b                 -lea ecx, [ebx + ebp]
    cpu.ecx = x86::reg32(cpu.ebx + cpu.ebp * 1);
    // 00510b5a  29eb                   -sub ebx, ebp
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510b5c  8b2df4cd5600           -mov ebp, dword ptr [0x56cdf4]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(5688820) /* 0x56cdf4 */);
    // 00510b62  894f6c                 -mov dword ptr [edi + 0x6c], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(108) /* 0x6c */) = cpu.ecx;
    // 00510b65  899f90000000           -mov dword ptr [edi + 0x90], ebx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(144) /* 0x90 */) = cpu.ebx;
    // 00510b6b  8d1c30                 -lea ebx, [eax + esi]
    cpu.ebx = x86::reg32(cpu.eax + cpu.esi * 1);
    // 00510b6e  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 00510b70  8d0c2a                 -lea ecx, [edx + ebp]
    cpu.ecx = x86::reg32(cpu.edx + cpu.ebp * 1);
    // 00510b73  29ea                   -sub edx, ebp
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510b75  890f                   -mov dword ptr [edi], ecx
    app->getMemory<x86::reg32>(cpu.edi) = cpu.ecx;
    // 00510b77  895f24                 -mov dword ptr [edi + 0x24], ebx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */) = cpu.ebx;
    // 00510b7a  8987d8000000           -mov dword ptr [edi + 0xd8], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(216) /* 0xd8 */) = cpu.eax;
    // 00510b80  8997fc000000           -mov dword ptr [edi + 0xfc], edx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(252) /* 0xfc */) = cpu.edx;
    // 00510b86  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00510b87:
    // 00510b87  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00510b89  8b3de4cd5600           -mov edi, dword ptr [0x56cde4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */);
    // 00510b8f  8907                   -mov dword ptr [edi], eax
    app->getMemory<x86::reg32>(cpu.edi) = cpu.eax;
    // 00510b91  894724                 -mov dword ptr [edi + 0x24], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */) = cpu.eax;
    // 00510b94  894748                 -mov dword ptr [edi + 0x48], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(72) /* 0x48 */) = cpu.eax;
    // 00510b97  89476c                 -mov dword ptr [edi + 0x6c], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(108) /* 0x6c */) = cpu.eax;
    // 00510b9a  898790000000           -mov dword ptr [edi + 0x90], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(144) /* 0x90 */) = cpu.eax;
    // 00510ba0  8987b4000000           -mov dword ptr [edi + 0xb4], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(180) /* 0xb4 */) = cpu.eax;
    // 00510ba6  8987d8000000           -mov dword ptr [edi + 0xd8], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(216) /* 0xd8 */) = cpu.eax;
    // 00510bac  8987fc000000           -mov dword ptr [edi + 0xfc], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(252) /* 0xfc */) = cpu.eax;
    // 00510bb2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_510bb3(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2;
    // 00510bb3  8b5e14                 -mov ebx, dword ptr [esi + 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 00510bb6  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 00510bb9  8b4e04                 -mov ecx, dword ptr [esi + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00510bbc  8b561c                 -mov edx, dword ptr [esi + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(28) /* 0x1c */);
    // 00510bbf  8d3c03                 -lea edi, [ebx + eax]
    cpu.edi = x86::reg32(cpu.ebx + cpu.eax * 1);
    // 00510bc2  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00510bc4  8d0411                 -lea eax, [ecx + edx]
    cpu.eax = x86::reg32(cpu.ecx + cpu.edx * 1);
    // 00510bc7  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00510bc9  891de8cd5600           -mov dword ptr [0x56cde8], ebx
    app->getMemory<x86::reg32>(x86::reg32(5688808) /* 0x56cde8 */) = cpu.ebx;
    // 00510bcf  890deccd5600           -mov dword ptr [0x56cdec], ecx
    app->getMemory<x86::reg32>(x86::reg32(5688812) /* 0x56cdec */) = cpu.ecx;
    // 00510bd5  8d2c38                 -lea ebp, [eax + edi]
    cpu.ebp = x86::reg32(cpu.eax + cpu.edi * 1);
    // 00510bd8  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00510bda  db05e8cd5600           -fild dword ptr [0x56cde8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5688808) /* 0x56cde8 */)));
    // 00510be0  db05eccd5600           -fild dword ptr [0x56cdec]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5688812) /* 0x56cdec */)));
    // 00510be6  d9c0                   -fld st(0)
    fp2 = x86::Float(fp1);
    // 00510be8  d80d00ce5600           -fmul dword ptr [0x56ce00]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(x86::reg32(5688832) /* 0x56ce00 */)));
    // 00510bee  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00510bf0  dcc1                   -fadd st(1), st(0)
    fp1 = fp::add(fp1, x86::Float(fp2));
    // 00510bf2  d80dfccd5600           -fmul dword ptr [0x56cdfc]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(x86::reg32(5688828) /* 0x56cdfc */)));
    // 00510bf8  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00510bfa  d80d04ce5600           -fmul dword ptr [0x56ce04]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(x86::reg32(5688836) /* 0x56ce04 */)));
    // 00510c00  f72df8cd5600           -imul dword ptr [0x56cdf8]
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5688824) /* 0x56cdf8 */))));
    // 00510c06  dcc1                   -fadd st(1), st(0)
    fp1 = fp::add(fp1, x86::Float(fp2));
    // 00510c08  deea                   -fsubp st(2)
    fp0 = fp::sub(fp0, fp2);
    // 00510c0a  d80508ce5600           -fadd dword ptr [0x56ce08]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(x86::reg32(5688840) /* 0x56ce08 */)));
    // 00510c10  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00510c12  d80508ce5600           -fadd dword ptr [0x56ce08]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(x86::reg32(5688840) /* 0x56ce08 */)));
    // 00510c18  8b4608                 -mov eax, dword ptr [esi + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 00510c1b  8b5e18                 -mov ebx, dword ptr [esi + 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 00510c1e  d1e2                   -shl edx, 1
    cpu.edx <<= 1 /*0x1*/ % 32;
    // 00510c20  dd1df0cd5600           -fstp qword ptr [0x56cdf0]
    app->getMemory<double>(x86::reg32(5688816) /* 0x56cdf0 */) = double(fp1);
    // 00510c26  dd1de8cd5600           -fstp qword ptr [0x56cde8]
    app->getMemory<double>(x86::reg32(5688808) /* 0x56cde8 */) = double(fp0);
    // 00510c2c  8b0de8cd5600           -mov ecx, dword ptr [0x56cde8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5688808) /* 0x56cde8 */);
    // 00510c32  8b3df0cd5600           -mov edi, dword ptr [0x56cdf0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5688816) /* 0x56cdf0 */);
    // 00510c38  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 00510c3a  01fa                   -add edx, edi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edi));
    // 00510c3c  01ef                   -add edi, ebp
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ebp));
    // 00510c3e  890deccd5600           -mov dword ptr [0x56cdec], ecx
    app->getMemory<x86::reg32>(x86::reg32(5688812) /* 0x56cdec */) = cpu.ecx;
    // 00510c44  8915f0cd5600           -mov dword ptr [0x56cdf0], edx
    app->getMemory<x86::reg32>(x86::reg32(5688816) /* 0x56cdf0 */) = cpu.edx;
    // 00510c4a  893df4cd5600           -mov dword ptr [0x56cdf4], edi
    app->getMemory<x86::reg32>(x86::reg32(5688820) /* 0x56cdf4 */) = cpu.edi;
    // 00510c50  8b0e                   -mov ecx, dword ptr [esi]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi);
    // 00510c52  8b5610                 -mov edx, dword ptr [esi + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 00510c55  8d3418                 -lea esi, [eax + ebx]
    cpu.esi = x86::reg32(cpu.eax + cpu.ebx * 1);
    // 00510c58  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00510c5a  8d1c11                 -lea ebx, [ecx + edx]
    cpu.ebx = x86::reg32(cpu.ecx + cpu.edx * 1);
    // 00510c5d  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00510c5f  f72df8cd5600           -imul dword ptr [0x56cdf8]
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5688824) /* 0x56cdf8 */))));
    // 00510c65  d1e2                   -shl edx, 1
    cpu.edx <<= 1 /*0x1*/ % 32;
    // 00510c67  8b3de4cd5600           -mov edi, dword ptr [0x56cde4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */);
    // 00510c6d  01d6                   -add esi, edx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edx));
    // 00510c6f  8b2deccd5600           -mov ebp, dword ptr [0x56cdec]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(5688812) /* 0x56cdec */);
    // 00510c75  8d0411                 -lea eax, [ecx + edx]
    cpu.eax = x86::reg32(cpu.ecx + cpu.edx * 1);
    // 00510c78  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00510c7a  8d1433                 -lea edx, [ebx + esi]
    cpu.edx = x86::reg32(cpu.ebx + cpu.esi * 1);
    // 00510c7d  29f3                   -sub ebx, esi
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 00510c7f  8d3429                 -lea esi, [ecx + ebp]
    cpu.esi = x86::reg32(cpu.ecx + cpu.ebp * 1);
    // 00510c82  29e9                   -sub ecx, ebp
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510c84  8b2de8cd5600           -mov ebp, dword ptr [0x56cde8]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(5688808) /* 0x56cde8 */);
    // 00510c8a  897708                 -mov dword ptr [edi + 8], esi
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8) /* 0x8 */) = cpu.esi;
    // 00510c8d  8b35f0cd5600           -mov esi, dword ptr [0x56cdf0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5688816) /* 0x56cdf0 */);
    // 00510c93  894f14                 -mov dword ptr [edi + 0x14], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(20) /* 0x14 */) = cpu.ecx;
    // 00510c96  8d0c2b                 -lea ecx, [ebx + ebp]
    cpu.ecx = x86::reg32(cpu.ebx + cpu.ebp * 1);
    // 00510c99  29eb                   -sub ebx, ebp
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510c9b  8b2df4cd5600           -mov ebp, dword ptr [0x56cdf4]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(5688820) /* 0x56cdf4 */);
    // 00510ca1  894f0c                 -mov dword ptr [edi + 0xc], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(12) /* 0xc */) = cpu.ecx;
    // 00510ca4  895f10                 -mov dword ptr [edi + 0x10], ebx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */) = cpu.ebx;
    // 00510ca7  8d1c30                 -lea ebx, [eax + esi]
    cpu.ebx = x86::reg32(cpu.eax + cpu.esi * 1);
    // 00510caa  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 00510cac  8d0c2a                 -lea ecx, [edx + ebp]
    cpu.ecx = x86::reg32(cpu.edx + cpu.ebp * 1);
    // 00510caf  29ea                   -sub edx, ebp
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510cb1  890f                   -mov dword ptr [edi], ecx
    app->getMemory<x86::reg32>(cpu.edi) = cpu.ecx;
    // 00510cb3  895f04                 -mov dword ptr [edi + 4], ebx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */) = cpu.ebx;
    // 00510cb6  894718                 -mov dword ptr [edi + 0x18], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 00510cb9  89571c                 -mov dword ptr [edi + 0x1c], edx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */) = cpu.edx;
    // 00510cbc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_510cbd(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00510cbd  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00510cbe  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00510cbf  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00510cc0  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00510cc1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00510cc2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00510cc3  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00510cc6  8915e0cd5600           -mov dword ptr [0x56cde0], edx
    app->getMemory<x86::reg32>(x86::reg32(5688800) /* 0x56cde0 */) = cpu.edx;
    // 00510ccc  be2ccf5600             -mov esi, 0x56cf2c
    cpu.esi = 5689132 /*0x56cf2c*/;
    // 00510cd1  c705e4cd56000cce5600   -mov dword ptr [0x56cde4], 0x56ce0c
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = 5688844 /*0x56ce0c*/;
    // 00510cdb  e870fdffff             -call 0x510a50
    cpu.esp -= 4;
    sub_510a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510ce0  be4ccf5600             -mov esi, 0x56cf4c
    cpu.esi = 5689164 /*0x56cf4c*/;
    // 00510ce5  c705e4cd560010ce5600   -mov dword ptr [0x56cde4], 0x56ce10
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = 5688848 /*0x56ce10*/;
    // 00510cef  e85cfdffff             -call 0x510a50
    cpu.esp -= 4;
    sub_510a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510cf4  be6ccf5600             -mov esi, 0x56cf6c
    cpu.esi = 5689196 /*0x56cf6c*/;
    // 00510cf9  c705e4cd560014ce5600   -mov dword ptr [0x56cde4], 0x56ce14
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = 5688852 /*0x56ce14*/;
    // 00510d03  e848fdffff             -call 0x510a50
    cpu.esp -= 4;
    sub_510a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510d08  be8ccf5600             -mov esi, 0x56cf8c
    cpu.esi = 5689228 /*0x56cf8c*/;
    // 00510d0d  c705e4cd560018ce5600   -mov dword ptr [0x56cde4], 0x56ce18
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = 5688856 /*0x56ce18*/;
    // 00510d17  e834fdffff             -call 0x510a50
    cpu.esp -= 4;
    sub_510a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510d1c  beaccf5600             -mov esi, 0x56cfac
    cpu.esi = 5689260 /*0x56cfac*/;
    // 00510d21  c705e4cd56001cce5600   -mov dword ptr [0x56cde4], 0x56ce1c
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = 5688860 /*0x56ce1c*/;
    // 00510d2b  e820fdffff             -call 0x510a50
    cpu.esp -= 4;
    sub_510a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510d30  becccf5600             -mov esi, 0x56cfcc
    cpu.esi = 5689292 /*0x56cfcc*/;
    // 00510d35  c705e4cd560020ce5600   -mov dword ptr [0x56cde4], 0x56ce20
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = 5688864 /*0x56ce20*/;
    // 00510d3f  e80cfdffff             -call 0x510a50
    cpu.esp -= 4;
    sub_510a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510d44  beeccf5600             -mov esi, 0x56cfec
    cpu.esi = 5689324 /*0x56cfec*/;
    // 00510d49  c705e4cd560024ce5600   -mov dword ptr [0x56cde4], 0x56ce24
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = 5688868 /*0x56ce24*/;
    // 00510d53  e8f8fcffff             -call 0x510a50
    cpu.esp -= 4;
    sub_510a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510d58  be0cd05600             -mov esi, 0x56d00c
    cpu.esi = 5689356 /*0x56d00c*/;
    // 00510d5d  c705e4cd560028ce5600   -mov dword ptr [0x56cde4], 0x56ce28
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = 5688872 /*0x56ce28*/;
    // 00510d67  e8e4fcffff             -call 0x510a50
    cpu.esp -= 4;
    sub_510a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510d6c  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510d6d  be0cce5600             -mov esi, 0x56ce0c
    cpu.esi = 5688844 /*0x56ce0c*/;
    // 00510d72  a3e4cd5600             -mov dword ptr [0x56cde4], eax
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = cpu.eax;
    // 00510d77  e837feffff             -call 0x510bb3
    cpu.esp -= 4;
    sub_510bb3(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510d7c  a1e4cd5600             -mov eax, dword ptr [0x56cde4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */);
    // 00510d81  8b15e0cd5600           -mov edx, dword ptr [0x56cde0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5688800) /* 0x56cde0 */);
    // 00510d87  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00510d89  be30ce5600             -mov esi, 0x56ce30
    cpu.esi = 5688880 /*0x56ce30*/;
    // 00510d8e  a3e4cd5600             -mov dword ptr [0x56cde4], eax
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = cpu.eax;
    // 00510d93  e81bfeffff             -call 0x510bb3
    cpu.esp -= 4;
    sub_510bb3(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510d98  a1e4cd5600             -mov eax, dword ptr [0x56cde4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */);
    // 00510d9d  8b15e0cd5600           -mov edx, dword ptr [0x56cde0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5688800) /* 0x56cde0 */);
    // 00510da3  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00510da5  be54ce5600             -mov esi, 0x56ce54
    cpu.esi = 5688916 /*0x56ce54*/;
    // 00510daa  a3e4cd5600             -mov dword ptr [0x56cde4], eax
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = cpu.eax;
    // 00510daf  e8fffdffff             -call 0x510bb3
    cpu.esp -= 4;
    sub_510bb3(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510db4  a1e4cd5600             -mov eax, dword ptr [0x56cde4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */);
    // 00510db9  8b15e0cd5600           -mov edx, dword ptr [0x56cde0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5688800) /* 0x56cde0 */);
    // 00510dbf  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00510dc1  be78ce5600             -mov esi, 0x56ce78
    cpu.esi = 5688952 /*0x56ce78*/;
    // 00510dc6  a3e4cd5600             -mov dword ptr [0x56cde4], eax
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = cpu.eax;
    // 00510dcb  e8e3fdffff             -call 0x510bb3
    cpu.esp -= 4;
    sub_510bb3(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510dd0  a1e4cd5600             -mov eax, dword ptr [0x56cde4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */);
    // 00510dd5  8b15e0cd5600           -mov edx, dword ptr [0x56cde0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5688800) /* 0x56cde0 */);
    // 00510ddb  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00510ddd  be9cce5600             -mov esi, 0x56ce9c
    cpu.esi = 5688988 /*0x56ce9c*/;
    // 00510de2  a3e4cd5600             -mov dword ptr [0x56cde4], eax
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = cpu.eax;
    // 00510de7  e8c7fdffff             -call 0x510bb3
    cpu.esp -= 4;
    sub_510bb3(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510dec  a1e4cd5600             -mov eax, dword ptr [0x56cde4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */);
    // 00510df1  8b15e0cd5600           -mov edx, dword ptr [0x56cde0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5688800) /* 0x56cde0 */);
    // 00510df7  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00510df9  bec0ce5600             -mov esi, 0x56cec0
    cpu.esi = 5689024 /*0x56cec0*/;
    // 00510dfe  a3e4cd5600             -mov dword ptr [0x56cde4], eax
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = cpu.eax;
    // 00510e03  e8abfdffff             -call 0x510bb3
    cpu.esp -= 4;
    sub_510bb3(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510e08  a1e4cd5600             -mov eax, dword ptr [0x56cde4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */);
    // 00510e0d  8b15e0cd5600           -mov edx, dword ptr [0x56cde0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5688800) /* 0x56cde0 */);
    // 00510e13  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00510e15  bee4ce5600             -mov esi, 0x56cee4
    cpu.esi = 5689060 /*0x56cee4*/;
    // 00510e1a  a3e4cd5600             -mov dword ptr [0x56cde4], eax
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = cpu.eax;
    // 00510e1f  e88ffdffff             -call 0x510bb3
    cpu.esp -= 4;
    sub_510bb3(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510e24  a1e4cd5600             -mov eax, dword ptr [0x56cde4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */);
    // 00510e29  8b15e0cd5600           -mov edx, dword ptr [0x56cde0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5688800) /* 0x56cde0 */);
    // 00510e2f  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00510e31  be08cf5600             -mov esi, 0x56cf08
    cpu.esi = 5689096 /*0x56cf08*/;
    // 00510e36  a3e4cd5600             -mov dword ptr [0x56cde4], eax
    app->getMemory<x86::reg32>(x86::reg32(5688804) /* 0x56cde4 */) = cpu.eax;
    // 00510e3b  e873fdffff             -call 0x510bb3
    cpu.esp -= 4;
    sub_510bb3(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00510e40  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510e41  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510e42  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510e43  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510e44  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510e45  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 */
void Application::sub_510e48(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00510e48  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00510e49  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00510e4a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00510e4b  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00510e4c  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00510e4d  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00510e4e  8b3548259f00           -mov esi, dword ptr [0x9f2548]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10429768) /* 0x9f2548 */);
    // 00510e54  8b3d3c259f00           -mov edi, dword ptr [0x9f253c]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10429756) /* 0x9f253c */);
    // 00510e5a  8b2d44259f00           -mov ebp, dword ptr [0x9f2544]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(10429764) /* 0x9f2544 */);
    // 00510e60  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00510e62  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 00510e65  bb30cf5600             -mov ebx, 0x56cf30
    cpu.ebx = 5689136 /*0x56cf30*/;
    // 00510e6a  f72d380c9f00           -imul dword ptr [0x9f0c38]
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(10423352) /* 0x9f0c38 */))));
    // 00510e70  a32ccf5600             -mov dword ptr [0x56cf2c], eax
    app->getMemory<x86::reg32>(x86::reg32(5689132) /* 0x56cf2c */) = cpu.eax;
    // 00510e75  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00510e77  c1e708                 -shl edi, 8
    cpu.edi <<= 8 /*0x8*/ % 32;
    // 00510e7a  83ed08                 -sub ebp, 8
    (cpu.ebp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00510e7d  83fd10                 +cmp ebp, 0x10
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00510e80  7d14                   -jge 0x510e96
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00510e96;
    }
    // 00510e82  668b06                 -mov ax, word ptr [esi]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi);
    // 00510e85  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 00510e8a  29e9                   -sub ecx, ebp
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510e8c  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00510e8f  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 00510e91  09c7                   -or edi, eax
    cpu.edi |= x86::reg32(x86::sreg32(cpu.eax));
    // 00510e93  83c510                 -add ebp, 0x10
    (cpu.ebp) += x86::reg32(x86::sreg32(16 /*0x10*/));
L_0x00510e96:
    // 00510e96  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00510e98  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
L_0x00510e9d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00510e9d  8903                   -mov dword ptr [ebx], eax
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.eax;
    // 00510e9f  894304                 -mov dword ptr [ebx + 4], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 00510ea2  894308                 -mov dword ptr [ebx + 8], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 00510ea5  89430c                 -mov dword ptr [ebx + 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 00510ea8  894310                 -mov dword ptr [ebx + 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 00510eab  894314                 -mov dword ptr [ebx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 00510eae  894318                 -mov dword ptr [ebx + 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 00510eb1  89431c                 -mov dword ptr [ebx + 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(28) /* 0x1c */) = cpu.eax;
    // 00510eb4  894320                 -mov dword ptr [ebx + 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(32) /* 0x20 */) = cpu.eax;
    // 00510eb7  83c324                 +add ebx, 0x24
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(36 /*0x24*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00510eba  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 00510ebb  75e0                   -jne 0x510e9d
    if (!cpu.flags.zf)
    {
        goto L_0x00510e9d;
    }
    // 00510ebd  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
L_0x00510ec2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00510ec2  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00510ec4  c1e817                 -shr eax, 0x17
    cpu.eax >>= 23 /*0x17*/ % 32;
    // 00510ec7  8b148538199f00         -mov edx, dword ptr [eax*4 + 0x9f1938]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10426680) /* 0x9f1938 */ + cpu.eax * 4);
    // 00510ece  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00510ed0  80fa09                 +cmp dl, 9
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(9 /*0x9*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00510ed3  0f8eb9000000           -jle 0x510f92
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00510f92;
    }
    // 00510ed9  80fa20                 +cmp dl, 0x20
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(32 /*0x20*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00510edc  7362                   -jae 0x510f40
    if (!cpu.flags.cf)
    {
        goto L_0x00510f40;
    }
    // 00510ede  80fa10                 +cmp dl, 0x10
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
    // 00510ee1  7330                   -jae 0x510f13
    if (!cpu.flags.cf)
    {
        goto L_0x00510f13;
    }
    // 00510ee3  c1e709                 -shl edi, 9
    cpu.edi <<= 9 /*0x9*/ % 32;
    // 00510ee6  83ed09                 -sub ebp, 9
    (cpu.ebp) -= x86::reg32(x86::sreg32(9 /*0x9*/));
    // 00510ee9  83fd10                 +cmp ebp, 0x10
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00510eec  7d14                   -jge 0x510f02
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00510f02;
    }
    // 00510eee  668b06                 -mov ax, word ptr [esi]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi);
    // 00510ef1  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 00510ef6  29e9                   -sub ecx, ebp
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510ef8  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00510efb  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 00510efd  09c7                   -or edi, eax
    cpu.edi |= x86::reg32(x86::sreg32(cpu.eax));
    // 00510eff  83c510                 -add ebp, 0x10
    (cpu.ebp) += x86::reg32(x86::sreg32(16 /*0x10*/));
L_0x00510f02:
    // 00510f02  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00510f04  c1e818                 +shr eax, 0x18
    {
        x86::reg8 tmp = 24 /*0x18*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 00510f07  8b148538119f00         -mov edx, dword ptr [eax*4 + 0x9f1138]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10424632) /* 0x9f1138 */ + cpu.eax * 4);
    // 00510f0e  e97f000000             -jmp 0x510f92
    goto L_0x00510f92;
L_0x00510f13:
    // 00510f13  c1e706                 -shl edi, 6
    cpu.edi <<= 6 /*0x6*/ % 32;
    // 00510f16  83ed06                 -sub ebp, 6
    (cpu.ebp) -= x86::reg32(x86::sreg32(6 /*0x6*/));
    // 00510f19  83fd10                 +cmp ebp, 0x10
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00510f1c  7d14                   -jge 0x510f32
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00510f32;
    }
    // 00510f1e  668b06                 -mov ax, word ptr [esi]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi);
    // 00510f21  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 00510f26  29e9                   -sub ecx, ebp
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510f28  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00510f2b  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 00510f2d  09c7                   -or edi, eax
    cpu.edi |= x86::reg32(x86::sreg32(cpu.eax));
    // 00510f2f  83c510                 -add ebp, 0x10
    (cpu.ebp) += x86::reg32(x86::sreg32(16 /*0x10*/));
L_0x00510f32:
    // 00510f32  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00510f34  c1e818                 +shr eax, 0x18
    {
        x86::reg8 tmp = 24 /*0x18*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 00510f37  8b148538159f00         -mov edx, dword ptr [eax*4 + 0x9f1538]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10425656) /* 0x9f1538 */ + cpu.eax * 4);
    // 00510f3e  eb52                   -jmp 0x510f92
    goto L_0x00510f92;
L_0x00510f40:
    // 00510f40  80fa30                 +cmp dl, 0x30
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(48 /*0x30*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00510f43  732c                   -jae 0x510f71
    if (!cpu.flags.cf)
    {
        goto L_0x00510f71;
    }
    // 00510f45  c1e706                 -shl edi, 6
    cpu.edi <<= 6 /*0x6*/ % 32;
    // 00510f48  83ed06                 -sub ebp, 6
    (cpu.ebp) -= x86::reg32(x86::sreg32(6 /*0x6*/));
    // 00510f4b  83fd10                 +cmp ebp, 0x10
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00510f4e  7d14                   -jge 0x510f64
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00510f64;
    }
    // 00510f50  668b06                 -mov ax, word ptr [esi]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi);
    // 00510f53  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 00510f58  29e9                   -sub ecx, ebp
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510f5a  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00510f5d  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 00510f5f  09c7                   -or edi, eax
    cpu.edi |= x86::reg32(x86::sreg32(cpu.eax));
    // 00510f61  83c510                 -add ebp, 0x10
    (cpu.ebp) += x86::reg32(x86::sreg32(16 /*0x10*/));
L_0x00510f64:
    // 00510f64  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 00510f66  81e20000ffff           -and edx, 0xffff0000
    cpu.edx &= x86::reg32(x86::sreg32(4294901760 /*0xffff0000*/));
    // 00510f6c  83ca10                 +or edx, 0x10
    cpu.clear_co();
    cpu.set_szp((cpu.edx |= x86::reg32(x86::sreg32(16 /*0x10*/))));
    // 00510f6f  eb21                   -jmp 0x510f92
    goto L_0x00510f92;
L_0x00510f71:
    // 00510f71  c1e702                 -shl edi, 2
    cpu.edi <<= 2 /*0x2*/ % 32;
    // 00510f74  83ed02                 -sub ebp, 2
    (cpu.ebp) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00510f77  83fd10                 +cmp ebp, 0x10
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00510f7a  7d61                   -jge 0x510fdd
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00510fdd;
    }
    // 00510f7c  668b06                 -mov ax, word ptr [esi]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi);
    // 00510f7f  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 00510f84  29e9                   -sub ecx, ebp
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510f86  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00510f89  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 00510f8b  09c7                   -or edi, eax
    cpu.edi |= x86::reg32(x86::sreg32(cpu.eax));
    // 00510f8d  83c510                 +add ebp, 0x10
    {
        x86::reg32& tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00510f90  eb4b                   -jmp 0x510fdd
    goto L_0x00510fdd;
L_0x00510f92:
    // 00510f92  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00510f94  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510f96  c1e810                 -shr eax, 0x10
    cpu.eax >>= 16 /*0x10*/ % 32;
    // 00510f99  88d1                   -mov cl, dl
    cpu.cl = cpu.dl;
    // 00510f9b  83e03f                 -and eax, 0x3f
    cpu.eax &= x86::reg32(x86::sreg32(63 /*0x3f*/));
    // 00510f9e  29cd                   -sub ebp, ecx
    (cpu.ebp) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00510fa0  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00510fa2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00510fa4  d3e7                   -shl edi, cl
    cpu.edi <<= cpu.cl % 32;
    // 00510fa6  83fd10                 +cmp ebp, 0x10
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00510fa9  7d14                   -jge 0x510fbf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00510fbf;
    }
    // 00510fab  668b06                 -mov ax, word ptr [esi]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi);
    // 00510fae  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 00510fb3  29e9                   -sub ecx, ebp
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 00510fb5  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00510fb8  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 00510fba  09c7                   -or edi, eax
    cpu.edi |= x86::reg32(x86::sreg32(cpu.eax));
    // 00510fbc  83c510                 -add ebp, 0x10
    (cpu.ebp) += x86::reg32(x86::sreg32(16 /*0x10*/));
L_0x00510fbf:
    // 00510fbf  8b0c9dcc885600         -mov ecx, dword ptr [ebx*4 + 0x5688cc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5671116) /* 0x5688cc */ + cpu.ebx * 4);
    // 00510fc6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00510fc8  c1f816                 -sar eax, 0x16
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (22 /*0x16*/ % 32));
    // 00510fcb  43                     -inc ebx
    (cpu.ebx)++;
    // 00510fcc  f7a9380c9f00           +imul dword ptr [ecx + 0x9f0c38]
    {
        cpu.edx_eax = x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(10423352) /* 0x9f0c38 */)));
        cpu.flags.of = cpu.flags.cf = (static_cast<x86::sreg64>(cpu.edx_eax) != x86::sreg64(static_cast<x86::sreg32>(cpu.eax)));
    }
    // 00510fd2  89812ccf5600           -mov dword ptr [ecx + 0x56cf2c], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5689132) /* 0x56cf2c */) = cpu.eax;
    // 00510fd8  e9e5feffff             -jmp 0x510ec2
    goto L_0x00510ec2;
L_0x00510fdd:
    // 00510fdd  893548259f00           -mov dword ptr [0x9f2548], esi
    app->getMemory<x86::reg32>(x86::reg32(10429768) /* 0x9f2548 */) = cpu.esi;
    // 00510fe3  893d3c259f00           -mov dword ptr [0x9f253c], edi
    app->getMemory<x86::reg32>(x86::reg32(10429756) /* 0x9f253c */) = cpu.edi;
    // 00510fe9  892d44259f00           -mov dword ptr [0x9f2544], ebp
    app->getMemory<x86::reg32>(x86::reg32(10429764) /* 0x9f2544 */) = cpu.ebp;
    // 00510fef  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00510ff1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510ff2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510ff3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510ff4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510ff5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510ff6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00510ff7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_510ff8(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00510ff8  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00510ff9  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00510ffa  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00510ffb  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00510ffc  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00510ffe  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00511000  89dd                   -mov ebp, ebx
    cpu.ebp = cpu.ebx;
    // 00511002  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00511004  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00511006  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00511008  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051100a  8a4602                 -mov al, byte ptr [esi + 2]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 0051100d  8a5f02                 -mov bl, byte ptr [edi + 2]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(2) /* 0x2 */);
    // 00511010  8a4e06                 -mov cl, byte ptr [esi + 6]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 00511013  8a9702010000           -mov dl, byte ptr [edi + 0x102]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(258) /* 0x102 */);
    // 00511019  8a80380d9f00           -mov al, byte ptr [eax + 0x9f0d38]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10423608) /* 0x9f0d38 */);
    // 0051101f  8a9b380d9f00           -mov bl, byte ptr [ebx + 0x9f0d38]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511025  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 00511028  8a89380d9f00           -mov cl, byte ptr [ecx + 0x9f0d38]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 0051102e  c1e308                 -shl ebx, 8
    cpu.ebx <<= 8 /*0x8*/ % 32;
    // 00511031  8a92380d9f00           -mov dl, byte ptr [edx + 0x9f0d38]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511037  09c8                   -or eax, ecx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ecx));
    // 00511039  09d3                   -or ebx, edx
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.edx));
    // 0051103b  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 0051103e  09d8                   -or eax, ebx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ebx));
    // 00511040  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00511042  894500                 -mov dword ptr [ebp], eax
    app->getMemory<x86::reg32>(cpu.ebp) = cpu.eax;
    // 00511045  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00511047  8a460a                 -mov al, byte ptr [esi + 0xa]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(10) /* 0xa */);
    // 0051104a  8a5f06                 -mov bl, byte ptr [edi + 6]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(6) /* 0x6 */);
    // 0051104d  8a4e0e                 -mov cl, byte ptr [esi + 0xe]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(14) /* 0xe */);
    // 00511050  8a9706010000           -mov dl, byte ptr [edi + 0x106]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(262) /* 0x106 */);
    // 00511056  8a80380d9f00           -mov al, byte ptr [eax + 0x9f0d38]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10423608) /* 0x9f0d38 */);
    // 0051105c  8a9b380d9f00           -mov bl, byte ptr [ebx + 0x9f0d38]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511062  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 00511065  8a89380d9f00           -mov cl, byte ptr [ecx + 0x9f0d38]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 0051106b  c1e308                 -shl ebx, 8
    cpu.ebx <<= 8 /*0x8*/ % 32;
    // 0051106e  8a92380d9f00           -mov dl, byte ptr [edx + 0x9f0d38]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511074  09c8                   -or eax, ecx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ecx));
    // 00511076  09d3                   -or ebx, edx
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.edx));
    // 00511078  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 0051107b  09d8                   -or eax, ebx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051107d  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051107f  894504                 -mov dword ptr [ebp + 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 00511082  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00511084  8a4612                 -mov al, byte ptr [esi + 0x12]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(18) /* 0x12 */);
    // 00511087  8a5f0a                 -mov bl, byte ptr [edi + 0xa]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(10) /* 0xa */);
    // 0051108a  8a4e16                 -mov cl, byte ptr [esi + 0x16]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(22) /* 0x16 */);
    // 0051108d  8a970a010000           -mov dl, byte ptr [edi + 0x10a]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(266) /* 0x10a */);
    // 00511093  8a80380d9f00           -mov al, byte ptr [eax + 0x9f0d38]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511099  8a9b380d9f00           -mov bl, byte ptr [ebx + 0x9f0d38]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 0051109f  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 005110a2  8a89380d9f00           -mov cl, byte ptr [ecx + 0x9f0d38]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 005110a8  c1e308                 -shl ebx, 8
    cpu.ebx <<= 8 /*0x8*/ % 32;
    // 005110ab  8a92380d9f00           -mov dl, byte ptr [edx + 0x9f0d38]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 005110b1  09c8                   -or eax, ecx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ecx));
    // 005110b3  09d3                   -or ebx, edx
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.edx));
    // 005110b5  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 005110b8  09d8                   -or eax, ebx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ebx));
    // 005110ba  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 005110bc  894508                 -mov dword ptr [ebp + 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 005110bf  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005110c1  8a461a                 -mov al, byte ptr [esi + 0x1a]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(26) /* 0x1a */);
    // 005110c4  8a5f0e                 -mov bl, byte ptr [edi + 0xe]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 005110c7  8a4e1e                 -mov cl, byte ptr [esi + 0x1e]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(30) /* 0x1e */);
    // 005110ca  8a970e010000           -mov dl, byte ptr [edi + 0x10e]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(270) /* 0x10e */);
    // 005110d0  8a80380d9f00           -mov al, byte ptr [eax + 0x9f0d38]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10423608) /* 0x9f0d38 */);
    // 005110d6  8a9b380d9f00           -mov bl, byte ptr [ebx + 0x9f0d38]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 005110dc  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 005110df  8a89380d9f00           -mov cl, byte ptr [ecx + 0x9f0d38]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 005110e5  c1e308                 -shl ebx, 8
    cpu.ebx <<= 8 /*0x8*/ % 32;
    // 005110e8  8a92380d9f00           -mov dl, byte ptr [edx + 0x9f0d38]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 005110ee  09c8                   -or eax, ecx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ecx));
    // 005110f0  09d3                   -or ebx, edx
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.edx));
    // 005110f2  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 005110f5  09d8                   -or eax, ebx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ebx));
    // 005110f7  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 005110f9  89450c                 -mov dword ptr [ebp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 005110fc  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005110fe  8a4622                 -mov al, byte ptr [esi + 0x22]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(34) /* 0x22 */);
    // 00511101  8a5f12                 -mov bl, byte ptr [edi + 0x12]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(18) /* 0x12 */);
    // 00511104  8a4e26                 -mov cl, byte ptr [esi + 0x26]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(38) /* 0x26 */);
    // 00511107  8a9712010000           -mov dl, byte ptr [edi + 0x112]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(274) /* 0x112 */);
    // 0051110d  8a80380d9f00           -mov al, byte ptr [eax + 0x9f0d38]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511113  8a9b380d9f00           -mov bl, byte ptr [ebx + 0x9f0d38]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511119  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 0051111c  8a89380d9f00           -mov cl, byte ptr [ecx + 0x9f0d38]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511122  c1e308                 -shl ebx, 8
    cpu.ebx <<= 8 /*0x8*/ % 32;
    // 00511125  8a92380d9f00           -mov dl, byte ptr [edx + 0x9f0d38]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 0051112b  09c8                   -or eax, ecx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051112d  09d3                   -or ebx, edx
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.edx));
    // 0051112f  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 00511132  09d8                   -or eax, ebx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ebx));
    // 00511134  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00511136  894510                 -mov dword ptr [ebp + 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 00511139  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051113b  8a462a                 -mov al, byte ptr [esi + 0x2a]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(42) /* 0x2a */);
    // 0051113e  8a5f16                 -mov bl, byte ptr [edi + 0x16]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(22) /* 0x16 */);
    // 00511141  8a4e2e                 -mov cl, byte ptr [esi + 0x2e]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(46) /* 0x2e */);
    // 00511144  8a9716010000           -mov dl, byte ptr [edi + 0x116]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(278) /* 0x116 */);
    // 0051114a  8a80380d9f00           -mov al, byte ptr [eax + 0x9f0d38]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511150  8a9b380d9f00           -mov bl, byte ptr [ebx + 0x9f0d38]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511156  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 00511159  8a89380d9f00           -mov cl, byte ptr [ecx + 0x9f0d38]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 0051115f  c1e308                 -shl ebx, 8
    cpu.ebx <<= 8 /*0x8*/ % 32;
    // 00511162  8a92380d9f00           -mov dl, byte ptr [edx + 0x9f0d38]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511168  09c8                   -or eax, ecx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051116a  09d3                   -or ebx, edx
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.edx));
    // 0051116c  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 0051116f  09d8                   -or eax, ebx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ebx));
    // 00511171  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00511173  894514                 -mov dword ptr [ebp + 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 00511176  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00511178  8a4632                 -mov al, byte ptr [esi + 0x32]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(50) /* 0x32 */);
    // 0051117b  8a5f1a                 -mov bl, byte ptr [edi + 0x1a]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(26) /* 0x1a */);
    // 0051117e  8a4e36                 -mov cl, byte ptr [esi + 0x36]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(54) /* 0x36 */);
    // 00511181  8a971a010000           -mov dl, byte ptr [edi + 0x11a]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(282) /* 0x11a */);
    // 00511187  8a80380d9f00           -mov al, byte ptr [eax + 0x9f0d38]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10423608) /* 0x9f0d38 */);
    // 0051118d  8a9b380d9f00           -mov bl, byte ptr [ebx + 0x9f0d38]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 00511193  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 00511196  8a89380d9f00           -mov cl, byte ptr [ecx + 0x9f0d38]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 0051119c  c1e308                 -shl ebx, 8
    cpu.ebx <<= 8 /*0x8*/ % 32;
    // 0051119f  8a92380d9f00           -mov dl, byte ptr [edx + 0x9f0d38]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 005111a5  09c8                   -or eax, ecx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ecx));
    // 005111a7  09d3                   -or ebx, edx
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.edx));
    // 005111a9  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 005111ac  09d8                   -or eax, ebx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ebx));
    // 005111ae  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 005111b0  894518                 -mov dword ptr [ebp + 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 005111b3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005111b5  8a463a                 -mov al, byte ptr [esi + 0x3a]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(58) /* 0x3a */);
    // 005111b8  8a5f1e                 -mov bl, byte ptr [edi + 0x1e]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(30) /* 0x1e */);
    // 005111bb  8a4e3e                 -mov cl, byte ptr [esi + 0x3e]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(62) /* 0x3e */);
    // 005111be  8a971e010000           -mov dl, byte ptr [edi + 0x11e]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(286) /* 0x11e */);
    // 005111c4  8a80380d9f00           -mov al, byte ptr [eax + 0x9f0d38]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10423608) /* 0x9f0d38 */);
    // 005111ca  8a9b380d9f00           -mov bl, byte ptr [ebx + 0x9f0d38]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 005111d0  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 005111d3  8a89380d9f00           -mov cl, byte ptr [ecx + 0x9f0d38]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 005111d9  c1e308                 -shl ebx, 8
    cpu.ebx <<= 8 /*0x8*/ % 32;
    // 005111dc  8a92380d9f00           -mov dl, byte ptr [edx + 0x9f0d38]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(10423608) /* 0x9f0d38 */);
    // 005111e2  09c8                   -or eax, ecx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ecx));
    // 005111e4  09d3                   -or ebx, edx
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.edx));
    // 005111e6  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 005111e9  09d8                   -or eax, ebx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ebx));
    // 005111eb  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 005111ed  89451c                 -mov dword ptr [ebp + 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */) = cpu.eax;
    // 005111f0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005111f2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005111f3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005111f4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005111f5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005111f6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_511200: native, nfs3hp::dead::stub_0x511200 */

/* sub_511210: native, nfs3hp::port::playerMemory */

/* sub_511228: native, nfs3hp::port::SNDPKTPLAY_create */

/* sub_5112d0: native, nfs3hp::port::SNDPKTPLAY_start */

/* sub_5114d4: native, nfs3hp::port::SNDPKTPLAY_submit */

/* sub_5115b8: native, nfs3hp::port::SNDPKTPLAY_submitspace */

/* sub_51162c: native, nfs3hp::port::framesOutstanding */

/* sub_511680: native, nfs3hp::port::SNDPKTPLAY_framesoutstanding */

/* sub_5116ac: native, nfs3hp::dead::stub_0x5116ac */

/* sub_511718: native, nfs3hp::port::SNDPKTPLAY_purge */

/* sub_51181c: native, nfs3hp::port::SNDPKTPLAY_stop */

/* sub_5118d0: native, nfs3hp::port::SNDPKTPLAY_destroy */

/* sub_5118fc: native, nfs3hp::port::nextPacket */

/* sub_511998: native, nfs3hp::port::packetFramesPlayed */

/* sub_5119b0: native, nfs3hp::port::patchDefaults */

/* sub_5119d8: native, nfs3hp::port::iSNDpatchtohdr */

/* sub_511bf0: native, nfs3hp::port::iSNDplatformcalcdatarate */

/* sub_511ca0: native, nfs3hp::port::streamChunksAtOnce */

/* sub_511cd0: native, nfs3hp::watcom::fatalWrite */

/* sub_511d0c: native, nfs3hp::watcom::fatal_runtime_error */

/* sub_511d30: native, nfs3hp::dead::stub_0x511d30 */

/* sub_511dc0: native, nfs3hp::dead::stub_0x511dc0 */

/* align: skip 0x90 */
void Application::sub_512180(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (commEventTrace(app, cpu.sync())) /* port: native (tools/apply_native_vertices.py) */
    {
        cpu.reload();
        return;
    }
    // 00512180  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00512181  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00512182  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00512183  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00512185  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00512188  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0051218b  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 0051218e  895df4                 -mov dword ptr [ebp - 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;
    // 00512191  894df0                 -mov dword ptr [ebp - 0x10], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ecx;
    // 00512194  833dd089560000         +cmp dword ptr [0x5689d0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5671376) /* 0x5689d0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051219b  751a                   -jne 0x5121b7
    if (!cpu.flags.zf)
    {
        goto L_0x005121b7;
    }
L_0x0051219d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051219d  833dd089560000         +cmp dword ptr [0x5689d0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5671376) /* 0x5689d0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005121a4  0f84f4010000           -je 0x51239e
    if (cpu.flags.zf)
    {
        goto L_0x0051239e;
    }
    // 005121aa  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 005121af  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 005121b1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005121b2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005121b3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005121b4  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x005121b7:
    // 005121b7  8b0dcc895600           -mov ecx, dword ptr [0x5689cc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5671372) /* 0x5689cc */);
    // 005121bd  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005121be  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005121c4  8b1594b0a000           -mov edx, dword ptr [0xa0b094]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10530964) /* 0xa0b094 */);
    // 005121ca  8b1d98b0a000           -mov ebx, dword ptr [0xa0b098]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10530968) /* 0xa0b098 */);
    // 005121d0  01da                   -add edx, ebx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 005121d2  8b3590b0a000           -mov esi, dword ptr [0xa0b090]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10530960) /* 0xa0b090 */);
    // 005121d8  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 005121da  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 005121dd  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 005121df  8b35d0895600           -mov esi, dword ptr [0x5689d0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5671376) /* 0x5689d0 */);
    // 005121e5  c1e206                 -shl edx, 6
    cpu.edx <<= 6 /*0x6*/ % 32;
    // 005121e8  8b3d90b0a000           -mov edi, dword ptr [0xa0b090]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10530960) /* 0xa0b090 */);
    // 005121ee  01d6                   -add esi, edx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edx));
    // 005121f0  39fb                   +cmp ebx, edi
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
    // 005121f2  0f8565010000           -jne 0x51235d
    if (!cpu.flags.zf)
    {
        goto L_0x0051235d;
    }
    // 005121f8  8b1594b0a000           -mov edx, dword ptr [0xa0b094]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10530964) /* 0xa0b094 */);
    // 005121fe  42                     -inc edx
    (cpu.edx)++;
    // 005121ff  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00512201  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00512204  f7ff                   -idiv edi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.edi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00512206  891594b0a000           -mov dword ptr [0xa0b094], edx
    app->getMemory<x86::reg32>(x86::reg32(10530964) /* 0xa0b094 */) = cpu.edx;
L_0x0051220c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051220c  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0051220f  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00512211  0f845d010000           -je 0x512374
    if (cpu.flags.zf)
    {
        goto L_0x00512374;
    }
    // 00512217  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00512219  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051221b  49                     -dec ecx
    (cpu.ecx)--;
    // 0051221c  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051221e  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00512220  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 00512222  49                     -dec ecx
    (cpu.ecx)--;
    // 00512223  83f935                 +cmp ecx, 0x35
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(53 /*0x35*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512226  0f833e010000           -jae 0x51236a
    if (!cpu.flags.cf)
    {
        goto L_0x0051236a;
    }
    // 0051222c  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0051222e  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512230  49                     -dec ecx
    (cpu.ecx)--;
    // 00512231  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00512233  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00512235  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 00512237  49                     -dec ecx
    (cpu.ecx)--;
    // 00512238  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
L_0x0051223a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051223a  88df                   -mov bh, bl
    cpu.bh = cpu.bl;
L_0x0051223c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051223c  837df400               +cmp dword ptr [ebp - 0xc], 0
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
    // 00512240  0f8435010000           -je 0x51237b
    if (cpu.flags.zf)
    {
        goto L_0x0051237b;
    }
    // 00512246  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00512248  ba35000000             -mov edx, 0x35
    cpu.edx = 53 /*0x35*/;
    // 0051224d  88f8                   -mov al, bh
    cpu.al = cpu.bh;
    // 0051224f  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00512251  8b7df0                 -mov edi, dword ptr [ebp - 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00512254  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00512256  39fa                   +cmp edx, edi
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
    // 00512258  7e02                   -jle 0x51225c
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051225c;
    }
    // 0051225a  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
L_0x0051225c:
    // 0051225c  884608                 -mov byte ptr [esi + 8], al
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(8) /* 0x8 */) = cpu.al;
L_0x0051225f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051225f  8b4510                 -mov eax, dword ptr [ebp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 00512262  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00512264  7405                   -je 0x51226b
    if (cpu.flags.zf)
    {
        goto L_0x0051226b;
    }
    // 00512266  b808000000             -mov eax, 8
    cpu.eax = 8 /*0x8*/;
L_0x0051226b:
    // 0051226b  88460a                 -mov byte ptr [esi + 0xa], al
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(10) /* 0xa */) = cpu.al;
    // 0051226e  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 00512273  8906                   -mov dword ptr [esi], eax
    app->getMemory<x86::reg32>(cpu.esi) = cpu.eax;
    // 00512275  8a4514                 -mov al, byte ptr [ebp + 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00512278  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0051227b  884609                 -mov byte ptr [esi + 9], al
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(9) /* 0x9 */) = cpu.al;
    // 0051227e  8d460b                 -lea eax, [esi + 0xb]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(11) /* 0xb */);
    // 00512281  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00512283  742b                   -je 0x5122b0
    if (cpu.flags.zf)
    {
        goto L_0x005122b0;
    }
    // 00512285  807e0800               +cmp byte ptr [esi + 8], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(8) /* 0x8 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00512289  7425                   -je 0x5122b0
    if (cpu.flags.zf)
    {
        goto L_0x005122b0;
    }
    // 0051228b  8a760a                 -mov dh, byte ptr [esi + 0xa]
    cpu.dh = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(10) /* 0xa */);
    // 0051228e  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00512291  80ce04                 -or dh, 4
    cpu.dh |= x86::reg8(x86::sreg8(4 /*0x4*/));
    // 00512294  8a5e08                 -mov bl, byte ptr [esi + 8]
    cpu.bl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 00512297  88760a                 -mov byte ptr [esi + 0xa], dh
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(10) /* 0xa */) = cpu.dh;
    // 0051229a  30d2                   -xor dl, dl
    cpu.dl ^= x86::reg8(x86::sreg8(cpu.dl));
    // 0051229c  84db                   +test bl, bl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bl & cpu.bl));
    // 0051229e  7610                   -jbe 0x5122b0
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x005122b0;
    }
L_0x005122a0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005122a0  8a31                   -mov dh, byte ptr [ecx]
    cpu.dh = app->getMemory<x86::reg8>(cpu.ecx);
    // 005122a2  8830                   -mov byte ptr [eax], dh
    app->getMemory<x86::reg8>(cpu.eax) = cpu.dh;
    // 005122a4  fec2                   -inc dl
    (cpu.dl)++;
    // 005122a6  41                     -inc ecx
    (cpu.ecx)++;
    // 005122a7  8a7608                 -mov dh, byte ptr [esi + 8]
    cpu.dh = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 005122aa  40                     -inc eax
    (cpu.eax)++;
    // 005122ab  38f2                   +cmp dl, dh
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dh));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 005122ad  72f1                   -jb 0x5122a0
    if (cpu.flags.cf)
    {
        goto L_0x005122a0;
    }
    // 005122af  90                     -nop 
    ;
L_0x005122b0:
    // 005122b0  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 005122b3  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 005122b5  742c                   -je 0x5122e3
    if (cpu.flags.zf)
    {
        goto L_0x005122e3;
    }
    // 005122b7  84ff                   +test bh, bh
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bh & cpu.bh));
    // 005122b9  7428                   -je 0x5122e3
    if (cpu.flags.zf)
    {
        goto L_0x005122e3;
    }
    // 005122bb  804e0a01               -or byte ptr [esi + 0xa], 1
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(10) /* 0xa */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 005122bf  30db                   -xor bl, bl
    cpu.bl ^= x86::reg8(x86::sreg8(cpu.bl));
    // 005122c1  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 005122c3  84ff                   +test bh, bh
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bh & cpu.bh));
    // 005122c5  7619                   -jbe 0x5122e0
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x005122e0;
    }
L_0x005122c7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005122c7  40                     -inc eax
    (cpu.eax)++;
    // 005122c8  8a0a                   -mov cl, byte ptr [edx]
    cpu.cl = app->getMemory<x86::reg8>(cpu.edx);
    // 005122ca  42                     -inc edx
    (cpu.edx)++;
    // 005122cb  fec3                   -inc bl
    (cpu.bl)++;
    // 005122cd  8848ff                 -mov byte ptr [eax - 1], cl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-1) /* -0x1 */) = cpu.cl;
    // 005122d0  38fb                   +cmp bl, bh
    {
        x86::reg8 tmp1 = cpu.bl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.bh));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 005122d2  72f3                   -jb 0x5122c7
    if (cpu.flags.cf)
    {
        goto L_0x005122c7;
    }
    // 005122d4  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 005122da  8d9200000000           -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
L_0x005122e0:
    // 005122e0  c60000                 -mov byte ptr [eax], 0
    app->getMemory<x86::reg8>(cpu.eax) = 0 /*0x0*/;
L_0x005122e3:
    // 005122e3  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 005122e6  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 005122e8  0f8496000000           -je 0x512384
    if (cpu.flags.zf)
    {
        goto L_0x00512384;
    }
    // 005122ee  8b03                   -mov eax, dword ptr [ebx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx);
    // 005122f0  8a7e0a                 -mov bh, byte ptr [esi + 0xa]
    cpu.bh = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(10) /* 0xa */);
    // 005122f3  894604                 -mov dword ptr [esi + 4], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 005122f6  80cf02                 -or bh, 2
    cpu.bh |= x86::reg8(x86::sreg8(2 /*0x2*/));
    // 005122f9  887e0a                 -mov byte ptr [esi + 0xa], bh
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(10) /* 0xa */) = cpu.bh;
L_0x005122fc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005122fc  833dd489560000         +cmp dword ptr [0x5689d4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5671380) /* 0x5689d4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512303  7447                   -je 0x51234c
    if (cpu.flags.zf)
    {
        goto L_0x0051234c;
    }
    // 00512305  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00512308  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0051230b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051230c  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051230e  0f8478000000           -je 0x51238c
    if (cpu.flags.zf)
    {
        goto L_0x0051238c;
    }
    // 00512314  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x00512316:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00512316  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00512319  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051231a  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0051231c  7475                   -je 0x512393
    if (cpu.flags.zf)
    {
        goto L_0x00512393;
    }
    // 0051231e  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
L_0x00512320:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00512320  8b5d10                 -mov ebx, dword ptr [ebp + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 00512323  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512324  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00512326  746f                   -je 0x512397
    if (cpu.flags.zf)
    {
        goto L_0x00512397;
    }
    // 00512328  b808015500             -mov eax, 0x550108
    cpu.eax = 5570824 /*0x550108*/;
L_0x0051232d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051232d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051232e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00512330  8a4514                 -mov al, byte ptr [ebp + 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00512333  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512334  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00512336  a0d8895600             -mov al, byte ptr [0x5689d8]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5671384) /* 0x5689d8 */);
    // 0051233b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051233c  8b3e                   -mov edi, dword ptr [esi]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi);
    // 0051233e  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051233f  686c015500             -push 0x55016c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5570924 /*0x55016c*/;
    cpu.esp -= 4;
    // 00512344  e8a7e5fcff             -call 0x4e08f0
    cpu.esp -= 4;
    nfs3hp::ea::monoPrintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512349  83c420                 +add esp, 0x20
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(32 /*0x20*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
L_0x0051234c:
    // 0051234c  a1cc895600             -mov eax, dword ptr [0x5689cc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5671372) /* 0x5689cc */);
    // 00512351  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512352  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512358  e940feffff             -jmp 0x51219d
    goto L_0x0051219d;
L_0x0051235d:
    // 0051235d  8d4301                 -lea eax, [ebx + 1]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 00512360  a398b0a000             -mov dword ptr [0xa0b098], eax
    app->getMemory<x86::reg32>(x86::reg32(10530968) /* 0xa0b098 */) = cpu.eax;
    // 00512365  e9a2feffff             -jmp 0x51220c
    goto L_0x0051220c;
L_0x0051236a:
    // 0051236a  bb35000000             -mov ebx, 0x35
    cpu.ebx = 53 /*0x35*/;
    // 0051236f  e9c6feffff             -jmp 0x51223a
    goto L_0x0051223a;
L_0x00512374:
    // 00512374  30ff                   +xor bh, bh
    cpu.clear_co();
    cpu.set_szp((cpu.bh ^= x86::reg8(x86::sreg8(cpu.bh))));
    // 00512376  e9c1feffff             -jmp 0x51223c
    goto L_0x0051223c;
L_0x0051237b:
    // 0051237b  c6460800               -mov byte ptr [esi + 8], 0
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 0051237f  e9dbfeffff             -jmp 0x51225f
    goto L_0x0051225f;
L_0x00512384:
    // 00512384  895e04                 -mov dword ptr [esi + 4], ebx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = cpu.ebx;
    // 00512387  e970ffffff             -jmp 0x5122fc
    goto L_0x005122fc;
L_0x0051238c:
    // 0051238c  b868015500             -mov eax, 0x550168
    cpu.eax = 5570920 /*0x550168*/;
    // 00512391  eb83                   -jmp 0x512316
    goto L_0x00512316;
L_0x00512393:
    // 00512393  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00512395  eb89                   -jmp 0x512320
    goto L_0x00512320;
L_0x00512397:
    // 00512397  b810015500             -mov eax, 0x550110
    cpu.eax = 5570832 /*0x550110*/;
    // 0051239c  eb8f                   -jmp 0x51232d
    goto L_0x0051232d;
L_0x0051239e:
    // 0051239e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005123a0  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 005123a2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005123a3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005123a4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005123a5  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* sub_5123b0: native, nfs3hp::ea::layerSend */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_512470(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512470  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512471  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00512472  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00512473  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00512474  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00512477  89442404               -mov dword ptr [esp + 4], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 0051247b  89542408               -mov dword ptr [esp + 8], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.edx;
    // 0051247f  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 00512481  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00512483  31ed                   -xor ebp, ebp
    cpu.ebp ^= x86::reg32(x86::sreg32(cpu.ebp));
    // 00512485  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00512487  7e17                   -jle 0x5124a0
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x005124a0;
    }
    // 00512489  8b7c2408               -mov edi, dword ptr [esp + 8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051248d  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
L_0x0051248f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051248f  8b18                   -mov ebx, dword ptr [eax]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax);
    // 00512491  83c008                 -add eax, 8
    (cpu.eax) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00512494  46                     -inc esi
    (cpu.esi)++;
    // 00512495  01dd                   -add ebp, ebx
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00512497  39fe                   +cmp esi, edi
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
    // 00512499  7cf4                   -jl 0x51248f
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051248f;
    }
    // 0051249b  8d4000                 -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 0051249e  8bc9                   -mov ecx, ecx
    cpu.ecx = cpu.ecx;
L_0x005124a0:
    // 005124a0  b894015500             -mov eax, 0x550194
    cpu.eax = 5570964 /*0x550194*/;
    // 005124a5  bab4015500             -mov edx, 0x5501b4
    cpu.edx = 5570996 /*0x5501b4*/;
    // 005124aa  bb4a000000             -mov ebx, 0x4a
    cpu.ebx = 74 /*0x4a*/;
    // 005124af  a390215500             -mov dword ptr [0x552190], eax
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.eax;
    // 005124b4  891594215500           -mov dword ptr [0x552194], edx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.edx;
    // 005124ba  891d98215500           -mov dword ptr [0x552198], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebx;
    // 005124c0  b8c4015500             -mov eax, 0x5501c4
    cpu.eax = 5571012 /*0x5501c4*/;
    // 005124c5  89ea                   -mov edx, ebp
    cpu.edx = cpu.ebp;
    // 005124c7  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 005124c9  e852f1fcff             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005124ce  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 005124d1  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 005124d3  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 005124d7  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 005124d9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005124db  7e23                   -jle 0x512500
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00512500;
    }
L_0x005124dd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005124dd  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 005124df  8b19                   -mov ebx, dword ptr [ecx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx);
    // 005124e1  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 005124e4  83c108                 -add ecx, 8
    (cpu.ecx) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 005124e7  e80480fdff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005124ec  47                     -inc edi
    (cpu.edi)++;
    // 005124ed  8b51f8                 -mov edx, dword ptr [ecx - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(-8) /* -0x8 */);
    // 005124f0  8b5c2408               -mov ebx, dword ptr [esp + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 005124f4  01d6                   -add esi, edx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edx));
    // 005124f6  39df                   +cmp edi, ebx
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
    // 005124f8  7ce3                   -jl 0x5124dd
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005124dd;
    }
    // 005124fa  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
L_0x00512500:
    // 00512500  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00512504  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00512506  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 00512509  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 0051250b  ff5120                 -call dword ptr [ecx + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051250e  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00512510  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 00512513  e878f3fcff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512518  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051251a  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0051251d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051251e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051251f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512520  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512521  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_512530: native, nfs3hp::ea::layerCallPacker */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_5125b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005125b0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005125b1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005125b2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005125b3  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 005125b5  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 005125b7  8b5c2410               -mov ebx, dword ptr [esp + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 005125bb  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 005125bd  7c59                   -jl 0x512618
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00512618;
    }
    // 005125bf  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 005125c1  e80abcffff             -call 0x50e1d0
    cpu.esp -= 4;
    nfs3hp::ea::qlength(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005125c6  39c2                   +cmp edx, eax
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
    // 005125c8  734e                   -jae 0x512618
    if (!cpu.flags.cf)
    {
        goto L_0x00512618;
    }
    // 005125ca  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 005125cc  89531c                 -mov dword ptr [ebx + 0x1c], edx
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(28) /* 0x1c */) = cpu.edx;
    // 005125cf  e89cbeffff             -call 0x50e470
    cpu.esp -= 4;
    nfs3hp::ea::qfindindex(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005125d4  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 005125d6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005125d8  752d                   -jne 0x512607
    if (!cpu.flags.zf)
    {
        goto L_0x00512607;
    }
    // 005125da  ba94015500             -mov edx, 0x550194
    cpu.edx = 5570964 /*0x550194*/;
    // 005125df  b8f4015500             -mov eax, 0x5501f4
    cpu.eax = 5571060 /*0x5501f4*/;
    // 005125e4  68e0015500             -push 0x5501e0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5571040 /*0x5501e0*/;
    cpu.esp -= 4;
    // 005125e9  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 005125ef  ba78000000             -mov edx, 0x78
    cpu.edx = 120 /*0x78*/;
    // 005125f4  a394215500             -mov dword ptr [0x552194], eax
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.eax;
    // 005125f9  891598215500           -mov dword ptr [0x552198], edx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edx;
    // 005125ff  e80ceaeeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512604  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x00512607:
    // 00512607  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512608  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051260a  8b550c                 -mov edx, dword ptr [ebp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 0051260d  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0051260f  ff5508                 -call dword ptr [ebp + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512612  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512613  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512614  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512615  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00512618:
    // 00512618  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0051261a  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0051261c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051261e  e84dfeffff             -call 0x512470
    cpu.esp -= 4;
    sub_512470(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512623  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512624  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512625  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512626  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* sub_512630: native, nfs3hp::ea::layerPackFromTop */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_512650(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512650  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512651  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 00512653  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 00512655  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00512657  e854ffffff             -call 0x5125b0
    cpu.esp -= 4;
    sub_5125b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051265c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_512660(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512660  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512661  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512662  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00512663  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00512664  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00512666  ba94015500             -mov edx, 0x550194
    cpu.edx = 5570964 /*0x550194*/;
    // 0051266b  bb08025500             -mov ebx, 0x550208
    cpu.ebx = 5571080 /*0x550208*/;
    // 00512670  be9a000000             -mov esi, 0x9a
    cpu.esi = 154 /*0x9a*/;
    // 00512675  b8c4015500             -mov eax, 0x5501c4
    cpu.eax = 5571012 /*0x5501c4*/;
    // 0051267a  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 00512680  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 00512686  ba20000000             -mov edx, 0x20
    cpu.edx = 32 /*0x20*/;
    // 0051268b  8b1df4435600           -mov ebx, dword ptr [0x5643f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 00512691  893598215500           -mov dword ptr [0x552198], esi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.esi;
    // 00512697  e884effcff             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051269c  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051269e  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 005126a0  894130                 -mov dword ptr [ecx + 0x30], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(48) /* 0x30 */) = cpu.eax;
    // 005126a3  e828b6ffff             -call 0x50dcd0
    cpu.esp -= 4;
    sub_50dcd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005126a8  83793000               +cmp dword ptr [ecx + 0x30], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(48) /* 0x30 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005126ac  740a                   -je 0x5126b8
    if (cpu.flags.zf)
    {
        goto L_0x005126b8;
    }
    // 005126ae  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 005126b3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005126b4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005126b5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005126b6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005126b7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005126b8:
    // 005126b8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005126ba  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005126bb  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005126bc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005126bd  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005126be  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_5126c0: native, nfs3hp::ea::layerFree */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_5126f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005126f0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005126f1  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 005126f4  89542404               -mov dword ptr [esp + 4], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 005126f8  891c24                 -mov dword ptr [esp], ebx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ebx;
    // 005126fb  89e3                   -mov ebx, esp
    cpu.ebx = cpu.esp;
    // 005126fd  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00512702  8b4830                 -mov ecx, dword ptr [eax + 0x30]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */);
    // 00512705  e846ffffff             -call 0x512650
    cpu.esp -= 4;
    sub_512650(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051270a  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051270d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051270e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_512710: native, nfs3hp::dead::stub_0x512710 */

/* sub_512770: native, nfs3hp::ea::layerSendPacket */

/* sub_512790: native, nfs3hp::dead::stub_0x512790 */

/* sub_5127b0: native, nfs3hp::dead::stub_0x5127b0 */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_5127d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005127d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 005127d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005127d2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 005127d3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005127d4  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 005127d7  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 005127d9  ba06000000             -mov edx, 6
    cpu.edx = 6 /*0x6*/;
    // 005127de  6852040000             -push 0x452
    app->getMemory<x86::reg32>(cpu.esp-4) = 1106 /*0x452*/;
    cpu.esp -= 4;
    // 005127e3  bbff000000             -mov ebx, 0xff
    cpu.ebx = 255 /*0xff*/;
    // 005127e8  6689542404             -mov word ptr [esp + 4], dx
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.dx;
    // 005127ed  e858160200             -call 0x533e4a
    cpu.esp -= 4;
    sub_533e4a(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005127f2  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 005127f7  668944240c             -mov word ptr [esp + 0xc], ax
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.ax;
    // 005127fc  8d442402               -lea eax, [esp + 2]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(2) /* 0x2 */);
    // 00512800  e811dffcff             -call 0x4e0716
    cpu.esp -= 4;
    nfs3hp::fillByte(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512805  bbff000000             -mov ebx, 0xff
    cpu.ebx = 255 /*0xff*/;
    // 0051280a  ba06000000             -mov edx, 6
    cpu.edx = 6 /*0x6*/;
    // 0051280f  8d442406               -lea eax, [esp + 6]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(6) /* 0x6 */);
    // 00512813  e8fedefcff             -call 0x4e0716
    cpu.esp -= 4;
    nfs3hp::fillByte(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512818  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051281d  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 0051281f  c1e810                 -shr eax, 0x10
    cpu.eax >>= 16 /*0x10*/ % 32;
    // 00512822  6689442410             -mov word ptr [esp + 0x10], ax
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.ax;
    // 00512827  b8be080000             -mov eax, 0x8be
    cpu.eax = 2238 /*0x8be*/;
    // 0051282c  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 0051282e  8b1580445600           -mov edx, dword ptr [0x564480]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653632) /* 0x564480 */);
    // 00512834  c1e810                 -shr eax, 0x10
    cpu.eax >>= 16 /*0x10*/ % 32;
    // 00512837  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00512838  6689442416             -mov word ptr [esp + 0x16], ax
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(22) /* 0x16 */) = cpu.ax;
    // 0051283d  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512843  6a0e                   -push 0xe
    app->getMemory<x86::reg32>(cpu.esp-4) = 14 /*0xe*/;
    cpu.esp -= 4;
    // 00512845  8d442404               -lea eax, [esp + 4]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00512849  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051284a  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051284c  6a04                   -push 4
    app->getMemory<x86::reg32>(cpu.esp-4) = 4 /*0x4*/;
    cpu.esp -= 4;
    // 0051284e  8d442420               -lea eax, [esp + 0x20]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(32) /* 0x20 */);
    // 00512852  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512853  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00512854  e8d9150200             -call 0x533e32
    cpu.esp -= 4;
    sub_533e32(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512859  8b0d80445600           -mov ecx, dword ptr [0x564480]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5653632) /* 0x564480 */);
    // 0051285f  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512860  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00512862  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512868  83feff                 +cmp esi, -1
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
    // 0051286b  740d                   -je 0x51287a
    if (cpu.flags.zf)
    {
        goto L_0x0051287a;
    }
    // 0051286d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00512872  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 00512875  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512876  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512877  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512878  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512879  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051287a:
    // 0051287a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051287c  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0051287f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512880  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512881  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512882  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512883  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_512890(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512890  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00512891  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00512892  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00512893  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00512896  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 00512899  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0051289b  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 0051289d  6852040000             -push 0x452
    app->getMemory<x86::reg32>(cpu.esp-4) = 1106 /*0x452*/;
    cpu.esp -= 4;
    // 005128a2  e8a3150200             -call 0x533e4a
    cpu.esp -= 4;
    sub_533e4a(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005128a7  663b460c               +cmp ax, word ptr [esi + 0xc]
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.esi + x86::reg32(12) /* 0xc */)));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 005128ab  7409                   -je 0x5128b6
    if (cpu.flags.zf)
    {
        goto L_0x005128b6;
    }
L_0x005128ad:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005128ad  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005128af  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 005128b2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005128b3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005128b4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005128b5  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005128b6:
    // 005128b6  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 005128bb  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 005128bd  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 005128bf  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 005128c1  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 005128c3  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 005128ca  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 005128cc  83f802                 +cmp eax, 2
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
    // 005128cf  75dc                   -jne 0x5128ad
    if (!cpu.flags.zf)
    {
        goto L_0x005128ad;
    }
    // 005128d1  29c7                   -sub edi, eax
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 005128d3  c1ef06                 -shr edi, 6
    cpu.edi >>= 6 /*0x6*/ % 32;
    // 005128d6  8d3403                 -lea esi, [ebx + eax]
    cpu.esi = x86::reg32(cpu.ebx + cpu.eax * 1);
    // 005128d9  897c2404               -mov dword ptr [esp + 4], edi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edi;
    // 005128dd  8d7e32                 -lea edi, [esi + 0x32]
    cpu.edi = x86::reg32(cpu.esi + x86::reg32(50) /* 0x32 */);
    // 005128e0  8d2c06                 -lea ebp, [esi + eax]
    cpu.ebp = x86::reg32(cpu.esi + cpu.eax * 1);
L_0x005128e3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005128e3  8b542404               -mov edx, dword ptr [esp + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 005128e7  4a                     -dec edx
    (cpu.edx)--;
    // 005128e8  89542404               -mov dword ptr [esp + 4], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 005128ec  83faff                 +cmp edx, -1
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
    // 005128ef  750c                   -jne 0x5128fd
    if (!cpu.flags.zf)
    {
        goto L_0x005128fd;
    }
    // 005128f1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 005128f6  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 005128f9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005128fa  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005128fb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005128fc  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005128fd:
    // 005128fd  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 00512902  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00512904  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00512906  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00512908  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051290a  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00512911  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00512913  3dbe080000             +cmp eax, 0x8be
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2238 /*0x8be*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512918  740b                   -je 0x512925
    if (cpu.flags.zf)
    {
        goto L_0x00512925;
    }
    // 0051291a  83c540                 -add ebp, 0x40
    (cpu.ebp) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 0051291d  83c640                 -add esi, 0x40
    (cpu.esi) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 00512920  83c740                 +add edi, 0x40
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00512923  ebbe                   -jmp 0x5128e3
    goto L_0x005128e3;
L_0x00512925:
    // 00512925  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 00512928  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 0051292a  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0051292c  e8ff30feff             -call 0x4f5a30
    cpu.esp -= 4;
    sub_4f5a30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512931  83c540                 -add ebp, 0x40
    (cpu.ebp) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 00512934  83c640                 -add esi, 0x40
    (cpu.esi) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 00512937  83c740                 +add edi, 0x40
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051293a  eba7                   -jmp 0x5128e3
    goto L_0x005128e3;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_512940(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512940  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512941  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512942  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00512943  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00512945  8b1580445600           -mov edx, dword ptr [0x564480]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653632) /* 0x564480 */);
    // 0051294b  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051294c  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512952  6a0e                   -push 0xe
    app->getMemory<x86::reg32>(cpu.esp-4) = 14 /*0xe*/;
    cpu.esp -= 4;
    // 00512954  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512955  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00512957  6a42                   -push 0x42
    app->getMemory<x86::reg32>(cpu.esp-4) = 66 /*0x42*/;
    cpu.esp -= 4;
    // 00512959  68d2b0a000             -push 0xa0b0d2
    app->getMemory<x86::reg32>(cpu.esp-4) = 10531026 /*0xa0b0d2*/;
    cpu.esp -= 4;
    // 0051295e  8b0dbcb0a000           -mov ecx, dword ptr [0xa0b0bc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10531004) /* 0xa0b0bc */);
    // 00512964  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512965  e8c8140200             -call 0x533e32
    cpu.esp -= 4;
    sub_533e32(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051296a  8b1d80445600           -mov ebx, dword ptr [0x564480]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653632) /* 0x564480 */);
    // 00512970  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512971  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512977  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512978  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512979  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051297a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_512980(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512980  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512981  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512982  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00512984  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 00512989  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 0051298b  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 0051298d  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051298f  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00512996  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00512998  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0051299d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051299f  8d4302                 -lea eax, [ebx + 2]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 005129a2  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 005129a4  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 005129a6  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 005129a8  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 005129af  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 005129b1  83fa01                 +cmp edx, 1
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
    // 005129b4  7516                   -jne 0x5129cc
    if (!cpu.flags.zf)
    {
        goto L_0x005129cc;
    }
    // 005129b6  3dbe080000             +cmp eax, 0x8be
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2238 /*0x8be*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005129bb  7407                   -je 0x5129c4
    if (cpu.flags.zf)
    {
        goto L_0x005129c4;
    }
    // 005129bd  3dffff0000             +cmp eax, 0xffff
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(65535 /*0xffff*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005129c2  7508                   -jne 0x5129cc
    if (!cpu.flags.zf)
    {
        goto L_0x005129cc;
    }
L_0x005129c4:
    // 005129c4  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 005129c9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005129ca  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005129cb  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005129cc:
    // 005129cc  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005129ce  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005129cf  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005129d0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_5129e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005129e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 005129e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005129e2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 005129e3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005129e4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005129e5  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 005129e8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 005129ea  68e8030000             -push 0x3e8
    app->getMemory<x86::reg32>(cpu.esp-4) = 1000 /*0x3e8*/;
    cpu.esp -= 4;
    // 005129ef  6a02                   -push 2
    app->getMemory<x86::reg32>(cpu.esp-4) = 2 /*0x2*/;
    cpu.esp -= 4;
    // 005129f1  6a06                   -push 6
    app->getMemory<x86::reg32>(cpu.esp-4) = 6 /*0x6*/;
    cpu.esp -= 4;
    // 005129f3  e8b2140200             -call 0x533eaa
    cpu.esp -= 4;
    sub_533eaa(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005129f8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 005129fa  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 005129fc  83f8ff                 +cmp eax, -1
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
    // 005129ff  7510                   -jne 0x512a11
    if (!cpu.flags.zf)
    {
        goto L_0x00512a11;
    }
    // 00512a01  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
L_0x00512a06:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00512a06  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00512a08  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00512a0b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512a0c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512a0d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512a0e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512a0f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512a10  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00512a11:
    // 00512a11  6a04                   -push 4
    app->getMemory<x86::reg32>(cpu.esp-4) = 4 /*0x4*/;
    cpu.esp -= 4;
    // 00512a13  8d442404               -lea eax, [esp + 4]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00512a17  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512a18  6a20                   -push 0x20
    app->getMemory<x86::reg32>(cpu.esp-4) = 32 /*0x20*/;
    cpu.esp -= 4;
    // 00512a1a  68ffff0000             -push 0xffff
    app->getMemory<x86::reg32>(cpu.esp-4) = 65535 /*0xffff*/;
    cpu.esp -= 4;
    // 00512a1f  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00512a24  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512a25  89542414               -mov dword ptr [esp + 0x14], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */) = cpu.edx;
    // 00512a29  e828140200             -call 0x533e56
    cpu.esp -= 4;
    sub_533e56(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512a2e  6a04                   -push 4
    app->getMemory<x86::reg32>(cpu.esp-4) = 4 /*0x4*/;
    cpu.esp -= 4;
    // 00512a30  8d442404               -lea eax, [esp + 4]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00512a34  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512a35  680f400000             -push 0x400f
    app->getMemory<x86::reg32>(cpu.esp-4) = 16399 /*0x400f*/;
    cpu.esp -= 4;
    // 00512a3a  68ffff0000             -push 0xffff
    app->getMemory<x86::reg32>(cpu.esp-4) = 65535 /*0xffff*/;
    cpu.esp -= 4;
    // 00512a3f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512a40  e811140200             -call 0x533e56
    cpu.esp -= 4;
    sub_533e56(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512a45  6a04                   -push 4
    app->getMemory<x86::reg32>(cpu.esp-4) = 4 /*0x4*/;
    cpu.esp -= 4;
    // 00512a47  8d442404               -lea eax, [esp + 4]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00512a4b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512a4c  680f400000             -push 0x400f
    app->getMemory<x86::reg32>(cpu.esp-4) = 16399 /*0x400f*/;
    cpu.esp -= 4;
    // 00512a51  68e8030000             -push 0x3e8
    app->getMemory<x86::reg32>(cpu.esp-4) = 1000 /*0x3e8*/;
    cpu.esp -= 4;
    // 00512a56  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512a57  e8fa130200             -call 0x533e56
    cpu.esp -= 4;
    sub_533e56(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512a5c  ba0e000000             -mov edx, 0xe
    cpu.edx = 14 /*0xe*/;
    // 00512a61  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00512a63  e8a4dcfcff             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512a68  6852040000             -push 0x452
    app->getMemory<x86::reg32>(cpu.esp-4) = 1106 /*0x452*/;
    cpu.esp -= 4;
    // 00512a6d  66c7060600             -mov word ptr [esi], 6
    app->getMemory<x86::reg16>(cpu.esi) = 6 /*0x6*/;
    // 00512a72  e8d3130200             -call 0x533e4a
    cpu.esp -= 4;
    sub_533e4a(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512a77  6a0e                   -push 0xe
    app->getMemory<x86::reg32>(cpu.esp-4) = 14 /*0xe*/;
    cpu.esp -= 4;
    // 00512a79  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00512a7a  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512a7b  6689460c               -mov word ptr [esi + 0xc], ax
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(12) /* 0xc */) = cpu.ax;
    // 00512a7f  e8c0130200             -call 0x533e44
    cpu.esp -= 4;
    sub_533e44(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512a84  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00512a86  7524                   -jne 0x512aac
    if (!cpu.flags.zf)
    {
        goto L_0x00512aac;
    }
    // 00512a88  8d442404               -lea eax, [esp + 4]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00512a8c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512a8d  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00512a8e  b90e000000             -mov ecx, 0xe
    cpu.ecx = 14 /*0xe*/;
    // 00512a93  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512a94  894c2410               -mov dword ptr [esp + 0x10], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.ecx;
    // 00512a98  e8a1130200             -call 0x533e3e
    cpu.esp -= 4;
    sub_533e3e(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512a9d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00512a9f  750b                   -jne 0x512aac
    if (!cpu.flags.zf)
    {
        goto L_0x00512aac;
    }
    // 00512aa1  837c24040e             +cmp dword ptr [esp + 4], 0xe
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(14 /*0xe*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512aa6  0f845affffff           -je 0x512a06
    if (cpu.flags.zf)
    {
        goto L_0x00512a06;
    }
L_0x00512aac:
    // 00512aac  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00512aad  e8f2130200             -call 0x533ea4
    cpu.esp -= 4;
    sub_533ea4(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512ab2  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 00512ab7  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00512ab9  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00512abc  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512abd  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512abe  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512abf  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512ac0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512ac1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_512ad0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512ad0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512ad1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512ad2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00512ad3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00512ad4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00512ad5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00512ad6  81ec30030000           -sub esp, 0x330
    (cpu.esp) -= x86::reg32(x86::sreg32(816 /*0x330*/));
    // 00512adc  e8affcfdff             -call 0x4f2790
    cpu.esp -= 4;
    sub_4f2790(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512ae1  ba06000000             -mov edx, 6
    cpu.edx = 6 /*0x6*/;
    // 00512ae6  6852040000             -push 0x452
    app->getMemory<x86::reg32>(cpu.esp-4) = 1106 /*0x452*/;
    cpu.esp -= 4;
    // 00512aeb  8d5804                 -lea ebx, [eax + 4]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00512aee  6689942418030000       -mov word ptr [esp + 0x318], dx
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(792) /* 0x318 */) = cpu.dx;
    // 00512af6  e84f130200             -call 0x533e4a
    cpu.esp -= 4;
    sub_533e4a(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512afb  8d942414030000         -lea edx, [esp + 0x314]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(788) /* 0x314 */);
    // 00512b02  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 00512b03  e8f82dfeff             -call 0x4f5900
    cpu.esp -= 4;
    sub_4f5900(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512b08  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00512b0d  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00512b0f  8915b8b0a000           -mov dword ptr [0xa0b0b8], edx
    app->getMemory<x86::reg32>(x86::reg32(10531000) /* 0xa0b0b8 */) = cpu.edx;
    // 00512b15  31ed                   -xor ebp, ebp
    cpu.ebp ^= x86::reg32(x86::sreg32(cpu.ebp));
L_0x00512b17:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00512b17  bee8030000             -mov esi, 0x3e8
    cpu.esi = 1000 /*0x3e8*/;
    // 00512b1c  a1bcb0a000             -mov eax, dword ptr [0xa0b0bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531004) /* 0xa0b0bc */);
    // 00512b21  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00512b23  89842404020000         -mov dword ptr [esp + 0x204], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(516) /* 0x204 */) = cpu.eax;
    // 00512b2a  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00512b2d  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00512b2f  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00512b31  89842424030000         -mov dword ptr [esp + 0x324], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(804) /* 0x324 */) = cpu.eax;
    // 00512b38  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00512b3a  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00512b3c  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00512b3f  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00512b41  69c2e8030000           -imul eax, edx, 0x3e8
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(1000 /*0x3e8*/)));
    // 00512b47  89842428030000         -mov dword ptr [esp + 0x328], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(808) /* 0x328 */) = cpu.eax;
    // 00512b4e  8d842424030000         -lea eax, [esp + 0x324]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(804) /* 0x324 */);
    // 00512b55  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512b56  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00512b57  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00512b58  8d84240c020000         -lea eax, [esp + 0x20c]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(524) /* 0x20c */);
    // 00512b5f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512b60  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00512b65  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00512b66  898c2414020000         -mov dword ptr [esp + 0x214], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(532) /* 0x214 */) = cpu.ecx;
    // 00512b6d  e8ae120200             -call 0x533e20
    cpu.esp -= 4;
    sub_533e20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512b72  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00512b74  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00512b76  0f84c9000000           -je 0x512c45
    if (cpu.flags.zf)
    {
        goto L_0x00512c45;
    }
    // 00512b7c  83f8ff                 +cmp eax, -1
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
    // 00512b7f  0f84c0000000           -je 0x512c45
    if (cpu.flags.zf)
    {
        goto L_0x00512c45;
    }
    // 00512b85  a180445600             -mov eax, dword ptr [0x564480]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653632) /* 0x564480 */);
    // 00512b8a  be10000000             -mov esi, 0x10
    cpu.esi = 16 /*0x10*/;
    // 00512b8f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512b90  89b42430030000         -mov dword ptr [esp + 0x330], esi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(816) /* 0x330 */) = cpu.esi;
    // 00512b97  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512b9d  8d84242c030000         -lea eax, [esp + 0x32c]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(812) /* 0x32c */);
    // 00512ba4  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512ba5  8d842408030000         -lea eax, [esp + 0x308]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(776) /* 0x308 */);
    // 00512bac  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512bad  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00512bae  6800020000             -push 0x200
    app->getMemory<x86::reg32>(cpu.esp-4) = 512 /*0x200*/;
    cpu.esp -= 4;
    // 00512bb3  8d442410               -lea eax, [esp + 0x10]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 00512bb7  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00512bb8  8b15bcb0a000           -mov edx, dword ptr [0xa0b0bc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531004) /* 0xa0b0bc */);
    // 00512bbe  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00512bbf  e898120200             -call 0x533e5c
    cpu.esp -= 4;
    sub_533e5c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512bc4  8b0d80445600           -mov ecx, dword ptr [0x564480]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5653632) /* 0x564480 */);
    // 00512bca  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512bcb  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00512bcd  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512bd3  83fe04                 +cmp esi, 4
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
    // 00512bd6  7519                   -jne 0x512bf1
    if (!cpu.flags.zf)
    {
        goto L_0x00512bf1;
    }
    // 00512bd8  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 00512bda  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00512bdc  e89ffdffff             -call 0x512980
    cpu.esp -= 4;
    sub_512980(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512be1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00512be3  740c                   -je 0x512bf1
    if (cpu.flags.zf)
    {
        goto L_0x00512bf1;
    }
    // 00512be5  8d842404030000         -lea eax, [esp + 0x304]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(772) /* 0x304 */);
    // 00512bec  e84ffdffff             -call 0x512940
    cpu.esp -= 4;
    sub_512940(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00512bf1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00512bf1  3b2db8b0a000           +cmp ebp, dword ptr [0xa0b0b8]
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(10531000) /* 0xa0b0b8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512bf7  7425                   -je 0x512c1e
    if (cpu.flags.zf)
    {
        goto L_0x00512c1e;
    }
    // 00512bf9  e892fbfdff             -call 0x4f2790
    cpu.esp -= 4;
    sub_4f2790(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512bfe  39c3                   +cmp ebx, eax
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
    // 00512c00  731c                   -jae 0x512c1e
    if (!cpu.flags.cf)
    {
        goto L_0x00512c1e;
    }
    // 00512c02  8d842414030000         -lea eax, [esp + 0x314]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(788) /* 0x314 */);
    // 00512c09  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 00512c0e  e82dfdffff             -call 0x512940
    cpu.esp -= 4;
    sub_512940(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512c13  e878fbfdff             -call 0x4f2790
    cpu.esp -= 4;
    sub_4f2790(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512c18  8d9860ea0000           -lea ebx, [eax + 0xea60]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(60000) /* 0xea60 */);
L_0x00512c1e:
    // 00512c1e  3b2db8b0a000           +cmp ebp, dword ptr [0xa0b0b8]
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(10531000) /* 0xa0b0b8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512c24  0f85edfeffff           -jne 0x512b17
    if (!cpu.flags.zf)
    {
        goto L_0x00512b17;
    }
L_0x00512c2a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00512c2a  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00512c2c  7526                   -jne 0x512c54
    if (!cpu.flags.zf)
    {
        goto L_0x00512c54;
    }
L_0x00512c2e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00512c2e  c705b4b0a00001000000   -mov dword ptr [0xa0b0b4], 1
    app->getMemory<x86::reg32>(x86::reg32(10530996) /* 0xa0b0b4 */) = 1 /*0x1*/;
    // 00512c38  81c430030000           -add esp, 0x330
    (cpu.esp) += x86::reg32(x86::sreg32(816 /*0x330*/));
    // 00512c3e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512c3f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512c40  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512c41  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512c42  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512c43  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512c44  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00512c45:
    // 00512c45  83faff                 +cmp edx, -1
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
    // 00512c48  75a7                   -jne 0x512bf1
    if (!cpu.flags.zf)
    {
        goto L_0x00512bf1;
    }
    // 00512c4a  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00512c4c  891db8b0a000           -mov dword ptr [0xa0b0b8], ebx
    app->getMemory<x86::reg32>(x86::reg32(10531000) /* 0xa0b0b8 */) = cpu.ebx;
    // 00512c52  ebd6                   -jmp 0x512c2a
    goto L_0x00512c2a;
L_0x00512c54:
    // 00512c54  b810000000             -mov eax, 0x10
    cpu.eax = 16 /*0x10*/;
    // 00512c59  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00512c5b  c1e810                 +shr eax, 0x10
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
    // 00512c5e  66a312b1a000           -mov word ptr [0xa0b112], ax
    app->getMemory<x86::reg16>(x86::reg32(10531090) /* 0xa0b112 */) = cpu.ax;
    // 00512c64  8d842414030000         -lea eax, [esp + 0x314]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(788) /* 0x314 */);
    // 00512c6b  e8d0fcffff             -call 0x512940
    cpu.esp -= 4;
    sub_512940(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512c70  ebbc                   -jmp 0x512c2e
    goto L_0x00512c2e;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_512c80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512c80  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512c81  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00512c82  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00512c83  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00512c85  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 00512c87  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00512c89  833db8b0a00000         +cmp dword ptr [0xa0b0b8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10531000) /* 0xa0b0b8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512c90  7409                   -je 0x512c9b
    if (cpu.flags.zf)
    {
        goto L_0x00512c9b;
    }
L_0x00512c92:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00512c92  a1b8b0a000             -mov eax, dword ptr [0xa0b0b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531000) /* 0xa0b0b8 */);
    // 00512c97  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512c98  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512c99  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512c9a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00512c9b:
    // 00512c9b  b8c4b0a000             -mov eax, 0xa0b0c4
    cpu.eax = 10531012 /*0xa0b0c4*/;
    // 00512ca0  e83bfdffff             -call 0x5129e0
    cpu.esp -= 4;
    sub_5129e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512ca5  a3bcb0a000             -mov dword ptr [0xa0b0bc], eax
    app->getMemory<x86::reg32>(x86::reg32(10531004) /* 0xa0b0bc */) = cpu.eax;
    // 00512caa  83f8ff                 +cmp eax, -1
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
    // 00512cad  74e3                   -je 0x512c92
    if (cpu.flags.zf)
    {
        goto L_0x00512c92;
    }
    // 00512caf  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00512cb4  890dc0b0a000           -mov dword ptr [0xa0b0c0], ecx
    app->getMemory<x86::reg32>(x86::reg32(10531008) /* 0xa0b0c0 */) = cpu.ecx;
    // 00512cba  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00512cbc  c1e810                 -shr eax, 0x10
    cpu.eax >>= 16 /*0x10*/ % 32;
    // 00512cbf  66a3d2b0a000           -mov word ptr [0xa0b0d2], ax
    app->getMemory<x86::reg16>(x86::reg32(10531026) /* 0xa0b0d2 */) = cpu.ax;
    // 00512cc5  b8be080000             -mov eax, 0x8be
    cpu.eax = 2238 /*0x8be*/;
    // 00512cca  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00512ccc  c1e810                 -shr eax, 0x10
    cpu.eax >>= 16 /*0x10*/ % 32;
    // 00512ccf  66a3d4b0a000           -mov word ptr [0xa0b0d4], ax
    app->getMemory<x86::reg16>(x86::reg32(10531028) /* 0xa0b0d4 */) = cpu.ax;
    // 00512cd5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00512cda  bb30000000             -mov ebx, 0x30
    cpu.ebx = 48 /*0x30*/;
    // 00512cdf  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00512ce1  c1e810                 -shr eax, 0x10
    cpu.eax >>= 16 /*0x10*/ % 32;
    // 00512ce4  bf06b1a000             -mov edi, 0xa0b106
    cpu.edi = 10531078 /*0xa0b106*/;
    // 00512ce9  66a312b1a000           -mov word ptr [0xa0b112], ax
    app->getMemory<x86::reg16>(x86::reg32(10531090) /* 0xa0b112 */) = cpu.ax;
    // 00512cef  b8d6b0a000             -mov eax, 0xa0b0d6
    cpu.eax = 10531030 /*0xa0b0d6*/;
    // 00512cf4  b9ffffffff             -mov ecx, 0xffffffff
    cpu.ecx = 4294967295 /*0xffffffff*/;
    // 00512cf9  e832e1fcff             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512cfe  68a0b0a000             -push 0xa0b0a0
    app->getMemory<x86::reg32>(cpu.esp-4) = 10530976 /*0xa0b0a0*/;
    cpu.esp -= 4;
    // 00512d03  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00512d08  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00512d09  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00512d0a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00512d0b  b8d02a5100             -mov eax, 0x512ad0
    cpu.eax = 5319376 /*0x512ad0*/;
    // 00512d10  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00512d12  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00512d14  8935b4b0a000           -mov dword ptr [0xa0b0b4], esi
    app->getMemory<x86::reg32>(x86::reg32(10530996) /* 0xa0b0b4 */) = cpu.esi;
    // 00512d1a  e881cafcff             -call 0x4df7a0
    cpu.esp -= 4;
    nfs3hp::ea::createThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512d1f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00512d21  7422                   -je 0x512d45
    if (cpu.flags.zf)
    {
        goto L_0x00512d45;
    }
    // 00512d23  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00512d28  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00512d2a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00512d2a  8b3db8b0a000           -mov edi, dword ptr [0xa0b0b8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10531000) /* 0xa0b0b8 */);
    // 00512d30  39fa                   +cmp edx, edi
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
    // 00512d32  7511                   -jne 0x512d45
    if (!cpu.flags.zf)
    {
        goto L_0x00512d45;
    }
    // 00512d34  3b3db4b0a000           +cmp edi, dword ptr [0xa0b0b4]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(10530996) /* 0xa0b0b4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512d3a  7509                   -jne 0x512d45
    if (!cpu.flags.zf)
    {
        goto L_0x00512d45;
    }
    // 00512d3c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00512d3e  e89dcbfcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512d43  ebe5                   -jmp 0x512d2a
    goto L_0x00512d2a;
L_0x00512d45:
    // 00512d45  833db4b0a00000         +cmp dword ptr [0xa0b0b4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10530996) /* 0xa0b0b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512d4c  0f8440ffffff           -je 0x512c92
    if (cpu.flags.zf)
    {
        goto L_0x00512c92;
    }
    // 00512d52  8b15bcb0a000           -mov edx, dword ptr [0xa0b0bc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531004) /* 0xa0b0bc */);
    // 00512d58  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00512d59  e846110200             -call 0x533ea4
    cpu.esp -= 4;
    sub_533ea4(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512d5e  a1b8b0a000             -mov eax, dword ptr [0xa0b0b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531000) /* 0xa0b0b8 */);
    // 00512d63  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512d64  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512d65  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512d66  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_512d70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512d70  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512d71  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512d72  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00512d73  833db8b0a00000         +cmp dword ptr [0xa0b0b8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10531000) /* 0xa0b0b8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512d7a  750d                   -jne 0x512d89
    if (!cpu.flags.zf)
    {
        goto L_0x00512d89;
    }
    // 00512d7c  8d442000               -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
L_0x00512d80:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00512d80  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00512d85  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512d86  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512d87  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512d88  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00512d89:
    // 00512d89  8b1dbcb0a000           -mov ebx, dword ptr [0xa0b0bc]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10531004) /* 0xa0b0bc */);
    // 00512d8f  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512d91  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00512d92  890db8b0a000           -mov dword ptr [0xa0b0b8], ecx
    app->getMemory<x86::reg32>(x86::reg32(10531000) /* 0xa0b0b8 */) = cpu.ecx;
    // 00512d98  e807110200             -call 0x533ea4
    cpu.esp -= 4;
    sub_533ea4(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512d9d  833db4b0a00000         +cmp dword ptr [0xa0b0b4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10530996) /* 0xa0b0b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512da4  75da                   -jne 0x512d80
    if (!cpu.flags.zf)
    {
        goto L_0x00512d80;
    }
L_0x00512da6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00512da6  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00512dab  e830cbfcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512db0  833db4b0a00000         +cmp dword ptr [0xa0b0b4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10530996) /* 0xa0b0b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00512db7  74ed                   -je 0x512da6
    if (cpu.flags.zf)
    {
        goto L_0x00512da6;
    }
    // 00512db9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00512dbe  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512dbf  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512dc0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512dc1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_512dd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512dd0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dd2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dd4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dd6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dd8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dda  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ddc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dde  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512de0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512de2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512de4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512de6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512de8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dea  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dec  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dee  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512df0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512df2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512df4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512df6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512df8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dfa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dfc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dfe  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e00  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e02  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e04  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e06  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e08  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e0a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e0c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e0e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e10  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e12  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e14  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e16  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e18  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e1a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e1c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e1e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e20  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e22  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e24  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e26  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e28  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e2a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e2c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e2e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e30  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e32  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e34  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e36  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e38  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e3a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e3c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e3e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e40  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e42  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e44  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e46  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e48  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e4a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e4c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e4e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e50  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e52  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e54  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e56  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e58  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e5a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e5c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e5e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e60  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e62  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e64  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e66  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e68  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e6a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e6c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e6e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e70  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e72  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e74  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e76  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e78  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e7a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e7c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e7e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e80  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e82  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e84  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e86  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e88  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e8a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e8c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e8e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e90  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e92  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e94  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e96  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e98  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e9a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e9c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e9e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ea0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ea2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ea4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ea6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ea8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eaa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eac  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eae  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eb0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eb2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eb4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eb6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eb8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eba  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ebc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ebe  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ec0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ec2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ec4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ec6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ec8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eca  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ecc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ece  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ed0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ed2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ed4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ed6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ed8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eda  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512edc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ede  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ee0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ee2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ee4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ee6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ee8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eea  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eec  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eee  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ef0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ef2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ef4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ef6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ef8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512efa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512efc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512efe  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f00  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f02  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f04  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f06  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f08  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f0a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f0c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f0e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f10  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f12  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f14  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f16  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f18  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f1a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f1c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f1e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f20  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f22  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f24  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f26  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f28  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f2a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f2c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f2e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f30  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f32  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f34  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f36  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f38  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f3a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f3c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f3e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f40  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f42  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f44  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f46  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f48  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f4a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f4c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f4e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f50  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f52  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f54  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f56  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f58  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f5a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f5c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f5e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f60  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f61  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f63  8a480f                 -mov cl, byte ptr [eax + 0xf]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(15) /* 0xf */);
    // 00512f66  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f67  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f69  8a480e                 -mov cl, byte ptr [eax + 0xe]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(14) /* 0xe */);
    // 00512f6c  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f6d  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f6f  8a480d                 -mov cl, byte ptr [eax + 0xd]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(13) /* 0xd */);
    // 00512f72  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f73  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f75  8a480c                 -mov cl, byte ptr [eax + 0xc]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00512f78  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f79  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f7b  8a480b                 -mov cl, byte ptr [eax + 0xb]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(11) /* 0xb */);
    // 00512f7e  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f7f  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f81  8a480a                 -mov cl, byte ptr [eax + 0xa]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10) /* 0xa */);
    // 00512f84  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f85  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f87  8a4809                 -mov cl, byte ptr [eax + 9]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(9) /* 0x9 */);
    // 00512f8a  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f8b  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f8d  8a4808                 -mov cl, byte ptr [eax + 8]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00512f90  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f91  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f93  668b4806               -mov cx, word ptr [eax + 6]
    cpu.cx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 00512f97  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f98  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f9a  668b4804               -mov cx, word ptr [eax + 4]
    cpu.cx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00512f9e  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f9f  8b08                   -mov ecx, dword ptr [eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax);
    // 00512fa1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512fa2  682c025500             -push 0x55022c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5571116 /*0x55022c*/;
    cpu.esp -= 4;
    // 00512fa7  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00512fa8  e8e3c6fcff             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512fad  83c434                 -add esp, 0x34
    (cpu.esp) += x86::reg32(x86::sreg32(52 /*0x34*/));
    // 00512fb0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512fb1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_512f60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x00512f60;
    // 00512dd0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dd2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dd4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dd6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dd8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dda  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ddc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dde  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512de0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512de2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512de4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512de6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512de8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dea  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dec  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dee  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512df0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512df2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512df4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512df6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512df8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dfa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dfc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512dfe  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e00  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e02  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e04  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e06  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e08  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e0a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e0c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e0e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e10  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e12  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e14  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e16  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e18  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e1a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e1c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e1e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e20  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e22  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e24  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e26  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e28  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e2a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e2c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e2e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e30  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e32  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e34  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e36  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e38  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e3a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e3c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e3e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e40  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e42  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e44  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e46  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e48  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e4a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e4c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e4e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e50  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e52  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e54  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e56  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e58  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e5a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e5c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e5e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e60  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e62  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e64  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e66  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e68  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e6a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e6c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e6e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e70  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e72  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e74  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e76  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e78  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e7a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e7c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e7e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e80  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e82  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e84  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e86  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e88  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e8a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e8c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e8e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e90  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e92  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e94  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e96  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e98  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e9a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e9c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512e9e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ea0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ea2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ea4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ea6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ea8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eaa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eac  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eae  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eb0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eb2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eb4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eb6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eb8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eba  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ebc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ebe  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ec0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ec2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ec4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ec6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ec8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eca  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ecc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ece  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ed0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ed2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ed4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ed6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ed8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eda  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512edc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ede  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ee0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ee2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ee4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ee6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ee8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eea  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eec  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512eee  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ef0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ef2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ef4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ef6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512ef8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512efa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512efc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512efe  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f00  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f02  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f04  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f06  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f08  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f0a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f0c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f0e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f10  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f12  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f14  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f16  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f18  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f1a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f1c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f1e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f20  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f22  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f24  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f26  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f28  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f2a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f2c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f2e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f30  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f32  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f34  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f36  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f38  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f3a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f3c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f3e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f40  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f42  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f44  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f46  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f48  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f4a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f4c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f4e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f50  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f52  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f54  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f56  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f58  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f5a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f5c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00512f5e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
L_entry_0x00512f60:
    // 00512f60  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f61  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f63  8a480f                 -mov cl, byte ptr [eax + 0xf]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(15) /* 0xf */);
    // 00512f66  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f67  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f69  8a480e                 -mov cl, byte ptr [eax + 0xe]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(14) /* 0xe */);
    // 00512f6c  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f6d  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f6f  8a480d                 -mov cl, byte ptr [eax + 0xd]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(13) /* 0xd */);
    // 00512f72  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f73  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f75  8a480c                 -mov cl, byte ptr [eax + 0xc]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00512f78  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f79  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f7b  8a480b                 -mov cl, byte ptr [eax + 0xb]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(11) /* 0xb */);
    // 00512f7e  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f7f  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f81  8a480a                 -mov cl, byte ptr [eax + 0xa]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10) /* 0xa */);
    // 00512f84  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f85  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f87  8a4809                 -mov cl, byte ptr [eax + 9]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(9) /* 0x9 */);
    // 00512f8a  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f8b  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f8d  8a4808                 -mov cl, byte ptr [eax + 8]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00512f90  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f91  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f93  668b4806               -mov cx, word ptr [eax + 6]
    cpu.cx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 00512f97  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f98  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00512f9a  668b4804               -mov cx, word ptr [eax + 4]
    cpu.cx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00512f9e  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512f9f  8b08                   -mov ecx, dword ptr [eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax);
    // 00512fa1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00512fa2  682c025500             -push 0x55022c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5571116 /*0x55022c*/;
    cpu.esp -= 4;
    // 00512fa7  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00512fa8  e8e3c6fcff             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00512fad  83c434                 -add esp, 0x34
    (cpu.esp) += x86::reg32(x86::sreg32(52 /*0x34*/));
    // 00512fb0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512fb1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_512fc0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512fc0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00512fc1  8b15a0c17900           -mov edx, dword ptr [0x79c1a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7979424) /* 0x79c1a0 */);
    // 00512fc7  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00512fc9  a3e0895600             -mov dword ptr [0x5689e0], eax
    app->getMemory<x86::reg32>(x86::reg32(5671392) /* 0x5689e0 */) = cpu.eax;
    // 00512fce  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00512fcf  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_512fd0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512fd0  a1a0c17900             -mov eax, dword ptr [0x79c1a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979424) /* 0x79c1a0 */);
    // 00512fd5  2b05e0895600           -sub eax, dword ptr [0x5689e0]
    (cpu.eax) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5671392) /* 0x5689e0 */)));
    // 00512fdb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00512fdd  7c11                   -jl 0x512ff0
    if (cpu.flags.sf != cpu.flags.of)
    {
    return sub_512ff0(app, cpu.handOver());
    }
    // 00512fdf  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00512fe4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_512ff0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00512ff0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00512ff2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_513000(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00513000  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00513001  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00513002  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00513004  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00513006  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513008  7416                   -je 0x513020
    if (cpu.flags.zf)
    {
        goto L_0x00513020;
    }
L_0x0051300a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051300a  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051300c  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051300e  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 00513010  21c8                   -and eax, ecx
    cpu.eax &= x86::reg32(x86::sreg32(cpu.ecx));
    // 00513012  42                     -inc edx
    (cpu.edx)++;
    // 00513013  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513015  75f3                   -jne 0x51300a
    if (!cpu.flags.zf)
    {
        goto L_0x0051300a;
    }
    // 00513017  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 0051301d  8d5200                 -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
L_0x00513020:
    // 00513020  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00513022  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513023  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513024  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_513030(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00513030  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00513031  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00513032  81ecb4000000           -sub esp, 0xb4
    (cpu.esp) -= x86::reg32(x86::sreg32(180 /*0xb4*/));
    // 00513038  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051303a  899424b0000000         -mov dword ptr [esp + 0xb0], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(176) /* 0xb0 */) = cpu.edx;
    // 00513041  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00513043  8b6904                 -mov ebp, dword ptr [ecx + 4]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 00513046  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00513048  750a                   -jne 0x513054
    if (!cpu.flags.zf)
    {
        goto L_0x00513054;
    }
    // 0051304a  83790800               +cmp dword ptr [ecx + 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051304e  0f8528010000           -jne 0x51317c
    if (!cpu.flags.zf)
    {
        goto L_0x0051317c;
    }
L_0x00513054:
    // 00513054  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00513055  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00513056  c78424b4000000636564ea -mov dword ptr [esp + 0xb4], 0xea646563
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(180) /* 0xb4 */) = 3932448099 /*0xea646563*/;
    // 00513061  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513063  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 00513067  8a4174                 -mov al, byte ptr [ecx + 0x74]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(116) /* 0x74 */);
    // 0051306a  88442450               -mov byte ptr [esp + 0x50], al
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(80) /* 0x50 */) = cpu.al;
    // 0051306e  8a4175                 -mov al, byte ptr [ecx + 0x75]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(117) /* 0x75 */);
    // 00513071  88442451               -mov byte ptr [esp + 0x51], al
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(81) /* 0x51 */) = cpu.al;
    // 00513075  8a4176                 -mov al, byte ptr [ecx + 0x76]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(118) /* 0x76 */);
    // 00513078  88442452               -mov byte ptr [esp + 0x52], al
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(82) /* 0x52 */) = cpu.al;
    // 0051307c  8a4177                 -mov al, byte ptr [ecx + 0x77]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(119) /* 0x77 */);
    // 0051307f  88442453               -mov byte ptr [esp + 0x53], al
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(83) /* 0x53 */) = cpu.al;
    // 00513083  8b4176                 -mov eax, dword ptr [ecx + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(118) /* 0x76 */);
    // 00513086  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00513089  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 0051308b  c1e810                 -shr eax, 0x10
    cpu.eax >>= 16 /*0x10*/ % 32;
    // 0051308e  6689442454             -mov word ptr [esp + 0x54], ax
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(84) /* 0x54 */) = cpu.ax;
    // 00513093  8b4178                 -mov eax, dword ptr [ecx + 0x78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(120) /* 0x78 */);
    // 00513096  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00513099  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 0051309b  c1e810                 -shr eax, 0x10
    cpu.eax >>= 16 /*0x10*/ % 32;
    // 0051309e  6689442456             -mov word ptr [esp + 0x56], ax
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(86) /* 0x56 */) = cpu.ax;
    // 005130a3  8b417c                 -mov eax, dword ptr [ecx + 0x7c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(124) /* 0x7c */);
    // 005130a6  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 005130a8  89442458               -mov dword ptr [esp + 0x58], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(88) /* 0x58 */) = cpu.eax;
    // 005130ac  8b8180000000           -mov eax, dword ptr [ecx + 0x80]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 005130b2  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 005130b4  8944245c               -mov dword ptr [esp + 0x5c], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(92) /* 0x5c */) = cpu.eax;
    // 005130b8  8b8184000000           -mov eax, dword ptr [ecx + 0x84]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(132) /* 0x84 */);
    // 005130be  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 005130c0  89442460               -mov dword ptr [esp + 0x60], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(96) /* 0x60 */) = cpu.eax;
    // 005130c4  8b4150                 -mov eax, dword ptr [ecx + 0x50]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(80) /* 0x50 */);
    // 005130c7  8d7c240c               -lea edi, [esp + 0xc]
    cpu.edi = x86::reg32(cpu.esp + x86::reg32(12) /* 0xc */);
    // 005130cb  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 005130cd  89442474               -mov dword ptr [esp + 0x74], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(116) /* 0x74 */) = cpu.eax;
    // 005130d1  8d5154                 -lea edx, [ecx + 0x54]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(84) /* 0x54 */);
    // 005130d4  8d751c                 -lea esi, [ebp + 0x1c]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 005130d7  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 005130dc  8d442430               -lea eax, [esp + 0x30]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(48) /* 0x30 */);
    // 005130e0  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005130e1  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005130e2  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005130e3  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005130e4  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005130e5  e846ddfcff             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005130ea  89ea                   -mov edx, ebp
    cpu.edx = cpu.ebp;
    // 005130ec  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x005130ee:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005130ee  40                     -inc eax
    (cpu.eax)++;
    // 005130ef  8a9a94000000           -mov bl, byte ptr [edx + 0x94]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(148) /* 0x94 */);
    // 005130f5  42                     -inc edx
    (cpu.edx)++;
    // 005130f6  885c0427               -mov byte ptr [esp + eax + 0x27], bl
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(39) /* 0x27 */ + cpu.eax * 1) = cpu.bl;
    // 005130fa  83f808                 +cmp eax, 8
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
    // 005130fd  7cef                   -jl 0x5130ee
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005130ee;
    }
    // 005130ff  8b858c000000           -mov eax, dword ptr [ebp + 0x8c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(140) /* 0x8c */);
    // 00513105  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513107  89442420               -mov dword ptr [esp + 0x20], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(32) /* 0x20 */) = cpu.eax;
    // 0051310b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051310d  668b8590000000         -mov ax, word ptr [ebp + 0x90]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(144) /* 0x90 */);
    // 00513114  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513116  c1e810                 -shr eax, 0x10
    cpu.eax >>= 16 /*0x10*/ % 32;
    // 00513119  6689442424             -mov word ptr [esp + 0x24], ax
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(36) /* 0x24 */) = cpu.ax;
    // 0051311e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00513120  668b8592000000         -mov ax, word ptr [ebp + 0x92]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(146) /* 0x92 */);
    // 00513127  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513129  c1e810                 -shr eax, 0x10
    cpu.eax >>= 16 /*0x10*/ % 32;
    // 0051312c  8b35e06d5600           -mov esi, dword ptr [0x566de0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
    // 00513132  6689442426             -mov word ptr [esp + 0x26], ax
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(38) /* 0x26 */) = cpu.ax;
    // 00513137  83fe03                 +cmp esi, 3
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
    // 0051313a  7c1e                   -jl 0x51315a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051315a;
    }
    // 0051313c  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051313e  b9ac000000             -mov ecx, 0xac
    cpu.ecx = 172 /*0xac*/;
    // 00513143  8d5c240c               -lea ebx, [esp + 0xc]
    cpu.ebx = x86::reg32(cpu.esp + x86::reg32(12) /* 0xc */);
    // 00513147  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513149  8d9424bc000000         -lea edx, [esp + 0xbc]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 00513150  b864025500             -mov eax, 0x550264
    cpu.eax = 5571172 /*0x550264*/;
    // 00513155  e826f0ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051315a:
    // 0051315a  68ac000000             -push 0xac
    app->getMemory<x86::reg32>(cpu.esp-4) = 172 /*0xac*/;
    cpu.esp -= 4;
    // 0051315f  8d4c240c               -lea ecx, [esp + 0xc]
    cpu.ecx = x86::reg32(cpu.esp + x86::reg32(12) /* 0xc */);
    // 00513163  8b9c24b8000000         -mov ebx, dword ptr [esp + 0xb8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 0051316a  8b7508                 -mov esi, dword ptr [ebp + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 0051316d  8b9424bc000000         -mov edx, dword ptr [esp + 0xbc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 00513174  8b450c                 -mov eax, dword ptr [ebp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 00513177  ff5654                 -call dword ptr [esi + 0x54]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(84) /* 0x54 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051317a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051317b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0051317c:
    // 0051317c  81c4b4000000           -add esp, 0xb4
    (cpu.esp) += x86::reg32(x86::sreg32(180 /*0xb4*/));
    // 00513182  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513183  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513184  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_513190(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00513190  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00513191  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00513192  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00513193  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00513196  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00513198  8954240c               -mov dword ptr [esp + 0xc], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.edx;
    // 0051319c  891c24                 -mov dword ptr [esp], ebx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ebx;
    // 0051319f  894c2408               -mov dword ptr [esp + 8], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.ecx;
    // 005131a3  c7420400000000         -mov dword ptr [edx + 4], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */) = 0 /*0x0*/;
    // 005131aa  894210                 -mov dword ptr [edx + 0x10], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 005131ad  a1dc895600             -mov eax, dword ptr [0x5689dc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5671388) /* 0x5689dc */);
    // 005131b2  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 005131b7  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
    // 005131b9  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 005131bb  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 005131bd  a3dc895600             -mov dword ptr [0x5689dc], eax
    app->getMemory<x86::reg32>(x86::reg32(5671388) /* 0x5689dc */) = cpu.eax;
    // 005131c2  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 005131c4  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 005131c6  8b15e06d5600           -mov edx, dword ptr [0x566de0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
    // 005131cc  8901                   -mov dword ptr [ecx], eax
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.eax;
    // 005131ce  83fa03                 +cmp edx, 3
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
    // 005131d1  7d4e                   -jge 0x513221
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00513221;
    }
L_0x005131d3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005131d3  8b4708                 -mov eax, dword ptr [edi + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8) /* 0x8 */);
    // 005131d6  83784800               +cmp dword ptr [eax + 0x48], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(72) /* 0x48 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005131da  740b                   -je 0x5131e7
    if (cpu.flags.zf)
    {
        goto L_0x005131e7;
    }
    // 005131dc  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 005131de  8d571c                 -lea edx, [edi + 0x1c]
    cpu.edx = x86::reg32(cpu.edi + x86::reg32(28) /* 0x1c */);
    // 005131e1  8b470c                 -mov eax, dword ptr [edi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(12) /* 0xc */);
    // 005131e4  ff5548                 -call dword ptr [ebp + 0x48]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(72) /* 0x48 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x005131e7:
    // 005131e7  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 005131eb  894718                 -mov dword ptr [edi + 0x18], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 005131ee  8b442424               -mov eax, dword ptr [esp + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(36) /* 0x24 */);
    // 005131f2  e8c9fdffff             -call 0x512fc0
    cpu.esp -= 4;
    sub_512fc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005131f7  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 005131fb  83780400               +cmp dword ptr [eax + 4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005131ff  7510                   -jne 0x513211
    if (!cpu.flags.zf)
    {
        goto L_0x00513211;
    }
    // 00513201  8d471c                 -lea eax, [edi + 0x1c]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(28) /* 0x1c */);
    // 00513204  89442404               -mov dword ptr [esp + 4], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.eax;
L_0x00513208:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00513208  e8c3fdffff             -call 0x512fd0
    cpu.esp -= 4;
    sub_512fd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051320d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051320f  742c                   -je 0x51323d
    if (cpu.flags.zf)
    {
        goto L_0x0051323d;
    }
L_0x00513211:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00513211  c7471800000000         -mov dword ptr [edi + 0x18], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(24) /* 0x18 */) = 0 /*0x0*/;
    // 00513218  83c410                 +add esp, 0x10
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
    // 0051321b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051321c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051321d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051321e  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x00513221:
    // 00513221  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513223  8b4c2424               -mov ecx, dword ptr [esp + 0x24]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(36) /* 0x24 */);
    // 00513227  8b5c240c               -mov ebx, dword ptr [esp + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0051322b  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051322d  8d542408               -lea edx, [esp + 8]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 00513231  b870025500             -mov eax, 0x550270
    cpu.eax = 5571184 /*0x550270*/;
    // 00513236  e845efffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051323b  eb96                   -jmp 0x5131d3
    goto L_0x005131d3;
L_0x0051323d:
    // 0051323d  8b442420               -mov eax, dword ptr [esp + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(32) /* 0x20 */);
    // 00513241  8b4c2408               -mov ecx, dword ptr [esp + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 00513245  8b1c24                 -mov ebx, dword ptr [esp]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    // 00513248  8b6f08                 -mov ebp, dword ptr [edi + 8]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8) /* 0x8 */);
    // 0051324b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051324c  8b542408               -mov edx, dword ptr [esp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 00513250  8b470c                 -mov eax, dword ptr [edi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(12) /* 0xc */);
    // 00513253  ff5554                 -call dword ptr [ebp + 0x54]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(84) /* 0x54 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513256  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513258  7406                   -je 0x513260
    if (cpu.flags.zf)
    {
        goto L_0x00513260;
    }
    // 0051325a  8b35dc6d5600           -mov esi, dword ptr [0x566ddc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5664220) /* 0x566ddc */);
L_0x00513260:
    // 00513260  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00513262  e8c9c6fcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513267  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513269  7521                   -jne 0x51328c
    if (!cpu.flags.zf)
    {
        goto L_0x0051328c;
    }
    // 0051326b  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051326d  e86ec6fcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00513272:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00513272  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 00513276  83780400               +cmp dword ptr [eax + 4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051327a  748c                   -je 0x513208
    if (cpu.flags.zf)
    {
        goto L_0x00513208;
    }
    // 0051327c  c7471800000000         -mov dword ptr [edi + 0x18], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(24) /* 0x18 */) = 0 /*0x0*/;
    // 00513283  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00513286  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513287  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513288  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513289  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x0051328c:
    // 0051328c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051328e  e88d44fdff             -call 0x4e7720
    cpu.esp -= 4;
    nfs3hp::port::waitSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513293  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513295  0f8c76ffffff           -jl 0x513211
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00513211;
    }
    // 0051329b  ebd5                   -jmp 0x513272
    goto L_0x00513272;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_5132a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005132a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 005132a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005132a2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005132a3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005132a4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005132a5  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 005132a8  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 005132aa  89d5                   -mov ebp, edx
    cpu.ebp = cpu.edx;
    // 005132ac  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 005132ae  8db088000000           -lea esi, [eax + 0x88]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(136) /* 0x88 */);
L_0x005132b4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005132b4  891424                 -mov dword ptr [esp], edx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.edx;
    // 005132b7  8b8784000000           -mov eax, dword ptr [edi + 0x84]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(132) /* 0x84 */);
    // 005132bd  8b0c24                 -mov ecx, dword ptr [esp]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    // 005132c0  48                     -dec eax
    (cpu.eax)--;
    // 005132c1  39c8                   +cmp eax, ecx
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
    // 005132c3  7645                   -jbe 0x51330a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051330a;
    }
    // 005132c5  8a0c24                 -mov cl, byte ptr [esp]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esp);
    // 005132c8  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 005132cd  8b5f50                 -mov ebx, dword ptr [edi + 0x50]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(80) /* 0x50 */);
    // 005132d0  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 005132d2  85d8                   +test eax, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.ebx));
    // 005132d4  7509                   -jne 0x5132df
    if (!cpu.flags.zf)
    {
        goto L_0x005132df;
    }
L_0x005132d6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005132d6  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 005132d9  42                     -inc edx
    (cpu.edx)++;
    // 005132da  83c664                 +add esi, 0x64
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(100 /*0x64*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005132dd  ebd5                   -jmp 0x5132b4
    goto L_0x005132b4;
L_0x005132df:
    // 005132df  6b0c2464               -imul ecx, dword ptr [esp], 0x64
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp))) * x86::sreg64(x86::sreg32(100 /*0x64*/)));
    // 005132e3  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 005132e5  89ea                   -mov edx, ebp
    cpu.edx = cpu.ebp;
    // 005132e7  8b4704                 -mov eax, dword ptr [edi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 005132ea  e8511dfeff             -call 0x4f5040
    cpu.esp -= 4;
    nfs3hp::ea::netCheckPlayer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005132ef  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005132f1  74e3                   -je 0x5132d6
    if (cpu.flags.zf)
    {
        goto L_0x005132d6;
    }
    // 005132f3  a1d46d5600             -mov eax, dword ptr [0x566dd4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664212) /* 0x566dd4 */);
    // 005132f8  898439e4000000         -mov dword ptr [ecx + edi + 0xe4], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(228) /* 0xe4 */ + cpu.edi * 1) = cpu.eax;
    // 005132ff  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00513301  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00513304  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513305  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513306  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513307  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513308  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513309  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051330a:
    // 0051330a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051330c  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051330f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513310  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513311  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513312  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513313  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513314  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_513320(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00513320  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00513321  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00513322  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00513323  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00513325  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 00513327  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00513329:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00513329  8b9384000000           -mov edx, dword ptr [ebx + 0x84]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(132) /* 0x84 */);
    // 0051332f  4a                     -dec edx
    (cpu.edx)--;
    // 00513330  39d0                   +cmp eax, edx
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
    // 00513332  7356                   -jae 0x51338a
    if (!cpu.flags.cf)
    {
        goto L_0x0051338a;
    }
    // 00513334  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00513339  88c1                   -mov cl, al
    cpu.cl = cpu.al;
    // 0051333b  d3e2                   -shl edx, cl
    cpu.edx <<= cpu.cl % 32;
    // 0051333d  8b4b50                 -mov ecx, dword ptr [ebx + 0x50]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(80) /* 0x50 */);
    // 00513340  85ca                   +test edx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.ecx));
    // 00513342  7403                   -je 0x513347
    if (cpu.flags.zf)
    {
        goto L_0x00513347;
    }
    // 00513344  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00513345  ebe2                   -jmp 0x513329
    goto L_0x00513329;
L_0x00513347:
    // 00513347  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00513348  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 0051334a  09d7                   -or edi, edx
    cpu.edi |= x86::reg32(x86::sreg32(cpu.edx));
    // 0051334c  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 00513353  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00513355  c1e203                 -shl edx, 3
    cpu.edx <<= 3 /*0x3*/ % 32;
    // 00513358  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051335a  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 0051335d  897b50                 -mov dword ptr [ebx + 0x50], edi
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(80) /* 0x50 */) = cpu.edi;
    // 00513360  01d3                   -add ebx, edx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edx));
    // 00513362  b917000000             -mov ecx, 0x17
    cpu.ecx = 23 /*0x17*/;
    // 00513367  8dbb88000000           -lea edi, [ebx + 0x88]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(136) /* 0x88 */);
    // 0051336d  8b15d46d5600           -mov edx, dword ptr [0x566dd4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5664212) /* 0x566dd4 */);
    // 00513373  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00513375  8993e4000000           -mov dword ptr [ebx + 0xe4], edx
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(228) /* 0xe4 */) = cpu.edx;
    // 0051337b  8983e8000000           -mov dword ptr [ebx + 0xe8], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(232) /* 0xe8 */) = cpu.eax;
    // 00513381  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00513383  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513384  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00513386  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513387  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513388  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513389  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051338a:
    // 0051338a  687c025500             -push 0x55027c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5571196 /*0x55027c*/;
    cpu.esp -= 4;
    // 0051338f  e85cd5fcff             -call 0x4e08f0
    cpu.esp -= 4;
    nfs3hp::ea::monoPrintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513394  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00513396  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00513399  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051339b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051339c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051339d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051339e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_5133a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005133a0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005133a1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005133a2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005133a3  81ecd0000000           -sub esp, 0xd0
    (cpu.esp) -= x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 005133a9  8bbc24e0000000         -mov edi, dword ptr [esp + 0xe0]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(224) /* 0xe0 */);
    // 005133b0  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 005133b2  899424cc000000         -mov dword ptr [esp + 0xcc], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */) = cpu.edx;
    // 005133b9  899c24ac000000         -mov dword ptr [esp + 0xac], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(172) /* 0xac */) = cpu.ebx;
    // 005133c0  898c24b8000000         -mov dword ptr [esp + 0xb8], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */) = cpu.ecx;
    // 005133c7  8b6810                 -mov ebp, dword ptr [eax + 0x10]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 005133ca  81fb216f6eea           +cmp ebx, 0xea6e6f21
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3933105953 /*0xea6e6f21*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005133d0  7331                   -jae 0x513403
    if (!cpu.flags.cf)
    {
        goto L_0x00513403;
    }
    // 005133d2  81fb657962ea           +cmp ebx, 0xea627965
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3932322149 /*0xea627965*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005133d8  0f835e020000           -jae 0x51363c
    if (!cpu.flags.cf)
    {
        goto L_0x0051363c;
    }
    // 005133de  81fb646461ea           +cmp ebx, 0xea616464
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3932251236 /*0xea616464*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005133e4  0f83a9030000           -jae 0x513793
    if (!cpu.flags.cf)
    {
        goto L_0x00513793;
    }
L_0x005133ea:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005133ea  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005133f1  0f8dbd0c0000           -jge 0x5140b4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x005140b4;
    }
L_0x005133f7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005133f7  81c4d0000000           +add esp, 0xd0
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(208 /*0xd0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005133fd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005133fe  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005133ff  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513400  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513403:
    // 00513403  772c                   -ja 0x513431
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00513431;
    }
    // 00513405  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051340c  7ce9                   -jl 0x5133f7
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005133f7;
    }
    // 0051340e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00513410  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00513411  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 00513416  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 00513418  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00513419  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051341b  b880035500             -mov eax, 0x550380
    cpu.eax = 5571456 /*0x550380*/;
    // 00513420  e85bedffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513425  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 0051342b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051342c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051342d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051342e  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513431:
    // 00513431  81fb796c70ea           +cmp ebx, 0xea706c79
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3933236345 /*0xea706c79*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513437  0f83a0000000           -jae 0x5134dd
    if (!cpu.flags.cf)
    {
        goto L_0x005134dd;
    }
    // 0051343d  81fb6e706fea           +cmp ebx, 0xea6f706e
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3933171822 /*0xea6f706e*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513443  0f8353010000           -jae 0x51359c
    if (!cpu.flags.cf)
    {
        goto L_0x0051359c;
    }
    // 00513449  81fb616c6fea           +cmp ebx, 0xea6f6c61
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3933170785 /*0xea6f6c61*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051344f  7599                   -jne 0x5133ea
    if (!cpu.flags.zf)
    {
        goto L_0x005133ea;
    }
    // 00513451  8b4d14                 -mov ecx, dword ptr [ebp + 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00513454  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00513456  0f85900b0000           -jne 0x513fec
    if (!cpu.flags.zf)
    {
        goto L_0x00513fec;
    }
    // 0051345c  83bda400000000         +cmp dword ptr [ebp + 0xa4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(164) /* 0xa4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513463  7492                   -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 00513465  83bda000000000         +cmp dword ptr [ebp + 0xa0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(160) /* 0xa0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051346c  7489                   -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 0051346e  8d5d1c                 -lea ebx, [ebp + 0x1c]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 00513471  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00513473  e8c81bfeff             -call 0x4f5040
    cpu.esp -= 4;
    nfs3hp::ea::netCheckPlayer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513478  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051347a  0f8477ffffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 00513480  bf616c6fea             -mov edi, 0xea6f6c61
    cpu.edi = 3933170785 /*0xea6f6c61*/;
    // 00513485  a1d46d5600             -mov eax, dword ptr [0x566dd4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664212) /* 0x566dd4 */);
    // 0051348a  89bc24b0000000         -mov dword ptr [esp + 0xb0], edi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(176) /* 0xb0 */) = cpu.edi;
    // 00513491  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00513492  8985a4000000           -mov dword ptr [ebp + 0xa4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(164) /* 0xa4 */) = cpu.eax;
    // 00513498  8d551c                 -lea edx, [ebp + 0x1c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 0051349b  8b7508                 -mov esi, dword ptr [ebp + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 0051349e  8b9c24b4000000         -mov ebx, dword ptr [esp + 0xb4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(180) /* 0xb4 */);
    // 005134a5  8b450c                 -mov eax, dword ptr [ebp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 005134a8  ff5654                 -call dword ptr [esi + 0x54]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(84) /* 0x54 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005134ab  833de06d560004         +cmp dword ptr [0x566de0], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005134b2  0f8c3fffffff           -jl 0x5133f7
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005133f7;
    }
    // 005134b8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005134ba  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 005134bb  8d9424b0000000         -lea edx, [esp + 0xb0]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 005134c2  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 005134c4  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 005134c5  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 005134c7  b898035500             -mov eax, 0x550398
    cpu.eax = 5571480 /*0x550398*/;
    // 005134cc  e8afecffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005134d1  81c4d0000000           +add esp, 0xd0
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(208 /*0xd0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 005134d7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005134d8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005134d9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005134da  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x005134dd:
    // 005134dd  0f86c9030000           -jbe 0x5138ac
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x005138ac;
    }
    // 005134e3  81fb707372ea           +cmp ebx, 0xea727370
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3933369200 /*0xea727370*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005134e9  7358                   -jae 0x513543
    if (!cpu.flags.cf)
    {
        goto L_0x00513543;
    }
    // 005134eb  81fb716572ea           +cmp ebx, 0xea726571
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3933365617 /*0xea726571*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005134f1  0f85f3feffff           -jne 0x5133ea
    if (!cpu.flags.zf)
    {
        goto L_0x005133ea;
    }
    // 005134f7  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 005134f9  0f84f8feffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 005134ff  837d1400               +cmp dword ptr [ebp + 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513503  0f84eefeffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 00513509  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 0051350e  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513515  8b9424cc000000         -mov edx, dword ptr [esp + 0xcc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 0051351c  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 0051351e  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513520  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513522  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513529  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 0051352b  8b4d14                 -mov ecx, dword ptr [ebp + 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 0051352e  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00513530  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00513532  e8f9faffff             -call 0x513030
    cpu.esp -= 4;
    sub_513030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513537  81c4d0000000           +add esp, 0xd0
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(208 /*0xd0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051353d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051353e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051353f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513540  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513543:
    // 00513543  0f8678090000           -jbe 0x513ec1
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00513ec1;
    }
    // 00513549  81fb767273ea           +cmp ebx, 0xea737276
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3933434486 /*0xea737276*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051354f  0f8595feffff           -jne 0x5133ea
    if (!cpu.flags.zf)
    {
        goto L_0x005133ea;
    }
    // 00513555  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 00513557  0f849afeffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 0051355d  8b7514                 -mov esi, dword ptr [ebp + 0x14]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00513560  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00513562  0f848ffeffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 00513568  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 0051356d  8d460c                 -lea eax, [esi + 0xc]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(12) /* 0xc */);
    // 00513570  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00513572  e8197efdff             -call 0x4eb390
    cpu.esp -= 4;
    nfs3hp::watcom::strncmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513577  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513579  0f8578feffff           -jne 0x5133f7
    if (!cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 0051357f  8b9424cc000000         -mov edx, dword ptr [esp + 0xcc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 00513586  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00513589  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051358b  e8a0faffff             -call 0x513030
    cpu.esp -= 4;
    sub_513030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513590  81c4d0000000           +add esp, 0xd0
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(208 /*0xd0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00513596  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513597  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513598  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513599  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x0051359c:
    // 0051359c  7762                   -ja 0x513600
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00513600;
    }
    // 0051359e  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005135a5  7c17                   -jl 0x5135be
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005135be;
    }
    // 005135a7  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 005135a9  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 005135ae  b878035500             -mov eax, 0x550378
    cpu.eax = 5571448 /*0x550378*/;
    // 005135b3  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 005135b5  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 005135b7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 005135b9  e8c2ebffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x005135be:
    // 005135be  837d1000               +cmp dword ptr [ebp + 0x10], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005135c2  0f842ffeffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 005135c8  8b9424cc000000         -mov edx, dword ptr [esp + 0xcc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 005135cf  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 005135d1  ff5510                 -call dword ptr [ebp + 0x10]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005135d4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005135d6  0f851bfeffff           -jne 0x5133f7
    if (!cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 005135dc  bb216f6eea             -mov ebx, 0xea6e6f21
    cpu.ebx = 3933105953 /*0xea6e6f21*/;
    // 005135e1  8b9424cc000000         -mov edx, dword ptr [esp + 0xcc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 005135e8  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 005135e9  8b7508                 -mov esi, dword ptr [ebp + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 005135ec  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 005135ee  8b450c                 -mov eax, dword ptr [ebp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 005135f1  ff5654                 -call dword ptr [esi + 0x54]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(84) /* 0x54 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005135f4  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 005135fa  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005135fb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005135fc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005135fd  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513600:
    // 00513600  81fb74756fea           +cmp ebx, 0xea6f7574
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3933173108 /*0xea6f7574*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513606  0f85defdffff           -jne 0x5133ea
    if (!cpu.flags.zf)
    {
        goto L_0x005133ea;
    }
L_0x0051360c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051360c  833de06d560002         +cmp dword ptr [0x566de0], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513613  0f8d230a0000           -jge 0x51403c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051403c;
    }
L_0x00513619:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00513619  8b4d14                 -mov ecx, dword ptr [ebp + 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 0051361c  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0051361e  0f853b0a0000           -jne 0x51405f
    if (!cpu.flags.zf)
    {
        goto L_0x0051405f;
    }
    // 00513624  898da4000000           -mov dword ptr [ebp + 0xa4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(164) /* 0xa4 */) = cpu.ecx;
    // 0051362a  898da0000000           -mov dword ptr [ebp + 0xa0], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(160) /* 0xa0 */) = cpu.ecx;
    // 00513630  81c4d0000000           +add esp, 0xd0
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(208 /*0xd0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00513636  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513637  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513638  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513639  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x0051363c:
    // 0051363c  76ce                   -jbe 0x51360c
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051360c;
    }
    // 0051363e  81fb636564ea           +cmp ebx, 0xea646563
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3932448099 /*0xea646563*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513644  734d                   -jae 0x513693
    if (!cpu.flags.cf)
    {
        goto L_0x00513693;
    }
    // 00513646  81fb746164ea           +cmp ebx, 0xea646174
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3932447092 /*0xea646174*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051364c  0f8598fdffff           -jne 0x5133ea
    if (!cpu.flags.zf)
    {
        goto L_0x005133ea;
    }
    // 00513652  8b7d14                 -mov edi, dword ptr [ebp + 0x14]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00513655  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00513657  0f85cc090000           -jne 0x514029
    if (!cpu.flags.zf)
    {
        goto L_0x00514029;
    }
    // 0051365d  8d5d1c                 -lea ebx, [ebp + 0x1c]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 00513660  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00513662  e8d919feff             -call 0x4f5040
    cpu.esp -= 4;
    nfs3hp::ea::netCheckPlayer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513667  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513669  0f8488fdffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 0051366f  83bda000000000         +cmp dword ptr [ebp + 0xa0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(160) /* 0xa0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513676  0f847bfdffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 0051367c  a1d86d5600             -mov eax, dword ptr [0x566dd8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664216) /* 0x566dd8 */);
    // 00513681  8985a0000000           -mov dword ptr [ebp + 0xa0], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(160) /* 0xa0 */) = cpu.eax;
    // 00513687  81c4d0000000           +add esp, 0xd0
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(208 /*0xd0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051368d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051368e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051368f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513690  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513693:
    // 00513693  0f8624030000           -jbe 0x5139bd
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x005139bd;
    }
    // 00513699  81fb746567ea           +cmp ebx, 0xea676574
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3932644724 /*0xea676574*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051369f  0f8545fdffff           -jne 0x5133ea
    if (!cpu.flags.zf)
    {
        goto L_0x005133ea;
    }
    // 005136a5  837d1400               +cmp dword ptr [ebp + 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005136a9  0f8448fdffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 005136af  bf796c70ea             -mov edi, 0xea706c79
    cpu.edi = 3933236345 /*0xea706c79*/;
    // 005136b4  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 005136b6  89bc24b0000000         -mov dword ptr [esp + 0xb0], edi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(176) /* 0xb0 */) = cpu.edi;
    // 005136bd  8d7c2404               -lea edi, [esp + 4]
    cpu.edi = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 005136c1  8d7604                 -lea esi, [esi + 4]
    cpu.esi = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 005136c4  b878000000             -mov eax, 0x78
    cpu.eax = 120 /*0x78*/;
    // 005136c9  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005136ca  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005136cb  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005136cc  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005136cd  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005136ce  898424c8000000         -mov dword ptr [esp + 0xc8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(200) /* 0xc8 */) = cpu.eax;
    // 005136d5  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 005136d7  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 005136da  8b4118                 -mov eax, dword ptr [ecx + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */);
    // 005136dd  89442418               -mov dword ptr [esp + 0x18], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 005136e1  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 005136e8  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 005136ed  83c018                 -add eax, 0x18
    (cpu.eax) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 005136f0  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 005136f2  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 005136f4  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 005136f6  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 005136fd  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 005136ff  83f8ff                 +cmp eax, -1
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
    // 00513702  0f8439050000           -je 0x513c41
    if (cpu.flags.zf)
    {
        goto L_0x00513c41;
    }
    // 00513708  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051370d  88c1                   -mov cl, al
    cpu.cl = cpu.al;
    // 0051370f  8b5d14                 -mov ebx, dword ptr [ebp + 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00513712  d3e2                   +shl edx, cl
    {
        x86::reg8 tmp = cpu.cl % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00513714  855350                 -test dword ptr [ebx + 0x50], edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(80) /* 0x50 */) & cpu.edx));
    // 00513717  0f8435050000           -je 0x513c52
    if (cpu.flags.zf)
    {
        goto L_0x00513c52;
    }
    // 0051371d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051371f  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00513722  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00513724  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00513727  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 00513729  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0051372b  b917000000             -mov ecx, 0x17
    cpu.ecx = 23 /*0x17*/;
    // 00513730  8d7c241c               -lea edi, [esp + 0x1c]
    cpu.edi = x86::reg32(cpu.esp + x86::reg32(28) /* 0x1c */);
    // 00513734  8db48688000000         -lea esi, [esi + eax*4 + 0x88]
    cpu.esi = x86::reg32(cpu.esi + x86::reg32(136) /* 0x88 */ + cpu.eax * 4);
L_0x0051373b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051373b  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x0051373d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051373d  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513744  7c20                   -jl 0x513766
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00513766;
    }
    // 00513746  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513748  8b8c24cc000000         -mov ecx, dword ptr [esp + 0xcc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 0051374f  8d5c2404               -lea ebx, [esp + 4]
    cpu.ebx = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00513753  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513755  8d9424b8000000         -lea edx, [esp + 0xb8]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 0051375c  b8dc025500             -mov eax, 0x5502dc
    cpu.eax = 5571292 /*0x5502dc*/;
    // 00513761  e81aeaffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00513766:
    // 00513766  8b8424c8000000         -mov eax, dword ptr [esp + 0xc8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(200) /* 0xc8 */);
    // 0051376d  89e1                   -mov ecx, esp
    cpu.ecx = cpu.esp;
    // 0051376f  8b9c24b0000000         -mov ebx, dword ptr [esp + 0xb0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 00513776  8b7508                 -mov esi, dword ptr [ebp + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 00513779  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051377a  8b9424d0000000         -mov edx, dword ptr [esp + 0xd0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(208) /* 0xd0 */);
    // 00513781  8b450c                 -mov eax, dword ptr [ebp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 00513784  ff5654                 -call dword ptr [esi + 0x54]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(84) /* 0x54 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513787  81c4d0000000           +add esp, 0xd0
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(208 /*0xd0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051378d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051378e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051378f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513790  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513793:
    // 00513793  0f8707010000           -ja 0x5138a0
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x005138a0;
    }
    // 00513799  8b5514                 -mov edx, dword ptr [ebp + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 0051379c  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051379e  0f8453fcffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 005137a4  8b4a08                 -mov ecx, dword ptr [edx + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 005137a7  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 005137a9  0f8548fcffff           -jne 0x5133f7
    if (!cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 005137af  8b9424cc000000         -mov edx, dword ptr [esp + 0xcc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 005137b6  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 005137b9  bb707372ea             -mov ebx, 0xea727370
    cpu.ebx = 3933369200 /*0xea727370*/;
    // 005137be  e8ddfaffff             -call 0x5132a0
    cpu.esp -= 4;
    sub_5132a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005137c3  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 005137c5  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 005137cc  899c24b0000000         -mov dword ptr [esp + 0xb0], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(176) /* 0xb0 */) = cpu.ebx;
    // 005137d3  be20000000             -mov esi, 0x20
    cpu.esi = 32 /*0x20*/;
    // 005137d8  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 005137da  89b424bc000000         -mov dword ptr [esp + 0xbc], esi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */) = cpu.esi;
    // 005137e1  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 005137e4  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 005137e6  0f84fb040000           -je 0x513ce7
    if (cpu.flags.zf)
    {
        goto L_0x00513ce7;
    }
    // 005137ec  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 005137f1  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 005137f3  89442418               -mov dword ptr [esp + 0x18], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 005137f7  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 005137fa  0588000000             -add eax, 0x88
    (cpu.eax) += x86::reg32(x86::sreg32(136 /*0x88*/));
    // 005137ff  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00513801  b964000000             -mov ecx, 0x64
    cpu.ecx = 100 /*0x64*/;
    // 00513806  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00513808  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051380b  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0051380d  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 0051380f  8b3de06d5600           -mov edi, dword ptr [0x566de0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
    // 00513815  8944241c               -mov dword ptr [esp + 0x1c], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(28) /* 0x1c */) = cpu.eax;
    // 00513819  83ff04                 +cmp edi, 4
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
    // 0051381c  7c21                   -jl 0x51383f
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051383f;
    }
    // 0051381e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00513820  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00513821  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 00513826  8b9c24d0000000         -mov ebx, dword ptr [esp + 0xd0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(208) /* 0xd0 */);
    // 0051382d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051382e  8d9424b8000000         -lea edx, [esp + 0xb8]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513835  b83c035500             -mov eax, 0x55033c
    cpu.eax = 5571388 /*0x55033c*/;
L_0x0051383a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051383a  e841e9ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051383f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051383f  8b8424bc000000         -mov eax, dword ptr [esp + 0xbc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 00513846  89e1                   -mov ecx, esp
    cpu.ecx = cpu.esp;
    // 00513848  8b9c24b0000000         -mov ebx, dword ptr [esp + 0xb0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 0051384f  8b7508                 -mov esi, dword ptr [ebp + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 00513852  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00513853  8b9424d0000000         -mov edx, dword ptr [esp + 0xd0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(208) /* 0xd0 */);
    // 0051385a  8b450c                 -mov eax, dword ptr [ebp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 0051385d  ff5654                 -call dword ptr [esi + 0x54]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(84) /* 0x54 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513860  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513862  0f858ffbffff           -jne 0x5133f7
    if (!cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 00513868  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051386f  0f8c82fbffff           -jl 0x5133f7
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005133f7;
    }
    // 00513875  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00513876  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 0051387b  8b9c24d0000000         -mov ebx, dword ptr [esp + 0xd0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(208) /* 0xd0 */);
    // 00513882  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00513883  8d9424b8000000         -lea edx, [esp + 0xb8]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 0051388a  b848035500             -mov eax, 0x550348
    cpu.eax = 5571400 /*0x550348*/;
    // 0051388f  e8ece8ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513894  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 0051389a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051389b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051389c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051389d  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x005138a0:
    // 005138a0  81fb646162ea           +cmp ebx, 0xea626164
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3932316004 /*0xea626164*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005138a6  0f853efbffff           -jne 0x5133ea
    if (!cpu.flags.zf)
    {
        goto L_0x005133ea;
    }
L_0x005138ac:
    // 005138ac  8d5d1c                 -lea ebx, [ebp + 0x1c]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 005138af  8b9424cc000000         -mov edx, dword ptr [esp + 0xcc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 005138b6  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 005138b8  e88317feff             -call 0x4f5040
    cpu.esp -= 4;
    nfs3hp::ea::netCheckPlayer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005138bd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005138bf  0f84ed030000           -je 0x513cb2
    if (cpu.flags.zf)
    {
        goto L_0x00513cb2;
    }
    // 005138c5  83bda000000000         +cmp dword ptr [ebp + 0xa0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(160) /* 0xa0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005138cc  740b                   -je 0x5138d9
    if (cpu.flags.zf)
    {
        goto L_0x005138d9;
    }
    // 005138ce  a1d86d5600             -mov eax, dword ptr [0x566dd8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664216) /* 0x566dd8 */);
    // 005138d3  8985a0000000           -mov dword ptr [ebp + 0xa0], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(160) /* 0xa0 */) = cpu.eax;
L_0x005138d9:
    // 005138d9  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 005138de  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 005138e5  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 005138e7  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 005138e9  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 005138eb  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 005138f2  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 005138f4  837e1800               +cmp dword ptr [esi + 0x18], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005138f8  0f847d030000           -je 0x513c7b
    if (cpu.flags.zf)
    {
        goto L_0x00513c7b;
    }
    // 005138fe  3b06                   +cmp eax, dword ptr [esi]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513900  0f8575030000           -jne 0x513c7b
    if (!cpu.flags.zf)
    {
        goto L_0x00513c7b;
    }
    // 00513906  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051390d  7c1e                   -jl 0x51392d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051392d;
    }
    // 0051390f  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513911  8b9c24bc000000         -mov ebx, dword ptr [esp + 0xbc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 00513918  8d9424b0000000         -lea edx, [esp + 0xb0]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 0051391f  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513921  b8e8025500             -mov eax, 0x5502e8
    cpu.eax = 5571304 /*0x5502e8*/;
    // 00513926  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 00513928  e853e8ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051392d:
    // 0051392d  81bc24ac000000796c70ea +cmp dword ptr [esp + 0xac], 0xea706c79
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(172) /* 0xac */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3933236345 /*0xea706c79*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513938  756a                   -jne 0x5139a4
    if (!cpu.flags.zf)
    {
        goto L_0x005139a4;
    }
    // 0051393a  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513941  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 00513946  83c018                 -add eax, 0x18
    (cpu.eax) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 00513949  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 0051394b  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 0051394d  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051394f  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513956  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513958  83f8ff                 +cmp eax, -1
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
    // 0051395b  0f8513030000           -jne 0x513c74
    if (!cpu.flags.zf)
    {
        goto L_0x00513c74;
    }
    // 00513961  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00513966:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00513966  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 0051396d  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 00513972  83c018                 -add eax, 0x18
    (cpu.eax) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 00513975  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513977  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513979  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051397b  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513982  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513984  8d5801                 -lea ebx, [eax + 1]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 00513987  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 0051398e  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00513990  83c01c                 -add eax, 0x1c
    (cpu.eax) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 00513993  8b7e10                 -mov edi, dword ptr [esi + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 00513996  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00513998  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051399a  ff5618                 -call dword ptr [esi + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051399d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051399f  7503                   -jne 0x5139a4
    if (!cpu.flags.zf)
    {
        goto L_0x005139a4;
    }
    // 005139a1  894618                 -mov dword ptr [esi + 0x18], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */) = cpu.eax;
L_0x005139a4:
    // 005139a4  c70600000000           -mov dword ptr [esi], 0
    app->getMemory<x86::reg32>(cpu.esi) = 0 /*0x0*/;
    // 005139aa  c7460401000000         -mov dword ptr [esi + 4], 1
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = 1 /*0x1*/;
    // 005139b1  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 005139b7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005139b8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005139b9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005139ba  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x005139bd:
    // 005139bd  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 005139c2  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 005139c9  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 005139cb  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 005139cd  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 005139cf  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 005139d6  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 005139d8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 005139da  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005139dc  0f8551020000           -jne 0x513c33
    if (!cpu.flags.zf)
    {
        goto L_0x00513c33;
    }
    // 005139e2  833e00                 +cmp dword ptr [esi], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005139e5  0f8548020000           -jne 0x513c33
    if (!cpu.flags.zf)
    {
        goto L_0x00513c33;
    }
    // 005139eb  837e1400               +cmp dword ptr [esi + 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005139ef  0f843e020000           -je 0x513c33
    if (cpu.flags.zf)
    {
        goto L_0x00513c33;
    }
    // 005139f5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x005139fa:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005139fa  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 005139fc  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005139fe  753c                   -jne 0x513a3c
    if (!cpu.flags.zf)
    {
        goto L_0x00513a3c;
    }
    // 00513a00  837e1400               +cmp dword ptr [esi + 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513a04  0f8530020000           -jne 0x513c3a
    if (!cpu.flags.zf)
    {
        goto L_0x00513c3a;
    }
    // 00513a0a  8b0e                   -mov ecx, dword ptr [esi]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi);
    // 00513a0c  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00513a0e  0f8426020000           -je 0x513c3a
    if (cpu.flags.zf)
    {
        goto L_0x00513c3a;
    }
    // 00513a14  39ca                   +cmp edx, ecx
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
    // 00513a16  0f851e020000           -jne 0x513c3a
    if (!cpu.flags.zf)
    {
        goto L_0x00513c3a;
    }
    // 00513a1c  8d5d1c                 -lea ebx, [ebp + 0x1c]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 00513a1f  8b9424cc000000         -mov edx, dword ptr [esp + 0xcc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 00513a26  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00513a28  e81316feff             -call 0x4f5040
    cpu.esp -= 4;
    nfs3hp::ea::netCheckPlayer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513a2d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513a2f  0f8405020000           -je 0x513c3a
    if (cpu.flags.zf)
    {
        goto L_0x00513c3a;
    }
    // 00513a35  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x00513a3a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00513a3a  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
L_0x00513a3c:
    // 00513a3c  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00513a3e  0f84b3f9ffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 00513a44  83bda000000000         +cmp dword ptr [ebp + 0xa0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(160) /* 0xa0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513a4b  7411                   -je 0x513a5e
    if (cpu.flags.zf)
    {
        goto L_0x00513a5e;
    }
    // 00513a4d  837e1400               +cmp dword ptr [esi + 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513a51  750b                   -jne 0x513a5e
    if (!cpu.flags.zf)
    {
        goto L_0x00513a5e;
    }
    // 00513a53  a1d86d5600             -mov eax, dword ptr [0x566dd8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664216) /* 0x566dd8 */);
    // 00513a58  8985a0000000           -mov dword ptr [ebp + 0xa0], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(160) /* 0xa0 */) = cpu.eax;
L_0x00513a5e:
    // 00513a5e  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513a65  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00513a6a  83c048                 -add eax, 0x48
    (cpu.eax) += x86::reg32(x86::sreg32(72 /*0x48*/));
    // 00513a6d  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513a6f  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513a71  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513a73  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513a7a  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513a7c  88463c                 -mov byte ptr [esi + 0x3c], al
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(60) /* 0x3c */) = cpu.al;
    // 00513a7f  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513a86  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00513a8b  83c049                 -add eax, 0x49
    (cpu.eax) += x86::reg32(x86::sreg32(73 /*0x49*/));
    // 00513a8e  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513a90  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513a92  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513a94  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513a9b  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513a9d  88463d                 -mov byte ptr [esi + 0x3d], al
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(61) /* 0x3d */) = cpu.al;
    // 00513aa0  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513aa7  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00513aac  83c04a                 -add eax, 0x4a
    (cpu.eax) += x86::reg32(x86::sreg32(74 /*0x4a*/));
    // 00513aaf  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513ab1  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513ab3  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513ab5  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513abc  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513abe  88463e                 -mov byte ptr [esi + 0x3e], al
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(62) /* 0x3e */) = cpu.al;
    // 00513ac1  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513ac8  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00513acd  83c04b                 -add eax, 0x4b
    (cpu.eax) += x86::reg32(x86::sreg32(75 /*0x4b*/));
    // 00513ad0  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513ad2  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513ad4  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513ad6  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513add  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513adf  88463f                 -mov byte ptr [esi + 0x3f], al
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(63) /* 0x3f */) = cpu.al;
    // 00513ae2  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513ae9  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 00513aee  83c04c                 -add eax, 0x4c
    (cpu.eax) += x86::reg32(x86::sreg32(76 /*0x4c*/));
    // 00513af1  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513af3  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513af5  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513af7  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513afe  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513b00  66894640               -mov word ptr [esi + 0x40], ax
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(64) /* 0x40 */) = cpu.ax;
    // 00513b04  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513b0b  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 00513b10  83c04e                 -add eax, 0x4e
    (cpu.eax) += x86::reg32(x86::sreg32(78 /*0x4e*/));
    // 00513b13  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513b15  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513b17  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513b19  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513b20  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513b22  66894642               -mov word ptr [esi + 0x42], ax
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(66) /* 0x42 */) = cpu.ax;
    // 00513b26  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513b2d  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 00513b32  83c050                 -add eax, 0x50
    (cpu.eax) += x86::reg32(x86::sreg32(80 /*0x50*/));
    // 00513b35  8b9424b8000000         -mov edx, dword ptr [esp + 0xb8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513b3c  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513b3e  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513b40  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513b42  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513b49  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513b4b  894644                 -mov dword ptr [esi + 0x44], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(68) /* 0x44 */) = cpu.eax;
    // 00513b4e  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513b55  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 00513b5a  83c054                 -add eax, 0x54
    (cpu.eax) += x86::reg32(x86::sreg32(84 /*0x54*/));
    // 00513b5d  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 00513b62  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513b64  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513b66  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513b68  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513b6f  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513b71  894648                 -mov dword ptr [esi + 0x48], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(72) /* 0x48 */) = cpu.eax;
    // 00513b74  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513b7b  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 00513b80  83c058                 -add eax, 0x58
    (cpu.eax) += x86::reg32(x86::sreg32(88 /*0x58*/));
    // 00513b83  83c228                 -add edx, 0x28
    (cpu.edx) += x86::reg32(x86::sreg32(40 /*0x28*/));
    // 00513b86  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513b88  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513b8a  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513b8c  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513b93  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513b95  89464c                 -mov dword ptr [esi + 0x4c], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(76) /* 0x4c */) = cpu.eax;
    // 00513b98  8d461c                 -lea eax, [esi + 0x1c]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(28) /* 0x1c */);
    // 00513b9b  e890d2fcff             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513ba0  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513ba7  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 00513bac  83c06c                 -add eax, 0x6c
    (cpu.eax) += x86::reg32(x86::sreg32(108 /*0x6c*/));
    // 00513baf  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513bb1  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513bb3  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513bb5  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513bbc  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513bbe  c7460401000000         -mov dword ptr [esi + 4], 1
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = 1 /*0x1*/;
    // 00513bc5  8b0de06d5600           -mov ecx, dword ptr [0x566de0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
    // 00513bcb  894608                 -mov dword ptr [esi + 8], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 00513bce  83f903                 +cmp ecx, 3
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
    // 00513bd1  7c1e                   -jl 0x513bf1
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00513bf1;
    }
    // 00513bd3  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513bd5  8b9c24bc000000         -mov ebx, dword ptr [esp + 0xbc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 00513bdc  8d9424b0000000         -lea edx, [esp + 0xb0]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 00513be3  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513be5  b8d0025500             -mov eax, 0x5502d0
    cpu.eax = 5571280 /*0x5502d0*/;
    // 00513bea  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 00513bec  e88fe5ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00513bf1:
    // 00513bf1  837e1400               +cmp dword ptr [esi + 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513bf5  0f84fcf7ffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 00513bfb  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513c02  8b406c                 -mov eax, dword ptr [eax + 0x6c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(108) /* 0x6c */);
    // 00513c05  8b8c24cc000000         -mov ecx, dword ptr [esp + 0xcc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 00513c0c  e8eff3ffff             -call 0x513000
    cpu.esp -= 4;
    sub_513000(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513c11  8d5801                 -lea ebx, [eax + 1]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 00513c14  8d561c                 -lea edx, [esi + 0x1c]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(28) /* 0x1c */);
    // 00513c17  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00513c19  ff5614                 -call dword ptr [esi + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513c1c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513c1e  0f85d3f7ffff           -jne 0x5133f7
    if (!cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 00513c24  894614                 -mov dword ptr [esi + 0x14], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 00513c27  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 00513c2d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513c2e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513c2f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513c30  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513c33:
    // 00513c33  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00513c35  e9c0fdffff             -jmp 0x5139fa
    goto L_0x005139fa;
L_0x00513c3a:
    // 00513c3a  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00513c3c  e9f9fdffff             -jmp 0x513a3a
    goto L_0x00513a3a;
L_0x00513c41:
    // 00513c41  b917000000             -mov ecx, 0x17
    cpu.ecx = 23 /*0x17*/;
    // 00513c46  8d7c241c               -lea edi, [esp + 0x1c]
    cpu.edi = x86::reg32(cpu.esp + x86::reg32(28) /* 0x1c */);
    // 00513c4a  8d7530                 -lea esi, [ebp + 0x30]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(48) /* 0x30 */);
    // 00513c4d  e9e9faffff             -jmp 0x51373b
    goto L_0x0051373b;
L_0x00513c52:
    // 00513c52  8bb424c8000000         -mov esi, dword ptr [esp + 0xc8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(200) /* 0xc8 */);
    // 00513c59  bb646162ea             -mov ebx, 0xea626164
    cpu.ebx = 3932316004 /*0xea626164*/;
    // 00513c5e  83ee5c                 +sub esi, 0x5c
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(92 /*0x5c*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00513c61  899c24b0000000         -mov dword ptr [esp + 0xb0], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(176) /* 0xb0 */) = cpu.ebx;
    // 00513c68  89b424c8000000         -mov dword ptr [esp + 0xc8], esi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(200) /* 0xc8 */) = cpu.esi;
    // 00513c6f  e9c9faffff             -jmp 0x51373d
    goto L_0x0051373d;
L_0x00513c74:
    // 00513c74  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00513c76  e9ebfcffff             -jmp 0x513966
    goto L_0x00513966;
L_0x00513c7b:
    // 00513c7b  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513c82  0f8c6ff7ffff           -jl 0x5133f7
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005133f7;
    }
    // 00513c88  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513c8a  8b9c24bc000000         -mov ebx, dword ptr [esp + 0xbc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 00513c91  8d9424b0000000         -lea edx, [esp + 0xb0]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 00513c98  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513c9a  b8f4025500             -mov eax, 0x5502f4
    cpu.eax = 5571316 /*0x5502f4*/;
    // 00513c9f  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 00513ca1  e8dae4ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513ca6  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 00513cac  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513cad  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513cae  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513caf  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513cb2:
    // 00513cb2  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513cb9  0f8c38f7ffff           -jl 0x5133f7
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005133f7;
    }
    // 00513cbf  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00513cc0  8b9c24bc000000         -mov ebx, dword ptr [esp + 0xbc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 00513cc7  8d9424b0000000         -lea edx, [esp + 0xb0]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 00513cce  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00513ccf  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 00513cd1  b800035500             -mov eax, 0x550300
    cpu.eax = 5571328 /*0x550300*/;
    // 00513cd6  e8a5e4ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513cdb  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 00513ce1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513ce2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513ce3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513ce4  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513ce7:
    // 00513ce7  8b5514                 -mov edx, dword ptr [ebp + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00513cea  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00513ced  8b9284000000           -mov edx, dword ptr [edx + 0x84]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(132) /* 0x84 */);
    // 00513cf3  8b4050                 -mov eax, dword ptr [eax + 0x50]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(80) /* 0x50 */);
    // 00513cf6  4a                     -dec edx
    (cpu.edx)--;
    // 00513cf7  e804f3ffff             -call 0x513000
    cpu.esp -= 4;
    sub_513000(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513cfc  39d0                   +cmp eax, edx
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
    // 00513cfe  0f8386010000           -jae 0x513e8a
    if (!cpu.flags.cf)
    {
        goto L_0x00513e8a;
    }
    // 00513d04  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00513d07  80783c00               +cmp byte ptr [eax + 0x3c], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(60) /* 0x3c */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00513d0b  0f85cb000000           -jne 0x513ddc
    if (!cpu.flags.zf)
    {
        goto L_0x00513ddc;
    }
L_0x00513d11:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00513d11  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513d18  83c028                 -add eax, 0x28
    (cpu.eax) += x86::reg32(x86::sreg32(40 /*0x28*/));
    // 00513d1b  898424c0000000         -mov dword ptr [esp + 0xc0], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(192) /* 0xc0 */) = cpu.eax;
    // 00513d22  8bbc24b8000000         -mov edi, dword ptr [esp + 0xb8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513d29  8b9424c0000000         -mov edx, dword ptr [esp + 0xc0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(192) /* 0xc0 */);
    // 00513d30  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00513d33  8bb424cc000000         -mov esi, dword ptr [esp + 0xcc]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 00513d3a  e8e1f5ffff             -call 0x513320
    cpu.esp -= 4;
    sub_513320(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513d3f  8d7f28                 -lea edi, [edi + 0x28]
    cpu.edi = x86::reg32(cpu.edi + x86::reg32(40) /* 0x28 */);
    // 00513d42  898424c4000000         -mov dword ptr [esp + 0xc4], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(196) /* 0xc4 */) = cpu.eax;
    // 00513d49  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00513d4a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00513d4b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00513d4c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00513d4d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00513d4e  898424b4000000         -mov dword ptr [esp + 0xb4], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(180) /* 0xb4 */) = cpu.eax;
    // 00513d55  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00513d58  83784c00               +cmp dword ptr [eax + 0x4c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(76) /* 0x4c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513d5c  7420                   -je 0x513d7e
    if (cpu.flags.zf)
    {
        goto L_0x00513d7e;
    }
    // 00513d5e  8b9c24c4000000         -mov ebx, dword ptr [esp + 0xc4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(196) /* 0xc4 */);
    // 00513d65  8b9424c0000000         -mov edx, dword ptr [esp + 0xc0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(192) /* 0xc0 */);
    // 00513d6c  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00513d6e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00513d70  43                     -inc ebx
    (cpu.ebx)++;
    // 00513d71  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00513d73  ff564c                 -call dword ptr [esi + 0x4c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(76) /* 0x4c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513d76  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513d78  0f84b6000000           -je 0x513e34
    if (cpu.flags.zf)
    {
        goto L_0x00513e34;
    }
L_0x00513d7e:
    // 00513d7e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00513d83  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513d85  89442418               -mov dword ptr [esp + 0x18], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 00513d89  8b8424b4000000         -mov eax, dword ptr [esp + 0xb4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(180) /* 0xb4 */);
    // 00513d90  bb14000000             -mov ebx, 0x14
    cpu.ebx = 20 /*0x14*/;
    // 00513d95  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513d97  8944241c               -mov dword ptr [esp + 0x1c], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(28) /* 0x1c */) = cpu.eax;
    // 00513d9b  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513da2  8d542404               -lea edx, [esp + 4]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00513da6  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00513da9  e84267fdff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513dae  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513db5  0f8c84faffff           -jl 0x51383f
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051383f;
    }
    // 00513dbb  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513dbd  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 00513dc2  8b9c24d0000000         -mov ebx, dword ptr [esp + 0xd0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(208) /* 0xd0 */);
    // 00513dc9  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513dcb  8d9424b8000000         -lea edx, [esp + 0xb8]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513dd2  b80c035500             -mov eax, 0x55030c
    cpu.eax = 5571340 /*0x55030c*/;
    // 00513dd7  e95efaffff             -jmp 0x51383a
    goto L_0x0051383a;
L_0x00513ddc:
    // 00513ddc  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 00513de1  8b9424b8000000         -mov edx, dword ptr [esp + 0xb8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513de8  83c03c                 -add eax, 0x3c
    (cpu.eax) += x86::reg32(x86::sreg32(60 /*0x3c*/));
    // 00513deb  83c218                 -add edx, 0x18
    (cpu.edx) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 00513dee  e89d75fdff             -call 0x4eb390
    cpu.esp -= 4;
    nfs3hp::watcom::strncmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513df3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513df5  0f8416ffffff           -je 0x513d11
    if (cpu.flags.zf)
    {
        goto L_0x00513d11;
    }
    // 00513dfb  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 00513e00  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513e02  8b15e06d5600           -mov edx, dword ptr [0x566de0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
    // 00513e08  89442418               -mov dword ptr [esp + 0x18], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 00513e0c  83fa03                 +cmp edx, 3
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
    // 00513e0f  0f8c2afaffff           -jl 0x51383f
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051383f;
    }
    // 00513e15  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00513e16  8b9c24d0000000         -mov ebx, dword ptr [esp + 0xd0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(208) /* 0xd0 */);
    // 00513e1d  8d9424b4000000         -lea edx, [esp + 0xb4]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(180) /* 0xb4 */);
    // 00513e24  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00513e25  b824035500             -mov eax, 0x550324
    cpu.eax = 5571364 /*0x550324*/;
    // 00513e2a  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 00513e2f  e906faffff             -jmp 0x51383a
    goto L_0x0051383a;
L_0x00513e34:
    // 00513e34  8a8c24c4000000         -mov cl, byte ptr [esp + 0xc4]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(196) /* 0xc4 */);
    // 00513e3b  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00513e40  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00513e43  d3e2                   -shl edx, cl
    cpu.edx <<= cpu.cl % 32;
    // 00513e45  8b5850                 -mov ebx, dword ptr [eax + 0x50]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(80) /* 0x50 */);
    // 00513e48  f7d2                   -not edx
    cpu.edx = ~cpu.edx;
    // 00513e4a  21d3                   -and ebx, edx
    cpu.ebx &= x86::reg32(x86::sreg32(cpu.edx));
    // 00513e4c  895850                 -mov dword ptr [eax + 0x50], ebx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(80) /* 0x50 */) = cpu.ebx;
    // 00513e4f  b8fdffffff             -mov eax, 0xfffffffd
    cpu.eax = 4294967293 /*0xfffffffd*/;
    // 00513e54  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513e56  8b35e06d5600           -mov esi, dword ptr [0x566de0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
    // 00513e5c  89442418               -mov dword ptr [esp + 0x18], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 00513e60  83fe03                 +cmp esi, 3
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
    // 00513e63  0f8cd6f9ffff           -jl 0x51383f
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051383f;
    }
    // 00513e69  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513e6b  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 00513e70  8b9c24d0000000         -mov ebx, dword ptr [esp + 0xd0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(208) /* 0xd0 */);
    // 00513e77  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513e79  8d9424b8000000         -lea edx, [esp + 0xb8]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513e80  b818035500             -mov eax, 0x550318
    cpu.eax = 5571352 /*0x550318*/;
    // 00513e85  e9b0f9ffff             -jmp 0x51383a
    goto L_0x0051383a;
L_0x00513e8a:
    // 00513e8a  b8feffffff             -mov eax, 0xfffffffe
    cpu.eax = 4294967294 /*0xfffffffe*/;
    // 00513e8f  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513e91  89442418               -mov dword ptr [esp + 0x18], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 00513e95  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513e9c  0f8c9df9ffff           -jl 0x51383f
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051383f;
    }
    // 00513ea2  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00513ea3  8b9c24d0000000         -mov ebx, dword ptr [esp + 0xd0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(208) /* 0xd0 */);
    // 00513eaa  8d9424b4000000         -lea edx, [esp + 0xb4]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(180) /* 0xb4 */);
    // 00513eb1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00513eb2  b830035500             -mov eax, 0x550330
    cpu.eax = 5571376 /*0x550330*/;
    // 00513eb7  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 00513ebc  e979f9ffff             -jmp 0x51383a
    goto L_0x0051383a;
L_0x00513ec1:
    // 00513ec1  8d5d1c                 -lea ebx, [ebp + 0x1c]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 00513ec4  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00513ec6  e87511feff             -call 0x4f5040
    cpu.esp -= 4;
    nfs3hp::ea::netCheckPlayer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513ecb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513ecd  7535                   -jne 0x513f04
    if (!cpu.flags.zf)
    {
        goto L_0x00513f04;
    }
    // 00513ecf  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513ed6  0f8c1bf5ffff           -jl 0x5133f7
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005133f7;
    }
    // 00513edc  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00513edd  8b9c24bc000000         -mov ebx, dword ptr [esp + 0xbc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 00513ee4  8d9424b0000000         -lea edx, [esp + 0xb0]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 00513eeb  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00513eec  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 00513eee  b86c035500             -mov eax, 0x55036c
    cpu.eax = 5571436 /*0x55036c*/;
    // 00513ef3  e888e2ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513ef8  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 00513efe  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513eff  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513f00  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513f01  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513f04:
    // 00513f04  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 00513f09  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513f10  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513f12  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513f14  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513f16  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513f1d  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513f1f  3b06                   +cmp eax, dword ptr [esi]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513f21  0f858e000000           -jne 0x513fb5
    if (!cpu.flags.zf)
    {
        goto L_0x00513fb5;
    }
    // 00513f27  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513f2e  7c1e                   -jl 0x513f4e
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00513f4e;
    }
    // 00513f30  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513f32  8b9c24bc000000         -mov ebx, dword ptr [esp + 0xbc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 00513f39  8d9424b0000000         -lea edx, [esp + 0xb0]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 00513f40  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513f42  b854035500             -mov eax, 0x550354
    cpu.eax = 5571412 /*0x550354*/;
    // 00513f47  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 00513f49  e832e2ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00513f4e:
    // 00513f4e  8b8424b8000000         -mov eax, dword ptr [esp + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513f55  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 00513f5a  83c018                 -add eax, 0x18
    (cpu.eax) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 00513f5d  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00513f5f  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00513f61  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00513f63  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00513f6a  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00513f6c  894604                 -mov dword ptr [esi + 4], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 00513f6f  8b7e04                 -mov edi, dword ptr [esi + 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00513f72  c70600000000           -mov dword ptr [esi], 0
    app->getMemory<x86::reg32>(cpu.esi) = 0 /*0x0*/;
    // 00513f78  83ff01                 +cmp edi, 1
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
    // 00513f7b  0f8576f4ffff           -jne 0x5133f7
    if (!cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 00513f81  8bb424b8000000         -mov esi, dword ptr [esp + 0xb8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00513f88  8d7d1c                 -lea edi, [ebp + 0x1c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 00513f8b  8d7604                 -lea esi, [esi + 4]
    cpu.esi = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00513f8e  a1d46d5600             -mov eax, dword ptr [0x566dd4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664212) /* 0x566dd4 */);
    // 00513f93  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00513f94  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00513f95  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00513f96  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00513f97  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00513f98  8985a4000000           -mov dword ptr [ebp + 0xa4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(164) /* 0xa4 */) = cpu.eax;
    // 00513f9e  a1d86d5600             -mov eax, dword ptr [0x566dd8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664216) /* 0x566dd8 */);
    // 00513fa3  8985a0000000           -mov dword ptr [ebp + 0xa0], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(160) /* 0xa0 */) = cpu.eax;
    // 00513fa9  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 00513faf  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513fb0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513fb1  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513fb2  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513fb5:
    // 00513fb5  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00513fbc  0f8c35f4ffff           -jl 0x5133f7
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005133f7;
    }
    // 00513fc2  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513fc4  8b9c24bc000000         -mov ebx, dword ptr [esp + 0xbc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 00513fcb  8d9424b0000000         -lea edx, [esp + 0xb0]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 00513fd2  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00513fd4  b860035500             -mov eax, 0x550360
    cpu.eax = 5571424 /*0x550360*/;
    // 00513fd9  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 00513fdb  e8a0e1ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513fe0  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 00513fe6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513fe7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513fe8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00513fe9  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00513fec:
    // 00513fec  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00513fee  e8adf2ffff             -call 0x5132a0
    cpu.esp -= 4;
    sub_5132a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00513ff3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00513ff5  0f84fcf3ffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 00513ffb  833de06d560004         +cmp dword ptr [0x566de0], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00514002  0f8ceff3ffff           -jl 0x5133f7
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005133f7;
    }
    // 00514008  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051400a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051400b  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051400d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051400e  8d5060                 -lea edx, [eax + 0x60]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(96) /* 0x60 */);
    // 00514011  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00514013  b888035500             -mov eax, 0x550388
    cpu.eax = 5571464 /*0x550388*/;
    // 00514018  e863e1ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051401d  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 00514023  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514024  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514025  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514026  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00514029:
    // 00514029  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051402b  e870f2ffff             -call 0x5132a0
    cpu.esp -= 4;
    sub_5132a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514030  81c4d0000000           +add esp, 0xd0
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(208 /*0xd0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00514036  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514037  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514038  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514039  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x0051403c:
    // 0051403c  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051403e  8b9c24bc000000         -mov ebx, dword ptr [esp + 0xbc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 00514045  8d9424b0000000         -lea edx, [esp + 0xb0]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 0051404c  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051404e  b8a8035500             -mov eax, 0x5503a8
    cpu.eax = 5571496 /*0x5503a8*/;
    // 00514053  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 00514055  e826e1ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051405a  e9baf5ffff             -jmp 0x513619
    goto L_0x00513619;
L_0x0051405f:
    // 0051405f  8b9424cc000000         -mov edx, dword ptr [esp + 0xcc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 00514066  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00514068  e833f2ffff             -call 0x5132a0
    cpu.esp -= 4;
    sub_5132a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051406d  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051406f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00514071  0f8480f3ffff           -je 0x5133f7
    if (cpu.flags.zf)
    {
        goto L_0x005133f7;
    }
    // 00514077  8b7d14                 -mov edi, dword ptr [ebp + 0x14]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 0051407a  8d9788000000           -lea edx, [edi + 0x88]
    cpu.edx = x86::reg32(cpu.edi + x86::reg32(136) /* 0x88 */);
    // 00514080  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00514082  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00514084  b964000000             -mov ecx, 0x64
    cpu.ecx = 100 /*0x64*/;
    // 00514089  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051408c  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0051408e  88c1                   -mov cl, al
    cpu.cl = cpu.al;
    // 00514090  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00514095  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 00514097  8b5f50                 -mov ebx, dword ptr [edi + 0x50]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(80) /* 0x50 */);
    // 0051409a  f7d0                   -not eax
    cpu.eax = ~cpu.eax;
    // 0051409c  21c3                   -and ebx, eax
    cpu.ebx &= x86::reg32(x86::sreg32(cpu.eax));
    // 0051409e  895f50                 -mov dword ptr [edi + 0x50], ebx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(80) /* 0x50 */) = cpu.ebx;
    // 005140a1  c7465c00000000         -mov dword ptr [esi + 0x5c], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(92) /* 0x5c */) = 0 /*0x0*/;
    // 005140a8  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 005140ae  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005140af  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005140b0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005140b1  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x005140b4:
    // 005140b4  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 005140b6  8b9c24bc000000         -mov ebx, dword ptr [esp + 0xbc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 005140bd  8d9424b0000000         -lea edx, [esp + 0xb0]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(176) /* 0xb0 */);
    // 005140c4  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 005140c6  b8b0035500             -mov eax, 0x5503b0
    cpu.eax = 5571504 /*0x5503b0*/;
    // 005140cb  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 005140cd  e8aee0ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005140d2  81c4d0000000           -add esp, 0xd0
    (cpu.esp) += x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 005140d8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005140d9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005140da  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005140db  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_5140e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005140e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 005140e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005140e2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005140e3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005140e4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005140e5  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 005140e8  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 005140eb  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 005140ed  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 005140ef  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 005140f1  0f854f010000           -jne 0x514246
    if (!cpu.flags.zf)
    {
        goto L_0x00514246;
    }
    // 005140f7  83b8a800000000         +cmp dword ptr [eax + 0xa8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(168) /* 0xa8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005140fe  751f                   -jne 0x51411f
    if (!cpu.flags.zf)
    {
        goto L_0x0051411f;
    }
    // 00514100  83780c00               +cmp dword ptr [eax + 0xc], 0
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
    // 00514104  7419                   -je 0x51411f
    if (cpu.flags.zf)
    {
        goto L_0x0051411f;
    }
    // 00514106  8ba8a4000000           -mov ebp, dword ptr [eax + 0xa4]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(164) /* 0xa4 */);
    // 0051410c  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0051410e  740f                   -je 0x51411f
    if (cpu.flags.zf)
    {
        goto L_0x0051411f;
    }
    // 00514110  83781400               +cmp dword ptr [eax + 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00514114  7509                   -jne 0x51411f
    if (!cpu.flags.zf)
    {
        goto L_0x0051411f;
    }
    // 00514116  8d4dff                 -lea ecx, [ebp - 1]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-1) /* -0x1 */);
    // 00514119  8988a4000000           -mov dword ptr [eax + 0xa4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(164) /* 0xa4 */) = cpu.ecx;
L_0x0051411f:
    // 0051411f  8b5f14                 -mov ebx, dword ptr [edi + 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(20) /* 0x14 */);
    // 00514122  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00514124  0f841c010000           -je 0x514246
    if (cpu.flags.zf)
    {
        goto L_0x00514246;
    }
    // 0051412a  8b6b08                 -mov ebp, dword ptr [ebx + 8]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 0051412d  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 0051412f  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 00514131  0f850f010000           -jne 0x514246
    if (!cpu.flags.zf)
    {
        goto L_0x00514246;
    }
    // 00514137  837b5000               +cmp dword ptr [ebx + 0x50], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(80) /* 0x50 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051413b  0f8405010000           -je 0x514246
    if (cpu.flags.zf)
    {
        goto L_0x00514246;
    }
    // 00514141  c7442404616c6fea       -mov dword ptr [esp + 4], 0xea6f6c61
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = 3933170785 /*0xea6f6c61*/;
    // 00514149  8b9fa0000000           -mov ebx, dword ptr [edi + 0xa0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(160) /* 0xa0 */);
    // 0051414f  896c240c               -mov dword ptr [esp + 0xc], ebp
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.ebp;
    // 00514153  83fb02                 +cmp ebx, 2
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
    // 00514156  7c40                   -jl 0x514198
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00514198;
    }
    // 00514158  8d6bff                 -lea ebp, [ebx - 1]
    cpu.ebp = x86::reg32(cpu.ebx + x86::reg32(-1) /* -0x1 */);
    // 0051415b  89afa0000000           -mov dword ptr [edi + 0xa0], ebp
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(160) /* 0xa0 */) = cpu.ebp;
L_0x00514161:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00514161  8d8688000000           -lea eax, [esi + 0x88]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(136) /* 0x88 */);
    // 00514167  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00514169  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0051416d  890c24                 -mov dword ptr [esp], ecx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ecx;
L_0x00514170:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00514170  8b8684000000           -mov eax, dword ptr [esi + 0x84]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(132) /* 0x84 */);
    // 00514176  8b1c24                 -mov ebx, dword ptr [esp]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    // 00514179  48                     -dec eax
    (cpu.eax)--;
    // 0051417a  39d8                   +cmp eax, ebx
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
    // 0051417c  0f86c4000000           -jbe 0x514246
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00514246;
    }
    // 00514182  8a0c24                 -mov cl, byte ptr [esp]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esp);
    // 00514185  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051418a  8b6e50                 -mov ebp, dword ptr [esi + 0x50]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(80) /* 0x50 */);
    // 0051418d  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 0051418f  85e8                   +test eax, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.ebp));
    // 00514191  7537                   -jne 0x5141ca
    if (!cpu.flags.zf)
    {
        goto L_0x005141ca;
    }
L_0x00514193:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00514193  ff0424                 +inc dword ptr [esp]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.esp);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00514196  ebd8                   -jmp 0x514170
    goto L_0x00514170;
L_0x00514198:
    // 00514198  a1d86d5600             -mov eax, dword ptr [0x566dd8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664216) /* 0x566dd8 */);
    // 0051419d  8987a0000000           -mov dword ptr [edi + 0xa0], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(160) /* 0xa0 */) = cpu.eax;
    // 005141a3  833de06d560003         +cmp dword ptr [0x566de0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005141aa  7c14                   -jl 0x5141c0
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005141c0;
    }
    // 005141ac  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005141ad  8d542408               -lea edx, [esp + 8]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 005141b1  b8bc035500             -mov eax, 0x5503bc
    cpu.eax = 5571516 /*0x5503bc*/;
    // 005141b6  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005141b7  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 005141b9  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 005141bb  e8c0dfffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x005141c0:
    // 005141c0  c744240c01000000       -mov dword ptr [esp + 0xc], 1
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = 1 /*0x1*/;
    // 005141c8  eb97                   -jmp 0x514161
    goto L_0x00514161;
L_0x005141ca:
    // 005141ca  6bc364                 -imul eax, ebx, 0x64
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(100 /*0x64*/)));
    // 005141cd  8b9406e4000000         -mov edx, dword ptr [esi + eax + 0xe4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(228) /* 0xe4 */ + cpu.eax * 1);
    // 005141d4  8d6aff                 -lea ebp, [edx - 1]
    cpu.ebp = x86::reg32(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 005141d7  89ac06e4000000         -mov dword ptr [esi + eax + 0xe4], ebp
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(228) /* 0xe4 */ + cpu.eax * 1) = cpu.ebp;
    // 005141de  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 005141e0  7536                   -jne 0x514218
    if (!cpu.flags.zf)
    {
        goto L_0x00514218;
    }
    // 005141e2  833de06d560002         +cmp dword ptr [0x566de0], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664224) /* 0x566de0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005141e9  7c16                   -jl 0x514201
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00514201;
    }
    // 005141eb  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 005141ed  8d542404               -lea edx, [esp + 4]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 005141f1  b8cc035500             -mov eax, 0x5503cc
    cpu.eax = 5571532 /*0x5503cc*/;
    // 005141f6  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 005141f8  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 005141fa  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 005141fc  e87fdfffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00514201:
    // 00514201  8a0c24                 -mov cl, byte ptr [esp]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esp);
    // 00514204  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00514209  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 0051420b  f7d0                   -not eax
    cpu.eax = ~cpu.eax;
    // 0051420d  214650                 +and dword ptr [esi + 0x50], eax
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg32>(cpu.esi + x86::reg32(80) /* 0x50 */) &= x86::reg32(x86::sreg32(cpu.eax))));
    // 00514210  ff0424                 +inc dword ptr [esp]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.esp);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00514213  e958ffffff             -jmp 0x514170
    goto L_0x00514170;
L_0x00514218:
    // 00514218  837c240c00             +cmp dword ptr [esp + 0xc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051421d  0f8470ffffff           -je 0x514193
    if (cpu.flags.zf)
    {
        goto L_0x00514193;
    }
    // 00514223  6b142464               -imul edx, dword ptr [esp], 0x64
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp))) * x86::sreg64(x86::sreg32(100 /*0x64*/)));
    // 00514227  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00514229  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0051422d  8b6f08                 -mov ebp, dword ptr [edi + 8]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8) /* 0x8 */);
    // 00514230  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00514232  8b5c2408               -mov ebx, dword ptr [esp + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 00514236  01c2                   +add edx, eax
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
    // 00514238  8b470c                 -mov eax, dword ptr [edi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(12) /* 0xc */);
    // 0051423b  ff5554                 -call dword ptr [ebp + 0x54]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(84) /* 0x54 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051423e  ff0424                 +inc dword ptr [esp]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.esp);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00514241  e92affffff             -jmp 0x514170
    goto L_0x00514170;
L_0x00514246:
    // 00514246  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051424b  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0051424e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051424f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514250  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514251  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514252  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514253  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_514260(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00514260  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00514261  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00514262  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00514267  bae0405100             -mov edx, 0x5140e0
    cpu.edx = 5325024 /*0x5140e0*/;
    // 0051426c  b8f0259f00             -mov eax, 0x9f25f0
    cpu.eax = 10429936 /*0x9f25f0*/;
    // 00514271  e86aa6ffff             -call 0x50e8e0
    cpu.esp -= 4;
    sub_50e8e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514276  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514277  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514278  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_514280(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00514280  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00514281  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00514282  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00514283  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00514285  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00514287  8b15e4895600           -mov edx, dword ptr [0x5689e4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5671396) /* 0x5689e4 */);
    // 0051428d  42                     -inc edx
    (cpu.edx)++;
    // 0051428e  8915e4895600           -mov dword ptr [0x5689e4], edx
    app->getMemory<x86::reg32>(x86::reg32(5671396) /* 0x5689e4 */) = cpu.edx;
    // 00514294  83fa01                 +cmp edx, 1
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
    // 00514297  7514                   -jne 0x5142ad
    if (!cpu.flags.zf)
    {
        goto L_0x005142ad;
    }
    // 00514299  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 0051429c  ff5014                 -call dword ptr [eax + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051429f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005142a1  7420                   -je 0x5142c3
    if (cpu.flags.zf)
    {
        goto L_0x005142c3;
    }
    // 005142a3  b860425100             -mov eax, 0x514260
    cpu.eax = 5325408 /*0x514260*/;
    // 005142a8  e883e6fdff             -call 0x4f2930
    cpu.esp -= 4;
    sub_4f2930(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x005142ad:
    // 005142ad  bbf0534f00             -mov ebx, 0x4f53f0
    cpu.ebx = 5198832 /*0x4f53f0*/;
    // 005142b2  8b6e08                 -mov ebp, dword ptr [esi + 8]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 005142b5  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 005142b7  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 005142b9  ff551c                 -call dword ptr [ebp + 0x1c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005142bc  89460c                 -mov dword ptr [esi + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 005142bf  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005142c1  7506                   -jne 0x5142c9
    if (!cpu.flags.zf)
    {
        goto L_0x005142c9;
    }
L_0x005142c3:
    // 005142c3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005142c4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005142c5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005142c6  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x005142c9:
    // 005142c9  8b4e08                 -mov ecx, dword ptr [esi + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 005142cc  8d5630                 -lea edx, [esi + 0x30]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(48) /* 0x30 */);
    // 005142cf  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 005142d4  ff5144                 -call dword ptr [ecx + 0x44]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(68) /* 0x44 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005142d7  8d968c000000           -lea edx, [esi + 0x8c]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(140) /* 0x8c */);
    // 005142dd  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 005142df  c7869c00000001000000   -mov dword ptr [esi + 0x9c], 1
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(156) /* 0x9c */) = 1 /*0x1*/;
    // 005142e9  e80262fdff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005142ee  bb33000000             -mov ebx, 0x33
    cpu.ebx = 51 /*0x33*/;
    // 005142f3  8b542410               -mov edx, dword ptr [esp + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 005142f7  8d4644                 -lea eax, [esi + 0x44]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(68) /* 0x44 */);
    // 005142fa  e831cbfcff             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005142ff  bb13000000             -mov ebx, 0x13
    cpu.ebx = 19 /*0x13*/;
    // 00514304  8b542414               -mov edx, dword ptr [esp + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */);
    // 00514308  8d4678                 -lea eax, [esi + 0x78]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(120) /* 0x78 */);
    // 0051430b  e820cbfcff             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514310  b8fa000000             -mov eax, 0xfa
    cpu.eax = 250 /*0xfa*/;
    // 00514315  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514316  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514317  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514318  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_514320(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00514320  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00514321  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00514322  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00514323  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00514324  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00514325  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00514327  8b580c                 -mov ebx, dword ptr [eax + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 0051432a  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0051432f  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00514331  7465                   -je 0x514398
    if (cpu.flags.zf)
    {
        goto L_0x00514398;
    }
    // 00514333  83781400               +cmp dword ptr [eax + 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00514337  7509                   -jne 0x514342
    if (!cpu.flags.zf)
    {
        goto L_0x00514342;
    }
    // 00514339  83b8a000000000         +cmp dword ptr [eax + 0xa0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(160) /* 0xa0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00514340  7409                   -je 0x51434b
    if (cpu.flags.zf)
    {
        goto L_0x0051434b;
    }
L_0x00514342:
    // 00514342  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00514344  e8470cfeff             -call 0x4f4f90
    cpu.esp -= 4;
    nfs3hp::ea::netEndGame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514349  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x0051434b:
    // 0051434b  8baaa8000000           -mov ebp, dword ptr [edx + 0xa8]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(168) /* 0xa8 */);
    // 00514351  c7829c00000000000000   -mov dword ptr [edx + 0x9c], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(156) /* 0x9c */) = 0 /*0x0*/;
    // 0051435b  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0051435d  741b                   -je 0x51437a
    if (cpu.flags.zf)
    {
        goto L_0x0051437a;
    }
L_0x0051435f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051435f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00514361  e8cab5fcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514366  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00514368  7436                   -je 0x5143a0
    if (cpu.flags.zf)
    {
        goto L_0x005143a0;
    }
    app->unlockContext(cpu);
    win32::Thread::sleep(0);
    app->lockContext(cpu);
    // 0051436a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051436c  e8bf32fdff             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00514371:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00514371  83baa800000000         +cmp dword ptr [edx + 0xa8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(168) /* 0xa8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00514378  75e5                   -jne 0x51435f
    if (!cpu.flags.zf)
    {
        goto L_0x0051435f;
    }
L_0x0051437a:
    // 0051437a  8b7208                 -mov esi, dword ptr [edx + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 0051437d  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051437f  c7420c00000000         -mov dword ptr [edx + 0xc], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */) = 0 /*0x0*/;
    // 00514386  ff5620                 -call dword ptr [esi + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514389  8b1de4895600           -mov ebx, dword ptr [0x5689e4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5671396) /* 0x5689e4 */);
    // 0051438f  4b                     +dec ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 00514390  891de4895600           -mov dword ptr [0x5689e4], ebx
    app->getMemory<x86::reg32>(x86::reg32(5671396) /* 0x5689e4 */) = cpu.ebx;
    // 00514396  740f                   -je 0x5143a7
    if (cpu.flags.zf)
    {
        goto L_0x005143a7;
    }
L_0x00514398:
    // 00514398  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051439a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051439b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051439c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051439d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051439e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051439f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005143a0:
    // 005143a0  e83bb5fcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005143a5  ebca                   -jmp 0x514371
    goto L_0x00514371;
L_0x005143a7:
    // 005143a7  b860425100             -mov eax, 0x514260
    cpu.eax = 5325408 /*0x514260*/;
    // 005143ac  e8ffe5fdff             -call 0x4f29b0
    cpu.esp -= 4;
    nfs3hp::ea::removeLowTimer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005143b1  8b4208                 -mov eax, dword ptr [edx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 005143b4  ff5018                 -call dword ptr [eax + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005143b7  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 005143b9  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 005143bb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005143bc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005143bd  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005143be  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005143bf  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005143c0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_5143d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005143d0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005143d1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005143d2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005143d3  81ec88000000           -sub esp, 0x88
    (cpu.esp) -= x86::reg32(x86::sreg32(136 /*0x88*/));
    // 005143d9  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 005143db  89942480000000         -mov dword ptr [esp + 0x80], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(128) /* 0x80 */) = cpu.edx;
    // 005143e2  898c2484000000         -mov dword ptr [esp + 0x84], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(132) /* 0x84 */) = cpu.ecx;
    // 005143e9  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 005143ee  89e7                   -mov edi, esp
    cpu.edi = cpu.esp;
    // 005143f0  bed02d5100             -mov esi, 0x512dd0
    cpu.esi = 5320144 /*0x512dd0*/;
    // 005143f5  8d542450               -lea edx, [esp + 0x50]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(80) /* 0x50 */);
    // 005143f9  058c000000             -add eax, 0x8c
    (cpu.eax) += x86::reg32(x86::sreg32(140 /*0x8c*/));
    // 005143fe  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00514400  e85bebffff             -call 0x512f60
    cpu.esp -= 4;
    sub_512f60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514405  8b842480000000         -mov eax, dword ptr [esp + 0x80]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(128) /* 0x80 */);
    // 0051440c  896c2410               -mov dword ptr [esp + 0x10], ebp
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.ebp;
    // 00514410  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00514412  89442414               -mov dword ptr [esp + 0x14], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 00514416  8b842484000000         -mov eax, dword ptr [esp + 0x84]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(132) /* 0x84 */);
    // 0051441d  891424                 -mov dword ptr [esp], edx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.edx;
    // 00514420  8944240c               -mov dword ptr [esp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 00514424  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 00514426  894518                 -mov dword ptr [ebp + 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 00514429  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051442b  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 00514430  e88bebffff             -call 0x512fc0
    cpu.esp -= 4;
    sub_512fc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514435  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x00514437:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00514437  8b4d08                 -mov ecx, dword ptr [ebp + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 0051443a  8d542450               -lea edx, [esp + 0x50]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(80) /* 0x50 */);
    // 0051443e  8b450c                 -mov eax, dword ptr [ebp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 00514441  ff5130                 -call dword ptr [ecx + 0x30]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(48) /* 0x30 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514444  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
L_0x00514446:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00514446  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00514448  e8e3b4fcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051444d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051444f  751c                   -jne 0x51446d
    if (!cpu.flags.zf)
    {
        goto L_0x0051446d;
    }
    // 00514451  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00514453  e888b4fcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00514458:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00514458  e873ebffff             -call 0x512fd0
    cpu.esp -= 4;
    sub_512fd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051445d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051445f  7517                   -jne 0x514478
    if (!cpu.flags.zf)
    {
        goto L_0x00514478;
    }
    // 00514461  3b5c2414               +cmp ebx, dword ptr [esp + 0x14]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00514465  7411                   -je 0x514478
    if (cpu.flags.zf)
    {
        goto L_0x00514478;
    }
    // 00514467  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00514469  74cc                   -je 0x514437
    if (cpu.flags.zf)
    {
        goto L_0x00514437;
    }
    // 0051446b  ebd9                   -jmp 0x514446
    goto L_0x00514446;
L_0x0051446d:
    // 0051446d  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051446f  e8ac32fdff             -call 0x4e7720
    cpu.esp -= 4;
    nfs3hp::port::waitSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514474  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00514476  7de0                   -jge 0x514458
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00514458;
    }
L_0x00514478:
    // 00514478  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051447a  7513                   -jne 0x51448f
    if (!cpu.flags.zf)
    {
        goto L_0x0051448f;
    }
    // 0051447c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051447e  c7451800000000         -mov dword ptr [ebp + 0x18], 0
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */) = 0 /*0x0*/;
    // 00514485  81c488000000           -add esp, 0x88
    (cpu.esp) += x86::reg32(x86::sreg32(136 /*0x88*/));
    // 0051448b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051448c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051448d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051448e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051448f:
    // 0051448f  8b4d08                 -mov ecx, dword ptr [ebp + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 00514492  8b450c                 -mov eax, dword ptr [ebp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 00514495  ff5134                 -call dword ptr [ecx + 0x34]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(52) /* 0x34 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514498  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051449a  c7451800000000         -mov dword ptr [ebp + 0x18], 0
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */) = 0 /*0x0*/;
    // 005144a1  81c488000000           -add esp, 0x88
    (cpu.esp) += x86::reg32(x86::sreg32(136 /*0x88*/));
    // 005144a7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005144a8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005144a9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005144aa  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_5144b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005144b0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005144b1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005144b2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005144b3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005144b4  81ec04010000           -sub esp, 0x104
    (cpu.esp) -= x86::reg32(x86::sreg32(260 /*0x104*/));
    // 005144ba  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 005144bc  89942400010000         -mov dword ptr [esp + 0x100], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */) = cpu.edx;
    // 005144c3  899c24fc000000         -mov dword ptr [esp + 0xfc], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(252) /* 0xfc */) = cpu.ebx;
    // 005144ca  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 005144cf  8dbc24ac000000         -lea edi, [esp + 0xac]
    cpu.edi = x86::reg32(cpu.esp + x86::reg32(172) /* 0xac */);
    // 005144d6  be202e5100             -mov esi, 0x512e20
    cpu.esi = 5320224 /*0x512e20*/;
    // 005144db  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005144dd  83b8a400000000         +cmp dword ptr [eax + 0xa4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(164) /* 0xa4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005144e4  0f857e000000           -jne 0x514568
    if (!cpu.flags.zf)
    {
        goto L_0x00514568;
    }
L_0x005144ea:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005144ea  b917000000             -mov ecx, 0x17
    cpu.ecx = 23 /*0x17*/;
    // 005144ef  8d7c2428               -lea edi, [esp + 0x28]
    cpu.edi = x86::reg32(cpu.esp + x86::reg32(40) /* 0x28 */);
    // 005144f3  8d7530                 -lea esi, [ebp + 0x30]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(48) /* 0x30 */);
    // 005144f6  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005144f8  8d7c2404               -lea edi, [esp + 4]
    cpu.edi = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 005144fc  8bb42400010000         -mov esi, dword ptr [esp + 0x100]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 00514503  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00514504  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00514505  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00514506  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00514507  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00514508  8bb42400010000         -mov esi, dword ptr [esp + 0x100]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 0051450f  8d7d1c                 -lea edi, [ebp + 0x1c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 00514512  8b8c24fc000000         -mov ecx, dword ptr [esp + 0xfc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(252) /* 0xfc */);
    // 00514519  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0051451a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0051451b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0051451c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0051451d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0051451e  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00514520  7476                   -je 0x514598
    if (cpu.flags.zf)
    {
        goto L_0x00514598;
    }
    // 00514522  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 00514527  8d442418               -lea eax, [esp + 0x18]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(24) /* 0x18 */);
    // 0051452b  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0051452d  e8fec8fcff             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00514532:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00514532  a1d8435600             -mov eax, dword ptr [0x5643d8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653464) /* 0x5643d8 */);
    // 00514537  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 00514539  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051453a  8d4c2404               -lea ecx, [esp + 4]
    cpu.ecx = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051453e  bb646461ea             -mov ebx, 0xea616464
    cpu.ebx = 3932251236 /*0xea616464*/;
    // 00514543  6888000000             -push 0x88
    app->getMemory<x86::reg32>(cpu.esp-4) = 136 /*0x88*/;
    cpu.esp -= 4;
    // 00514548  8d9424b4000000         -lea edx, [esp + 0xb4]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(180) /* 0xb4 */);
    // 0051454f  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00514551  e83aecffff             -call 0x513190
    cpu.esp -= 4;
    sub_513190(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514556  8b8424b0000000         -mov eax, dword ptr [esp + 0xb0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(176) /* 0xb0 */);
L_0x0051455d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051455d  81c404010000           -add esp, 0x104
    (cpu.esp) += x86::reg32(x86::sreg32(260 /*0x104*/));
    // 00514563  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514564  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514565  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514566  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514567  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00514568:
    // 00514568  8b9c2400010000         -mov ebx, dword ptr [esp + 0x100]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 0051456f  8d501c                 -lea edx, [eax + 0x1c]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 00514572  e8c90afeff             -call 0x4f5040
    cpu.esp -= 4;
    nfs3hp::ea::netCheckPlayer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514577  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00514579  7407                   -je 0x514582
    if (cpu.flags.zf)
    {
        goto L_0x00514582;
    }
    // 0051457b  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00514580  ebdb                   -jmp 0x51455d
    goto L_0x0051455d;
L_0x00514582:
    // 00514582  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00514584  e8070afeff             -call 0x4f4f90
    cpu.esp -= 4;
    nfs3hp::ea::netEndGame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514589  c785a400000000000000   -mov dword ptr [ebp + 0xa4], 0
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(164) /* 0xa4 */) = 0 /*0x0*/;
    // 00514593  e952ffffff             -jmp 0x5144ea
    goto L_0x005144ea;
L_0x00514598:
    // 00514598  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 0051459b  c6403c00               -mov byte ptr [eax + 0x3c], 0
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(60) /* 0x3c */) = 0 /*0x0*/;
    // 0051459f  eb91                   -jmp 0x514532
    goto L_0x00514532;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_5145b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005145b0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005145b1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005145b2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005145b3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005145b4  81ecfc000000           -sub esp, 0xfc
    (cpu.esp) -= x86::reg32(x86::sreg32(252 /*0xfc*/));
    // 005145ba  89dd                   -mov ebp, ebx
    cpu.ebp = cpu.ebx;
    // 005145bc  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 005145c1  8dbc24ac000000         -lea edi, [esp + 0xac]
    cpu.edi = x86::reg32(cpu.esp + x86::reg32(172) /* 0xac */);
    // 005145c8  be702e5100             -mov esi, 0x512e70
    cpu.esi = 5320304 /*0x512e70*/;
    // 005145cd  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 005145cf  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 005145d1  7554                   -jne 0x514627
    if (!cpu.flags.zf)
    {
        goto L_0x00514627;
    }
    // 005145d3  83b8a400000000         +cmp dword ptr [eax + 0xa4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(164) /* 0xa4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005145da  744b                   -je 0x514627
    if (cpu.flags.zf)
    {
        goto L_0x00514627;
    }
L_0x005145dc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005145dc  8b15d8435600           -mov edx, dword ptr [0x5643d8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653464) /* 0x5643d8 */);
    // 005145e2  01d2                   -add edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx));
    // 005145e4  89e1                   -mov ecx, esp
    cpu.ecx = cpu.esp;
    // 005145e6  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 005145e7  bb716572ea             -mov ebx, 0xea726571
    cpu.ebx = 3933365617 /*0xea726571*/;
    // 005145ec  8db424cc000000         -lea esi, [esp + 0xcc]
    cpu.esi = x86::reg32(cpu.esp + x86::reg32(204) /* 0xcc */);
    // 005145f3  68ac000000             -push 0xac
    app->getMemory<x86::reg32>(cpu.esp-4) = 172 /*0xac*/;
    cpu.esp -= 4;
    // 005145f8  8d9424b4000000         -lea edx, [esp + 0xb4]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(180) /* 0xb4 */);
    // 005145ff  89ef                   -mov edi, ebp
    cpu.edi = cpu.ebp;
    // 00514601  e88aebffff             -call 0x513190
    cpu.esp -= 4;
    sub_513190(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00514606  b90d000000             -mov ecx, 0xd
    cpu.ecx = 13 /*0xd*/;
    // 0051460b  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0051460d  83bc24b000000000       +cmp dword ptr [esp + 0xb0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(176) /* 0xb0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00514615  7429                   -je 0x514640
    if (cpu.flags.zf)
    {
        goto L_0x00514640;
    }
    // 00514617  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051461c  81c4fc000000           -add esp, 0xfc
    (cpu.esp) += x86::reg32(x86::sreg32(252 /*0xfc*/));
    // 00514622  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514623  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514624  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514625  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514626  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00514627:
    // 00514627  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00514629  7415                   -je 0x514640
    if (cpu.flags.zf)
    {
        goto L_0x00514640;
    }
    // 0051462b  83b8a400000000         +cmp dword ptr [eax + 0xa4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(164) /* 0xa4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00514632  750c                   -jne 0x514640
    if (!cpu.flags.zf)
    {
        goto L_0x00514640;
    }
    // 00514634  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 00514636  8d781c                 -lea edi, [eax + 0x1c]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 00514639  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0051463a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0051463b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0051463c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0051463d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0051463e  eb9c                   -jmp 0x5145dc
    goto L_0x005145dc;
L_0x00514640:
    // 00514640  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00514642  81c4fc000000           -add esp, 0xfc
    (cpu.esp) += x86::reg32(x86::sreg32(252 /*0xfc*/));
    // 00514648  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00514649  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051464a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051464b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051464c  c3                     -ret 
    cpu.esp += 4;
    return;
}

}
