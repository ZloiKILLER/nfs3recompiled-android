#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_native_vertices.py): defined in native_vertices.cpp.
bool commSendTrace(win32::WinApplication* app, x86::CPU& cpu);

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_518e40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00518e40  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00518e41  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00518e42  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00518e43  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00518e45  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00518e47  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 00518e49  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 00518e4b  8b5804                 -mov ebx, dword ptr [eax + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00518e4e  0fafd3                 -imul edx, ebx
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 00518e51  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00518e53  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00518e56  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00518e58  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00518e5a  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00518e5c  8b5108                 -mov edx, dword ptr [ecx + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 00518e5f  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00518e61  e88a16fdff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00518e66  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 00518e6b  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00518e6d  8b510c                 -mov edx, dword ptr [ecx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 00518e70  e8dbfeffff             -call 0x518d50
    cpu.esp -= 4;
    sub_518d50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00518e75  bb40000000             -mov ebx, 0x40
    cpu.ebx = 64 /*0x40*/;
    // 00518e7a  8b5110                 -mov edx, dword ptr [ecx + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */);
    // 00518e7d  8b410c                 -mov eax, dword ptr [ecx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 00518e80  e86b16fdff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00518e85  803d1050560008         +cmp byte ptr [0x565010], 8
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5656592) /* 0x565010 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(8 /*0x8*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00518e8c  7609                   -jbe 0x518e97
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00518e97;
    }
    // 00518e8e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00518e93  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518e94  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518e95  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518e96  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00518e97:
    // 00518e97  8b4110                 -mov eax, dword ptr [ecx + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */);
    // 00518e9a  8d5040                 -lea edx, [eax + 0x40]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 00518e9d  e8eef7ffff             -call 0x518690
    cpu.esp -= 4;
    sub_518690(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00518ea2  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00518ea7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518ea8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518ea9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518eaa  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_518eb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00518eb0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00518eb1  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00518eb3  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00518eb6  e8d589fcff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00518ebb  8b420c                 -mov eax, dword ptr [edx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    // 00518ebe  e8cd89fcff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00518ec3  8b4210                 -mov eax, dword ptr [edx + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(16) /* 0x10 */);
    // 00518ec6  e8c589fcff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00518ecb  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00518ecd  e8be89fcff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00518ed2  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518ed3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_518ee0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00518ee0  a334905600             -mov dword ptr [0x569034], eax
    app->getMemory<x86::reg32>(x86::reg32(5673012) /* 0x569034 */) = cpu.eax;
    // 00518ee5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00518eea  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_518ef0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    // 00518ef0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00518ef1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00518ef2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00518ef3  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00518ef6  8d2c8500000000         -lea ebp, [eax*4]
    cpu.ebp = x86::reg32(cpu.eax * 4);
    // 00518efd  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 00518eff  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00518f01  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 00518f03  7e68                   -jle 0x518f6d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00518f6d;
    }
    // 00518f05  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
L_0x00518f06:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00518f06  81c680000000           -add esi, 0x80
    (cpu.esi) += x86::reg32(x86::sreg32(128 /*0x80*/));
    // 00518f0c  d946a0                 -fld dword ptr [esi - 0x60]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(-96) /* -0x60 */));
    // 00518f0f  d946c4                 -fld dword ptr [esi - 0x3c]
    fp1 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(-60) /* -0x3c */));
    // 00518f12  83c704                 -add edi, 4
    (cpu.edi) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00518f15  8b4680                 -mov eax, dword ptr [esi - 0x80]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(-128) /* -0x80 */);
    // 00518f18  8b5e98                 -mov ebx, dword ptr [esi - 0x68]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(-104) /* -0x68 */);
    // 00518f1b  8b4e9c                 -mov ecx, dword ptr [esi - 0x64]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(-100) /* -0x64 */);
    // 00518f1e  89442404               -mov dword ptr [esp + 4], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 00518f22  8b4684                 -mov eax, dword ptr [esi - 0x7c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(-124) /* -0x7c */);
    // 00518f25  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00518f27  d8642404               -fsub dword ptr [esp + 4]
    fp1 = fp::sub(fp1, x86::Float(app->getMemory<float>(cpu.esp + x86::reg32(4) /* 0x4 */)));
    // 00518f2b  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 00518f2f  d95c240c               -fstp dword ptr [esp + 0xc]
    app->getMemory<float>(cpu.esp + x86::reg32(12) /* 0xc */) = float(fp1);
    // 00518f33  d8642408               -fsub dword ptr [esp + 8]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.esp + x86::reg32(8) /* 0x8 */)));
    // 00518f37  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00518f3a  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00518f3d  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518f3e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00518f3f  d9442410               -fld dword ptr [esp + 0x10]
    fp0 = x86::Float(app->getMemory<float>(cpu.esp + x86::reg32(16) /* 0x10 */));
    // 00518f43  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00518f46  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00518f49  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518f4a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00518f4b  d9442410               -fld dword ptr [esp + 0x10]
    fp0 = x86::Float(app->getMemory<float>(cpu.esp + x86::reg32(16) /* 0x10 */));
    // 00518f4f  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00518f52  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00518f55  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518f56  d944240c               -fld dword ptr [esp + 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.esp + x86::reg32(12) /* 0xc */));
    // 00518f5a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00518f5c  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00518f5f  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00518f62  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518f63  e808f9ffff             -call 0x518870
    cpu.esp -= 4;
    sub_518870(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00518f68  39ef                   +cmp edi, ebp
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00518f6a  7c9a                   -jl 0x518f06
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00518f06;
    }
    // 00518f6c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x00518f6d:
    // 00518f6d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00518f72  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00518f75  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518f76  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518f77  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518f78  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_518f80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00518f80  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00518f81  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00518f82  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00518f83  ba80075500             -mov edx, 0x550780
    cpu.edx = 5572480 /*0x550780*/;
    // 00518f88  b990075500             -mov ecx, 0x550790
    cpu.ecx = 5572496 /*0x550790*/;
    // 00518f8d  bb27010000             -mov ebx, 0x127
    cpu.ebx = 295 /*0x127*/;
    // 00518f92  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 00518f98  891d98215500           -mov dword ptr [0x552198], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebx;
    // 00518f9e  8b1df4435600           -mov ebx, dword ptr [0x5643f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 00518fa4  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00518fa6  b8a0075500             -mov eax, 0x5507a0
    cpu.eax = 5572512 /*0x5507a0*/;
    // 00518fab  890d94215500           -mov dword ptr [0x552194], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ecx;
    // 00518fb1  e86a86fcff             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00518fb6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518fb7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518fb8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00518fb9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_518fc0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00518fc0  e9cb88fcff             -jmp 0x4e1890
    return nfs3hp::ea::purgememadr(app, cpu.handOver());
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_518fd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00518fd0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00518fd1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00518fd2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00518fd3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00518fd4  8b1d54905600           -mov ebx, dword ptr [0x569054]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5673044) /* 0x569054 */);
    // 00518fda  8b355c905600           -mov esi, dword ptr [0x56905c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5673052) /* 0x56905c */);
    // 00518fe0  833d5890560000         +cmp dword ptr [0x569058], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5673048) /* 0x569058 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00518fe7  0f8573000000           -jne 0x519060
    if (!cpu.flags.zf)
    {
        goto L_0x00519060;
    }
    // 00518fed  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00518fef  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00518ff1  7505                   -jne 0x518ff8
    if (!cpu.flags.zf)
    {
        goto L_0x00518ff8;
    }
    // 00518ff3  bb90010000             -mov ebx, 0x190
    cpu.ebx = 400 /*0x190*/;
L_0x00518ff8:
    // 00518ff8  8b158c715600           -mov edx, dword ptr [0x56718c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5665164) /* 0x56718c */);
    // 00518ffe  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00519000  891d54905600           -mov dword ptr [0x569054], ebx
    app->getMemory<x86::reg32>(x86::reg32(5673044) /* 0x569054 */) = cpu.ebx;
    // 00519006  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 00519009  89355c905600           -mov dword ptr [0x56905c], esi
    app->getMemory<x86::reg32>(x86::reg32(5673052) /* 0x56905c */) = cpu.esi;
    // 0051900f  ff5214                 -call dword ptr [edx + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00519012  8b1d54905600           -mov ebx, dword ptr [0x569054]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5673044) /* 0x569054 */);
    // 00519018  8b158c715600           -mov edx, dword ptr [0x56718c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5665164) /* 0x56718c */);
    // 0051901e  a358905600             -mov dword ptr [0x569058], eax
    app->getMemory<x86::reg32>(x86::reg32(5673048) /* 0x569058 */) = cpu.eax;
    // 00519023  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 0051902a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051902c  ff5214                 -call dword ptr [edx + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051902f  8b1d54905600           -mov ebx, dword ptr [0x569054]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5673044) /* 0x569054 */);
    // 00519035  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00519037  890d60905600           -mov dword ptr [0x569060], ecx
    app->getMemory<x86::reg32>(x86::reg32(5673056) /* 0x569060 */) = cpu.ecx;
    // 0051903d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051903f  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00519041  7e1d                   -jle 0x519060
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00519060;
    }
    // 00519043  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 00519045  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00519047:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00519047  890411                 -mov dword ptr [ecx + edx], eax
    app->getMemory<x86::reg32>(cpu.ecx + cpu.edx * 1) = cpu.eax;
    // 0051904a  40                     -inc eax
    (cpu.eax)++;
    // 0051904b  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051904e  39d8                   +cmp eax, ebx
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
    // 00519050  7cf5                   -jl 0x519047
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00519047;
    }
    // 00519052  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 00519058  8d9200000000           -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
    // 0051905e  8bc0                   -mov eax, eax
    cpu.eax = cpu.eax;
L_0x00519060:
    // 00519060  89355c905600           -mov dword ptr [0x56905c], esi
    app->getMemory<x86::reg32>(x86::reg32(5673052) /* 0x56905c */) = cpu.esi;
    // 00519066  891d54905600           -mov dword ptr [0x569054], ebx
    app->getMemory<x86::reg32>(x86::reg32(5673044) /* 0x569054 */) = cpu.ebx;
    // 0051906c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051906d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051906e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051906f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519070  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_519080: native, nfs3hp::dead::stub_0x519080 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_519140(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00519140  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00519141  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00519142  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00519143  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00519144  833d5890560000         +cmp dword ptr [0x569058], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5673048) /* 0x569058 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051914b  751d                   -jne 0x51916a
    if (!cpu.flags.zf)
    {
        goto L_0x0051916a;
    }
L_0x0051914d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051914d  8b1d5c905600           -mov ebx, dword ptr [0x56905c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5673052) /* 0x56905c */);
    // 00519153  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00519155  752b                   -jne 0x519182
    if (!cpu.flags.zf)
    {
        goto L_0x00519182;
    }
    // 00519157  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00519159  893d54905600           -mov dword ptr [0x569054], edi
    app->getMemory<x86::reg32>(x86::reg32(5673044) /* 0x569054 */) = cpu.edi;
    // 0051915f  893d60905600           -mov dword ptr [0x569060], edi
    app->getMemory<x86::reg32>(x86::reg32(5673056) /* 0x569060 */) = cpu.edi;
    // 00519165  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519166  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519167  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519168  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519169  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051916a:
    // 0051916a  8b158c715600           -mov edx, dword ptr [0x56718c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5665164) /* 0x56718c */);
    // 00519170  a158905600             -mov eax, dword ptr [0x569058]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5673048) /* 0x569058 */);
    // 00519175  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00519177  ff5218                 -call dword ptr [edx + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051917a  890d58905600           -mov dword ptr [0x569058], ecx
    app->getMemory<x86::reg32>(x86::reg32(5673048) /* 0x569058 */) = cpu.ecx;
    // 00519180  ebcb                   -jmp 0x51914d
    goto L_0x0051914d;
L_0x00519182:
    // 00519182  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00519183  8b158c715600           -mov edx, dword ptr [0x56718c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5665164) /* 0x56718c */);
    // 00519189  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051918b  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0051918d  ff5218                 -call dword ptr [edx + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00519190  89355c905600           -mov dword ptr [0x56905c], esi
    app->getMemory<x86::reg32>(x86::reg32(5673052) /* 0x56905c */) = cpu.esi;
    // 00519196  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519197  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00519199  893d54905600           -mov dword ptr [0x569054], edi
    app->getMemory<x86::reg32>(x86::reg32(5673044) /* 0x569054 */) = cpu.edi;
    // 0051919f  893d60905600           -mov dword ptr [0x569060], edi
    app->getMemory<x86::reg32>(x86::reg32(5673056) /* 0x569060 */) = cpu.edi;
    // 005191a5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005191a6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005191a7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005191a8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005191a9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_5191b0: native, nfs3hp::dead::stub_0x5191b0 */

/* sub_519560: native, nfs3hp::dead::stub_0x519560 */

/* sub_5195b0: native, nfs3hp::dead::stub_0x5195b0 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_5199b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005199b0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005199b1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005199b2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005199b3  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 005199b5  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 005199b7  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 005199b9  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 005199bb  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 005199bd  7436                   -je 0x5199f5
    if (cpu.flags.zf)
    {
        goto L_0x005199f5;
    }
L_0x005199bf:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005199bf  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 005199c1  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 005199c3  0fafc6                 -imul eax, esi
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.esi)));
    // 005199c6  8d1c07                 -lea ebx, [edi + eax]
    cpu.ebx = x86::reg32(cpu.edi + cpu.eax * 1);
    // 005199c9  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 005199ce  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 005199d0  8b4401fc               -mov eax, dword ptr [ecx + eax - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(-4) /* -0x4 */ + cpu.eax * 1);
    // 005199d4  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 005199d6  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 005199dd  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 005199df  89e9                   -mov ecx, ebp
    cpu.ecx = cpu.ebp;
    // 005199e1  29c1                   +sub ecx, eax
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
    // 005199e3  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 005199e5  7414                   -je 0x5199fb
    if (cpu.flags.zf)
    {
        goto L_0x005199fb;
    }
    // 005199e7  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 005199e9  7e04                   -jle 0x5199ef
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x005199ef;
    }
    // 005199eb  4a                     -dec edx
    (cpu.edx)--;
    // 005199ec  8d3c33                 -lea edi, [ebx + esi]
    cpu.edi = x86::reg32(cpu.ebx + cpu.esi * 1);
L_0x005199ef:
    // 005199ef  d1fa                   -sar edx, 1
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (1 /*0x1*/ % 32));
    // 005199f1  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 005199f3  75ca                   -jne 0x5199bf
    if (!cpu.flags.zf)
    {
        goto L_0x005199bf;
    }
L_0x005199f5:
    // 005199f5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005199f7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005199f8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005199f9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005199fa  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005199fb:
    // 005199fb  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 005199fd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005199fe  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005199ff  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519a00  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_519a10: native, nfs3hp::dead::stub_0x519a10 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_519a50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00519a50  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00519a51  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00519a52  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00519a53  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00519a54  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00519a55  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00519a57  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 00519a59  8b5028                 -mov edx, dword ptr [eax + 0x28]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(40) /* 0x28 */);
    // 00519a5c  8b7830                 -mov edi, dword ptr [eax + 0x30]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */);
    // 00519a5f  83fa64                 +cmp edx, 0x64
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(100 /*0x64*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00519a62  7d2d                   -jge 0x519a91
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00519a91;
    }
    // 00519a64  b9ac075500             -mov ecx, 0x5507ac
    cpu.ecx = 5572524 /*0x5507ac*/;
    // 00519a69  bdbc075500             -mov ebp, 0x5507bc
    cpu.ebp = 5572540 /*0x5507bc*/;
    // 00519a6e  b860010000             -mov eax, 0x160
    cpu.eax = 352 /*0x160*/;
    // 00519a73  68d0075500             -push 0x5507d0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5572560 /*0x5507d0*/;
    cpu.esp -= 4;
    // 00519a78  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 00519a7e  892d94215500           -mov dword ptr [0x552194], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebp;
    // 00519a84  a398215500             -mov dword ptr [0x552198], eax
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.eax;
    // 00519a89  e88275eeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00519a8e  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x00519a91:
    // 00519a91  8d56e0                 -lea edx, [esi - 0x20]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(-32) /* -0x20 */);
    // 00519a94  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00519a9b  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00519a9d  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00519aa0  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00519aa2  8d1407                 -lea edx, [edi + eax]
    cpu.edx = x86::reg32(cpu.edi + cpu.eax * 1);
    // 00519aa5  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 00519aaa  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00519aac  8b4401fc               -mov eax, dword ptr [ecx + eax - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(-4) /* -0x4 */ + cpu.eax * 1);
    // 00519ab0  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00519ab2  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00519ab9  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00519abb  39f0                   +cmp eax, esi
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
    // 00519abd  7508                   -jne 0x519ac7
    if (!cpu.flags.zf)
    {
        goto L_0x00519ac7;
    }
    // 00519abf  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00519ac1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519ac2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519ac3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519ac4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519ac5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519ac6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00519ac7:
    // 00519ac7  b90b000000             -mov ecx, 0xb
    cpu.ecx = 11 /*0xb*/;
    // 00519acc  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 00519ace  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00519ad0  8b5b20                 -mov ebx, dword ptr [ebx + 0x20]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(32) /* 0x20 */);
    // 00519ad3  e8d8feffff             -call 0x5199b0
    cpu.esp -= 4;
    sub_5199b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00519ad8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519ad9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519ada  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519adb  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519adc  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519add  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_519ae0: native, nfs3hp::dead::stub_0x519ae0 */

/* sub_519e30: native, nfs3hp::dead::stub_0x519e30 */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_519ea0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00519ea0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00519ea1  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00519ea3  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00519ea8  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 00519ead  83f850                 +cmp eax, 0x50
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
    // 00519eb0  7313                   -jae 0x519ec5
    if (!cpu.flags.cf)
    {
        goto L_0x00519ec5;
    }
    // 00519eb2  83f803                 +cmp eax, 3
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
    // 00519eb5  733c                   -jae 0x519ef3
    if (!cpu.flags.cf)
    {
        goto L_0x00519ef3;
    }
    // 00519eb7  83f801                 +cmp eax, 1
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
    // 00519eba  7505                   -jne 0x519ec1
    if (!cpu.flags.zf)
    {
        goto L_0x00519ec1;
    }
L_0x00519ebc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00519ebc  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00519ec1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00519ec1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00519ec3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519ec4  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00519ec5:
    // 00519ec5  7623                   -jbe 0x519eea
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00519eea;
    }
    // 00519ec7  83f863                 +cmp eax, 0x63
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(99 /*0x63*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00519eca  7310                   -jae 0x519edc
    if (!cpu.flags.cf)
    {
        goto L_0x00519edc;
    }
    // 00519ecc  83f856                 +cmp eax, 0x56
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(86 /*0x56*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00519ecf  72f0                   -jb 0x519ec1
    if (cpu.flags.cf)
    {
        goto L_0x00519ec1;
    }
    // 00519ed1  76e9                   -jbe 0x519ebc
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00519ebc;
    }
    // 00519ed3  83f860                 +cmp eax, 0x60
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(96 /*0x60*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00519ed6  74e4                   -je 0x519ebc
    if (cpu.flags.zf)
    {
        goto L_0x00519ebc;
    }
    // 00519ed8  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00519eda  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519edb  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00519edc:
    // 00519edc  760c                   -jbe 0x519eea
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00519eea;
    }
    // 00519ede  83f879                 +cmp eax, 0x79
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(121 /*0x79*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00519ee1  72de                   -jb 0x519ec1
    if (cpu.flags.cf)
    {
        goto L_0x00519ec1;
    }
    // 00519ee3  76d7                   -jbe 0x519ebc
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00519ebc;
    }
    // 00519ee5  83f87a                 +cmp eax, 0x7a
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(122 /*0x7a*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00519ee8  75d7                   -jne 0x519ec1
    if (!cpu.flags.zf)
    {
        goto L_0x00519ec1;
    }
L_0x00519eea:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00519eea  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 00519eef  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00519ef1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519ef2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00519ef3:
    // 00519ef3  76f5                   -jbe 0x519eea
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00519eea;
    }
    // 00519ef5  83f840                 +cmp eax, 0x40
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00519ef8  72c7                   -jb 0x519ec1
    if (cpu.flags.cf)
    {
        goto L_0x00519ec1;
    }
    // 00519efa  76ee                   -jbe 0x519eea
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00519eea;
    }
    // 00519efc  83f844                 +cmp eax, 0x44
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(68 /*0x44*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00519eff  74bb                   -je 0x519ebc
    if (cpu.flags.zf)
    {
        goto L_0x00519ebc;
    }
    // 00519f01  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00519f03  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519f04  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_519f10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00519f10  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00519f11  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00519f12  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00519f13  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00519f14  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00519f17  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 00519f19  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00519f1b  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 00519f1e  01e8                   -add eax, ebp
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebp));
    // 00519f20  8b158c715600           -mov edx, dword ptr [0x56718c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5665164) /* 0x56718c */);
    // 00519f26  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 00519f29  b8e0000000             -mov eax, 0xe0
    cpu.eax = 224 /*0xe0*/;
    // 00519f2e  ff5214                 -call dword ptr [edx + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00519f31  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00519f33  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00519f35  750a                   -jne 0x519f41
    if (!cpu.flags.zf)
    {
        goto L_0x00519f41;
    }
    // 00519f37  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00519f39  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00519f3c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519f3d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519f3e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519f3f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00519f40  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00519f41:
    // 00519f41  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00519f42  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00519f43  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 00519f48  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00519f4a  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00519f4c  0fc8                   -bswap eax
    {
        x86::reg32& tmp = cpu.eax;
        tmp = ( tmp               << 16) ^  (tmp >> 16);
        tmp = ((tmp & 0x00ff00ff) <<  8) ^ ((tmp >>  8) & 0x00ff00ff);
    }
    // 00519f4e  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00519f50  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00519f57  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00519f59  30c0                   -xor al, al
    cpu.al ^= x86::reg8(x86::sreg8(cpu.al));
    // 00519f5b  3d00544e46             +cmp eax, 0x464e5400
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1179538432 /*0x464e5400*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00519f60  742f                   -je 0x519f91
    if (cpu.flags.zf)
    {
        goto L_0x00519f91;
    }
    // 00519f62  ba40085500             -mov edx, 0x550840
    cpu.edx = 5572672 /*0x550840*/;
    // 00519f67  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00519f68  b950085500             -mov ecx, 0x550850
    cpu.ecx = 5572688 /*0x550850*/;
    // 00519f6d  bb6d000000             -mov ebx, 0x6d
    cpu.ebx = 109 /*0x6d*/;
    // 00519f72  685c085500             -push 0x55085c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5572700 /*0x55085c*/;
    cpu.esp -= 4;
    // 00519f77  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 00519f7d  890d94215500           -mov dword ptr [0x552194], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ecx;
    // 00519f83  891d98215500           -mov dword ptr [0x552198], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebx;
    // 00519f89  e88270eeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00519f8e  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
L_0x00519f91:
    // 00519f91  bbe0000000             -mov ebx, 0xe0
    cpu.ebx = 224 /*0xe0*/;
    // 00519f96  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00519f98  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00519f9a  e8a166fcff             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00519f9f  c7462864000000         -mov dword ptr [esi + 0x28], 0x64
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(40) /* 0x28 */) = 100 /*0x64*/;
    // 00519fa6  8b470d                 -mov eax, dword ptr [edi + 0xd]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 00519fa9  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 00519fac  894604                 -mov dword ptr [esi + 4], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 00519faf  8b470e                 -mov eax, dword ptr [edi + 0xe]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 00519fb2  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 00519fb5  894608                 -mov dword ptr [esi + 8], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 00519fb8  8b470f                 -mov eax, dword ptr [edi + 0xf]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(15) /* 0xf */);
    // 00519fbb  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 00519fbe  894610                 -mov dword ptr [esi + 0x10], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 00519fc1  8b4710                 -mov eax, dword ptr [edi + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */);
    // 00519fc4  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 00519fc7  894614                 -mov dword ptr [esi + 0x14], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 00519fca  8b5710                 -mov edx, dword ptr [edi + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */);
    // 00519fcd  8b470f                 -mov eax, dword ptr [edi + 0xf]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(15) /* 0xf */);
    // 00519fd0  c1fa18                 -sar edx, 0x18
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (24 /*0x18*/ % 32));
    // 00519fd3  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 00519fd6  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00519fd8  895618                 -mov dword ptr [esi + 0x18], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */) = cpu.edx;
    // 00519fdb  8b5710                 -mov edx, dword ptr [edi + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */);
    // 00519fde  8b470f                 -mov eax, dword ptr [edi + 0xf]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(15) /* 0xf */);
    // 00519fe1  c1fa18                 -sar edx, 0x18
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (24 /*0x18*/ % 32));
    // 00519fe4  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 00519fe7  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00519fe9  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00519feb  89561c                 -mov dword ptr [esi + 0x1c], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(28) /* 0x1c */) = cpu.edx;
    // 00519fee  668b470a               -mov ax, word ptr [edi + 0xa]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edi + x86::reg32(10) /* 0xa */);
    // 00519ff2  c7464800000000         -mov dword ptr [esi + 0x48], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(72) /* 0x48 */) = 0 /*0x0*/;
    // 00519ff9  894620                 -mov dword ptr [esi + 0x20], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */) = cpu.eax;
    // 00519ffc  896e2c                 -mov dword ptr [esi + 0x2c], ebp
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(44) /* 0x2c */) = cpu.ebp;
    // 00519fff  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051a003  894634                 -mov dword ptr [esi + 0x34], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(52) /* 0x34 */) = cpu.eax;
    // 0051a006  8b4714                 -mov eax, dword ptr [edi + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(20) /* 0x14 */);
    // 0051a009  8d1407                 -lea edx, [edi + eax]
    cpu.edx = x86::reg32(cpu.edi + cpu.eax * 1);
    // 0051a00c  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051a010  895630                 -mov dword ptr [esi + 0x30], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(48) /* 0x30 */) = cpu.edx;
    // 0051a013  8b542408               -mov edx, dword ptr [esp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051a017  e884feffff             -call 0x519ea0
    cpu.esp -= 4;
    sub_519ea0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a01c  89460c                 -mov dword ptr [esi + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 0051a01f  8b5202                 -mov edx, dword ptr [edx + 2]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(2) /* 0x2 */);
    // 0051a022  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051a026  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0051a029  e872feffff             -call 0x519ea0
    cpu.esp -= 4;
    sub_519ea0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a02e  0fafc2                 -imul eax, edx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 0051a031  83c007                 -add eax, 7
    (cpu.eax) += x86::reg32(x86::sreg32(7 /*0x7*/));
    // 0051a034  24f8                   -and al, 0xf8
    cpu.al &= x86::reg8(x86::sreg8(248 /*0xf8*/));
    // 0051a036  c1f803                 -sar eax, 3
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (3 /*0x3*/ % 32));
    // 0051a039  894624                 -mov dword ptr [esi + 0x24], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = cpu.eax;
    // 0051a03c  8b470c                 -mov eax, dword ptr [edi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(12) /* 0xc */);
    // 0051a03f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0051a042  83e003                 -and eax, 3
    cpu.eax &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0051a045  83f802                 +cmp eax, 2
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
    // 0051a048  0f85de000000           -jne 0x51a12c
    if (!cpu.flags.zf)
    {
        goto L_0x0051a12c;
    }
L_0x0051a04e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a04e  c7464070565200         -mov dword ptr [esi + 0x40], 0x525670
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */) = 5396080 /*0x525670*/;
L_0x0051a055:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a055  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a057  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051a059  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051a05b  e820bb0000             -call 0x525b80
    cpu.esp -= 4;
    sub_525b80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a060  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a062  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051a064  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051a066  e855b70000             -call 0x5257c0
    cpu.esp -= 4;
    sub_5257c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a06b  ba05000000             -mov edx, 5
    cpu.edx = 5 /*0x5*/;
    // 0051a070  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a072  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051a074  e847b70000             -call 0x5257c0
    cpu.esp -= 4;
    sub_5257c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a079  ba06000000             -mov edx, 6
    cpu.edx = 6 /*0x6*/;
    // 0051a07e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a080  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051a082  e839b70000             -call 0x5257c0
    cpu.esp -= 4;
    sub_5257c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a087  ba07000000             -mov edx, 7
    cpu.edx = 7 /*0x7*/;
    // 0051a08c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a08e  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051a090  e82bb70000             -call 0x5257c0
    cpu.esp -= 4;
    sub_5257c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a095  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 0051a09a  ba0c000000             -mov edx, 0xc
    cpu.edx = 12 /*0xc*/;
    // 0051a09f  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a0a1  e81ab70000             -call 0x5257c0
    cpu.esp -= 4;
    sub_5257c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a0a6  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 0051a0ab  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 0051a0b0  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a0b2  e809b70000             -call 0x5257c0
    cpu.esp -= 4;
    sub_5257c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a0b7  bbffffff00             -mov ebx, 0xffffff
    cpu.ebx = 16777215 /*0xffffff*/;
    // 0051a0bc  ba09000000             -mov edx, 9
    cpu.edx = 9 /*0x9*/;
    // 0051a0c1  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a0c3  e8f8b60000             -call 0x5257c0
    cpu.esp -= 4;
    sub_5257c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a0c8  bb000000ff             -mov ebx, 0xff000000
    cpu.ebx = 4278190080 /*0xff000000*/;
    // 0051a0cd  ba0a000000             -mov edx, 0xa
    cpu.edx = 10 /*0xa*/;
    // 0051a0d2  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a0d4  e8e7b60000             -call 0x5257c0
    cpu.esp -= 4;
    sub_5257c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a0d9  bb000000ff             -mov ebx, 0xff000000
    cpu.ebx = 4278190080 /*0xff000000*/;
    // 0051a0de  ba0b000000             -mov edx, 0xb
    cpu.edx = 11 /*0xb*/;
    // 0051a0e3  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a0e5  e8d6b60000             -call 0x5257c0
    cpu.esp -= 4;
    sub_5257c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a0ea  ba41000000             -mov edx, 0x41
    cpu.edx = 65 /*0x41*/;
    // 0051a0ef  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a0f1  c686bc00000001         -mov byte ptr [esi + 0xbc], 1
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(188) /* 0xbc */) = 1 /*0x1*/;
    // 0051a0f8  e853f9ffff             -call 0x519a50
    cpu.esp -= 4;
    sub_519a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a0fd  8b7807                 -mov edi, dword ptr [eax + 7]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7) /* 0x7 */);
    // 0051a100  0fb66803               -movzx ebp, byte ptr [eax + 3]
    cpu.ebp = x86::reg32(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(3) /* 0x3 */));
    // 0051a104  c1ff18                 -sar edi, 0x18
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (24 /*0x18*/ % 32));
    // 0051a107  01fd                   -add ebp, edi
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.edi));
    // 0051a109  8b7e10                 -mov edi, dword ptr [esi + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 0051a10c  29ef                   -sub edi, ebp
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 0051a10e  897e14                 -mov dword ptr [esi + 0x14], edi
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */) = cpu.edi;
    // 0051a111  0fb67803               -movzx edi, byte ptr [eax + 3]
    cpu.edi = x86::reg32(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(3) /* 0x3 */));
    // 0051a115  8b4007                 -mov eax, dword ptr [eax + 7]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7) /* 0x7 */);
    // 0051a118  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 0051a11b  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051a11d  897e10                 -mov dword ptr [esi + 0x10], edi
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */) = cpu.edi;
    // 0051a120  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a121  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a122  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a124  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051a127  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a128  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a129  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a12a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a12b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051a12c:
    // 0051a12c  817e20ff000000         +cmp dword ptr [esi + 0x20], 0xff
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(255 /*0xff*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a133  0f8f15ffffff           -jg 0x51a04e
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0051a04e;
    }
    // 0051a139  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0051a13e  8d4520                 -lea eax, [ebp + 0x20]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(32) /* 0x20 */);
    // 0051a141  8b4401fc               -mov eax, dword ptr [ecx + eax - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(-4) /* -0x4 */ + cpu.eax * 1);
    // 0051a145  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 0051a147  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 0051a14e  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 0051a150  3dff000000             +cmp eax, 0xff
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
    // 0051a155  0f87f3feffff           -ja 0x51a04e
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051a04e;
    }
    // 0051a15b  c7464030565200         -mov dword ptr [esi + 0x40], 0x525630
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */) = 5396016 /*0x525630*/;
    // 0051a162  e9eefeffff             -jmp 0x51a055
    goto L_0x0051a055;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_51a170(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051a170  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051a171  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051a172  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051a174  e887b40000             -call 0x525600
    cpu.esp -= 4;
    sub_525600(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a179  8b0d8c715600           -mov ecx, dword ptr [0x56718c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5665164) /* 0x56718c */);
    // 0051a17f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051a181  ff5118                 -call dword ptr [ecx + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a184  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a185  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a186  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51a188: native, nfs3hp::dead::stub_0x51a188 */

/* align: skip  */
void Application::sub_51a1b8(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051a1b8  b003                   -mov al, 3
    cpu.al = 3 /*0x3*/;
    // 0051a1ba  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051a1bb  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0051a1bd  d9e4                   -ftst 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = cpu.fpu.st(0);
    cpu.fpu.compare(fpu1, 0.0);
    // 0051a1bf  83ec10                 +sub esp, 0x10
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051a1c2  9b                     -wait 
    /*nothing*/;
    // 0051a1c3  dd7df8                 -fnstsw word ptr [ebp - 8]
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.fpu.status.word;
    // 0051a1c6  9b                     -wait 
    /*nothing*/;
    // 0051a1c7  668745f8               -xchg word ptr [ebp - 8], ax
    {
        x86::reg16 tmp = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ax;
        cpu.ax = tmp;
    }
    // 0051a1cb  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 0051a1cc  7319                   -jae 0x51a1e7
    if (!cpu.flags.cf)
    {
        goto L_0x0051a1e7;
    }
    // 0051a1ce  dd5df0                 -fstp qword ptr [ebp - 0x10]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu2 = cpu.fpu.st(0);
    app->getMemory<double>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = double(fpu2);
    // 0051a1d1  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0051a1d4  3c03                   +cmp al, 3
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
    cpu.fpu.count -= 1;
    }
    // 0051a1d6  7403                   -je 0x51a1db
    if (cpu.flags.zf)
    {
        goto L_0x0051a1db;
    }
    // 0051a1d8  dd5df0                 +fstp qword ptr [ebp - 0x10]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu3 = cpu.fpu.st(0);
    app->getMemory<double>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = double(fpu3);
    cpu.fpu.count -= 1;
    }
L_0x0051a1db:
    // 0051a1db  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0051a1de  e879c3ffff             -call 0x51655c
    cpu.esp -= 4;
    nfs3hp::watcom::math87_err(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a1e3  b001                   -mov al, 1
    cpu.al = 1 /*0x1*/;
    // 0051a1e5  eb04                   -jmp 0x51a1eb
    goto L_0x0051a1eb;
L_0x0051a1e7:
    // 0051a1e7  d9fa                   -fsqrt 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu4 = cpu.fpu.st(0);
    x86::Float fpu5 = fp::sqrt(fpu4);
    // 0051a1e9  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    cpu.fpu.st(0) = fpu5;
    }
L_0x0051a1eb:
    // 0051a1eb  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0051a1ed  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a1ee  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51a1ba: native, nfs3hp::watcom::DSQRT */

/* sub_51a1f0: native, nfs3hp::dead::stub_0x51a1f0 */

/* sub_51a200: native, nfs3hp::dead::stub_0x51a200 */

/* sub_51a260: native, nfs3hp::dead::stub_0x51a260 */

/* sub_51a2a0: native, nfs3hp::dead::stub_0x51a2a0 */

/* sub_51a310: native, nfs3hp::dead::stub_0x51a310 */

/* sub_51a3b0: native, nfs3hp::dead::stub_0x51a3b0 */

/* sub_51a3e0: native, nfs3hp::dead::stub_0x51a3e0 */

/* sub_51a400: native, nfs3hp::dead::stub_0x51a400 */

/* sub_51a420: native, nfs3hp::dead::stub_0x51a420 */

/* align: skip  */
void Application::sub_51a470(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051a470  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051a471  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051a472  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051a473  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051a474  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051a475  83ec40                 -sub esp, 0x40
    (cpu.esp) -= x86::reg32(x86::sreg32(64 /*0x40*/));
    // 0051a478  8944242c               -mov dword ptr [esp + 0x2c], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(44) /* 0x2c */) = cpu.eax;
    // 0051a47c  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0051a47e  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051a480  8b1db8aa5600           -mov ebx, dword ptr [0x56aab8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5679800) /* 0x56aab8 */);
    // 0051a486  89542434               -mov dword ptr [esp + 0x34], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(52) /* 0x34 */) = cpu.edx;
    // 0051a48a  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051a48c  7518                   -jne 0x51a4a6
    if (!cpu.flags.zf)
    {
        goto L_0x0051a4a6;
    }
    // 0051a48e  8b15d8435600           -mov edx, dword ptr [0x5643d8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653464) /* 0x5643d8 */);
    // 0051a494  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051a496  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051a499  c1e202                 +shl edx, 2
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
    // 0051a49c  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0051a49e  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 0051a4a1  a3b8aa5600             -mov dword ptr [0x56aab8], eax
    app->getMemory<x86::reg32>(x86::reg32(5679800) /* 0x56aab8 */) = cpu.eax;
L_0x0051a4a6:
    // 0051a4a6  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0051a4a8  8935b4ab5600           -mov dword ptr [0x56abb4], esi
    app->getMemory<x86::reg32>(x86::reg32(5680052) /* 0x56abb4 */) = cpu.esi;
    // 0051a4ae  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 0051a4b3  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0051a4b5  89442438               -mov dword ptr [esp + 0x38], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(56) /* 0x38 */) = cpu.eax;
    // 0051a4b9  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051a4bb  48                     -dec eax
    (cpu.eax)--;
    // 0051a4bc  31ed                   -xor ebp, ebp
    cpu.ebp ^= x86::reg32(x86::sreg32(cpu.ebp));
    // 0051a4be  8944243c               -mov dword ptr [esp + 0x3c], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(60) /* 0x3c */) = cpu.eax;
L_0x0051a4c2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a4c2  8b7c2438               -mov edi, dword ptr [esp + 0x38]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(56) /* 0x38 */);
    // 0051a4c6  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 0051a4cb  39f8                   +cmp eax, edi
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
    // 0051a4cd  0f8daa020000           -jge 0x51a77d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051a77d;
    }
    // 0051a4d3  833d5844560000         +cmp dword ptr [0x564458], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a4da  0f859d020000           -jne 0x51a77d
    if (!cpu.flags.zf)
    {
        goto L_0x0051a77d;
    }
    // 0051a4e0  89e3                   -mov ebx, esp
    cpu.ebx = cpu.esp;
    // 0051a4e2  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0051a4e7  8b44242c               -mov eax, dword ptr [esp + 0x2c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(44) /* 0x2c */);
    // 0051a4eb  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0051a4ed  01eb                   -add ebx, ebp
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ebp));
    // 0051a4ef  e8dc160000             -call 0x51bbd0
    cpu.esp -= 4;
    sub_51bbd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a4f4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a4f6  0f844c020000           -je 0x51a748
    if (cpu.flags.zf)
    {
        goto L_0x0051a748;
    }
    // 0051a4fc  8b54243c               -mov edx, dword ptr [esp + 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(60) /* 0x3c */);
    // 0051a500  8a242c                 -mov ah, byte ptr [esp + ebp]
    cpu.ah = app->getMemory<x86::reg8>(cpu.esp + cpu.ebp * 1);
    // 0051a503  42                     -inc edx
    (cpu.edx)++;
    // 0051a504  45                     -inc ebp
    (cpu.ebp)++;
    // 0051a505  8954243c               -mov dword ptr [esp + 0x3c], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(60) /* 0x3c */) = cpu.edx;
    // 0051a509  80fc20                 +cmp ah, 0x20
    {
        x86::reg8 tmp1 = cpu.ah;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(32 /*0x20*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051a50c  0f83d1000000           -jae 0x51a5e3
    if (!cpu.flags.cf)
    {
        goto L_0x0051a5e3;
    }
    // 0051a512  c6442cff5f             -mov byte ptr [esp + ebp - 1], 0x5f
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(-1) /* -0x1 */ + cpu.ebp * 1) = 95 /*0x5f*/;
L_0x0051a517:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a517  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0051a519:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a519  8b4c2434               -mov ecx, dword ptr [esp + 0x34]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(52) /* 0x34 */);
    // 0051a51d  89442430               -mov dword ptr [esp + 0x30], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(48) /* 0x30 */) = cpu.eax;
    // 0051a521  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0051a523  0f84d6000000           -je 0x51a5ff
    if (cpu.flags.zf)
    {
        goto L_0x0051a5ff;
    }
L_0x0051a529:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a529  30ff                   -xor bh, bh
    cpu.bh ^= x86::reg8(x86::sreg8(cpu.bh));
    // 0051a52b  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0051a52d  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0051a52f  883c2c                 -mov byte ptr [esp + ebp], bh
    app->getMemory<x86::reg8>(cpu.esp + cpu.ebp * 1) = cpu.bh;
L_0x0051a532:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a532  8b0db8ab5600           -mov ecx, dword ptr [0x56abb8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5680056) /* 0x56abb8 */);
    // 0051a538  49                     -dec ecx
    (cpu.ecx)--;
    // 0051a539  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0051a53b  0f8c1b010000           -jl 0x51a65c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051a65c;
    }
    // 0051a541  6bd90c                 -imul ebx, ecx, 0xc
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(12 /*0xc*/)));
    // 0051a544  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
L_0x0051a546:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a546  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051a548  8b93c0aa5600           -mov edx, dword ptr [ebx + 0x56aac0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5679808) /* 0x56aac0 */);
    // 0051a54e  e8cd62fcff             -call 0x4e0820
    cpu.esp -= 4;
    nfs3hp::watcom::strstr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a553  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a555  0f84f5000000           -je 0x51a650
    if (cpu.flags.zf)
    {
        goto L_0x0051a650;
    }
    // 0051a55b  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051a55d  0f8eb1000000           -jle 0x51a614
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051a614;
    }
    // 0051a563  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0051a568:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a568  8b2dbcab5600           -mov ebp, dword ptr [0x56abbc]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(5680060) /* 0x56abbc */);
    // 0051a56e  09c5                   -or ebp, eax
    cpu.ebp |= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a570  a154445600             -mov eax, dword ptr [0x564454]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653588) /* 0x564454 */);
    // 0051a575  892dbcab5600           -mov dword ptr [0x56abbc], ebp
    app->getMemory<x86::reg32>(x86::reg32(5680060) /* 0x56abbc */) = cpu.ebp;
    // 0051a57b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a57d  742a                   -je 0x51a5a9
    if (cpu.flags.zf)
    {
        goto L_0x0051a5a9;
    }
    // 0051a57f  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0051a586  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051a588  8b1485c0aa5600         -mov edx, dword ptr [eax*4 + 0x56aac0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5679808) /* 0x56aac0 */ + cpu.eax * 4);
    // 0051a58f  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051a590  8d442404               -lea eax, [esp + 4]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051a594  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051a595  68dc0a5500             -push 0x550adc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573340 /*0x550adc*/;
    cpu.esp -= 4;
    // 0051a59a  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051a59c  6854445600             -push 0x564454
    app->getMemory<x86::reg32>(cpu.esp-4) = 5653588 /*0x564454*/;
    cpu.esp -= 4;
    // 0051a5a1  e8aa6afeff             -call 0x501050
    cpu.esp -= 4;
    nfs3hp::ea::fileReport(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a5a6  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
L_0x0051a5a9:
    // 0051a5a9  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a5ab  e88053fcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a5b0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a5b2  0f8463000000           -je 0x51a61b
    if (cpu.flags.zf)
    {
        goto L_0x0051a61b;
    }
    // 0051a5b8  a1b8aa5600             -mov eax, dword ptr [0x56aab8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5679800) /* 0x56aab8 */);
    // 0051a5bd  e85ed1fcff             -call 0x4e7720
    cpu.esp -= 4;
    nfs3hp::port::waitSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a5c2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a5c4  0f8d5b000000           -jge 0x51a625
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051a625;
    }
    // 0051a5ca  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0051a5cf  b8fdffffff             -mov eax, 0xfffffffd
    cpu.eax = 4294967293 /*0xfffffffd*/;
    // 0051a5d4  890d58445600           -mov dword ptr [0x564458], ecx
    app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */) = cpu.ecx;
    // 0051a5da  83c440                 -add esp, 0x40
    (cpu.esp) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 0051a5dd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a5de  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a5df  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a5e0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a5e1  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a5e2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051a5e3:
    // 0051a5e3  80fc30                 +cmp ah, 0x30
    {
        x86::reg8 tmp1 = cpu.ah;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(48 /*0x30*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051a5e6  0f822bffffff           -jb 0x51a517
    if (cpu.flags.cf)
    {
        goto L_0x0051a517;
    }
    // 0051a5ec  80fc39                 +cmp ah, 0x39
    {
        x86::reg8 tmp1 = cpu.ah;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(57 /*0x39*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051a5ef  0f8722ffffff           -ja 0x51a517
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051a517;
    }
    // 0051a5f5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051a5fa  e91affffff             -jmp 0x51a519
    goto L_0x0051a519;
L_0x0051a5ff:
    // 0051a5ff  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a601  0f8422ffffff           -je 0x51a529
    if (cpu.flags.zf)
    {
        goto L_0x0051a529;
    }
    // 0051a607  8b44243c               -mov eax, dword ptr [esp + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(60) /* 0x3c */);
    // 0051a60b  89442434               -mov dword ptr [esp + 0x34], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(52) /* 0x34 */) = cpu.eax;
    // 0051a60f  e915ffffff             -jmp 0x51a529
    goto L_0x0051a529;
L_0x0051a614:
    // 0051a614  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051a616  e94dffffff             -jmp 0x51a568
    goto L_0x0051a568;
L_0x0051a61b:
    // 0051a61b  a1b8aa5600             -mov eax, dword ptr [0x56aab8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5679800) /* 0x56aab8 */);
    // 0051a620  e8bb52fcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051a625:
    // 0051a625  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0051a62c  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051a62e  8b1485c0aa5600         -mov edx, dword ptr [eax*4 + 0x56aac0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5679808) /* 0x56aac0 */ + cpu.eax * 4);
    // 0051a635  8b0485c8aa5600         -mov eax, dword ptr [eax*4 + 0x56aac8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5679816) /* 0x56aac8 */ + cpu.eax * 4);
    // 0051a63c  8915b4ab5600           -mov dword ptr [0x56abb4], edx
    app->getMemory<x86::reg32>(x86::reg32(5680052) /* 0x56abb4 */) = cpu.edx;
    // 0051a642  e8e95dfdff             -call 0x4f0430
    cpu.esp -= 4;
    nfs3hp::watcom::atoi(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a647  83c440                 -add esp, 0x40
    (cpu.esp) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 0051a64a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a64b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a64c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a64d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a64e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a64f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051a650:
    // 0051a650  49                     -dec ecx
    (cpu.ecx)--;
    // 0051a651  83eb0c                 -sub ebx, 0xc
    (cpu.ebx) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0051a654  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0051a656  0f8deafeffff           -jge 0x51a546
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051a546;
    }
L_0x0051a65c:
    // 0051a65c  83c604                 -add esi, 4
    (cpu.esi) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051a65f  47                     -inc edi
    (cpu.edi)++;
    // 0051a660  83fe08                 +cmp esi, 8
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a663  0f8cc9feffff           -jl 0x51a532
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051a532;
    }
    // 0051a669  8b5c2434               -mov ebx, dword ptr [esp + 0x34]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(52) /* 0x34 */);
    // 0051a66d  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051a66f  0f84d3000000           -je 0x51a748
    if (cpu.flags.zf)
    {
        goto L_0x0051a748;
    }
    // 0051a675  837c243000             +cmp dword ptr [esp + 0x30], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(48) /* 0x30 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a67a  0f85c8000000           -jne 0x51a748
    if (!cpu.flags.zf)
    {
        goto L_0x0051a748;
    }
    // 0051a680  833dbcab560000         +cmp dword ptr [0x56abbc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680060) /* 0x56abbc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a687  0f84bb000000           -je 0x51a748
    if (cpu.flags.zf)
    {
        goto L_0x0051a748;
    }
    // 0051a68d  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051a68f  e89c5dfdff             -call 0x4f0430
    cpu.esp -= 4;
    nfs3hp::watcom::atoi(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a694  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051a696  83f80a                 +cmp eax, 0xa
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(10 /*0xa*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a699  0f8c79000000           -jl 0x51a718
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051a718;
    }
    // 0051a69f  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x0051a6a4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a6a4  833d5444560000         +cmp dword ptr [0x564454], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653588) /* 0x564454 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a6ab  742e                   -je 0x51a6db
    if (cpu.flags.zf)
    {
        goto L_0x0051a6db;
    }
    // 0051a6ad  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051a6af  7c2a                   -jl 0x51a6db
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051a6db;
    }
    // 0051a6b1  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0051a6b8  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0051a6ba  8b1c85c0aa5600         -mov ebx, dword ptr [eax*4 + 0x56aac0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5679808) /* 0x56aac0 */ + cpu.eax * 4);
    // 0051a6c1  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051a6c2  8b742438               -mov esi, dword ptr [esp + 0x38]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(56) /* 0x38 */);
    // 0051a6c6  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051a6c7  68dc0a5500             -push 0x550adc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573340 /*0x550adc*/;
    cpu.esp -= 4;
    // 0051a6cc  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051a6ce  6854445600             -push 0x564454
    app->getMemory<x86::reg32>(cpu.esp-4) = 5653588 /*0x564454*/;
    cpu.esp -= 4;
    // 0051a6d3  e87869feff             -call 0x501050
    cpu.esp -= 4;
    nfs3hp::ea::fileReport(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a6d8  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
L_0x0051a6db:
    // 0051a6db  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051a6dd  7c15                   -jl 0x51a6f4
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051a6f4;
    }
    // 0051a6df  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0051a6e6  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051a6e8  8b0485c0aa5600         -mov eax, dword ptr [eax*4 + 0x56aac0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5679808) /* 0x56aac0 */ + cpu.eax * 4);
    // 0051a6ef  a3b4ab5600             -mov dword ptr [0x56abb4], eax
    app->getMemory<x86::reg32>(x86::reg32(5680052) /* 0x56abb4 */) = cpu.eax;
L_0x0051a6f4:
    // 0051a6f4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a6f6  e83552fcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a6fb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a6fd  7424                   -je 0x51a723
    if (cpu.flags.zf)
    {
        goto L_0x0051a723;
    }
    // 0051a6ff  a1b8aa5600             -mov eax, dword ptr [0x56aab8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5679800) /* 0x56aab8 */);
    // 0051a704  e817d0fcff             -call 0x4e7720
    cpu.esp -= 4;
    nfs3hp::port::waitSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a709  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a70b  7c22                   -jl 0x51a72f
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051a72f;
    }
L_0x0051a70d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a70d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051a70f  83c440                 -add esp, 0x40
    (cpu.esp) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 0051a712  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a713  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a714  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a715  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a716  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a717  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051a718:
    // 0051a718  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a71a  7d88                   -jge 0x51a6a4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051a6a4;
    }
    // 0051a71c  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
    // 0051a721  eb81                   -jmp 0x51a6a4
    goto L_0x0051a6a4;
L_0x0051a723:
    // 0051a723  a1b8aa5600             -mov eax, dword ptr [0x56aab8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5679800) /* 0x56aab8 */);
    // 0051a728  e8b351fcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a72d  ebde                   -jmp 0x51a70d
    goto L_0x0051a70d;
L_0x0051a72f:
    // 0051a72f  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 0051a734  b8fdffffff             -mov eax, 0xfffffffd
    cpu.eax = 4294967293 /*0xfffffffd*/;
    // 0051a739  893d58445600           -mov dword ptr [0x564458], edi
    app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */) = cpu.edi;
    // 0051a73f  83c440                 -add esp, 0x40
    (cpu.esp) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 0051a742  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a743  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a744  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a745  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a746  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a747  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051a748:
    // 0051a748  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a74a  e8e151fcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a74f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a751  740f                   -je 0x51a762
    if (cpu.flags.zf)
    {
        goto L_0x0051a762;
    }
    // 0051a753  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a755  e8d6cefcff             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a75a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a75c  7c0b                   -jl 0x51a769
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051a769;
    }
L_0x0051a75e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a75e  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051a760  eb0c                   -jmp 0x51a76e
    goto L_0x0051a76e;
L_0x0051a762:
    // 0051a762  e87951fcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a767  ebf5                   -jmp 0x51a75e
    goto L_0x0051a75e;
L_0x0051a769:
    // 0051a769  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0051a76e:
    // 0051a76e  090558445600           -or dword ptr [0x564458], eax
    app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */) |= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a774  83fd29                 +cmp ebp, 0x29
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(41 /*0x29*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a777  0f8c45fdffff           -jl 0x51a4c2
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051a4c2;
    }
L_0x0051a77d:
    // 0051a77d  833d5844560000         +cmp dword ptr [0x564458], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a784  740e                   -je 0x51a794
    if (cpu.flags.zf)
    {
        goto L_0x0051a794;
    }
    // 0051a786  b8fdffffff             -mov eax, 0xfffffffd
    cpu.eax = 4294967293 /*0xfffffffd*/;
    // 0051a78b  83c440                 -add esp, 0x40
    (cpu.esp) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 0051a78e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a78f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a790  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a791  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a792  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a793  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051a794:
    // 0051a794  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0051a799  83c440                 -add esp, 0x40
    (cpu.esp) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 0051a79c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a79d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a79e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a79f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a7a0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a7a1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_51a7b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051a7b0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051a7b1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051a7b2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051a7b3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051a7b4  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051a7b7  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 0051a7b9  891424                 -mov dword ptr [esp], edx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.edx;
    // 0051a7bc  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0051a7be  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051a7c0  49                     -dec ecx
    (cpu.ecx)--;
    // 0051a7c1  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051a7c3  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0051a7c5  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 0051a7c7  49                     -dec ecx
    (cpu.ecx)--;
    // 0051a7c8  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051a7ca  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 0051a7cc  8915b0ab5600           -mov dword ptr [0x56abb0], edx
    app->getMemory<x86::reg32>(x86::reg32(5680048) /* 0x56abb0 */) = cpu.edx;
    // 0051a7d2  83f931                 +cmp ecx, 0x31
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(49 /*0x31*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a7d5  7f22                   -jg 0x51a7f9
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0051a7f9;
    }
    // 0051a7d7  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051a7d9  7522                   -jne 0x51a7fd
    if (!cpu.flags.zf)
    {
        goto L_0x0051a7fd;
    }
L_0x0051a7db:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a7db  833d5844560000         +cmp dword ptr [0x564458], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a7e2  7422                   -je 0x51a806
    if (cpu.flags.zf)
    {
        goto L_0x0051a806;
    }
L_0x0051a7e4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a7e4  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0051a7e9  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a7eb  891d58445600           -mov dword ptr [0x564458], ebx
    app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */) = cpu.ebx;
L_0x0051a7f1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a7f1  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051a7f4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a7f5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a7f6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a7f7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a7f8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051a7f9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a7f9  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051a7fb  ebf4                   -jmp 0x51a7f1
    goto L_0x0051a7f1;
L_0x0051a7fd:
    // 0051a7fd  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0051a7ff  e8dc090000             -call 0x51b1e0
    cpu.esp -= 4;
    sub_51b1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a804  ebd5                   -jmp 0x51a7db
    goto L_0x0051a7db;
L_0x0051a806:
    // 0051a806  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a808  e82351fcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a80d  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051a80f  a1d8435600             -mov eax, dword ptr [0x5643d8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653464) /* 0x5643d8 */);
    // 0051a814  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051a816  bb0a000000             -mov ebx, 0xa
    cpu.ebx = 10 /*0xa*/;
    // 0051a81b  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051a81e  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0051a820  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0051a822  0f8593000000           -jne 0x51a8bb
    if (!cpu.flags.zf)
    {
        goto L_0x0051a8bb;
    }
    // 0051a828  e8b350fcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051a82d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a82d  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0051a82f  8b1c24                 -mov ebx, dword ptr [esp]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051a832  e889090000             -call 0x51b1c0
    cpu.esp -= 4;
    sub_51b1c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a837  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051a839  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0051a83b  e8b00d0000             -call 0x51b5f0
    cpu.esp -= 4;
    sub_51b5f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a840  bb040b5500             -mov ebx, 0x550b04
    cpu.ebx = 5573380 /*0x550b04*/;
    // 0051a845  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051a84a  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0051a84c  e89f0d0000             -call 0x51b5f0
    cpu.esp -= 4;
    sub_51b5f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a851  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 0051a856  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051a858  a1d8435600             -mov eax, dword ptr [0x5643d8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653464) /* 0x5643d8 */);
    // 0051a85d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051a85f  b905000000             -mov ecx, 5
    cpu.ecx = 5 /*0x5*/;
    // 0051a864  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051a867  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0051a869  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0051a86b  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
L_0x0051a86d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a86d  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 0051a872  39f8                   +cmp eax, edi
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
    // 0051a874  7d57                   -jge 0x51a8cd
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051a8cd;
    }
    // 0051a876  833db0ab560000         +cmp dword ptr [0x56abb0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680048) /* 0x56abb0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a87d  754e                   -jne 0x51a8cd
    if (!cpu.flags.zf)
    {
        goto L_0x0051a8cd;
    }
    // 0051a87f  833d5844560000         +cmp dword ptr [0x564458], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a886  7545                   -jne 0x51a8cd
    if (!cpu.flags.zf)
    {
        goto L_0x0051a8cd;
    }
    // 0051a888  bb60b1a000             -mov ebx, 0xa0b160
    cpu.ebx = 10531168 /*0xa0b160*/;
    // 0051a88d  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0051a892  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0051a894  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0051a896  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
    // 0051a898  e833130000             -call 0x51bbd0
    cpu.esp -= 4;
    sub_51bbd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a89d  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051a89f  30e4                   +xor ah, ah
    cpu.clear_co();
    cpu.set_szp((cpu.ah ^= x86::reg8(x86::sreg8(cpu.ah))));
    // 0051a8a1  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051a8a4  88a660b1a000           -mov byte ptr [esi + 0xa0b160], ah
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(10531168) /* 0xa0b160 */) = cpu.ah;
    // 0051a8aa  b860b1a000             -mov eax, 0xa0b160
    cpu.eax = 10531168 /*0xa0b160*/;
    // 0051a8af  e86c5ffcff             -call 0x4e0820
    cpu.esp -= 4;
    nfs3hp::watcom::strstr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a8b4  a3b0ab5600             -mov dword ptr [0x56abb0], eax
    app->getMemory<x86::reg32>(x86::reg32(5680048) /* 0x56abb0 */) = cpu.eax;
    // 0051a8b9  ebb2                   -jmp 0x51a86d
    goto L_0x0051a86d;
L_0x0051a8bb:
    // 0051a8bb  e860cefcff             -call 0x4e7720
    cpu.esp -= 4;
    nfs3hp::port::waitSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a8c0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a8c2  0f8c1cffffff           -jl 0x51a7e4
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051a7e4;
    }
    // 0051a8c8  e960ffffff             -jmp 0x51a82d
    goto L_0x0051a82d;
L_0x0051a8cd:
    // 0051a8cd  833d5444560000         +cmp dword ptr [0x564454], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653588) /* 0x564454 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a8d4  741f                   -je 0x51a8f5
    if (cpu.flags.zf)
    {
        goto L_0x0051a8f5;
    }
    // 0051a8d6  8b1db0ab5600           -mov ebx, dword ptr [0x56abb0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5680048) /* 0x56abb0 */);
    // 0051a8dc  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051a8de  742f                   -je 0x51a90f
    if (cpu.flags.zf)
    {
        goto L_0x0051a90f;
    }
    // 0051a8e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051a8e1  68080b5500             -push 0x550b08
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573384 /*0x550b08*/;
    cpu.esp -= 4;
    // 0051a8e6  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051a8e8  6854445600             -push 0x564454
    app->getMemory<x86::reg32>(cpu.esp-4) = 5653588 /*0x564454*/;
    cpu.esp -= 4;
    // 0051a8ed  e85e67feff             -call 0x501050
    cpu.esp -= 4;
    nfs3hp::ea::fileReport(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a8f2  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
L_0x0051a8f5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a8f5  833db0ab560000         +cmp dword ptr [0x56abb0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680048) /* 0x56abb0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a8fc  0f84f7feffff           -je 0x51a7f9
    if (cpu.flags.zf)
    {
        goto L_0x0051a7f9;
    }
    // 0051a902  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051a907  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051a90a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a90b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a90c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a90d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a90e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051a90f:
    // 0051a90f  681c0b5500             -push 0x550b1c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573404 /*0x550b1c*/;
    cpu.esp -= 4;
    // 0051a914  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051a916  6854445600             -push 0x564454
    app->getMemory<x86::reg32>(cpu.esp-4) = 5653588 /*0x564454*/;
    cpu.esp -= 4;
    // 0051a91b  e83067feff             -call 0x501050
    cpu.esp -= 4;
    nfs3hp::ea::fileReport(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a920  83c40c                 +add esp, 0xc
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
    // 0051a923  ebd0                   -jmp 0x51a8f5
    goto L_0x0051a8f5;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_51a930(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051a930  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051a931  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051a932  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051a933  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051a934  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051a935  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051a937  833d5444560000         +cmp dword ptr [0x564454], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653588) /* 0x564454 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a93e  753d                   -jne 0x51a97d
    if (!cpu.flags.zf)
    {
        goto L_0x0051a97d;
    }
L_0x0051a940:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a940  833db8aa560000         +cmp dword ptr [0x56aab8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5679800) /* 0x56aab8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a947  7518                   -jne 0x51a961
    if (!cpu.flags.zf)
    {
        goto L_0x0051a961;
    }
    // 0051a949  8b15d8435600           -mov edx, dword ptr [0x5643d8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653464) /* 0x5643d8 */);
    // 0051a94f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051a951  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051a954  c1e202                 +shl edx, 2
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
    // 0051a957  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0051a959  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 0051a95c  a3b8aa5600             -mov dword ptr [0x56aab8], eax
    app->getMemory<x86::reg32>(x86::reg32(5679800) /* 0x56aab8 */) = cpu.eax;
L_0x0051a961:
    // 0051a961  833d5844560000         +cmp dword ptr [0x564458], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a968  7429                   -je 0x51a993
    if (cpu.flags.zf)
    {
        goto L_0x0051a993;
    }
L_0x0051a96a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a96a  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 0051a96f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a971  893558445600           -mov dword ptr [0x564458], esi
    app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */) = cpu.esi;
    // 0051a977  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a978  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a979  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a97a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a97b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a97c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051a97d:
    // 0051a97d  683c0b5500             -push 0x550b3c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573436 /*0x550b3c*/;
    cpu.esp -= 4;
    // 0051a982  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051a984  6854445600             -push 0x564454
    app->getMemory<x86::reg32>(cpu.esp-4) = 5653588 /*0x564454*/;
    cpu.esp -= 4;
    // 0051a989  e8c266feff             -call 0x501050
    cpu.esp -= 4;
    nfs3hp::ea::fileReport(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a98e  83c40c                 +add esp, 0xc
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
    // 0051a991  ebad                   -jmp 0x51a940
    goto L_0x0051a940;
L_0x0051a993:
    // 0051a993  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a995  e8964ffcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a99a  8b15d8435600           -mov edx, dword ptr [0x5643d8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653464) /* 0x5643d8 */);
    // 0051a9a0  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051a9a2  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0051a9a9  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0051a9ab  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051a9ad  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051a9b0  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0051a9b2  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0051a9b4  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051a9b6  7532                   -jne 0x51a9ea
    if (!cpu.flags.zf)
    {
        goto L_0x0051a9ea;
    }
    // 0051a9b8  e8234ffcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051a9bd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a9bd  bb580b5500             -mov ebx, 0x550b58
    cpu.ebx = 5573464 /*0x550b58*/;
    // 0051a9c2  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 0051a9c7  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051a9c9  e8220c0000             -call 0x51b5f0
    cpu.esp -= 4;
    sub_51b5f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a9ce  833d5844560000         +cmp dword ptr [0x564458], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051a9d5  7422                   -je 0x51a9f9
    if (cpu.flags.zf)
    {
        goto L_0x0051a9f9;
    }
L_0x0051a9d7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051a9d7  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0051a9dc  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a9de  891d58445600           -mov dword ptr [0x564458], ebx
    app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */) = cpu.ebx;
    // 0051a9e4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a9e5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a9e6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a9e7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a9e8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051a9e9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051a9ea:
    // 0051a9ea  e831cdfcff             -call 0x4e7720
    cpu.esp -= 4;
    nfs3hp::port::waitSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051a9ef  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051a9f1  0f8c73ffffff           -jl 0x51a96a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051a96a;
    }
    // 0051a9f7  ebc4                   -jmp 0x51a9bd
    goto L_0x0051a9bd;
L_0x0051a9f9:
    // 0051a9f9  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051a9fb  e8304ffcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aa00  8b15d8435600           -mov edx, dword ptr [0x5643d8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653464) /* 0x5643d8 */);
    // 0051aa06  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051aa08  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0051aa0f  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0051aa11  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051aa13  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051aa16  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0051aa18  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0051aa1a  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051aa1c  753c                   -jne 0x51aa5a
    if (!cpu.flags.zf)
    {
        goto L_0x0051aa5a;
    }
    // 0051aa1e  e8bd4efcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051aa23:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051aa23  8b1dbcaa5600           -mov ebx, dword ptr [0x56aabc]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5679804) /* 0x56aabc */);
    // 0051aa29  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0051aa2b  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051aa2d  49                     -dec ecx
    (cpu.ecx)--;
    // 0051aa2e  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051aa30  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0051aa32  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 0051aa34  49                     -dec ecx
    (cpu.ecx)--;
    // 0051aa35  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0051aa37  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051aa39  e8b20b0000             -call 0x51b5f0
    cpu.esp -= 4;
    sub_51b5f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aa3e  833d5844560000         +cmp dword ptr [0x564458], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051aa45  7422                   -je 0x51aa69
    if (cpu.flags.zf)
    {
        goto L_0x0051aa69;
    }
L_0x0051aa47:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051aa47  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0051aa4c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051aa4e  890d58445600           -mov dword ptr [0x564458], ecx
    app->getMemory<x86::reg32>(x86::reg32(5653592) /* 0x564458 */) = cpu.ecx;
    // 0051aa54  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aa55  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aa56  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aa57  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aa58  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aa59  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051aa5a:
    // 0051aa5a  e8c1ccfcff             -call 0x4e7720
    cpu.esp -= 4;
    nfs3hp::port::waitSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aa5f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051aa61  0f8c70ffffff           -jl 0x51a9d7
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051a9d7;
    }
    // 0051aa67  ebba                   -jmp 0x51aa23
    goto L_0x0051aa23;
L_0x0051aa69:
    // 0051aa69  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051aa6b  e8c04efcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aa70  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051aa72  7548                   -jne 0x51aabc
    if (!cpu.flags.zf)
    {
        goto L_0x0051aabc;
    }
    // 0051aa74  a1b8aa5600             -mov eax, dword ptr [0x56aab8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5679800) /* 0x56aab8 */);
    // 0051aa79  e8624efcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051aa7e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051aa7e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051aa80  ba5c0b5500             -mov edx, 0x550b5c
    cpu.edx = 5573468 /*0x550b5c*/;
    // 0051aa85  e836070000             -call 0x51b1c0
    cpu.esp -= 4;
    sub_51b1c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aa8a  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051aa8c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051aa8e  e81dfdffff             -call 0x51a7b0
    cpu.esp -= 4;
    sub_51a7b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aa93  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051aa95  7539                   -jne 0x51aad0
    if (!cpu.flags.zf)
    {
        goto L_0x0051aad0;
    }
L_0x0051aa97:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051aa97  833d5444560000         +cmp dword ptr [0x564454], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653588) /* 0x564454 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051aa9e  7414                   -je 0x51aab4
    if (cpu.flags.zf)
    {
        goto L_0x0051aab4;
    }
    // 0051aaa0  68740b5500             -push 0x550b74
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573492 /*0x550b74*/;
    cpu.esp -= 4;
    // 0051aaa5  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051aaa7  6854445600             -push 0x564454
    app->getMemory<x86::reg32>(cpu.esp-4) = 5653588 /*0x564454*/;
    cpu.esp -= 4;
    // 0051aaac  e89f65feff             -call 0x501050
    cpu.esp -= 4;
    nfs3hp::ea::fileReport(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aab1  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
L_0x0051aab4:
    // 0051aab4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051aab6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aab7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aab8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aab9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aaba  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aabb  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051aabc:
    // 0051aabc  a1b8aa5600             -mov eax, dword ptr [0x56aab8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5679800) /* 0x56aab8 */);
    // 0051aac1  e85accfcff             -call 0x4e7720
    cpu.esp -= 4;
    nfs3hp::port::waitSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aac6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051aac8  0f8c79ffffff           -jl 0x51aa47
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051aa47;
    }
    // 0051aace  ebae                   -jmp 0x51aa7e
    goto L_0x0051aa7e;
L_0x0051aad0:
    // 0051aad0  8b15d8435600           -mov edx, dword ptr [0x5643d8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653464) /* 0x5643d8 */);
    // 0051aad6  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051aad8  01d2                   -add edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0051aada  e891f9ffff             -call 0x51a470
    cpu.esp -= 4;
    sub_51a470(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aadf  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051aae1  75b4                   -jne 0x51aa97
    if (!cpu.flags.zf)
    {
        goto L_0x0051aa97;
    }
    // 0051aae3  833d5444560000         +cmp dword ptr [0x564454], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653588) /* 0x564454 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051aaea  7414                   -je 0x51ab00
    if (cpu.flags.zf)
    {
        goto L_0x0051ab00;
    }
    // 0051aaec  68600b5500             -push 0x550b60
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573472 /*0x550b60*/;
    cpu.esp -= 4;
    // 0051aaf1  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051aaf3  6854445600             -push 0x564454
    app->getMemory<x86::reg32>(cpu.esp-4) = 5653588 /*0x564454*/;
    cpu.esp -= 4;
    // 0051aaf8  e85365feff             -call 0x501050
    cpu.esp -= 4;
    nfs3hp::ea::fileReport(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aafd  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
L_0x0051ab00:
    // 0051ab00  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051ab05  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ab06  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ab07  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ab08  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ab09  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ab0a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51ab10: native, nfs3hp::dead::stub_0x51ab10 */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_51ab40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051ab40  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051ab41  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051ab43  833dc0ab560000         +cmp dword ptr [0x56abc0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680064) /* 0x56abc0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ab4a  7416                   -je 0x51ab62
    if (cpu.flags.zf)
    {
        goto L_0x0051ab62;
    }
L_0x0051ab4c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ab4c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051ab4e  42                     -inc edx
    (cpu.edx)++;
    // 0051ab4f  e8ac060000             -call 0x51b200
    cpu.esp -= 4;
    sub_51b200(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ab54  83fa10                 +cmp edx, 0x10
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
    // 0051ab57  7d09                   -jge 0x51ab62
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051ab62;
    }
    // 0051ab59  833dc0ab560000         +cmp dword ptr [0x56abc0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680064) /* 0x56abc0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ab60  75ea                   -jne 0x51ab4c
    if (!cpu.flags.zf)
    {
        goto L_0x0051ab4c;
    }
L_0x0051ab62:
    // 0051ab62  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ab63  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_51ab70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051ab70  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051ab71  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051ab72  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051ab73  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051ab75  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 0051ab77  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051ab7c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ab7d  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ab83  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 0051ab8a  8b90a0b1a000           -mov edx, dword ptr [eax + 0xa0b1a0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10531232) /* 0xa0b1a0 */);
    // 0051ab90  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051ab92  7510                   -jne 0x51aba4
    if (!cpu.flags.zf)
    {
        goto L_0x0051aba4;
    }
    // 0051ab94  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051ab99  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ab9a  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aba0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aba1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aba2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051aba3  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051aba4:
    // 0051aba4  891a                   -mov dword ptr [edx], ebx
    app->getMemory<x86::reg32>(cpu.edx) = cpu.ebx;
    // 0051aba6  a1c4ab5600             -mov eax, dword ptr [0x56abc4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680068) /* 0x56abc4 */);
    // 0051abab  e8804ffcff             -call 0x4dfb30
    cpu.esp -= 4;
    nfs3hp::ea::signalEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051abb0  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051abb5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051abb6  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051abbc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051abbd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051abbe  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051abbf  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_51abc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051abc0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051abc1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051abc2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051abc3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051abc4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051abc5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051abc6  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051abc8  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051abcd  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051abce  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051abd4  837b0800               +cmp dword ptr [ebx + 8], 0
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
    // 0051abd8  7518                   -jne 0x51abf2
    if (!cpu.flags.zf)
    {
        goto L_0x0051abf2;
    }
    // 0051abda  8d7318                 -lea esi, [ebx + 0x18]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(24) /* 0x18 */);
    // 0051abdd  8d6b14                 -lea ebp, [ebx + 0x14]
    cpu.ebp = x86::reg32(cpu.ebx + x86::reg32(20) /* 0x14 */);
    // 0051abe0  8d7b2c                 -lea edi, [ebx + 0x2c]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(44) /* 0x2c */);
L_0x0051abe3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051abe3  8b5310                 -mov edx, dword ptr [ebx + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */);
    // 0051abe6  81fa00020000           +cmp edx, 0x200
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(512 /*0x200*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051abec  0f8cbb000000           -jl 0x51acad
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051acad;
    }
L_0x0051abf2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051abf2  8dbb3c020000           -lea edi, [ebx + 0x23c]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(572) /* 0x23c */);
    // 0051abf8  8dab38020000           -lea ebp, [ebx + 0x238]
    cpu.ebp = x86::reg32(cpu.ebx + x86::reg32(568) /* 0x238 */);
    // 0051abfe  8db350020000           -lea esi, [ebx + 0x250]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(592) /* 0x250 */);
L_0x0051ac04:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ac04  83bb2c02000000         +cmp dword ptr [ebx + 0x22c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(556) /* 0x22c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ac0b  0f854c010000           -jne 0x51ad5d
    if (!cpu.flags.zf)
    {
        goto L_0x0051ad5d;
    }
    // 0051ac11  8b9334020000           -mov edx, dword ptr [ebx + 0x234]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(564) /* 0x234 */);
    // 0051ac17  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051ac19  0f8e3e010000           -jle 0x51ad5d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051ad5d;
    }
    // 0051ac1f  8b8330020000           -mov eax, dword ptr [ebx + 0x230]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(560) /* 0x230 */);
    // 0051ac25  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051ac27  81fa00020000           +cmp edx, 0x200
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(512 /*0x200*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ac2d  0f8e03010000           -jle 0x51ad36
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051ad36;
    }
    // 0051ac33  ba00020000             -mov edx, 0x200
    cpu.edx = 512 /*0x200*/;
    // 0051ac38  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
L_0x0051ac3a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ac3a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051ac3b  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051ac3c  c7833c02000000000000   -mov dword ptr [ebx + 0x23c], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(572) /* 0x23c */) = 0 /*0x0*/;
    // 0051ac46  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051ac47  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 0051ac49  c7834002000000000000   -mov dword ptr [ebx + 0x240], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(576) /* 0x240 */) = 0 /*0x0*/;
    // 0051ac53  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ac54  8b4b04                 -mov ecx, dword ptr [ebx + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0051ac57  c7834402000000000000   -mov dword ptr [ebx + 0x244], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(580) /* 0x244 */) = 0 /*0x0*/;
    // 0051ac61  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051ac62  c7834802000000000000   -mov dword ptr [ebx + 0x248], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(584) /* 0x248 */) = 0 /*0x0*/;
    // 0051ac6c  2eff1540465300         -call dword ptr cs:[0x534640]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457472) /* 0x534640 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ac73  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ac75  0f84c6000000           -je 0x51ad41
    if (cpu.flags.zf)
    {
        goto L_0x0051ad41;
    }
    // 0051ac7b  8b9334020000           -mov edx, dword ptr [ebx + 0x234]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(564) /* 0x234 */);
    // 0051ac81  8b8338020000           -mov eax, dword ptr [ebx + 0x238]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(568) /* 0x238 */);
    // 0051ac87  8b8b38020000           -mov ecx, dword ptr [ebx + 0x238]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(568) /* 0x238 */);
    // 0051ac8d  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0051ac8f  8b8330020000           -mov eax, dword ptr [ebx + 0x230]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(560) /* 0x230 */);
    // 0051ac95  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0051ac97  899334020000           -mov dword ptr [ebx + 0x234], edx
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(564) /* 0x234 */) = cpu.edx;
    // 0051ac9d  25ff010000             +and eax, 0x1ff
    cpu.clear_co();
    cpu.set_szp((cpu.eax &= x86::reg32(x86::sreg32(511 /*0x1ff*/))));
    // 0051aca2  898330020000           -mov dword ptr [ebx + 0x230], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(560) /* 0x230 */) = cpu.eax;
    // 0051aca8  e957ffffff             -jmp 0x51ac04
    goto L_0x0051ac04;
L_0x0051acad:
    // 0051acad  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 0051acb0  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0051acb2  25ff010000             -and eax, 0x1ff
    cpu.eax &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 0051acb7  8b530c                 -mov edx, dword ptr [ebx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 0051acba  39d0                   +cmp eax, edx
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
    // 0051acbc  7d4e                   -jge 0x51ad0c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051ad0c;
    }
L_0x0051acbe:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051acbe  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0051acc0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051acc1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051acc2  c7431800000000         -mov dword ptr [ebx + 0x18], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */) = 0 /*0x0*/;
    // 0051acc9  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051acca  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 0051accc  c7431c00000000         -mov dword ptr [ebx + 0x1c], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(28) /* 0x1c */) = 0 /*0x0*/;
    // 0051acd3  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051acd4  8b4b04                 -mov ecx, dword ptr [ebx + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0051acd7  c7432000000000         -mov dword ptr [ebx + 0x20], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(32) /* 0x20 */) = 0 /*0x0*/;
    // 0051acde  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051acdf  c7432400000000         -mov dword ptr [ebx + 0x24], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
    // 0051ace6  2eff15b0455300         -call dword ptr cs:[0x5345b0]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457328) /* 0x5345b0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051aced  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051acef  7422                   -je 0x51ad13
    if (cpu.flags.zf)
    {
        goto L_0x0051ad13;
    }
    // 0051acf1  8b4314                 -mov eax, dword ptr [ebx + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */);
    // 0051acf4  8b4b10                 -mov ecx, dword ptr [ebx + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */);
    // 0051acf7  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051acf9  8b4308                 -mov eax, dword ptr [ebx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 0051acfc  894b10                 -mov dword ptr [ebx + 0x10], ecx
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */) = cpu.ecx;
    // 0051acff  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ad01  0f84dcfeffff           -je 0x51abe3
    if (cpu.flags.zf)
    {
        goto L_0x0051abe3;
    }
    // 0051ad07  e9e6feffff             -jmp 0x51abf2
    goto L_0x0051abf2;
L_0x0051ad0c:
    // 0051ad0c  ba00020000             -mov edx, 0x200
    cpu.edx = 512 /*0x200*/;
    // 0051ad11  ebab                   -jmp 0x51acbe
    goto L_0x0051acbe;
L_0x0051ad13:
    // 0051ad13  2eff1534455300         -call dword ptr cs:[0x534534]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457204) /* 0x534534 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ad1a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ad1c  750c                   -jne 0x51ad2a
    if (!cpu.flags.zf)
    {
        goto L_0x0051ad2a;
    }
L_0x0051ad1e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ad1e  c7430801000000         -mov dword ptr [ebx + 8], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */) = 1 /*0x1*/;
    // 0051ad25  e9c8feffff             -jmp 0x51abf2
    goto L_0x0051abf2;
L_0x0051ad2a:
    // 0051ad2a  3de5030000             +cmp eax, 0x3e5
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(997 /*0x3e5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ad2f  74ed                   -je 0x51ad1e
    if (cpu.flags.zf)
    {
        goto L_0x0051ad1e;
    }
    // 0051ad31  e9bcfeffff             -jmp 0x51abf2
    goto L_0x0051abf2;
L_0x0051ad36:
    // 0051ad36  8b9334020000           -mov edx, dword ptr [ebx + 0x234]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(564) /* 0x234 */);
    // 0051ad3c  e9f9feffff             -jmp 0x51ac3a
    goto L_0x0051ac3a;
L_0x0051ad41:
    // 0051ad41  2eff1534455300         -call dword ptr cs:[0x534534]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457204) /* 0x534534 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ad48  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ad4a  7407                   -je 0x51ad53
    if (cpu.flags.zf)
    {
        goto L_0x0051ad53;
    }
    // 0051ad4c  3de5030000             +cmp eax, 0x3e5
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(997 /*0x3e5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ad51  750a                   -jne 0x51ad5d
    if (!cpu.flags.zf)
    {
        goto L_0x0051ad5d;
    }
L_0x0051ad53:
    // 0051ad53  c7832c02000001000000   -mov dword ptr [ebx + 0x22c], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(556) /* 0x22c */) = 1 /*0x1*/;
L_0x0051ad5d:
    // 0051ad5d  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051ad62  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ad63  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ad69  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ad6a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ad6b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ad6c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ad6d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ad6e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ad6f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_51ad70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051ad70  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051ad71  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051ad72  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051ad73  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051ad76  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051ad78  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051ad7d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ad7e  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ad84  837b0800               +cmp dword ptr [ebx + 8], 0
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
    // 0051ad88  7520                   -jne 0x51adaa
    if (!cpu.flags.zf)
    {
        goto L_0x0051adaa;
    }
L_0x0051ad8a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ad8a  83bb2c02000000         +cmp dword ptr [ebx + 0x22c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(556) /* 0x22c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ad91  0f8570000000           -jne 0x51ae07
    if (!cpu.flags.zf)
    {
        goto L_0x0051ae07;
    }
L_0x0051ad97:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ad97  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051ad9c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ad9d  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ada3  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051ada6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ada7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ada8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ada9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051adaa:
    // 0051adaa  8b4328                 -mov eax, dword ptr [ebx + 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(40) /* 0x28 */);
    // 0051adad  e88e4dfcff             -call 0x4dfb40
    cpu.esp -= 4;
    nfs3hp::ea::pollEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051adb2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051adb4  74d4                   -je 0x51ad8a
    if (cpu.flags.zf)
    {
        goto L_0x0051ad8a;
    }
    // 0051adb6  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051adb8  8d442404               -lea eax, [esp + 4]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051adbc  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051adbd  8d4318                 -lea eax, [ebx + 0x18]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(24) /* 0x18 */);
    // 0051adc0  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051adc1  8b4b04                 -mov ecx, dword ptr [ebx + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0051adc4  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051adc5  2eff1554455300         -call dword ptr cs:[0x534554]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457236) /* 0x534554 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051adcc  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051adce  7418                   -je 0x51ade8
    if (cpu.flags.zf)
    {
        goto L_0x0051ade8;
    }
    // 0051add0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051add1  8b742404               -mov esi, dword ptr [esp + 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051add5  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0051add7  750a                   -jne 0x51ade3
    if (!cpu.flags.zf)
    {
        goto L_0x0051ade3;
    }
L_0x0051add9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051add9  c7430800000000         -mov dword ptr [ebx + 8], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 0051ade0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ade1  eba7                   -jmp 0x51ad8a
    goto L_0x0051ad8a;
L_0x0051ade3:
    // 0051ade3  017310                 +add dword ptr [ebx + 0x10], esi
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051ade6  ebf1                   -jmp 0x51add9
    goto L_0x0051add9;
L_0x0051ade8:
    // 0051ade8  2eff1534455300         -call dword ptr cs:[0x534534]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457204) /* 0x534534 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051adef  3de5030000             +cmp eax, 0x3e5
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(997 /*0x3e5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051adf4  750a                   -jne 0x51ae00
    if (!cpu.flags.zf)
    {
        goto L_0x0051ae00;
    }
    // 0051adf6  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051adfb  894308                 -mov dword ptr [ebx + 8], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0051adfe  eb8a                   -jmp 0x51ad8a
    goto L_0x0051ad8a;
L_0x0051ae00:
    // 0051ae00  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051ae02  894308                 -mov dword ptr [ebx + 8], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0051ae05  eb83                   -jmp 0x51ad8a
    goto L_0x0051ad8a;
L_0x0051ae07:
    // 0051ae07  8b834c020000           -mov eax, dword ptr [ebx + 0x24c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(588) /* 0x24c */);
    // 0051ae0d  e82e4dfcff             -call 0x4dfb40
    cpu.esp -= 4;
    nfs3hp::ea::pollEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ae12  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ae14  7481                   -je 0x51ad97
    if (cpu.flags.zf)
    {
        goto L_0x0051ad97;
    }
    // 0051ae16  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051ae18  8d442404               -lea eax, [esp + 4]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051ae1c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ae1d  8d833c020000           -lea eax, [ebx + 0x23c]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(572) /* 0x23c */);
    // 0051ae23  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ae24  8b4304                 -mov eax, dword ptr [ebx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0051ae27  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ae28  2eff1554455300         -call dword ptr cs:[0x534554]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457236) /* 0x534554 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ae2f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ae31  743e                   -je 0x51ae71
    if (cpu.flags.zf)
    {
        goto L_0x0051ae71;
    }
    // 0051ae33  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051ae36  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051ae38  741a                   -je 0x51ae54
    if (cpu.flags.zf)
    {
        goto L_0x0051ae54;
    }
    // 0051ae3a  299334020000           -sub dword ptr [ebx + 0x234], edx
    (app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(564) /* 0x234 */)) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0051ae40  8b8330020000           -mov eax, dword ptr [ebx + 0x230]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(560) /* 0x230 */);
    // 0051ae46  030424                 -add eax, dword ptr [esp]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp)));
    // 0051ae49  25ff010000             -and eax, 0x1ff
    cpu.eax &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 0051ae4e  898330020000           -mov dword ptr [ebx + 0x230], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(560) /* 0x230 */) = cpu.eax;
L_0x0051ae54:
    // 0051ae54  c7832c02000000000000   -mov dword ptr [ebx + 0x22c], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(556) /* 0x22c */) = 0 /*0x0*/;
    // 0051ae5e  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051ae63  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ae64  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ae6a  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051ae6d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ae6e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ae6f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ae70  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051ae71:
    // 0051ae71  2eff1534455300         -call dword ptr cs:[0x534534]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457204) /* 0x534534 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ae78  3de5030000             +cmp eax, 0x3e5
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(997 /*0x3e5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ae7d  751e                   -jne 0x51ae9d
    if (!cpu.flags.zf)
    {
        goto L_0x0051ae9d;
    }
    // 0051ae7f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0051ae84:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ae84  89832c020000           -mov dword ptr [ebx + 0x22c], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(556) /* 0x22c */) = cpu.eax;
    // 0051ae8a  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051ae8f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ae90  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ae96  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051ae99  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ae9a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ae9b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ae9c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051ae9d:
    // 0051ae9d  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051ae9f  ebe3                   -jmp 0x51ae84
    goto L_0x0051ae84;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_51aeb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051aeb0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051aeb1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051aeb2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051aeb3  81ec84000000           -sub esp, 0x84
    (cpu.esp) -= x86::reg32(x86::sreg32(132 /*0x84*/));
    // 0051aeb9  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0051aebe  a1c4ab5600             -mov eax, dword ptr [0x56abc4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680068) /* 0x56abc4 */);
    // 0051aec3  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0051aec6  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0051aec8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051aec8  8b90a0b1a000           -mov edx, dword ptr [eax + 0xa0b1a0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10531232) /* 0xa0b1a0 */);
    // 0051aece  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051aed0  7422                   -je 0x51aef4
    if (cpu.flags.zf)
    {
        goto L_0x0051aef4;
    }
    // 0051aed2  837a0800               +cmp dword ptr [edx + 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051aed6  7408                   -je 0x51aee0
    if (cpu.flags.zf)
    {
        goto L_0x0051aee0;
    }
    // 0051aed8  41                     -inc ecx
    (cpu.ecx)++;
    // 0051aed9  8b5a28                 -mov ebx, dword ptr [edx + 0x28]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(40) /* 0x28 */);
    // 0051aedc  895c8cfc               -mov dword ptr [esp + ecx*4 - 4], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(-4) /* -0x4 */ + cpu.ecx * 4) = cpu.ebx;
L_0x0051aee0:
    // 0051aee0  83ba2c02000000         +cmp dword ptr [edx + 0x22c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(556) /* 0x22c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051aee7  740b                   -je 0x51aef4
    if (cpu.flags.zf)
    {
        goto L_0x0051aef4;
    }
    // 0051aee9  41                     -inc ecx
    (cpu.ecx)++;
    // 0051aeea  8b924c020000           -mov edx, dword ptr [edx + 0x24c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(588) /* 0x24c */);
    // 0051aef0  89548cfc               -mov dword ptr [esp + ecx*4 - 4], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(-4) /* -0x4 */ + cpu.ecx * 4) = cpu.edx;
L_0x0051aef4:
    // 0051aef4  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051aef7  83f840                 +cmp eax, 0x40
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051aefa  75cc                   -jne 0x51aec8
    if (!cpu.flags.zf)
    {
        goto L_0x0051aec8;
    }
    // 0051aefc  89e2                   -mov edx, esp
    cpu.edx = cpu.esp;
    // 0051aefe  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051af00  e8eb4cfcff             -call 0x4dfbf0
    cpu.esp -= 4;
    nfs3hp::ea::waitForAny(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051af05  81c484000000           -add esp, 0x84
    (cpu.esp) += x86::reg32(x86::sreg32(132 /*0x84*/));
    // 0051af0b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051af0c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051af0d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051af0e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_51af10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051af10  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051af11  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051af12  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051af13  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051af14  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051af15  e8e64bfcff             -call 0x4dfb00
    cpu.esp -= 4;
    nfs3hp::ea::createEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051af1a  a3c4ab5600             -mov dword ptr [0x56abc4], eax
    app->getMemory<x86::reg32>(x86::reg32(5680068) /* 0x56abc4 */) = cpu.eax;
    // 0051af1f  833dc0ab560000         +cmp dword ptr [0x56abc0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680064) /* 0x56abc0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051af26  0f847d000000           -je 0x51afa9
    if (cpu.flags.zf)
    {
        goto L_0x0051afa9;
    }
    // 0051af2c  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
L_0x0051af2e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051af2e  e87dffffff             -call 0x51aeb0
    cpu.esp -= 4;
    sub_51aeb0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051af33  89fe                   -mov esi, edi
    cpu.esi = cpu.edi;
L_0x0051af35:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051af35  8b9ea0b1a000           -mov ebx, dword ptr [esi + 0xa0b1a0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(10531232) /* 0xa0b1a0 */);
    // 0051af3b  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051af3d  745a                   -je 0x51af99
    if (cpu.flags.zf)
    {
        goto L_0x0051af99;
    }
    // 0051af3f  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051af41  e82afeffff             -call 0x51ad70
    cpu.esp -= 4;
    sub_51ad70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051af46  3b3b                   +cmp edi, dword ptr [ebx]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051af48  7440                   -je 0x51af8a
    if (cpu.flags.zf)
    {
        goto L_0x0051af8a;
    }
    // 0051af4a  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051af4f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051af50  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051af56  8b03                   -mov eax, dword ptr [ebx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx);
    // 0051af58  83f803                 +cmp eax, 3
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
    // 0051af5b  7364                   -jae 0x51afc1
    if (!cpu.flags.cf)
    {
        goto L_0x0051afc1;
    }
    // 0051af5d  83f802                 +cmp eax, 2
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
    // 0051af60  751a                   -jne 0x51af7c
    if (!cpu.flags.zf)
    {
        goto L_0x0051af7c;
    }
    // 0051af62  6a0a                   -push 0xa
    app->getMemory<x86::reg32>(cpu.esp-4) = 10 /*0xa*/;
    cpu.esp -= 4;
    // 0051af64  8b5304                 -mov edx, dword ptr [ebx + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0051af67  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051af68  2eff15a8455300         -call dword ptr cs:[0x5345a8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457320) /* 0x5345a8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051af6f  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051af71  e8fafdffff             -call 0x51ad70
    cpu.esp -= 4;
    sub_51ad70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051af76  897b0c                 -mov dword ptr [ebx + 0xc], edi
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */) = cpu.edi;
    // 0051af79  897b10                 -mov dword ptr [ebx + 0x10], edi
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */) = cpu.edi;
L_0x0051af7c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051af7c  893b                   -mov dword ptr [ebx], edi
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.edi;
    // 0051af7e  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051af83  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051af84  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051af8a:
    // 0051af8a  3bbea0b1a000           +cmp edi, dword ptr [esi + 0xa0b1a0]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(10531232) /* 0xa0b1a0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051af90  7407                   -je 0x51af99
    if (cpu.flags.zf)
    {
        goto L_0x0051af99;
    }
    // 0051af92  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051af94  e827fcffff             -call 0x51abc0
    cpu.esp -= 4;
    sub_51abc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051af99:
    // 0051af99  83c604                 -add esi, 4
    (cpu.esi) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051af9c  83fe40                 +cmp esi, 0x40
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051af9f  7594                   -jne 0x51af35
    if (!cpu.flags.zf)
    {
        goto L_0x0051af35;
    }
    // 0051afa1  3b3dc0ab5600           +cmp edi, dword ptr [0x56abc0]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5680064) /* 0x56abc0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051afa7  7585                   -jne 0x51af2e
    if (!cpu.flags.zf)
    {
        goto L_0x0051af2e;
    }
L_0x0051afa9:
    // 0051afa9  a1c4ab5600             -mov eax, dword ptr [0x56abc4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680068) /* 0x56abc4 */);
    // 0051afae  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0051afb0  e84b4cfcff             -call 0x4dfc00
    cpu.esp -= 4;
    nfs3hp::ea::closeEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051afb5  891dc4ab5600           -mov dword ptr [0x56abc4], ebx
    app->getMemory<x86::reg32>(x86::reg32(5680068) /* 0x56abc4 */) = cpu.ebx;
    // 0051afbb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051afbc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051afbd  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051afbe  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051afbf  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051afc0  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051afc1:
    // 0051afc1  7642                   -jbe 0x51b005
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051b005;
    }
    // 0051afc3  83f804                 +cmp eax, 4
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
    // 0051afc6  75b4                   -jne 0x51af7c
    if (!cpu.flags.zf)
    {
        goto L_0x0051af7c;
    }
    // 0051afc8  6a0f                   -push 0xf
    app->getMemory<x86::reg32>(cpu.esp-4) = 15 /*0xf*/;
    cpu.esp -= 4;
    // 0051afca  8b5304                 -mov edx, dword ptr [ebx + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0051afcd  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051afce  2eff15a8455300         -call dword ptr cs:[0x5345a8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457320) /* 0x5345a8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051afd5  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051afd7  e894fdffff             -call 0x51ad70
    cpu.esp -= 4;
    sub_51ad70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051afdc  8b4b04                 -mov ecx, dword ptr [ebx + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0051afdf  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051afe0  2eff1588445300         -call dword ptr cs:[0x534488]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457032) /* 0x534488 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051afe7  8b4328                 -mov eax, dword ptr [ebx + 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(40) /* 0x28 */);
    // 0051afea  e8114cfcff             -call 0x4dfc00
    cpu.esp -= 4;
    nfs3hp::ea::closeEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051afef  8b834c020000           -mov eax, dword ptr [ebx + 0x24c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(588) /* 0x24c */);
    // 0051aff5  e8064cfcff             -call 0x4dfc00
    cpu.esp -= 4;
    nfs3hp::ea::closeEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051affa  89bea0b1a000           -mov dword ptr [esi + 0xa0b1a0], edi
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(10531232) /* 0xa0b1a0 */) = cpu.edi;
    // 0051b000  e977ffffff             -jmp 0x51af7c
    goto L_0x0051af7c;
L_0x0051b005:
    // 0051b005  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0051b007  8b4304                 -mov eax, dword ptr [ebx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0051b00a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b00b  2eff15a8455300         -call dword ptr cs:[0x5345a8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457320) /* 0x5345a8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b012  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b014  e857fdffff             -call 0x51ad70
    cpu.esp -= 4;
    sub_51ad70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b019  89bb30020000           -mov dword ptr [ebx + 0x230], edi
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(560) /* 0x230 */) = cpu.edi;
    // 0051b01f  89bb34020000           -mov dword ptr [ebx + 0x234], edi
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(564) /* 0x234 */) = cpu.edi;
    // 0051b025  e952ffffff             -jmp 0x51af7c
    goto L_0x0051af7c;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_51b030(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b030  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b032  7c1c                   -jl 0x51b050
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051b050;
    }
    // 0051b034  83f810                 +cmp eax, 0x10
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
    // 0051b037  7d17                   -jge 0x51b050
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051b050;
    }
    // 0051b039  833c85a0b1a00000       +cmp dword ptr [eax*4 + 0xa0b1a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.eax * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b041  7510                   -jne 0x51b053
    if (!cpu.flags.zf)
    {
        goto L_0x0051b053;
    }
    // 0051b043  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 0051b049  8d9200000000           -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
    // 0051b04f  90                     -nop 
    ;
L_0x0051b050:
    // 0051b050  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051b052  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051b053:
    // 0051b053  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051b058  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51b060: native, nfs3hp::dead::stub_0x51b060 */

/* sub_51b0d0: native, nfs3hp::dead::stub_0x51b0d0 */

/* sub_51b140: native, nfs3hp::dead::stub_0x51b140 */

/* align: skip 0x90 */
void Application::sub_51b1c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b1c0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051b1c1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051b1c3  e868feffff             -call 0x51b030
    cpu.esp -= 4;
    sub_51b030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b1c8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b1ca  7502                   -jne 0x51b1ce
    if (!cpu.flags.zf)
    {
        goto L_0x0051b1ce;
    }
    // 0051b1cc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b1cd  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051b1ce:
    // 0051b1ce  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051b1cf  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 0051b1d4  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051b1d6  e895f9ffff             -call 0x51ab70
    cpu.esp -= 4;
    sub_51ab70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b1db  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b1dc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b1dd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51b1e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b1e0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051b1e1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051b1e3  e848feffff             -call 0x51b030
    cpu.esp -= 4;
    sub_51b030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b1e8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b1ea  7502                   -jne 0x51b1ee
    if (!cpu.flags.zf)
    {
        goto L_0x0051b1ee;
    }
    // 0051b1ec  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b1ed  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051b1ee:
    // 0051b1ee  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051b1ef  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 0051b1f4  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051b1f6  e875f9ffff             -call 0x51ab70
    cpu.esp -= 4;
    sub_51ab70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b1fb  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b1fc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b1fd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51b200(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b200  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051b201  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051b202  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051b203  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051b205  833dc8ab560000         +cmp dword ptr [0x56abc8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b20c  0f848b000000           -je 0x51b29d
    if (cpu.flags.zf)
    {
        goto L_0x0051b29d;
    }
L_0x0051b212:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051b212  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b217  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b218  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b21e  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b220  e80bfeffff             -call 0x51b030
    cpu.esp -= 4;
    sub_51b030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b225  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b227  0f847f000000           -je 0x51b2ac
    if (cpu.flags.zf)
    {
        goto L_0x0051b2ac;
    }
    // 0051b22d  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051b22e  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051b22f  8d349d00000000         -lea esi, [ebx*4]
    cpu.esi = x86::reg32(cpu.ebx * 4);
    // 0051b236  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 0051b23b  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b23d  8bbea0b1a000           -mov edi, dword ptr [esi + 0xa0b1a0]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(10531232) /* 0xa0b1a0 */);
    // 0051b243  e828f9ffff             -call 0x51ab70
    cpu.esp -= 4;
    sub_51ab70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b248  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b24d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b24e  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b254  8b8ea0b1a000           -mov ecx, dword ptr [esi + 0xa0b1a0]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(10531232) /* 0xa0b1a0 */);
    // 0051b25a  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051b25c  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0051b25e  7420                   -je 0x51b280
    if (cpu.flags.zf)
    {
        goto L_0x0051b280;
    }
    // 0051b260  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
L_0x0051b265:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051b265  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051b267  e87446fcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b26c  83baa0b1a00000         +cmp dword ptr [edx + 0xa0b1a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(10531232) /* 0xa0b1a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b273  75f0                   -jne 0x51b265
    if (!cpu.flags.zf)
    {
        goto L_0x0051b265;
    }
    // 0051b275  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 0051b27b  8d5200                 -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
    // 0051b27e  8bdb                   -mov ebx, ebx
    cpu.ebx = cpu.ebx;
L_0x0051b280:
    // 0051b280  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051b282  e80966fcff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b287  ff0dc0ab5600           -dec dword ptr [0x56abc0]
    (app->getMemory<x86::reg32>(x86::reg32(5680064) /* 0x56abc0 */))--;
    // 0051b28d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b28e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b28f  8b15c0ab5600           -mov edx, dword ptr [0x56abc0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5680064) /* 0x56abc0 */);
    // 0051b295  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051b297  742d                   -je 0x51b2c6
    if (cpu.flags.zf)
    {
        goto L_0x0051b2c6;
    }
    // 0051b299  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b29a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b29b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b29c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051b29d:
    // 0051b29d  e83e00fdff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b2a2  a3c8ab5600             -mov dword ptr [0x56abc8], eax
    app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */) = cpu.eax;
    // 0051b2a7  e966ffffff             -jmp 0x51b212
    goto L_0x0051b212;
L_0x0051b2ac:
    // 0051b2ac  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b2b1  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b2b2  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b2b8  8b15c0ab5600           -mov edx, dword ptr [0x56abc0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5680064) /* 0x56abc0 */);
    // 0051b2be  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051b2c0  7404                   -je 0x51b2c6
    if (cpu.flags.zf)
    {
        goto L_0x0051b2c6;
    }
    // 0051b2c2  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b2c3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b2c4  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b2c5  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051b2c6:
    // 0051b2c6  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b2c8  e8a3f8ffff             -call 0x51ab70
    cpu.esp -= 4;
    sub_51ab70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b2cd  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b2d2  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051b2d4  e88700fdff             -call 0x4eb360
    cpu.esp -= 4;
    nfs3hp::ea::freeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b2d9  890dc8ab5600           -mov dword ptr [0x56abc8], ecx
    app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */) = cpu.ecx;
    // 0051b2df  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b2e0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b2e1  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b2e2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_51b2f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b2f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051b2f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051b2f2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051b2f3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051b2f4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051b2f5  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0051b2f8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051b2fa  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051b2fc  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0051b2fe  2eff15f4455300         -call dword ptr cs:[0x5345f4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457396) /* 0x5345f4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b305  83feff                 +cmp esi, -1
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
    // 0051b308  752a                   -jne 0x51b334
    if (!cpu.flags.zf)
    {
        goto L_0x0051b334;
    }
    // 0051b30a  b83c000000             -mov eax, 0x3c
    cpu.eax = 60 /*0x3c*/;
    // 0051b30f  8b15dcb1a000           -mov edx, dword ptr [0xa0b1dc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531292) /* 0xa0b1dc */);
    // 0051b315  be0f000000             -mov esi, 0xf
    cpu.esi = 15 /*0xf*/;
    // 0051b31a  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051b31c  7412                   -je 0x51b330
    if (cpu.flags.zf)
    {
        goto L_0x0051b330;
    }
L_0x0051b31e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051b31e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b320  7c0e                   -jl 0x51b330
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051b330;
    }
    // 0051b322  8b889cb1a000           -mov ecx, dword ptr [eax + 0xa0b19c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10531228) /* 0xa0b19c */);
    // 0051b328  83e804                 -sub eax, 4
    (cpu.eax) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051b32b  4e                     -dec esi
    (cpu.esi)--;
    // 0051b32c  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0051b32e  75ee                   -jne 0x51b31e
    if (!cpu.flags.zf)
    {
        goto L_0x0051b31e;
    }
L_0x0051b330:
    // 0051b330  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0051b332  7c67                   -jl 0x51b39b
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051b39b;
    }
L_0x0051b334:
    // 0051b334  ff05c0ab5600           -inc dword ptr [0x56abc0]
    (app->getMemory<x86::reg32>(x86::reg32(5680064) /* 0x56abc0 */))++;
    // 0051b33a  833dc0ab560001         +cmp dword ptr [0x56abc0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680064) /* 0x56abc0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b341  7566                   -jne 0x51b3a9
    if (!cpu.flags.zf)
    {
        goto L_0x0051b3a9;
    }
    // 0051b343  b840ab5100             -mov eax, 0x51ab40
    cpu.eax = 5352256 /*0x51ab40*/;
    // 0051b348  e82b77fdff             -call 0x4f2a78
    cpu.esp -= 4;
    sub_4f2a78(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b34d  833dc8ab560000         +cmp dword ptr [0x56abc8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b354  750a                   -jne 0x51b360
    if (!cpu.flags.zf)
    {
        goto L_0x0051b360;
    }
    // 0051b356  e885fffcff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b35b  a3c8ab5600             -mov dword ptr [0x56abc8], eax
    app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */) = cpu.eax;
L_0x0051b360:
    // 0051b360  68e0b1a000             -push 0xa0b1e0
    app->getMemory<x86::reg32>(cpu.esp-4) = 10531296 /*0xa0b1e0*/;
    cpu.esp -= 4;
    // 0051b365  b9ffffffff             -mov ecx, 0xffffffff
    cpu.ecx = 4294967295 /*0xffffffff*/;
    // 0051b36a  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 0051b36f  b810af5100             -mov eax, 0x51af10
    cpu.eax = 5353232 /*0x51af10*/;
    // 0051b374  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051b376  e82544fcff             -call 0x4df7a0
    cpu.esp -= 4;
    nfs3hp::ea::createThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b37b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b37d  752a                   -jne 0x51b3a9
    if (!cpu.flags.zf)
    {
        goto L_0x0051b3a9;
    }
    // 0051b37f  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b384  e8d7fffcff             -call 0x4eb360
    cpu.esp -= 4;
    nfs3hp::ea::freeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b389  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051b38b  a3c8ab5600             -mov dword ptr [0x56abc8], eax
    app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */) = cpu.eax;
    // 0051b390  a1c0ab5600             -mov eax, dword ptr [0x56abc0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680064) /* 0x56abc0 */);
    // 0051b395  ff0dc0ab5600           -dec dword ptr [0x56abc0]
    (app->getMemory<x86::reg32>(x86::reg32(5680064) /* 0x56abc0 */))--;
L_0x0051b39b:
    // 0051b39b  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0051b3a0  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0051b3a3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b3a4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b3a5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b3a6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b3a7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b3a8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051b3a9:
    // 0051b3a9  ba9c0b5500             -mov edx, 0x550b9c
    cpu.edx = 5573532 /*0x550b9c*/;
    // 0051b3ae  b9a80b5500             -mov ecx, 0x550ba8
    cpu.ecx = 5573544 /*0x550ba8*/;
    // 0051b3b3  bb48020000             -mov ebx, 0x248
    cpu.ebx = 584 /*0x248*/;
    // 0051b3b8  b8bc0b5500             -mov eax, 0x550bbc
    cpu.eax = 5573564 /*0x550bbc*/;
    // 0051b3bd  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 0051b3c3  891d98215500           -mov dword ptr [0x552198], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebx;
    // 0051b3c9  ba50040000             -mov edx, 0x450
    cpu.edx = 1104 /*0x450*/;
    // 0051b3ce  8b1df4435600           -mov ebx, dword ptr [0x5643f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 0051b3d4  890d94215500           -mov dword ptr [0x552194], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ecx;
    // 0051b3da  e84162fcff             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b3df  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051b3e1  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051b3e3  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051b3e5  c7400800000000         -mov dword ptr [eax + 8], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 0051b3ec  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051b3ee  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051b3f0  897804                 -mov dword ptr [eax + 4], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.edi;
    // 0051b3f3  2eff1594445300         -call dword ptr cs:[0x534494]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457044) /* 0x534494 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b3fa  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051b3fc  c7430c00000000         -mov dword ptr [ebx + 0xc], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */) = 0 /*0x0*/;
    // 0051b403  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051b405  c7431000000000         -mov dword ptr [ebx + 0x10], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */) = 0 /*0x0*/;
    // 0051b40c  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051b40e  c7832c02000000000000   -mov dword ptr [ebx + 0x22c], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(556) /* 0x22c */) = 0 /*0x0*/;
    // 0051b418  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051b41a  894328                 -mov dword ptr [ebx + 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(40) /* 0x28 */) = cpu.eax;
    // 0051b41d  2eff1594445300         -call dword ptr cs:[0x534494]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457044) /* 0x534494 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b424  c7833002000000000000   -mov dword ptr [ebx + 0x230], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(560) /* 0x230 */) = 0 /*0x0*/;
    // 0051b42e  ba14000000             -mov edx, 0x14
    cpu.edx = 20 /*0x14*/;
    // 0051b433  89834c020000           -mov dword ptr [ebx + 0x24c], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(588) /* 0x24c */) = cpu.eax;
    // 0051b439  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051b43b  c7833402000000000000   -mov dword ptr [ebx + 0x234], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(564) /* 0x234 */) = 0 /*0x0*/;
    // 0051b445  e8c252fcff             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b44a  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051b44c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b44d  bd19000000             -mov ebp, 0x19
    cpu.ebp = 25 /*0x19*/;
    // 0051b452  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051b453  896c2410               -mov dword ptr [esp + 0x10], ebp
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.ebp;
    // 0051b457  2eff15c8455300         -call dword ptr cs:[0x5345c8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457352) /* 0x5345c8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b45e  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051b463  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051b465  891cb5a0b1a000         -mov dword ptr [esi*4 + 0xa0b1a0], ebx
    app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.esi * 4) = cpu.ebx;
    // 0051b46c  e8fff6ffff             -call 0x51ab70
    cpu.esp -= 4;
    sub_51ab70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b471  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051b473  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0051b476  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b477  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b478  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b479  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b47a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b47b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_51b480(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b480  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051b481  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051b482  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051b483  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051b484  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0051b487  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051b489  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b48b  7c05                   -jl 0x51b492
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051b492;
    }
    // 0051b48d  83f810                 +cmp eax, 0x10
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
    // 0051b490  7c31                   -jl 0x51b4c3
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051b4c3;
    }
L_0x0051b492:
    // 0051b492  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051b493  ba9c0b5500             -mov edx, 0x550b9c
    cpu.edx = 5573532 /*0x550b9c*/;
    // 0051b498  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051b499  b9c80b5500             -mov ecx, 0x550bc8
    cpu.ecx = 5573576 /*0x550bc8*/;
    // 0051b49e  be69020000             -mov esi, 0x269
    cpu.esi = 617 /*0x269*/;
    // 0051b4a3  68d40b5500             -push 0x550bd4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573588 /*0x550bd4*/;
    cpu.esp -= 4;
    // 0051b4a8  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 0051b4ae  890d94215500           -mov dword ptr [0x552194], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ecx;
    // 0051b4b4  893598215500           -mov dword ptr [0x552198], esi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.esi;
    // 0051b4ba  e8515beeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b4bf  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051b4c2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0051b4c3:
    // 0051b4c3  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051b4c5  2eff15f4455300         -call dword ptr cs:[0x5345f4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457396) /* 0x5345f4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b4cc  8b3c9da0b1a000         -mov edi, dword ptr [ebx*4 + 0xa0b1a0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.ebx * 4);
    // 0051b4d3  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051b4d5  7408                   -je 0x51b4df
    if (cpu.flags.zf)
    {
        goto L_0x0051b4df;
    }
L_0x0051b4d7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051b4d7  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0051b4da  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b4db  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b4dc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b4dd  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b4de  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051b4df:
    // 0051b4df  8d4301                 -lea eax, [ebx + 1]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 0051b4e2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b4e3  68f40b5500             -push 0x550bf4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573620 /*0x550bf4*/;
    cpu.esp -= 4;
    // 0051b4e8  8d442408               -lea eax, [esp + 8]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051b4ec  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b4ed  e89e41fcff             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b4f2  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0051b4f5  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051b4f6  6880000040             -push 0x40000080
    app->getMemory<x86::reg32>(cpu.esp-4) = 1073741952 /*0x40000080*/;
    cpu.esp -= 4;
    // 0051b4fb  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0051b4fd  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051b4fe  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051b4ff  68000000c0             -push 0xc0000000
    app->getMemory<x86::reg32>(cpu.esp-4) = 3221225472 /*0xc0000000*/;
    cpu.esp -= 4;
    // 0051b504  8d442418               -lea eax, [esp + 0x18]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(24) /* 0x18 */);
    // 0051b508  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b509  2eff1598445300         -call dword ptr cs:[0x534498]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457048) /* 0x534498 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b510  83f8ff                 +cmp eax, -1
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
    // 0051b513  74c2                   -je 0x51b4d7
    if (cpu.flags.zf)
    {
        goto L_0x0051b4d7;
    }
    // 0051b515  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051b517  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b519  e8d2fdffff             -call 0x51b2f0
    cpu.esp -= 4;
    sub_51b2f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b51e  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0051b521  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b522  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b523  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b524  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b525  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_51b530(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b530  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051b531  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051b532  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051b533  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051b535  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0051b537  833dc8ab560000         +cmp dword ptr [0x56abc8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b53e  750a                   -jne 0x51b54a
    if (!cpu.flags.zf)
    {
        goto L_0x0051b54a;
    }
    // 0051b540  e89bfdfcff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b545  a3c8ab5600             -mov dword ptr [0x56abc8], eax
    app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */) = cpu.eax;
L_0x0051b54a:
    // 0051b54a  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b54f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b550  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b556  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051b558  e8d3faffff             -call 0x51b030
    cpu.esp -= 4;
    sub_51b030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b55d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b55f  740b                   -je 0x51b56c
    if (cpu.flags.zf)
    {
        goto L_0x0051b56c;
    }
    // 0051b561  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051b563  e898030000             -call 0x51b900
    cpu.esp -= 4;
    sub_51b900(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b568  39f8                   +cmp eax, edi
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
    // 0051b56a  7d14                   -jge 0x51b580
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051b580;
    }
L_0x0051b56c:
    // 0051b56c  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051b56e  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b573  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b574  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b57a  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b57c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b57d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b57e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b57f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051b580:
    // 0051b580  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0051b582  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051b584  e867000000             -call 0x51b5f0
    cpu.esp -= 4;
    sub_51b5f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b589  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051b58b  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b590  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b591  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b597  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b599  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b59a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b59b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b59c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51b5a0: native, nfs3hp::dead::stub_0x51b5a0 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_51b5f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b5f0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051b5f1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051b5f2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051b5f3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051b5f4  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051b5f7  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051b5f9  89d5                   -mov ebp, edx
    cpu.ebp = cpu.edx;
    // 0051b5fb  891c24                 -mov dword ptr [esp], ebx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ebx;
    // 0051b5fe  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0051b600  897c2404               -mov dword ptr [esp + 4], edi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edi;
    // 0051b604  833dc8ab560000         +cmp dword ptr [0x56abc8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b60b  750a                   -jne 0x51b617
    if (!cpu.flags.zf)
    {
        goto L_0x0051b617;
    }
    // 0051b60d  e8cefcfcff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b612  a3c8ab5600             -mov dword ptr [0x56abc8], eax
    app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */) = cpu.eax;
L_0x0051b617:
    // 0051b617  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b61c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b61d  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b623  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051b625  e806faffff             -call 0x51b030
    cpu.esp -= 4;
    sub_51b030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b62a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b62c  0f84b7000000           -je 0x51b6e9
    if (cpu.flags.zf)
    {
        goto L_0x0051b6e9;
    }
    // 0051b632  8b04b5a0b1a000         -mov eax, dword ptr [esi*4 + 0xa0b1a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.esi * 4);
    // 0051b639  8b9034020000           -mov edx, dword ptr [eax + 0x234]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(564) /* 0x234 */);
    // 0051b63f  01ea                   -add edx, ebp
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebp));
    // 0051b641  81fa00020000           +cmp edx, 0x200
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(512 /*0x200*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b647  7e0b                   -jle 0x51b654
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051b654;
    }
    // 0051b649  bd00020000             -mov ebp, 0x200
    cpu.ebp = 512 /*0x200*/;
    // 0051b64e  2ba834020000           -sub ebp, dword ptr [eax + 0x234]
    (cpu.ebp) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(564) /* 0x234 */)));
L_0x0051b654:
    // 0051b654  8b14b5a0b1a000         -mov edx, dword ptr [esi*4 + 0xa0b1a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.esi * 4);
    // 0051b65b  8b8230020000           -mov eax, dword ptr [edx + 0x230]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(560) /* 0x230 */);
    // 0051b661  038234020000           -add eax, dword ptr [edx + 0x234]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(564) /* 0x234 */)));
    // 0051b667  25ff010000             -and eax, 0x1ff
    cpu.eax &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 0051b66c  8d1428                 -lea edx, [eax + ebp]
    cpu.edx = x86::reg32(cpu.eax + cpu.ebp * 1);
    // 0051b66f  89ef                   -mov edi, ebp
    cpu.edi = cpu.ebp;
    // 0051b671  81fa00020000           +cmp edx, 0x200
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(512 /*0x200*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b677  7e0d                   -jle 0x51b686
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051b686;
    }
    // 0051b679  bf00020000             -mov edi, 0x200
    cpu.edi = 512 /*0x200*/;
    // 0051b67e  29c7                   -sub edi, eax
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0051b680  29fd                   -sub ebp, edi
    (cpu.ebp) -= x86::reg32(x86::sreg32(cpu.edi));
    // 0051b682  896c2404               -mov dword ptr [esp + 4], ebp
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.ebp;
L_0x0051b686:
    // 0051b686  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051b688  7419                   -je 0x51b6a3
    if (cpu.flags.zf)
    {
        goto L_0x0051b6a3;
    }
    // 0051b68a  8b14b5a0b1a000         -mov edx, dword ptr [esi*4 + 0xa0b1a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.esi * 4);
    // 0051b691  81c250020000           -add edx, 0x250
    (cpu.edx) += x86::reg32(x86::sreg32(592 /*0x250*/));
    // 0051b697  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0051b699  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051b69b  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 0051b69e  e84deefcff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051b6a3:
    // 0051b6a3  8b5c2404               -mov ebx, dword ptr [esp + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051b6a7  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051b6a9  7417                   -je 0x51b6c2
    if (cpu.flags.zf)
    {
        goto L_0x0051b6c2;
    }
    // 0051b6ab  8b14b5a0b1a000         -mov edx, dword ptr [esi*4 + 0xa0b1a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.esi * 4);
    // 0051b6b2  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 0051b6b5  81c250020000           -add edx, 0x250
    (cpu.edx) += x86::reg32(x86::sreg32(592 /*0x250*/));
    // 0051b6bb  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 0051b6bd  e82eeefcff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051b6c2:
    // 0051b6c2  8b04b5a0b1a000         -mov eax, dword ptr [esi*4 + 0xa0b1a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.esi * 4);
    // 0051b6c9  8b542404               -mov edx, dword ptr [esp + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051b6cd  8ba834020000           -mov ebp, dword ptr [eax + 0x234]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(564) /* 0x234 */);
    // 0051b6d3  01fa                   -add edx, edi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edi));
    // 0051b6d5  01d5                   -add ebp, edx
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.edx));
    // 0051b6d7  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051b6dc  89a834020000           -mov dword ptr [eax + 0x234], ebp
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(564) /* 0x234 */) = cpu.ebp;
    // 0051b6e2  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051b6e4  e887f4ffff             -call 0x51ab70
    cpu.esp -= 4;
    sub_51ab70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051b6e9:
    // 0051b6e9  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b6ee  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b6ef  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b6f5  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051b6f9  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 0051b6fb  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051b6fe  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b6ff  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b700  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b701  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b702  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_51b710(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b710  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051b711  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051b712  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051b713  83ec1c                 -sub esp, 0x1c
    (cpu.esp) -= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 0051b716  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051b718  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0051b71a  89dd                   -mov ebp, ebx
    cpu.ebp = cpu.ebx;
    // 0051b71c  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0051b71e  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051b720  2eff15f4455300         -call dword ptr cs:[0x5345f4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457396) /* 0x5345f4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b727  833dc8ab560000         +cmp dword ptr [0x56abc8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b72e  750a                   -jne 0x51b73a
    if (!cpu.flags.zf)
    {
        goto L_0x0051b73a;
    }
    // 0051b730  e8abfbfcff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b735  a3c8ab5600             -mov dword ptr [0x56abc8], eax
    app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */) = cpu.eax;
L_0x0051b73a:
    // 0051b73a  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b73f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b740  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b746  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051b748  e8e3f8ffff             -call 0x51b030
    cpu.esp -= 4;
    sub_51b030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b74d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b74f  0f849f000000           -je 0x51b7f4
    if (cpu.flags.zf)
    {
        goto L_0x0051b7f4;
    }
    // 0051b755  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051b757  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b758  8b04b5a0b1a000         -mov eax, dword ptr [esi*4 + 0xa0b1a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.esi * 4);
    // 0051b75f  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0051b762  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051b763  2eff15f0445300         -call dword ptr cs:[0x5344f0]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457136) /* 0x5344f0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b76a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b76c  7454                   -je 0x51b7c2
    if (cpu.flags.zf)
    {
        goto L_0x0051b7c2;
    }
    // 0051b76e  804c240801             -or byte ptr [esp + 8], 1
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(8) /* 0x8 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 0051b773  6681642408fdb0         -and word ptr [esp + 8], 0xb0fd
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(8) /* 0x8 */) &= x86::reg16(x86::sreg16(45309 /*0xb0fd*/));
    // 0051b77a  83ff10                 +cmp edi, 0x10
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b77d  7f07                   -jg 0x51b786
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0051b786;
    }
    // 0051b77f  8b3cbdccab5600         -mov edi, dword ptr [edi*4 + 0x56abcc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5680076) /* 0x56abcc */ + cpu.edi * 4);
L_0x0051b786:
    // 0051b786  8a44242c               -mov al, byte ptr [esp + 0x2c]
    cpu.al = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(44) /* 0x2c */);
    // 0051b78a  0405                   -add al, 5
    (cpu.al) += x86::reg8(x86::sreg8(5 /*0x5*/));
    // 0051b78c  897c2404               -mov dword ptr [esp + 4], edi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edi;
    // 0051b790  88442412               -mov byte ptr [esp + 0x12], al
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(18) /* 0x12 */) = cpu.al;
    // 0051b794  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051b796  763f                   -jbe 0x51b7d7
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051b7d7;
    }
    // 0051b798  83fb01                 +cmp ebx, 1
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
    // 0051b79b  753a                   -jne 0x51b7d7
    if (!cpu.flags.zf)
    {
        goto L_0x0051b7d7;
    }
    // 0051b79d  c644241402             -mov byte ptr [esp + 0x14], 2
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(20) /* 0x14 */) = 2 /*0x2*/;
L_0x0051b7a2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051b7a2  83fd01                 +cmp ebp, 1
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b7a5  7338                   -jae 0x51b7df
    if (!cpu.flags.cf)
    {
        goto L_0x0051b7df;
    }
L_0x0051b7a7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051b7a7  30c9                   -xor cl, cl
    cpu.cl ^= x86::reg8(x86::sreg8(cpu.cl));
    // 0051b7a9  884c2413               -mov byte ptr [esp + 0x13], cl
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(19) /* 0x13 */) = cpu.cl;
L_0x0051b7ad:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051b7ad  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051b7af  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b7b0  8b04b5a0b1a000         -mov eax, dword ptr [esi*4 + 0xa0b1a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.esi * 4);
    // 0051b7b7  8b4804                 -mov ecx, dword ptr [eax + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0051b7ba  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051b7bb  2eff15c4455300         -call dword ptr cs:[0x5345c4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457348) /* 0x5345c4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051b7c2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051b7c2  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b7c7  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b7c8  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b7ce  83c41c                 -add esp, 0x1c
    (cpu.esp) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 0051b7d1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b7d2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b7d3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b7d4  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x0051b7d7:
    // 0051b7d7  30f6                   +xor dh, dh
    cpu.clear_co();
    cpu.set_szp((cpu.dh ^= x86::reg8(x86::sreg8(cpu.dh))));
    // 0051b7d9  88742414               -mov byte ptr [esp + 0x14], dh
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(20) /* 0x14 */) = cpu.dh;
    // 0051b7dd  ebc3                   -jmp 0x51b7a2
    goto L_0x0051b7a2;
L_0x0051b7df:
    // 0051b7df  7707                   -ja 0x51b7e8
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051b7e8;
    }
    // 0051b7e1  c644241301             -mov byte ptr [esp + 0x13], 1
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(19) /* 0x13 */) = 1 /*0x1*/;
    // 0051b7e6  ebc5                   -jmp 0x51b7ad
    goto L_0x0051b7ad;
L_0x0051b7e8:
    // 0051b7e8  83fd03                 +cmp ebp, 3
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b7eb  75ba                   -jne 0x51b7a7
    if (!cpu.flags.zf)
    {
        goto L_0x0051b7a7;
    }
    // 0051b7ed  c644241302             -mov byte ptr [esp + 0x13], 2
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(19) /* 0x13 */) = 2 /*0x2*/;
    // 0051b7f2  ebb9                   -jmp 0x51b7ad
    goto L_0x0051b7ad;
L_0x0051b7f4:
    // 0051b7f4  b99c0b5500             -mov ecx, 0x550b9c
    cpu.ecx = 5573532 /*0x550b9c*/;
    // 0051b7f9  bb000c5500             -mov ebx, 0x550c00
    cpu.ebx = 5573632 /*0x550c00*/;
    // 0051b7fe  be56030000             -mov esi, 0x356
    cpu.esi = 854 /*0x356*/;
    // 0051b803  680c0c5500             -push 0x550c0c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573644 /*0x550c0c*/;
    cpu.esp -= 4;
    // 0051b808  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 0051b80e  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 0051b814  893598215500           -mov dword ptr [0x552198], esi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.esi;
    // 0051b81a  e8f157eeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b81f  83c404                 +add esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051b822  eb9e                   -jmp 0x51b7c2
    goto L_0x0051b7c2;
}

/* sub_51b830: native, nfs3hp::dead::stub_0x51b830 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_51b8a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b8a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051b8a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051b8a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051b8a3  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051b8a5  833dc8ab560000         +cmp dword ptr [0x56abc8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b8ac  750a                   -jne 0x51b8b8
    if (!cpu.flags.zf)
    {
        goto L_0x0051b8b8;
    }
    // 0051b8ae  e82dfafcff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b8b3  a3c8ab5600             -mov dword ptr [0x56abc8], eax
    app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */) = cpu.eax;
L_0x0051b8b8:
    // 0051b8b8  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b8bd  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b8be  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b8c4  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b8c6  e865f7ffff             -call 0x51b030
    cpu.esp -= 4;
    sub_51b030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b8cb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b8cd  741c                   -je 0x51b8eb
    if (cpu.flags.zf)
    {
        goto L_0x0051b8eb;
    }
    // 0051b8cf  8b1c9da0b1a000         -mov ebx, dword ptr [ebx*4 + 0xa0b1a0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.ebx * 4);
    // 0051b8d6  8b5b10                 -mov ebx, dword ptr [ebx + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */);
    // 0051b8d9  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b8de  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b8df  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b8e5  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b8e7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b8e8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b8e9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b8ea  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051b8eb:
    // 0051b8eb  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051b8ed  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b8f2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b8f3  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b8f9  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b8fb  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b8fc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b8fd  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b8fe  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_51b900(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b900  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051b901  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051b902  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051b903  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051b905  833dc8ab560000         +cmp dword ptr [0x56abc8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b90c  750a                   -jne 0x51b918
    if (!cpu.flags.zf)
    {
        goto L_0x0051b918;
    }
    // 0051b90e  e8cdf9fcff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b913  a3c8ab5600             -mov dword ptr [0x56abc8], eax
    app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */) = cpu.eax;
L_0x0051b918:
    // 0051b918  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b91d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b91e  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b924  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b926  e805f7ffff             -call 0x51b030
    cpu.esp -= 4;
    sub_51b030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b92b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b92d  7428                   -je 0x51b957
    if (cpu.flags.zf)
    {
        goto L_0x0051b957;
    }
    // 0051b92f  8b1c9da0b1a000         -mov ebx, dword ptr [ebx*4 + 0xa0b1a0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.ebx * 4);
    // 0051b936  b800020000             -mov eax, 0x200
    cpu.eax = 512 /*0x200*/;
    // 0051b93b  8b8b34020000           -mov ecx, dword ptr [ebx + 0x234]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(564) /* 0x234 */);
    // 0051b941  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051b943  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051b945  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b94a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b94b  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b951  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b953  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b954  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b955  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b956  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051b957:
    // 0051b957  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051b959  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b95e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b95f  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b965  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051b967  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b968  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b969  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051b96a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_51b970(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051b970  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051b971  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051b972  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051b973  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051b974  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051b977  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051b979  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0051b97b  891c24                 -mov dword ptr [esp], ebx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ebx;
    // 0051b97e  31ed                   -xor ebp, ebp
    cpu.ebp ^= x86::reg32(x86::sreg32(cpu.ebp));
    // 0051b980  896c2404               -mov dword ptr [esp + 4], ebp
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.ebp;
    // 0051b984  833dc8ab560000         +cmp dword ptr [0x56abc8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b98b  750a                   -jne 0x51b997
    if (!cpu.flags.zf)
    {
        goto L_0x0051b997;
    }
    // 0051b98d  e84ef9fcff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b992  a3c8ab5600             -mov dword ptr [0x56abc8], eax
    app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */) = cpu.eax;
L_0x0051b997:
    // 0051b997  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051b99c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051b99d  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b9a3  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051b9a5  e886f6ffff             -call 0x51b030
    cpu.esp -= 4;
    sub_51b030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051b9aa  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051b9ac  0f847a000000           -je 0x51ba2c
    if (cpu.flags.zf)
    {
        goto L_0x0051ba2c;
    }
    // 0051b9b2  8b04bda0b1a000         -mov eax, dword ptr [edi*4 + 0xa0b1a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.edi * 4);
    // 0051b9b9  8b5810                 -mov ebx, dword ptr [eax + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0051b9bc  39de                   +cmp esi, ebx
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
    // 0051b9be  7e02                   -jle 0x51b9c2
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051b9c2;
    }
    // 0051b9c0  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
L_0x0051b9c2:
    // 0051b9c2  8b04bda0b1a000         -mov eax, dword ptr [edi*4 + 0xa0b1a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.edi * 4);
    // 0051b9c9  8b500c                 -mov edx, dword ptr [eax + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 0051b9cc  01f2                   -add edx, esi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.esi));
    // 0051b9ce  89f5                   -mov ebp, esi
    cpu.ebp = cpu.esi;
    // 0051b9d0  81fa00020000           +cmp edx, 0x200
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(512 /*0x200*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051b9d6  7e18                   -jle 0x51b9f0
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051b9f0;
    }
    // 0051b9d8  ba00020000             -mov edx, 0x200
    cpu.edx = 512 /*0x200*/;
    // 0051b9dd  8b680c                 -mov ebp, dword ptr [eax + 0xc]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 0051b9e0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051b9e2  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051b9e4  29e8                   -sub eax, ebp
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 0051b9e6  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0051b9e8  89f5                   -mov ebp, esi
    cpu.ebp = cpu.esi;
    // 0051b9ea  89542404               -mov dword ptr [esp + 4], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 0051b9ee  29d5                   -sub ebp, edx
    (cpu.ebp) -= x86::reg32(x86::sreg32(cpu.edx));
L_0x0051b9f0:
    // 0051b9f0  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0051b9f2  7419                   -je 0x51ba0d
    if (cpu.flags.zf)
    {
        goto L_0x0051ba0d;
    }
    // 0051b9f4  8b04bda0b1a000         -mov eax, dword ptr [edi*4 + 0xa0b1a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.edi * 4);
    // 0051b9fb  8d502c                 -lea edx, [eax + 0x2c]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(44) /* 0x2c */);
    // 0051b9fe  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 0051ba01  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 0051ba03  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0051ba05  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051ba08  e8e3eafcff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051ba0d:
    // 0051ba0d  837c240400             +cmp dword ptr [esp + 4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ba12  7418                   -je 0x51ba2c
    if (cpu.flags.zf)
    {
        goto L_0x0051ba2c;
    }
    // 0051ba14  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051ba17  8b04bda0b1a000         -mov eax, dword ptr [edi*4 + 0xa0b1a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.edi * 4);
    // 0051ba1e  8b5c2404               -mov ebx, dword ptr [esp + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051ba22  83c02c                 -add eax, 0x2c
    (cpu.eax) += x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 0051ba25  01ea                   -add edx, ebp
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebp));
    // 0051ba27  e8c4eafcff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051ba2c:
    // 0051ba2c  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051ba31  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ba32  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ba38  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051ba3c  01e8                   -add eax, ebp
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebp));
    // 0051ba3e  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051ba41  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ba42  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ba43  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ba44  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ba45  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_51ba50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051ba50  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051ba51  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051ba52  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051ba53  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051ba55  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0051ba57  833dc8ab560000         +cmp dword ptr [0x56abc8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ba5e  750a                   -jne 0x51ba6a
    if (!cpu.flags.zf)
    {
        goto L_0x0051ba6a;
    }
    // 0051ba60  e87bf8fcff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ba65  a3c8ab5600             -mov dword ptr [0x56abc8], eax
    app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */) = cpu.eax;
L_0x0051ba6a:
    // 0051ba6a  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051ba6f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ba70  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ba76  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0051ba78  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051ba7a  e8f1feffff             -call 0x51b970
    cpu.esp -= 4;
    sub_51b970(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ba7f  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051ba81  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ba83  7512                   -jne 0x51ba97
    if (!cpu.flags.zf)
    {
        goto L_0x0051ba97;
    }
    // 0051ba85  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051ba8a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ba8b  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ba91  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051ba93  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ba94  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ba95  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ba96  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051ba97:
    // 0051ba97  8b3cb5a0b1a000         -mov edi, dword ptr [esi*4 + 0xa0b1a0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.esi * 4);
    // 0051ba9e  294710                 -sub dword ptr [edi + 0x10], eax
    (app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */)) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0051baa1  8b14b5a0b1a000         -mov edx, dword ptr [esi*4 + 0xa0b1a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531232) /* 0xa0b1a0 */ + cpu.esi * 4);
    // 0051baa8  03420c                 -add eax, dword ptr [edx + 0xc]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 0051baab  25ff010000             -and eax, 0x1ff
    cpu.eax &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 0051bab0  89420c                 -mov dword ptr [edx + 0xc], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 0051bab3  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051bab8  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051baba  e8b1f0ffff             -call 0x51ab70
    cpu.esp -= 4;
    sub_51ab70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051babf  a1c8ab5600             -mov eax, dword ptr [0x56abc8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680072) /* 0x56abc8 */);
    // 0051bac4  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051bac5  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bacb  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051bacd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bace  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bacf  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bad0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51bae0: native, nfs3hp::dead::stub_0x51bae0 */

/* sub_51bb60: native, nfs3hp::dead::stub_0x51bb60 */

/* align: skip 0x8b 0xc0 */
void Application::sub_51bbd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051bbd0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051bbd1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051bbd2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051bbd3  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051bbd5  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0051bbd7  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051bbd9  81fa00020000           +cmp edx, 0x200
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(512 /*0x200*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051bbdf  7e32                   -jle 0x51bc13
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051bc13;
    }
    // 0051bbe1  6800020000             -push 0x200
    app->getMemory<x86::reg32>(cpu.esp-4) = 512 /*0x200*/;
    cpu.esp -= 4;
    // 0051bbe6  bd9c0b5500             -mov ebp, 0x550b9c
    cpu.ebp = 5573532 /*0x550b9c*/;
    // 0051bbeb  b8580c5500             -mov eax, 0x550c58
    cpu.eax = 5573720 /*0x550c58*/;
    // 0051bbf0  68680c5500             -push 0x550c68
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573736 /*0x550c68*/;
    cpu.esp -= 4;
    // 0051bbf5  892d90215500           -mov dword ptr [0x552190], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebp;
    // 0051bbfb  bd71040000             -mov ebp, 0x471
    cpu.ebp = 1137 /*0x471*/;
    // 0051bc00  a394215500             -mov dword ptr [0x552194], eax
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.eax;
    // 0051bc05  892d98215500           -mov dword ptr [0x552198], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebp;
    // 0051bc0b  e80054eeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bc10  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
L_0x0051bc13:
    // 0051bc13  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051bc15  e816f4ffff             -call 0x51b030
    cpu.esp -= 4;
    sub_51b030(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bc1a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051bc1c  7436                   -je 0x51bc54
    if (cpu.flags.zf)
    {
        goto L_0x0051bc54;
    }
    // 0051bc1e  8b1da0c17900           -mov ebx, dword ptr [0x79c1a0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7979424) /* 0x79c1a0 */);
    // 0051bc24  01cb                   -add ebx, ecx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0051bc26  8d0cb500000000         -lea ecx, [esi*4]
    cpu.ecx = x86::reg32(cpu.esi * 4);
L_0x0051bc2d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051bc2d  8b81a0b1a000           -mov eax, dword ptr [ecx + 0xa0b1a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(10531232) /* 0xa0b1a0 */);
    // 0051bc33  3b5010                 +cmp edx, dword ptr [eax + 0x10]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051bc36  7e11                   -jle 0x51bc49
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051bc49;
    }
    // 0051bc38  3b1da0c17900           +cmp ebx, dword ptr [0x79c1a0]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7979424) /* 0x79c1a0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051bc3e  7e09                   -jle 0x51bc49
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051bc49;
    }
    // 0051bc40  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051bc42  e8993cfcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bc47  ebe4                   -jmp 0x51bc2d
    goto L_0x0051bc2d;
L_0x0051bc49:
    // 0051bc49  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0051bc4b  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051bc4d  e8fefdffff             -call 0x51ba50
    cpu.esp -= 4;
    sub_51ba50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bc52  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
L_0x0051bc54:
    // 0051bc54  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051bc56  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bc57  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bc58  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bc59  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 0051bc5f  90                     -nop 
    ;
    // 0051bc60  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_51bc70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051bc70  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051bc71  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051bc73  c6400300               -mov byte ptr [eax + 3], 0
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(3) /* 0x3 */) = 0 /*0x0*/;
    // 0051bc77  8a4003                 -mov al, byte ptr [eax + 3]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(3) /* 0x3 */);
    // 0051bc7a  884102                 -mov byte ptr [ecx + 2], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(2) /* 0x2 */) = cpu.al;
    // 0051bc7d  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051bc7f  e8fc8afdff             -call 0x4f4780
    cpu.esp -= 4;
    sub_4f4780(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bc84  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051bc86  c1f808                 -sar eax, 8
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (8 /*0x8*/ % 32));
    // 0051bc89  885103                 -mov byte ptr [ecx + 3], dl
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(3) /* 0x3 */) = cpu.dl;
    // 0051bc8c  884102                 -mov byte ptr [ecx + 2], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(2) /* 0x2 */) = cpu.al;
    // 0051bc8f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bc90  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_51bca0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051bca0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051bca1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051bca2  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051bca4  8a5802                 -mov bl, byte ptr [eax + 2]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(2) /* 0x2 */);
    // 0051bca7  8a7803                 -mov bh, byte ptr [eax + 3]
    cpu.bh = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(3) /* 0x3 */);
    // 0051bcaa  e8c1ffffff             -call 0x51bc70
    cpu.esp -= 4;
    sub_51bc70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bcaf  663b5902               +cmp bx, word ptr [ecx + 2]
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(2) /* 0x2 */)));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0051bcb3  7508                   -jne 0x51bcbd
    if (!cpu.flags.zf)
    {
        goto L_0x0051bcbd;
    }
    // 0051bcb5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051bcba  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bcbb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bcbc  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051bcbd:
    // 0051bcbd  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051bcbf  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bcc0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bcc1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_51bcd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051bcd0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051bcd1  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 0051bcd3  8d5004                 -lea edx, [eax + 4]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0051bcd6  c600fe                 -mov byte ptr [eax], 0xfe
    app->getMemory<x86::reg8>(cpu.eax) = 254 /*0xfe*/;
    // 0051bcd9  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0051bcdb  884801                 -mov byte ptr [eax + 1], cl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1) /* 0x1 */) = cpu.cl;
    // 0051bcde  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051bce0  e80be8fcff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bce5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bce6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_51bcf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051bcf0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051bcf1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051bcf2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051bcf3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051bcf4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051bcf5  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051bcf7  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 0051bcf9  8b3b                   -mov edi, dword ptr [ebx]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx);
    // 0051bcfb  8b6b04                 -mov ebp, dword ptr [ebx + 4]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0051bcfe  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051bd00  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051bd02  8a4801                 -mov cl, byte ptr [eax + 1]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0051bd05  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051bd07  83c104                 -add ecx, 4
    (cpu.ecx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051bd0a  ff550c                 -call dword ptr [ebp + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bd0d  39c8                   +cmp eax, ecx
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
    // 0051bd0f  7308                   -jae 0x51bd19
    if (!cpu.flags.cf)
    {
        goto L_0x0051bd19;
    }
    // 0051bd11  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051bd13  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd14  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd15  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd16  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd17  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd18  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051bd19:
    // 0051bd19  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0051bd1b  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051bd1d  e84effffff             -call 0x51bc70
    cpu.esp -= 4;
    sub_51bc70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bd22  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051bd24  8b6b04                 -mov ebp, dword ptr [ebx + 4]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0051bd27  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051bd29  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0051bd2b  ff5518                 -call dword ptr [ebp + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bd2e  39c8                   +cmp eax, ecx
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
    // 0051bd30  750d                   -jne 0x51bd3f
    if (!cpu.flags.zf)
    {
        goto L_0x0051bd3f;
    }
    // 0051bd32  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051bd37  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051bd39  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd3a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd3b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd3c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd3d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd3e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051bd3f:
    // 0051bd3f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051bd41  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051bd43  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd44  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd45  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd46  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd47  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd48  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_51bd50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051bd50  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051bd51  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051bd52  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051bd53  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051bd54  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051bd57  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051bd59  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 0051bd5b  8b30                   -mov esi, dword ptr [eax]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax);
    // 0051bd5d  8b7804                 -mov edi, dword ptr [eax + 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0051bd60  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051bd62  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051bd64  ff570c                 -call dword ptr [edi + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bd67  83f804                 +cmp eax, 4
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
    // 0051bd6a  730a                   -jae 0x51bd76
    if (!cpu.flags.cf)
    {
        goto L_0x0051bd76;
    }
    // 0051bd6c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051bd6e  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051bd71  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd72  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd73  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd74  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bd75  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051bd76:
    // 0051bd76  b4fe                   -mov ah, 0xfe
    cpu.ah = 254 /*0xfe*/;
    // 0051bd78  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 0051bd7d  885c2401               -mov byte ptr [esp + 1], bl
    app->getMemory<x86::reg8>(cpu.esp + x86::reg32(1) /* 0x1 */) = cpu.bl;
    // 0051bd81  882424                 -mov byte ptr [esp], ah
    app->getMemory<x86::reg8>(cpu.esp) = cpu.ah;
    // 0051bd84  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051bd86  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 0051bd8b  e8e0feffff             -call 0x51bc70
    cpu.esp -= 4;
    sub_51bc70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bd90  8b4904                 -mov ecx, dword ptr [ecx + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0051bd93  89e2                   -mov edx, esp
    cpu.edx = cpu.esp;
    // 0051bd95  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051bd97  ff5118                 -call dword ptr [ecx + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bd9a  83f804                 +cmp eax, 4
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
    // 0051bd9d  750f                   -jne 0x51bdae
    if (!cpu.flags.zf)
    {
        goto L_0x0051bdae;
    }
    // 0051bd9f  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051bda4  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051bda6  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051bda9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bdaa  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bdab  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bdac  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bdad  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051bdae:
    // 0051bdae  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051bdb0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051bdb2  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051bdb5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bdb6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bdb7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bdb8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bdb9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_51bdc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051bdc0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051bdc1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051bdc2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051bdc3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051bdc4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051bdc5  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051bdc7  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051bdce  7c18                   -jl 0x51bde8
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051bde8;
    }
    // 0051bdd0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051bdd2  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051bdd5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051bdd6  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051bdd8  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051bdda  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051bddc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051bdde  b8b40c5500             -mov eax, 0x550cb4
    cpu.eax = 5573812 /*0x550cb4*/;
    // 0051bde3  e89863ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051bde8:
    // 0051bde8  b828269f00             -mov eax, 0x9f2628
    cpu.eax = 10429992 /*0x9f2628*/;
    // 0051bded  8b17                   -mov edx, dword ptr [edi]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi);
    // 0051bdef  e85c23ffff             -call 0x50e150
    cpu.esp -= 4;
    nfs3hp::ea::qremove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bdf4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051bdf6  746e                   -je 0x51be66
    if (cpu.flags.zf)
    {
        goto L_0x0051be66;
    }
    // 0051bdf8  8b1dec6d5600           -mov ebx, dword ptr [0x566dec]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
    // 0051bdfe  8b37                   -mov esi, dword ptr [edi]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edi);
    // 0051be00  83fb01                 +cmp ebx, 1
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
    // 0051be03  7c18                   -jl 0x51be1d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051be1d;
    }
    // 0051be05  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051be07  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051be0a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051be0b  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051be0d  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051be0f  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051be11  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051be13  b8c40c5500             -mov eax, 0x550cc4
    cpu.eax = 5573828 /*0x550cc4*/;
    // 0051be18  e86363ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051be1d:
    // 0051be1d  c7462400000000         -mov dword ptr [esi + 0x24], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
    // 0051be24  8b5704                 -mov edx, dword ptr [edi + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0051be27  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051be29  ff5204                 -call dword ptr [edx + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051be2c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051be2e  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051be30  ff5618                 -call dword ptr [esi + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051be33  b80c269f00             -mov eax, 0x9f260c
    cpu.eax = 10429964 /*0x9f260c*/;
    // 0051be38  e81320ffff             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051be3d  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051be44  7d06                   -jge 0x51be4c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051be4c;
    }
L_0x0051be46:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051be46  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051be47  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051be48  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051be49  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051be4a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051be4b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051be4c:
    // 0051be4c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051be4e  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051be51  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051be52  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051be54  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051be56  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051be58  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051be5a  b8d00c5500             -mov eax, 0x550cd0
    cpu.eax = 5573840 /*0x550cd0*/;
    // 0051be5f  e81c63ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051be64  ebe0                   -jmp 0x51be46
    goto L_0x0051be46;
L_0x0051be66:
    // 0051be66  833dec6d560001         +cmp dword ptr [0x566dec], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051be6d  7cd7                   -jl 0x51be46
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051be46;
    }
    // 0051be6f  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051be72  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051be73  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051be75  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051be77  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051be79  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051be7b  b8dc0c5500             -mov eax, 0x550cdc
    cpu.eax = 5573852 /*0x550cdc*/;
    // 0051be80  e8fb62ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051be85  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051be86  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051be87  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051be88  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051be89  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051be8a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_51be90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051be90  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051be91  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051be92  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051be93  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051be96  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051be98  89dd                   -mov ebp, ebx
    cpu.ebp = cpu.ebx;
    // 0051be9a  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0051be9c  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 0051be9e  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0051bea1  8b4704                 -mov eax, dword ptr [edi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0051bea4  8a501f                 -mov dl, byte ptr [eax + 0x1f]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(31) /* 0x1f */);
    // 0051bea7  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0051bea9  84d2                   +test dl, dl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & cpu.dl));
    // 0051beab  7512                   -jne 0x51bebf
    if (!cpu.flags.zf)
    {
        goto L_0x0051bebf;
    }
    // 0051bead  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0051beaf  8d1429                 -lea edx, [ecx + ebp]
    cpu.edx = x86::reg32(cpu.ecx + cpu.ebp * 1);
L_0x0051beb2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051beb2  39d0                   +cmp eax, edx
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
    // 0051beb4  7309                   -jae 0x51bebf
    if (!cpu.flags.cf)
    {
        goto L_0x0051bebf;
    }
    // 0051beb6  8038fe                 +cmp byte ptr [eax], 0xfe
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(254 /*0xfe*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051beb9  7404                   -je 0x51bebf
    if (cpu.flags.zf)
    {
        goto L_0x0051bebf;
    }
    // 0051bebb  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0051bebc  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0051bebd  ebf3                   -jmp 0x51beb2
    goto L_0x0051beb2;
L_0x0051bebf:
    // 0051bebf  8d042e                 -lea eax, [esi + ebp]
    cpu.eax = x86::reg32(cpu.esi + cpu.ebp * 1);
    // 0051bec2  8038fe                 +cmp byte ptr [eax], 0xfe
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(254 /*0xfe*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051bec5  0f8585000000           -jne 0x51bf50
    if (!cpu.flags.zf)
    {
        goto L_0x0051bf50;
    }
    // 0051becb  8d4e04                 -lea ecx, [esi + 4]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0051bece  39d9                   +cmp ecx, ebx
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
    // 0051bed0  0f8771000000           -ja 0x51bf47
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051bf47;
    }
    // 0051bed6  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051bed8  8a5001                 -mov dl, byte ptr [eax + 1]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0051bedb  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 0051bedd  3b5708                 +cmp edx, dword ptr [edi + 8]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8) /* 0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051bee0  0f87bb000000           -ja 0x51bfa1
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051bfa1;
    }
    // 0051bee6  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0051bee8  39d9                   +cmp ecx, ebx
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
    // 0051beea  775b                   -ja 0x51bf47
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051bf47;
    }
    // 0051beec  29f1                   -sub ecx, esi
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 0051beee  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0051bef0  e8abfdffff             -call 0x51bca0
    cpu.esp -= 4;
    sub_51bca0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bef5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051bef7  0f8498000000           -je 0x51bf95
    if (cpu.flags.zf)
    {
        goto L_0x0051bf95;
    }
    // 0051befd  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 0051bf00  83782400               +cmp dword ptr [eax + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051bf04  740f                   -je 0x51bf15
    if (cpu.flags.zf)
    {
        goto L_0x0051bf15;
    }
    // 0051bf06  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051bf09  a1f06d5600             -mov eax, dword ptr [0x566df0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664240) /* 0x566df0 */);
    // 0051bf0e  894224                 -mov dword ptr [edx + 0x24], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(36) /* 0x24 */) = cpu.eax;
    // 0051bf11  c6470d01               -mov byte ptr [edi + 0xd], 1
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */) = 1 /*0x1*/;
L_0x0051bf15:
    // 0051bf15  8a7501                 -mov dh, byte ptr [ebp + 1]
    cpu.dh = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(1) /* 0x1 */);
    // 0051bf18  01ce                   -add esi, ecx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0051bf1a  80feff                 +cmp dh, 0xff
    {
        x86::reg8 tmp1 = cpu.dh;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(255 /*0xff*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051bf1d  7535                   -jne 0x51bf54
    if (!cpu.flags.zf)
    {
        goto L_0x0051bf54;
    }
    // 0051bf1f  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051bf26  7c18                   -jl 0x51bf40
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051bf40;
    }
    // 0051bf28  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051bf2a  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051bf2d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051bf2e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051bf30  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051bf32  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051bf34  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051bf36  b8ec0c5500             -mov eax, 0x550cec
    cpu.eax = 5573868 /*0x550cec*/;
    // 0051bf3b  e84062ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051bf40:
    // 0051bf40  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051bf42  e879feffff             -call 0x51bdc0
    cpu.esp -= 4;
    sub_51bdc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051bf47:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051bf47  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
L_0x0051bf49:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051bf49  83c404                 +add esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051bf4c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bf4d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bf4e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bf4f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051bf50:
    // 0051bf50  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051bf52  ebf5                   -jmp 0x51bf49
    goto L_0x0051bf49;
L_0x0051bf54:
    // 0051bf54  84f6                   +test dh, dh
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dh & cpu.dh));
    // 0051bf56  74ef                   -je 0x51bf47
    if (cpu.flags.zf)
    {
        goto L_0x0051bf47;
    }
    // 0051bf58  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051bf5f  7c1b                   -jl 0x51bf7c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051bf7c;
    }
    // 0051bf61  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051bf63  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051bf66  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051bf68  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051bf69  8d5d04                 -lea ebx, [ebp + 4]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(4) /* 0x4 */);
    // 0051bf6c  88f1                   -mov cl, dh
    cpu.cl = cpu.dh;
    // 0051bf6e  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051bf70  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051bf72  b8fc0c5500             -mov eax, 0x550cfc
    cpu.eax = 5573884 /*0x550cfc*/;
    // 0051bf77  e80462ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051bf7c:
    // 0051bf7c  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 0051bf7f  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051bf81  8d5504                 -lea edx, [ebp + 4]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(4) /* 0x4 */);
    // 0051bf84  8a5d01                 -mov bl, byte ptr [ebp + 1]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(1) /* 0x1 */);
    // 0051bf87  e86467ffff             -call 0x5126f0
    cpu.esp -= 4;
    sub_5126f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051bf8c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051bf8e  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051bf91  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bf92  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bf93  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bf94  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051bf95:
    // 0051bf95  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0051bf98  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051bf9a  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051bf9d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bf9e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bf9f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bfa0  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051bfa1:
    // 0051bfa1  80fafe                 +cmp dl, 0xfe
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(254 /*0xfe*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051bfa4  7411                   -je 0x51bfb7
    if (cpu.flags.zf)
    {
        goto L_0x0051bfb7;
    }
    // 0051bfa6  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051bfab  40                     -inc eax
    (cpu.eax)++;
    // 0051bfac  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051bfae  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051bfb0  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051bfb3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bfb4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bfb5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bfb6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051bfb7:
    // 0051bfb7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051bfb9  40                     -inc eax
    (cpu.eax)++;
    // 0051bfba  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051bfbc  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051bfbe  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051bfc1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bfc2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bfc3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051bfc4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_51bfd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051bfd0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051bfd1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051bfd2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051bfd3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051bfd4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051bfd5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051bfd6  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0051bfd9  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051bfdb  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051bfdd  891424                 -mov dword ptr [esp], edx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.edx;
    // 0051bfe0  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 0051bfe2  89442404               -mov dword ptr [esp + 4], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 0051bfe6  8b4704                 -mov eax, dword ptr [edi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0051bfe9  8a501f                 -mov dl, byte ptr [eax + 0x1f]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(31) /* 0x1f */);
    // 0051bfec  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0051bff0  84d2                   +test dl, dl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & cpu.dl));
    // 0051bff2  0f858f000000           -jne 0x51c087
    if (!cpu.flags.zf)
    {
        goto L_0x0051c087;
    }
L_0x0051bff8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051bff8  8b4f1c                 -mov ecx, dword ptr [edi + 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */);
    // 0051bffb  8b4718                 -mov eax, dword ptr [edi + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(24) /* 0x18 */);
    // 0051bffe  39c8                   +cmp eax, ecx
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
    // 0051c000  7617                   -jbe 0x51c019
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051c019;
    }
    // 0051c002  8b742408               -mov esi, dword ptr [esp + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051c006  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051c008  8b5714                 -mov edx, dword ptr [edi + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(20) /* 0x14 */);
    // 0051c00b  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051c00f  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c011  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c013  ff5614                 -call dword ptr [esi + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c016  01471c                 -add dword ptr [edi + 0x1c], eax
    (app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */)) += x86::reg32(x86::sreg32(cpu.eax));
L_0x0051c019:
    // 0051c019  8b471c                 -mov eax, dword ptr [edi + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */);
    // 0051c01c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c01e  7452                   -je 0x51c072
    if (cpu.flags.zf)
    {
        goto L_0x0051c072;
    }
    // 0051c020  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051c022  31ed                   -xor ebp, ebp
    cpu.ebp ^= x86::reg32(x86::sreg32(cpu.ebp));
L_0x0051c024:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c024  89e2                   -mov edx, esp
    cpu.edx = cpu.esp;
    // 0051c026  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0051c028  8b5f14                 -mov ebx, dword ptr [edi + 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(20) /* 0x14 */);
    // 0051c02b  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051c02d  01eb                   -add ebx, ebp
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ebp));
    // 0051c02f  e85cfeffff             -call 0x51be90
    cpu.esp -= 4;
    sub_51be90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c034  8b542408               -mov edx, dword ptr [esp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051c038  807a1f00               +cmp byte ptr [edx + 0x1f], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(31) /* 0x1f */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051c03c  7516                   -jne 0x51c054
    if (!cpu.flags.zf)
    {
        goto L_0x0051c054;
    }
    // 0051c03e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c040  7412                   -je 0x51c054
    if (cpu.flags.zf)
    {
        goto L_0x0051c054;
    }
    // 0051c042  29c6                   -sub esi, eax
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c044  01c5                   -add ebp, eax
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051c046  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c048  740a                   -je 0x51c054
    if (cpu.flags.zf)
    {
        goto L_0x0051c054;
    }
    // 0051c04a  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051c04e  83782400               +cmp dword ptr [eax + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c052  75d0                   -jne 0x51c024
    if (!cpu.flags.zf)
    {
        goto L_0x0051c024;
    }
L_0x0051c054:
    // 0051c054  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051c058  80781f00               +cmp byte ptr [eax + 0x1f], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(31) /* 0x1f */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051c05c  7514                   -jne 0x51c072
    if (!cpu.flags.zf)
    {
        goto L_0x0051c072;
    }
    // 0051c05e  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0051c060  7410                   -je 0x51c072
    if (cpu.flags.zf)
    {
        goto L_0x0051c072;
    }
    // 0051c062  8b5714                 -mov edx, dword ptr [edi + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(20) /* 0x14 */);
    // 0051c065  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0051c067  8d042a                 -lea eax, [edx + ebp]
    cpu.eax = x86::reg32(cpu.edx + cpu.ebp * 1);
    // 0051c06a  e881e4fcff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c06f  89771c                 -mov dword ptr [edi + 0x1c], esi
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */) = cpu.esi;
L_0x0051c072:
    // 0051c072  833c2400               +cmp dword ptr [esp], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c076  741b                   -je 0x51c093
    if (cpu.flags.zf)
    {
        goto L_0x0051c093;
    }
    // 0051c078  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051c07d  83c40c                 +add esp, 0xc
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
    // 0051c080  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c081  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c082  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c083  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c084  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c085  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c086  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051c087:
    // 0051c087  c7471c00000000         -mov dword ptr [edi + 0x1c], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */) = 0 /*0x0*/;
    // 0051c08e  e965ffffff             -jmp 0x51bff8
    goto L_0x0051bff8;
L_0x0051c093:
    // 0051c093  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c095  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0051c098  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c099  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c09a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c09b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c09c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c09d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c09e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_51c0a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051c0a0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051c0a1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051c0a2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051c0a3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051c0a4  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051c0a6  8b7810                 -mov edi, dword ptr [eax + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0051c0a9  8b15ec6d5600           -mov edx, dword ptr [0x566dec]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
    // 0051c0af  8b6f04                 -mov ebp, dword ptr [edi + 4]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0051c0b2  83fa05                 +cmp edx, 5
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
    // 0051c0b5  7d14                   -jge 0x51c0cb
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051c0cb;
    }
L_0x0051c0b7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c0b7  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c0bb  7409                   -je 0x51c0c6
    if (cpu.flags.zf)
    {
        goto L_0x0051c0c6;
    }
L_0x0051c0bd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c0bd  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051c0bf  ff5510                 -call dword ptr [ebp + 0x10]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c0c2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c0c4  7523                   -jne 0x51c0e9
    if (!cpu.flags.zf)
    {
        goto L_0x0051c0e9;
    }
L_0x0051c0c6:
    // 0051c0c6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c0c7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c0c8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c0c9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c0ca  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051c0cb:
    // 0051c0cb  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051c0cc  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051c0cd  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c0cf  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051c0d2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c0d3  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c0d5  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051c0d7  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051c0d9  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051c0db  b8040d5500             -mov eax, 0x550d04
    cpu.eax = 5573892 /*0x550d04*/;
    // 0051c0e0  e89b60ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c0e5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c0e6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c0e7  ebce                   -jmp 0x51c0b7
    goto L_0x0051c0b7;
L_0x0051c0e9:
    // 0051c0e9  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051c0eb  e8e0feffff             -call 0x51bfd0
    cpu.esp -= 4;
    sub_51bfd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c0f0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c0f2  75c9                   -jne 0x51c0bd
    if (!cpu.flags.zf)
    {
        goto L_0x0051c0bd;
    }
    // 0051c0f4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c0f5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c0f6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c0f7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c0f8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_51c100(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051c100  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051c101  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051c102  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051c103  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0051c106  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051c108  895c240c               -mov dword ptr [esp + 0xc], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.ebx;
    // 0051c10c  31ed                   -xor ebp, ebp
    cpu.ebp ^= x86::reg32(x86::sreg32(cpu.ebp));
    // 0051c10e  89542408               -mov dword ptr [esp + 8], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.edx;
    // 0051c112  896c2404               -mov dword ptr [esp + 4], ebp
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.ebp;
    // 0051c116  83fb06                 +cmp ebx, 6
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6 /*0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c119  0f8cce020000           -jl 0x51c3ed
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c3ed;
    }
    // 0051c11f  81fbfa000000           +cmp ebx, 0xfa
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(250 /*0xfa*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c125  0f8ecf020000           -jle 0x51c3fa
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051c3fa;
    }
    // 0051c12b  bb100d5500             -mov ebx, 0x550d10
    cpu.ebx = 5573904 /*0x550d10*/;
    // 0051c130  68fa000000             -push 0xfa
    app->getMemory<x86::reg32>(cpu.esp-4) = 250 /*0xfa*/;
    cpu.esp -= 4;
    // 0051c135  bf200d5500             -mov edi, 0x550d20
    cpu.edi = 5573920 /*0x550d20*/;
    // 0051c13a  b828010000             -mov eax, 0x128
    cpu.eax = 296 /*0x128*/;
    // 0051c13f  683c0d5500             -push 0x550d3c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5573948 /*0x550d3c*/;
    cpu.esp -= 4;
    // 0051c144  891d90215500           -mov dword ptr [0x552190], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebx;
    // 0051c14a  893d94215500           -mov dword ptr [0x552194], edi
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.edi;
    // 0051c150  a398215500             -mov dword ptr [0x552198], eax
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.eax;
    // 0051c155  e8b64eeeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c15a  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
L_0x0051c15d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c15d  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0051c15f  7433                   -je 0x51c194
    if (cpu.flags.zf)
    {
        goto L_0x0051c194;
    }
    // 0051c161  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c165  742d                   -je 0x51c194
    if (cpu.flags.zf)
    {
        goto L_0x0051c194;
    }
    // 0051c167  bb100d5500             -mov ebx, 0x550d10
    cpu.ebx = 5573904 /*0x550d10*/;
    // 0051c16c  bf200d5500             -mov edi, 0x550d20
    cpu.edi = 5573920 /*0x550d20*/;
    // 0051c171  b82d010000             -mov eax, 0x12d
    cpu.eax = 301 /*0x12d*/;
    // 0051c176  68cc0d5500             -push 0x550dcc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5574092 /*0x550dcc*/;
    cpu.esp -= 4;
    // 0051c17b  891d90215500           -mov dword ptr [0x552190], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebx;
    // 0051c181  893d94215500           -mov dword ptr [0x552194], edi
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.edi;
    // 0051c187  a398215500             -mov dword ptr [0x552198], eax
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.eax;
    // 0051c18c  e87f4eeeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c191  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x0051c194:
    // 0051c194  8b542408               -mov edx, dword ptr [esp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051c198  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c19a  8a421e                 -mov al, byte ptr [edx + 0x1e]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(30) /* 0x1e */);
    // 0051c19d  3b44240c               +cmp eax, dword ptr [esp + 0xc]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c1a1  7e04                   -jle 0x51c1a7
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051c1a7;
    }
    // 0051c1a3  8944240c               -mov dword ptr [esp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.eax;
L_0x0051c1a7:
    // 0051c1a7  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051c1ab  8a501f                 -mov dl, byte ptr [eax + 0x1f]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(31) /* 0x1f */);
    // 0051c1ae  b9fe000000             -mov ecx, 0xfe
    cpu.ecx = 254 /*0xfe*/;
    // 0051c1b3  84d2                   +test dl, dl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & cpu.dl));
    // 0051c1b5  7505                   -jne 0x51c1bc
    if (!cpu.flags.zf)
    {
        goto L_0x0051c1bc;
    }
    // 0051c1b7  b9fc010000             -mov ecx, 0x1fc
    cpu.ecx = 508 /*0x1fc*/;
L_0x0051c1bc:
    // 0051c1bc  8d4124                 -lea eax, [ecx + 0x24]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(36) /* 0x24 */);
    // 0051c1bf  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0051c1c1  0f841f020000           -je 0x51c3e6
    if (cpu.flags.zf)
    {
        goto L_0x0051c3e6;
    }
    // 0051c1c7  8b5e10                 -mov ebx, dword ptr [esi + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 0051c1ca  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051c1cc  0f857b020000           -jne 0x51c44d
    if (!cpu.flags.zf)
    {
        goto L_0x0051c44d;
    }
    // 0051c1d2  bf100d5500             -mov edi, 0x550d10
    cpu.edi = 5573904 /*0x550d10*/;
    // 0051c1d7  ba200d5500             -mov edx, 0x550d20
    cpu.edx = 5573920 /*0x550d20*/;
    // 0051c1dc  bb45010000             -mov ebx, 0x145
    cpu.ebx = 325 /*0x145*/;
    // 0051c1e1  891594215500           -mov dword ptr [0x552194], edx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.edx;
    // 0051c1e7  891d98215500           -mov dword ptr [0x552198], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebx;
    // 0051c1ed  8b1df4435600           -mov ebx, dword ptr [0x5643f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 0051c1f3  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051c1f5  b8040e5500             -mov eax, 0x550e04
    cpu.eax = 5574148 /*0x550e04*/;
    // 0051c1fa  893d90215500           -mov dword ptr [0x552190], edi
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edi;
    // 0051c200  e81b54fcff             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c205  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051c207  c744240401000000       -mov dword ptr [esp + 4], 1
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = 1 /*0x1*/;
L_0x0051c20f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c20f  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051c211  0f84c2010000           -je 0x51c3d9
    if (cpu.flags.zf)
    {
        goto L_0x0051c3d9;
    }
    // 0051c217  8d5724                 -lea edx, [edi + 0x24]
    cpu.edx = x86::reg32(cpu.edi + x86::reg32(36) /* 0x24 */);
    // 0051c21a  c70600000000           -mov dword ptr [esi], 0
    app->getMemory<x86::reg32>(cpu.esi) = 0 /*0x0*/;
    // 0051c220  c7462400000000         -mov dword ptr [esi + 0x24], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
    // 0051c227  8b4618                 -mov eax, dword ptr [esi + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0051c22a  897e10                 -mov dword ptr [esi + 0x10], edi
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */) = cpu.edi;
    // 0051c22d  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0051c230  c74618b0504f00         -mov dword ptr [esi + 0x18], 0x4f50b0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */) = 5198000 /*0x4f50b0*/;
    // 0051c237  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0051c23b  8937                   -mov dword ptr [edi], esi
    app->getMemory<x86::reg32>(cpu.edi) = cpu.esi;
    // 0051c23d  894708                 -mov dword ptr [edi + 8], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0051c240  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c242  668b462a               -mov ax, word ptr [esi + 0x2a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(42) /* 0x2a */);
    // 0051c246  894720                 -mov dword ptr [edi + 0x20], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */) = cpu.eax;
    // 0051c249  8a442404               -mov al, byte ptr [esp + 4]
    cpu.al = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051c24d  88470c                 -mov byte ptr [edi + 0xc], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(12) /* 0xc */) = cpu.al;
    // 0051c250  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051c254  894704                 -mov dword ptr [edi + 4], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 0051c257  a10cac5600             -mov eax, dword ptr [0x56ac0c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680140) /* 0x56ac0c */);
    // 0051c25c  6689470e               -mov word ptr [edi + 0xe], ax
    app->getMemory<x86::reg16>(cpu.edi + x86::reg32(14) /* 0xe */) = cpu.ax;
    // 0051c260  40                     -inc eax
    (cpu.eax)++;
    // 0051c261  c6470d00               -mov byte ptr [edi + 0xd], 0
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */) = 0 /*0x0*/;
    // 0051c265  a30cac5600             -mov dword ptr [0x56ac0c], eax
    app->getMemory<x86::reg32>(x86::reg32(5680140) /* 0x56ac0c */) = cpu.eax;
    // 0051c26a  e871f0fcff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c26f  c7471c00000000         -mov dword ptr [edi + 0x1c], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */) = 0 /*0x0*/;
    // 0051c276  894710                 -mov dword ptr [edi + 0x10], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 0051c279  895714                 -mov dword ptr [edi + 0x14], edx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(20) /* 0x14 */) = cpu.edx;
    // 0051c27c  8b15f06d5600           -mov edx, dword ptr [0x566df0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5664240) /* 0x566df0 */);
    // 0051c282  894f18                 -mov dword ptr [edi + 0x18], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(24) /* 0x18 */) = cpu.ecx;
    // 0051c285  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051c287  752d                   -jne 0x51c2b6
    if (!cpu.flags.zf)
    {
        goto L_0x0051c2b6;
    }
    // 0051c289  b9100d5500             -mov ecx, 0x550d10
    cpu.ecx = 5573904 /*0x550d10*/;
    // 0051c28e  bb200d5500             -mov ebx, 0x550d20
    cpu.ebx = 5573920 /*0x550d20*/;
    // 0051c293  b86d010000             -mov eax, 0x16d
    cpu.eax = 365 /*0x16d*/;
    // 0051c298  680c0e5500             -push 0x550e0c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5574156 /*0x550e0c*/;
    cpu.esp -= 4;
    // 0051c29d  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 0051c2a3  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 0051c2a9  a398215500             -mov dword ptr [0x552198], eax
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.eax;
    // 0051c2ae  e85d4deeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c2b3  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x0051c2b6:
    // 0051c2b6  8b4c2408               -mov ecx, dword ptr [esp + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051c2ba  baa0c05100             -mov edx, 0x51c0a0
    cpu.edx = 5357728 /*0x51c0a0*/;
    // 0051c2bf  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051c2c1  ff11                   -call dword ptr [ecx]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c2c3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c2c5  0f84c2010000           -je 0x51c48d
    if (cpu.flags.zf)
    {
        goto L_0x0051c48d;
    }
    // 0051c2cb  c7462400ca9a3b         -mov dword ptr [esi + 0x24], 0x3b9aca00
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 1000000000 /*0x3b9aca00*/;
    // 0051c2d2  b828269f00             -mov eax, 0x9f2628
    cpu.eax = 10429992 /*0x9f2628*/;
    // 0051c2d7  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051c2d9  e8721bffff             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c2de  833dec6d560001         +cmp dword ptr [0x566dec], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c2e5  0f8d69010000           -jge 0x51c454
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051c454;
    }
L_0x0051c2eb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c2eb  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c2ed  e83e36fcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c2f2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c2f4  0f8477010000           -je 0x51c471
    if (cpu.flags.zf)
    {
        goto L_0x0051c471;
    }
    // 0051c2fa  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051c2ff  e81cb4fcff             -call 0x4e7720
    cpu.esp -= 4;
    nfs3hp::port::waitSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c304  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c306  7c09                   -jl 0x51c311
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c311;
    }
L_0x0051c308:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c308  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051c30a  ff5614                 -call dword ptr [esi + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c30d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c30f  7405                   -je 0x51c316
    if (cpu.flags.zf)
    {
        goto L_0x0051c316;
    }
L_0x0051c311:
    // 0051c311  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0051c316:
    // 0051c316  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 0051c318  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c31a  7421                   -je 0x51c33d
    if (cpu.flags.zf)
    {
        goto L_0x0051c33d;
    }
    // 0051c31c  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c323  7c18                   -jl 0x51c33d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c33d;
    }
    // 0051c325  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c327  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051c32a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c32b  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c32d  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051c32f  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051c331  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051c333  b8400e5500             -mov eax, 0x550e40
    cpu.eax = 5574208 /*0x550e40*/;
    // 0051c338  e8435effff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c33d:
    // 0051c33d  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c341  7407                   -je 0x51c34a
    if (cpu.flags.zf)
    {
        goto L_0x0051c34a;
    }
    // 0051c343  c7462400ca9a3b         -mov dword ptr [esi + 0x24], 0x3b9aca00
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 1000000000 /*0x3b9aca00*/;
L_0x0051c34a:
    // 0051c34a  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c34e  740a                   -je 0x51c35a
    if (cpu.flags.zf)
    {
        goto L_0x0051c35a;
    }
    // 0051c350  807f0d00               +cmp byte ptr [edi + 0xd], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051c354  7504                   -jne 0x51c35a
    if (!cpu.flags.zf)
    {
        goto L_0x0051c35a;
    }
    // 0051c356  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0051c358  7491                   -je 0x51c2eb
    if (cpu.flags.zf)
    {
        goto L_0x0051c2eb;
    }
L_0x0051c35a:
    // 0051c35a  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c35e  0f851c010000           -jne 0x51c480
    if (!cpu.flags.zf)
    {
        goto L_0x0051c480;
    }
L_0x0051c364:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c364  8b5e24                 -mov ebx, dword ptr [esi + 0x24]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
    // 0051c367  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051c369  751e                   -jne 0x51c389
    if (!cpu.flags.zf)
    {
        goto L_0x0051c389;
    }
    // 0051c36b  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c372  7c15                   -jl 0x51c389
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c389;
    }
    // 0051c374  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c376  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051c379  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c37a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c37c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051c37d  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051c37f  b8500e5500             -mov eax, 0x550e50
    cpu.eax = 5574224 /*0x550e50*/;
    // 0051c384  e8f75dffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c389:
    // 0051c389  c7462400000000         -mov dword ptr [esi + 0x24], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
    // 0051c390  8b542408               -mov edx, dword ptr [esp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051c394  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051c396  ff5204                 -call dword ptr [edx + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c399:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c399  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c39d  0f8412010000           -je 0x51c4b5
    if (cpu.flags.zf)
    {
        goto L_0x0051c4b5;
    }
    // 0051c3a3  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0051c3a5  0f850a010000           -jne 0x51c4b5
    if (!cpu.flags.zf)
    {
        goto L_0x0051c4b5;
    }
    // 0051c3ab  a1f06d5600             -mov eax, dword ptr [0x566df0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664240) /* 0x566df0 */);
    // 0051c3b0  894624                 -mov dword ptr [esi + 0x24], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = cpu.eax;
    // 0051c3b3  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 0051c3b6  894618                 -mov dword ptr [esi + 0x18], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 0051c3b9  833dec6d560001         +cmp dword ptr [0x566dec], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c3c0  7c17                   -jl 0x51c3d9
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c3d9;
    }
    // 0051c3c2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c3c4  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051c3c7  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c3c8  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c3ca  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051c3cc  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051c3cd  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051c3cf  b8700e5500             -mov eax, 0x550e70
    cpu.eax = 5574256 /*0x550e70*/;
    // 0051c3d4  e8a75dffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c3d9:
    // 0051c3d9  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051c3db  0f8423010000           -je 0x51c504
    if (cpu.flags.zf)
    {
        goto L_0x0051c504;
    }
    // 0051c3e1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0051c3e6:
    // 0051c3e6  83c410                 +add esp, 0x10
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
    // 0051c3e9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c3ea  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c3eb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c3ec  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051c3ed:
    // 0051c3ed  c744240c06000000       -mov dword ptr [esp + 0xc], 6
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = 6 /*0x6*/;
    // 0051c3f5  e963fdffff             -jmp 0x51c15d
    goto L_0x0051c15d;
L_0x0051c3fa:
    // 0051c3fa  668b5a1c               -mov bx, word ptr [edx + 0x1c]
    cpu.bx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(28) /* 0x1c */);
    // 0051c3fe  6685db                 +test bx, bx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.bx & cpu.bx));
    // 0051c401  0f8456fdffff           -je 0x51c15d
    if (cpu.flags.zf)
    {
        goto L_0x0051c15d;
    }
    // 0051c407  8b54240c               -mov edx, dword ptr [esp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0051c40b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c40d  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051c410  6689d8                 -mov ax, bx
    cpu.ax = cpu.bx;
    // 0051c413  39c2                   +cmp edx, eax
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
    // 0051c415  0f8642fdffff           -jbe 0x51c15d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051c15d;
    }
    // 0051c41b  b8100d5500             -mov eax, 0x550d10
    cpu.eax = 5573904 /*0x550d10*/;
    // 0051c420  ba200d5500             -mov edx, 0x550d20
    cpu.edx = 5573920 /*0x550d20*/;
    // 0051c425  b92a010000             -mov ecx, 0x12a
    cpu.ecx = 298 /*0x12a*/;
    // 0051c42a  687c0d5500             -push 0x550d7c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5574012 /*0x550d7c*/;
    cpu.esp -= 4;
    // 0051c42f  a390215500             -mov dword ptr [0x552190], eax
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.eax;
    // 0051c434  891594215500           -mov dword ptr [0x552194], edx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.edx;
    // 0051c43a  890d98215500           -mov dword ptr [0x552198], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ecx;
    // 0051c440  e8cb4beeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c445  83c404                 +add esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051c448  e910fdffff             -jmp 0x51c15d
    goto L_0x0051c15d;
L_0x0051c44d:
    // 0051c44d  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0051c44f  e9bbfdffff             -jmp 0x51c20f
    goto L_0x0051c20f;
L_0x0051c454:
    // 0051c454  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c456  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051c459  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c45a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c45c  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051c45e  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051c460  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051c462  b8340e5500             -mov eax, 0x550e34
    cpu.eax = 5574196 /*0x550e34*/;
    // 0051c467  e8145dffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c46c  e97afeffff             -jmp 0x51c2eb
    goto L_0x0051c2eb;
L_0x0051c471:
    // 0051c471  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051c476  e86534fcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c47b  e988feffff             -jmp 0x51c308
    goto L_0x0051c308;
L_0x0051c480:
    // 0051c480  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0051c482  0f85dcfeffff           -jne 0x51c364
    if (!cpu.flags.zf)
    {
        goto L_0x0051c364;
    }
    // 0051c488  e90cffffff             -jmp 0x51c399
    goto L_0x0051c399;
L_0x0051c48d:
    // 0051c48d  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c494  0f8cfffeffff           -jl 0x51c399
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c399;
    }
    // 0051c49a  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051c49d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c49e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c4a0  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051c4a2  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051c4a4  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051c4a6  b8600e5500             -mov eax, 0x550e60
    cpu.eax = 5574240 /*0x550e60*/;
    // 0051c4ab  e8d05cffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c4b0  e9e4feffff             -jmp 0x51c399
    goto L_0x0051c399;
L_0x0051c4b5:
    // 0051c4b5  833dec6d560001         +cmp dword ptr [0x566dec], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c4bc  7c18                   -jl 0x51c4d6
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c4d6;
    }
    // 0051c4be  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c4c0  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051c4c3  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c4c4  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c4c6  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051c4c8  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051c4ca  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051c4cc  b8840e5500             -mov eax, 0x550e84
    cpu.eax = 5574276 /*0x550e84*/;
    // 0051c4d1  e8aa5cffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c4d6:
    // 0051c4d6  b828269f00             -mov eax, 0x9f2628
    cpu.eax = 10429992 /*0x9f2628*/;
    // 0051c4db  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051c4dd  e86e1cffff             -call 0x50e150
    cpu.esp -= 4;
    nfs3hp::ea::qremove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c4e2  b80c269f00             -mov eax, 0x9f260c
    cpu.eax = 10429964 /*0x9f260c*/;
    // 0051c4e7  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051c4e9  8b5c2404               -mov ebx, dword ptr [esp + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051c4ed  e85e1cffff             -call 0x50e150
    cpu.esp -= 4;
    nfs3hp::ea::qremove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c4f2  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051c4f4  740e                   -je 0x51c504
    if (cpu.flags.zf)
    {
        goto L_0x0051c504;
    }
    // 0051c4f6  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051c4f8  e89353fcff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c4fd  c7461000000000         -mov dword ptr [esi + 0x10], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */) = 0 /*0x0*/;
L_0x0051c504:
    // 0051c504  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c506  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0051c509  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c50a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c50b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c50c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_51c510(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051c510  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051c511  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051c512  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051c513  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051c514  81ec00010000           -sub esp, 0x100
    (cpu.esp) -= x86::reg32(x86::sreg32(256 /*0x100*/));
    // 0051c51a  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 0051c51c  89e7                   -mov edi, esp
    cpu.edi = cpu.esp;
    // 0051c51e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c520  7431                   -je 0x51c553
    if (cpu.flags.zf)
    {
        goto L_0x0051c553;
    }
    // 0051c522  83782400               +cmp dword ptr [eax + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c526  742b                   -je 0x51c553
    if (cpu.flags.zf)
    {
        goto L_0x0051c553;
    }
    // 0051c528  8b7010                 -mov esi, dword ptr [eax + 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0051c52b  3b4e08                 +cmp ecx, dword ptr [esi + 8]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c52e  7730                   -ja 0x51c560
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051c560;
    }
L_0x0051c530:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c530  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051c532  755b                   -jne 0x51c58f
    if (!cpu.flags.zf)
    {
        goto L_0x0051c58f;
    }
    // 0051c534  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c53b  7c16                   -jl 0x51c553
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c553;
    }
    // 0051c53d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c53f  8a460e                 -mov al, byte ptr [esi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(14) /* 0xe */);
    // 0051c542  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c543  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 0051c545  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051c547  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051c549  b8e40e5500             -mov eax, 0x550ee4
    cpu.eax = 5574372 /*0x550ee4*/;
    // 0051c54e  e82d5cffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c553:
    // 0051c553  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c555  81c400010000           -add esp, 0x100
    (cpu.esp) += x86::reg32(x86::sreg32(256 /*0x100*/));
    // 0051c55b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c55c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c55d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c55e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c55f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051c560:
    // 0051c560  b8100d5500             -mov eax, 0x550d10
    cpu.eax = 5573904 /*0x550d10*/;
    // 0051c565  bb980e5500             -mov ebx, 0x550e98
    cpu.ebx = 5574296 /*0x550e98*/;
    // 0051c56a  bdb1010000             -mov ebp, 0x1b1
    cpu.ebp = 433 /*0x1b1*/;
    // 0051c56f  68b00e5500             -push 0x550eb0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5574320 /*0x550eb0*/;
    cpu.esp -= 4;
    // 0051c574  a390215500             -mov dword ptr [0x552190], eax
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.eax;
    // 0051c579  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 0051c57f  892d98215500           -mov dword ptr [0x552198], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebp;
    // 0051c585  e8864aeeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c58a  83c404                 +add esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051c58d  eba1                   -jmp 0x51c530
    goto L_0x0051c530;
L_0x0051c58f:
    // 0051c58f  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 0051c591  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051c593  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051c595  e836f7ffff             -call 0x51bcd0
    cpu.esp -= 4;
    sub_51bcd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c59a  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051c59c  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051c59e  e84df7ffff             -call 0x51bcf0
    cpu.esp -= 4;
    sub_51bcf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c5a3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051c5a8  81c400010000           -add esp, 0x100
    (cpu.esp) += x86::reg32(x86::sreg32(256 /*0x100*/));
    // 0051c5ae  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c5af  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c5b0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c5b1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c5b2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_51c5c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051c5c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051c5c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051c5c2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051c5c3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051c5c4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051c5c5  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051c5c7  8b15ec6d5600           -mov edx, dword ptr [0x566dec]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
    // 0051c5cd  8b7810                 -mov edi, dword ptr [eax + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0051c5d0  83fa05                 +cmp edx, 5
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
    // 0051c5d3  7d6a                   -jge 0x51c63f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051c63f;
    }
L_0x0051c5d5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c5d5  ff4e24                 -dec dword ptr [esi + 0x24]
    (app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */))--;
    // 0051c5d8  8b5e24                 -mov ebx, dword ptr [esi + 0x24]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
    // 0051c5db  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051c5dd  7526                   -jne 0x51c605
    if (!cpu.flags.zf)
    {
        goto L_0x0051c605;
    }
    // 0051c5df  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c5e6  7c15                   -jl 0x51c5fd
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c5fd;
    }
    // 0051c5e8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c5ea  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051c5ed  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c5ee  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c5f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051c5f1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051c5f3  b8fc0e5500             -mov eax, 0x550efc
    cpu.eax = 5574396 /*0x550efc*/;
    // 0051c5f8  e8835bffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c5fd:
    // 0051c5fd  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051c5ff  ff561c                 -call dword ptr [esi + 0x1c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(28) /* 0x1c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c602  894624                 -mov dword ptr [esi + 0x24], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = cpu.eax;
L_0x0051c605:
    // 0051c605  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c609  7551                   -jne 0x51c65c
    if (!cpu.flags.zf)
    {
        goto L_0x0051c65c;
    }
L_0x0051c60b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c60b  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c612  7c19                   -jl 0x51c62d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c62d;
    }
    // 0051c614  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c616  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051c619  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c61a  8d5624                 -lea edx, [esi + 0x24]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(36) /* 0x24 */);
    // 0051c61d  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c61f  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051c621  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051c623  b8080f5500             -mov eax, 0x550f08
    cpu.eax = 5574408 /*0x550f08*/;
    // 0051c628  e8535bffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c62d:
    // 0051c62d  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051c62f  e88cf7ffff             -call 0x51bdc0
    cpu.esp -= 4;
    sub_51bdc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c634:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c634  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051c639  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c63a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c63b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c63c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c63d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c63e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051c63f:
    // 0051c63f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c641  8a470e                 -mov al, byte ptr [edi + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
    // 0051c644  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c645  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c647  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051c649  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051c64b  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051c64d  b8f00e5500             -mov eax, 0x550ef0
    cpu.eax = 5574384 /*0x550ef0*/;
    // 0051c652  e8295bffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c657  e979ffffff             -jmp 0x51c5d5
    goto L_0x0051c5d5;
L_0x0051c65c:
    // 0051c65c  8b5704                 -mov edx, dword ptr [edi + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0051c65f  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051c661  ff5208                 -call dword ptr [edx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c664  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c666  74a3                   -je 0x51c60b
    if (cpu.flags.zf)
    {
        goto L_0x0051c60b;
    }
    // 0051c668  8b7720                 -mov esi, dword ptr [edi + 0x20]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    // 0051c66b  8d46ff                 -lea eax, [esi - 1]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(-1) /* -0x1 */);
    // 0051c66e  894720                 -mov dword ptr [edi + 0x20], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */) = cpu.eax;
    // 0051c671  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0051c673  7fbf                   -jg 0x51c634
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0051c634;
    }
    // 0051c675  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051c677  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051c679  e8d2f6ffff             -call 0x51bd50
    cpu.esp -= 4;
    sub_51bd50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c67e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051c683  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c684  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c685  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c686  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c687  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c688  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_51c690(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051c690  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051c691  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051c692  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051c693  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051c694  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051c695  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051c697  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c699  0f84b7000000           -je 0x51c756
    if (cpu.flags.zf)
    {
        goto L_0x0051c756;
    }
    // 0051c69f  8b5010                 -mov edx, dword ptr [eax + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0051c6a2  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051c6a4  0f84ac000000           -je 0x51c756
    if (cpu.flags.zf)
    {
        goto L_0x0051c756;
    }
    // 0051c6aa  8b0dec6d5600           -mov ecx, dword ptr [0x566dec]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
    // 0051c6b0  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0051c6b2  8b6a04                 -mov ebp, dword ptr [edx + 4]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 0051c6b5  83f901                 +cmp ecx, 1
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
    // 0051c6b8  7d49                   -jge 0x51c703
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051c703;
    }
L_0x0051c6ba:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c6ba  b828269f00             -mov eax, 0x9f2628
    cpu.eax = 10429992 /*0x9f2628*/;
    // 0051c6bf  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051c6c1  e88a1affff             -call 0x50e150
    cpu.esp -= 4;
    nfs3hp::ea::qremove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c6c6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c6c8  0f8490000000           -je 0x51c75e
    if (cpu.flags.zf)
    {
        goto L_0x0051c75e;
    }
    // 0051c6ce  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c6d2  7466                   -je 0x51c73a
    if (cpu.flags.zf)
    {
        goto L_0x0051c73a;
    }
    // 0051c6d4  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 0051c6d9  c7462400000000         -mov dword ptr [esi + 0x24], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
L_0x0051c6e0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c6e0  49                     -dec ecx
    (cpu.ecx)--;
    // 0051c6e1  83f9ff                 +cmp ecx, -1
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c6e4  7445                   -je 0x51c72b
    if (cpu.flags.zf)
    {
        goto L_0x0051c72b;
    }
    // 0051c6e6  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051c6e8  ff5508                 -call dword ptr [ebp + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c6eb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c6ed  743c                   -je 0x51c72b
    if (cpu.flags.zf)
    {
        goto L_0x0051c72b;
    }
    // 0051c6ef  baff000000             -mov edx, 0xff
    cpu.edx = 255 /*0xff*/;
    // 0051c6f4  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051c6f6  e855f6ffff             -call 0x51bd50
    cpu.esp -= 4;
    sub_51bd50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c6fb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c6fd  7420                   -je 0x51c71f
    if (cpu.flags.zf)
    {
        goto L_0x0051c71f;
    }
    // 0051c6ff  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 0051c701  ebdd                   -jmp 0x51c6e0
    goto L_0x0051c6e0;
L_0x0051c703:
    // 0051c703  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051c704  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c706  8a420e                 -mov al, byte ptr [edx + 0xe]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(14) /* 0xe */);
    // 0051c709  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c70a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c70c  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051c70e  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051c710  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051c712  b8140f5500             -mov eax, 0x550f14
    cpu.eax = 5574420 /*0x550f14*/;
    // 0051c717  e8645affff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c71c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c71d  eb9b                   -jmp 0x51c6ba
    goto L_0x0051c6ba;
L_0x0051c71f:
    // 0051c71f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051c724  e8b73ffdff             -call 0x4f06e0
    cpu.esp -= 4;
    sub_4f06e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c729  ebb5                   -jmp 0x51c6e0
    goto L_0x0051c6e0;
L_0x0051c72b:
    // 0051c72b  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 0051c730  e8ab3ffdff             -call 0x4f06e0
    cpu.esp -= 4;
    sub_4f06e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c735  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051c737  ff5504                 -call dword ptr [ebp + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c73a:
    // 0051c73a  b80c269f00             -mov eax, 0x9f260c
    cpu.eax = 10429964 /*0x9f260c*/;
    // 0051c73f  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051c741  e80a17ffff             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c746:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c746  b80c269f00             -mov eax, 0x9f260c
    cpu.eax = 10429964 /*0x9f260c*/;
    // 0051c74b  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051c74d  e8fe19ffff             -call 0x50e150
    cpu.esp -= 4;
    nfs3hp::ea::qremove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c752  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051c754  751d                   -jne 0x51c773
    if (!cpu.flags.zf)
    {
        goto L_0x0051c773;
    }
L_0x0051c756:
    // 0051c756  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c758  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c759  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c75a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c75b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c75c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c75d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051c75e:
    // 0051c75e  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c762  74e2                   -je 0x51c746
    if (cpu.flags.zf)
    {
        goto L_0x0051c746;
    }
    // 0051c764  68240f5500             -push 0x550f24
    app->getMemory<x86::reg32>(cpu.esp-4) = 5574436 /*0x550f24*/;
    cpu.esp -= 4;
    // 0051c769  e88241fcff             -call 0x4e08f0
    cpu.esp -= 4;
    nfs3hp::ea::monoPrintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c76e  83c404                 +add esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051c771  ebd3                   -jmp 0x51c746
    goto L_0x0051c746;
L_0x0051c773:
    // 0051c773  8b4710                 -mov eax, dword ptr [edi + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */);
    // 0051c776  e8e5ebfcff             -call 0x4eb360
    cpu.esp -= 4;
    nfs3hp::ea::freeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c77b  807f0c00               +cmp byte ptr [edi + 0xc], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(12) /* 0xc */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051c77f  740e                   -je 0x51c78f
    if (cpu.flags.zf)
    {
        goto L_0x0051c78f;
    }
    // 0051c781  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051c783  e80851fcff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c788  c7461000000000         -mov dword ptr [esi + 0x10], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */) = 0 /*0x0*/;
L_0x0051c78f:
    // 0051c78f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051c794  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c795  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c796  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c797  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c798  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c799  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_51c7a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051c7a0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051c7a1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051c7a3  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051c7a5  f7d0                   -not eax
    cpu.eax = ~cpu.eax;
    // 0051c7a7  21c8                   -and eax, ecx
    cpu.eax &= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c7a9  09d0                   -or eax, edx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.edx));
    // 0051c7ab  39c8                   +cmp eax, ecx
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
    // 0051c7ad  7303                   -jae 0x51c7b2
    if (!cpu.flags.cf)
    {
        goto L_0x0051c7b2;
    }
    // 0051c7af  43                     -inc ebx
    (cpu.ebx)++;
    // 0051c7b0  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
L_0x0051c7b2:
    // 0051c7b2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c7b3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_51c7c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051c7c0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051c7c1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051c7c3  c6400500               -mov byte ptr [eax + 5], 0
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) = 0 /*0x0*/;
    // 0051c7c7  8a4005                 -mov al, byte ptr [eax + 5]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */);
    // 0051c7ca  884104                 -mov byte ptr [ecx + 4], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(4) /* 0x4 */) = cpu.al;
    // 0051c7cd  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051c7cf  e8ac7ffdff             -call 0x4f4780
    cpu.esp -= 4;
    sub_4f4780(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c7d4  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051c7d6  c1f808                 -sar eax, 8
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (8 /*0x8*/ % 32));
    // 0051c7d9  885105                 -mov byte ptr [ecx + 5], dl
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(5) /* 0x5 */) = cpu.dl;
    // 0051c7dc  884104                 -mov byte ptr [ecx + 4], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(4) /* 0x4 */) = cpu.al;
    // 0051c7df  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c7e0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_51c7f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051c7f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051c7f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051c7f2  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051c7f4  8a5804                 -mov bl, byte ptr [eax + 4]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0051c7f7  8a7805                 -mov bh, byte ptr [eax + 5]
    cpu.bh = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */);
    // 0051c7fa  e8c1ffffff             -call 0x51c7c0
    cpu.esp -= 4;
    sub_51c7c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c7ff  663b5904               +cmp bx, word ptr [ecx + 4]
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(4) /* 0x4 */)));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0051c803  7508                   -jne 0x51c80d
    if (!cpu.flags.zf)
    {
        goto L_0x0051c80d;
    }
    // 0051c805  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051c80a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c80b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c80c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051c80d:
    // 0051c80d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c80f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c810  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c811  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_51c820(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051c820  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051c821  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051c822  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051c825  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051c827  83c078                 -add eax, 0x78
    (cpu.eax) += x86::reg32(x86::sreg32(120 /*0x78*/));
    // 0051c82a  e8a11affff             -call 0x50e2d0
    cpu.esp -= 4;
    sub_50e2d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c82f  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0051c832  83f8ff                 +cmp eax, -1
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
    // 0051c835  7428                   -je 0x51c85f
    if (cpu.flags.zf)
    {
        goto L_0x0051c85f;
    }
L_0x0051c837:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c837  48                     -dec eax
    (cpu.eax)--;
    // 0051c838  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0051c83b  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 0051c83d  668b402a               -mov ax, word ptr [eax + 0x2a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(42) /* 0x2a */);
    // 0051c841  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 0051c846  8b0dec6d5600           -mov ecx, dword ptr [0x566dec]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
    // 0051c84c  89422c                 -mov dword ptr [edx + 0x2c], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(44) /* 0x2c */) = cpu.eax;
    // 0051c84f  83f903                 +cmp ecx, 3
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
    // 0051c852  7d10                   -jge 0x51c864
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051c864;
    }
    // 0051c854  8a0424                 -mov al, byte ptr [esp]
    cpu.al = app->getMemory<x86::reg8>(cpu.esp);
    // 0051c857  247f                   -and al, 0x7f
    cpu.al &= x86::reg8(x86::sreg8(127 /*0x7f*/));
    // 0051c859  83c404                 +add esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051c85c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c85d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c85e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051c85f:
    // 0051c85f  8b4274                 -mov eax, dword ptr [edx + 0x74]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(116) /* 0x74 */);
    // 0051c862  ebd3                   -jmp 0x51c837
    goto L_0x0051c837;
L_0x0051c864:
    // 0051c864  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051c865  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c867  8a420d                 -mov al, byte ptr [edx + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(13) /* 0xd */);
    // 0051c86a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c86b  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c86d  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051c86f  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051c871  8d54240c               -lea edx, [esp + 0xc]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0051c875  b8680f5500             -mov eax, 0x550f68
    cpu.eax = 5574504 /*0x550f68*/;
    // 0051c87a  e80159ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c87f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c880  8a0424                 -mov al, byte ptr [esp]
    cpu.al = app->getMemory<x86::reg8>(cpu.esp);
    // 0051c883  247f                   -and al, 0x7f
    cpu.al &= x86::reg8(x86::sreg8(127 /*0x7f*/));
    // 0051c885  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051c888  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c889  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c88a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_51c890(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051c890  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051c891  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051c892  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051c893  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051c896  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051c898  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0051c89a  83c012                 -add eax, 0x12
    (cpu.eax) += x86::reg32(x86::sreg32(18 /*0x12*/));
    // 0051c89d  8d5a54                 -lea ebx, [edx + 0x54]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(84) /* 0x54 */);
    // 0051c8a0  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0051c8a3  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051c8a5  e87621ffff             -call 0x50ea20
    cpu.esp -= 4;
    sub_50ea20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c8aa  8b6a30                 -mov ebp, dword ptr [edx + 0x30]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(48) /* 0x30 */);
    // 0051c8ad  896c2404               -mov dword ptr [esp + 4], ebp
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.ebp;
    // 0051c8b1  45                     -inc ebp
    (cpu.ebp)++;
    // 0051c8b2  896a30                 -mov dword ptr [edx + 0x30], ebp
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(48) /* 0x30 */) = cpu.ebp;
    // 0051c8b5  8b542404               -mov edx, dword ptr [esp + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051c8b9  895604                 -mov dword ptr [esi + 4], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 0051c8bc  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051c8be  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051c8c0  e87b21ffff             -call 0x50ea40
    cpu.esp -= 4;
    sub_50ea40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c8c5  c74608ffffffff         -mov dword ptr [esi + 8], 0xffffffff
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = 4294967295 /*0xffffffff*/;
    // 0051c8cc  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051c8cf  8a4604                 -mov al, byte ptr [esi + 4]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0051c8d2  c6460cfe               -mov byte ptr [esi + 0xc], 0xfe
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(12) /* 0xc */) = 254 /*0xfe*/;
    // 0051c8d6  247f                   -and al, 0x7f
    cpu.al &= x86::reg8(x86::sreg8(127 /*0x7f*/));
    // 0051c8d8  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0051c8da  88460e                 -mov byte ptr [esi + 0xe], al
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(14) /* 0xe */) = cpu.al;
    // 0051c8dd  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051c8df  884e0d                 -mov byte ptr [esi + 0xd], cl
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */) = cpu.cl;
    // 0051c8e2  e809dcfcff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c8e7  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051c8ea  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c8eb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c8ec  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c8ed  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51c8f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051c8f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051c8f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051c8f2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051c8f3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051c8f4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051c8f5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0051c8f7  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0051c8fa  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051c8fc  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0051c8fe  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051c900  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 0051c902  8955f0                 -mov dword ptr [ebp - 0x10], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edx;
    // 0051c905  8b5104                 -mov edx, dword ptr [ecx + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0051c908  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0051c90b  ff520c                 -call dword ptr [edx + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c90e  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051c910  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0051c912  8a560d                 -mov dl, byte ptr [esi + 0xd]
    cpu.dl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051c915  83c304                 -add ebx, 4
    (cpu.ebx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051c918  83c206                 -add edx, 6
    (cpu.edx) += x86::reg32(x86::sreg32(6 /*0x6*/));
    // 0051c91b  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 0051c91e  39d0                   +cmp eax, edx
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
    // 0051c920  734d                   -jae 0x51c96f
    if (!cpu.flags.cf)
    {
        goto L_0x0051c96f;
    }
    // 0051c922  837e0800               +cmp dword ptr [esi + 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c926  0f8cec000000           -jl 0x51ca18
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051ca18;
    }
    // 0051c92c  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c933  7c19                   -jl 0x51c94e
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c94e;
    }
    // 0051c935  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c937  8a410d                 -mov al, byte ptr [ecx + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(13) /* 0xd */);
    // 0051c93a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c93b  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0051c93e  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051c940  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051c942  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c944  b8980f5500             -mov eax, 0x550f98
    cpu.eax = 5574552 /*0x550f98*/;
L_0x0051c949:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c949  e83258ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c94e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c94e  837df000               +cmp dword ptr [ebp - 0x10], 0
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
    // 0051c952  0f84e6000000           -je 0x51ca3e
    if (cpu.flags.zf)
    {
        goto L_0x0051ca3e;
    }
L_0x0051c958:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c958  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0051c95b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c95d  668b422e               -mov ax, word ptr [edx + 0x2e]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(46) /* 0x2e */);
    // 0051c961  894608                 -mov dword ptr [esi + 8], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0051c964  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0051c967  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0051c969  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c96a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c96b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c96c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c96d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051c96e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051c96f:
    // 0051c96f  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051c971  e8aafeffff             -call 0x51c820
    cpu.esp -= 4;
    sub_51c820(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c976  88c2                   -mov dl, al
    cpu.dl = cpu.al;
    // 0051c978  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c97a  8a460d                 -mov al, byte ptr [esi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051c97d  83c006                 -add eax, 6
    (cpu.eax) += x86::reg32(x86::sreg32(6 /*0x6*/));
    // 0051c980  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0051c983  88560f                 -mov byte ptr [esi + 0xf], dl
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(15) /* 0xf */) = cpu.dl;
    // 0051c986  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051c988  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051c98a  8a560d                 -mov dl, byte ptr [esi + 0xd]
    cpu.dl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051c98d  83c00c                 -add eax, 0xc
    (cpu.eax) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0051c990  83c206                 -add edx, 6
    (cpu.edx) += x86::reg32(x86::sreg32(6 /*0x6*/));
    // 0051c993  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0051c996  e825feffff             -call 0x51c7c0
    cpu.esp -= 4;
    sub_51c7c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c99b  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0051c99e  8b7904                 -mov edi, dword ptr [ecx + 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0051c9a1  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0051c9a4  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0051c9a7  ff5718                 -call dword ptr [edi + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051c9aa  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0051c9ad  39d0                   +cmp eax, edx
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
    // 0051c9af  7540                   -jne 0x51c9f1
    if (!cpu.flags.zf)
    {
        goto L_0x0051c9f1;
    }
    // 0051c9b1  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c9b8  7c24                   -jl 0x51c9de
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c9de;
    }
    // 0051c9ba  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051c9bc  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0051c9bf  8a410d                 -mov al, byte ptr [ecx + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(13) /* 0xd */);
    // 0051c9c2  8d5e12                 -lea ebx, [esi + 0x12]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(18) /* 0x12 */);
    // 0051c9c5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051c9c6  8b7e08                 -mov edi, dword ptr [esi + 8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 0051c9c9  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051c9cb  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051c9cd  8a4e0d                 -mov cl, byte ptr [esi + 0xd]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051c9d0  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051c9d2  7c16                   -jl 0x51c9ea
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c9ea;
    }
    // 0051c9d4  b8780f5500             -mov eax, 0x550f78
    cpu.eax = 5574520 /*0x550f78*/;
L_0x0051c9d9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051c9d9  e8a257ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051c9de:
    // 0051c9de  c745f001000000         -mov dword ptr [ebp - 0x10], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = 1 /*0x1*/;
    // 0051c9e5  e96effffff             -jmp 0x51c958
    goto L_0x0051c958;
L_0x0051c9ea:
    // 0051c9ea  b8700f5500             -mov eax, 0x550f70
    cpu.eax = 5574512 /*0x550f70*/;
    // 0051c9ef  ebe8                   -jmp 0x51c9d9
    goto L_0x0051c9d9;
L_0x0051c9f1:
    // 0051c9f1  833dec6d560001         +cmp dword ptr [0x566dec], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051c9f8  0f8c50ffffff           -jl 0x51c94e
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c94e;
    }
    // 0051c9fe  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051ca00  8a410d                 -mov al, byte ptr [ecx + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(13) /* 0xd */);
    // 0051ca03  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ca04  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0051ca07  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0051ca09  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051ca0b  8d55ec                 -lea edx, [ebp - 0x14]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0051ca0e  b8800f5500             -mov eax, 0x550f80
    cpu.eax = 5574528 /*0x550f80*/;
    // 0051ca13  e931ffffff             -jmp 0x51c949
    goto L_0x0051c949;
L_0x0051ca18:
    // 0051ca18  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ca1f  0f8c29ffffff           -jl 0x51c94e
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051c94e;
    }
    // 0051ca25  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051ca27  8a410d                 -mov al, byte ptr [ecx + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(13) /* 0xd */);
    // 0051ca2a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ca2b  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0051ca2e  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051ca30  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051ca32  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 0051ca34  b8900f5500             -mov eax, 0x550f90
    cpu.eax = 5574544 /*0x550f90*/;
    // 0051ca39  e90bffffff             -jmp 0x51c949
    goto L_0x0051c949;
L_0x0051ca3e:
    // 0051ca3e  c7460801000000         -mov dword ptr [esi + 8], 1
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = 1 /*0x1*/;
    // 0051ca45  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0051ca48  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0051ca4a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ca4b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ca4c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ca4d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ca4e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ca4f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_51ca50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051ca50  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051ca51  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051ca52  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051ca53  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051ca54  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051ca57  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051ca59  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0051ca5b  891c24                 -mov dword ptr [esp], ebx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ebx;
    // 0051ca5e  8b3e                   -mov edi, dword ptr [esi]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi);
    // 0051ca60  c7400801000000         -mov dword ptr [eax + 8], 1
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = 1 /*0x1*/;
    // 0051ca67  8b6e04                 -mov ebp, dword ptr [esi + 4]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0051ca6a  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051ca6c  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051ca6e  ff550c                 -call dword ptr [ebp + 0xc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ca71  83f806                 +cmp eax, 6
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
    // 0051ca74  730a                   -jae 0x51ca80
    if (!cpu.flags.cf)
    {
        goto L_0x0051ca80;
    }
L_0x0051ca76:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ca76  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051ca78  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051ca7b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ca7c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ca7d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ca7e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ca7f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051ca80:
    // 0051ca80  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051ca82  e899fdffff             -call 0x51c820
    cpu.esp -= 4;
    sub_51c820(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ca87  ba06000000             -mov edx, 6
    cpu.edx = 6 /*0x6*/;
    // 0051ca8c  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0051ca91  c6410cfe               -mov byte ptr [ecx + 0xc], 0xfe
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(12) /* 0xc */) = 254 /*0xfe*/;
    // 0051ca95  8a2424                 -mov ah, byte ptr [esp]
    cpu.ah = app->getMemory<x86::reg8>(cpu.esp);
    // 0051ca98  bb06000000             -mov ebx, 6
    cpu.ebx = 6 /*0x6*/;
    // 0051ca9d  88610e                 -mov byte ptr [ecx + 0xe], ah
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(14) /* 0xe */) = cpu.ah;
    // 0051caa0  8d690c                 -lea ebp, [ecx + 0xc]
    cpu.ebp = x86::reg32(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 0051caa3  88410f                 -mov byte ptr [ecx + 0xf], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(15) /* 0xf */) = cpu.al;
    // 0051caa6  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0051caa8  c6410d00               -mov byte ptr [ecx + 0xd], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(13) /* 0xd */) = 0 /*0x0*/;
    // 0051caac  e80ffdffff             -call 0x51c7c0
    cpu.esp -= 4;
    sub_51c7c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cab1  8b7604                 -mov esi, dword ptr [esi + 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0051cab4  89ea                   -mov edx, ebp
    cpu.edx = cpu.ebp;
    // 0051cab6  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051cab8  ff5618                 -call dword ptr [esi + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cabb  83f806                 +cmp eax, 6
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
    // 0051cabe  7524                   -jne 0x51cae4
    if (!cpu.flags.zf)
    {
        goto L_0x0051cae4;
    }
    // 0051cac0  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
L_0x0051cac5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051cac5  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051cac7  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0051cac9  74ab                   -je 0x51ca76
    if (cpu.flags.zf)
    {
        goto L_0x0051ca76;
    }
    // 0051cacb  833c247f               +cmp dword ptr [esp], 0x7f
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(127 /*0x7f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cacf  77a5                   -ja 0x51ca76
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051ca76;
    }
    // 0051cad1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cad3  668b472c               -mov ax, word ptr [edi + 0x2c]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edi + x86::reg32(44) /* 0x2c */);
    // 0051cad7  894108                 -mov dword ptr [ecx + 8], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0051cada  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051cadc  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051cadf  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cae0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cae1  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cae2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cae3  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051cae4:
    // 0051cae4  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 0051cae6  ebdd                   -jmp 0x51cac5
    goto L_0x0051cac5;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_51caf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051caf0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051caf1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051caf2  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051caf4  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0051caf6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051caf8  740d                   -je 0x51cb07
    if (cpu.flags.zf)
    {
        goto L_0x0051cb07;
    }
    // 0051cafa  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 0051cafd  8d50ff                 -lea edx, [eax - 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(-1) /* -0x1 */);
    // 0051cb00  895608                 -mov dword ptr [esi + 8], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = cpu.edx;
    // 0051cb03  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051cb05  7e08                   -jle 0x51cb0f
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051cb0f;
    }
L_0x0051cb07:
    // 0051cb07  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051cb0c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cb0d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cb0e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051cb0f:
    // 0051cb0f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051cb10  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cb17  7c1b                   -jl 0x51cb34
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051cb34;
    }
    // 0051cb19  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051cb1a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cb1c  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051cb1f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051cb20  8d5604                 -lea edx, [esi + 4]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0051cb23  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051cb25  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051cb27  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051cb29  b8a00f5500             -mov eax, 0x550fa0
    cpu.eax = 5574560 /*0x550fa0*/;
    // 0051cb2e  e84d56ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cb33  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0051cb34:
    // 0051cb34  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0051cb36  8b5e04                 -mov ebx, dword ptr [esi + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0051cb39  8b4718                 -mov eax, dword ptr [edi + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(24) /* 0x18 */);
    // 0051cb3c  83e37f                 -and ebx, 0x7f
    cpu.ebx &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 0051cb3f  e80cffffff             -call 0x51ca50
    cpu.esp -= 4;
    sub_51ca50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cb44  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cb45  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cb46  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cb47  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_51cb50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051cb50  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051cb51  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051cb52  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051cb53  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051cb54  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051cb57  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051cb59  bb7f000000             -mov ebx, 0x7f
    cpu.ebx = 127 /*0x7f*/;
    // 0051cb5e  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0051cb64  8b4034                 -mov eax, dword ptr [eax + 0x34]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(52) /* 0x34 */);
    // 0051cb67  e834fcffff             -call 0x51c7a0
    cpu.esp -= 4;
    sub_51c7a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cb6c  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0051cb6f  3b4630                 +cmp eax, dword ptr [esi + 0x30]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(48) /* 0x30 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cb72  7349                   -jae 0x51cbbd
    if (!cpu.flags.cf)
    {
        goto L_0x0051cbbd;
    }
    // 0051cb74  3b4634                 +cmp eax, dword ptr [esi + 0x34]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(52) /* 0x34 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cb77  7644                   -jbe 0x51cbbd
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051cbbd;
    }
    // 0051cb79  8b3dec6d5600           -mov edi, dword ptr [0x566dec]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
    // 0051cb7f  894634                 -mov dword ptr [esi + 0x34], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(52) /* 0x34 */) = cpu.eax;
    // 0051cb82  83ff03                 +cmp edi, 3
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
    // 0051cb85  7d1a                   -jge 0x51cba1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051cba1;
    }
L_0x0051cb87:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051cb87  8d5e38                 -lea ebx, [esi + 0x38]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(56) /* 0x38 */);
    // 0051cb8a  8d4e54                 -lea ecx, [esi + 0x54]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(84) /* 0x54 */);
L_0x0051cb8d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051cb8d  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051cb8f  e83c17ffff             -call 0x50e2d0
    cpu.esp -= 4;
    sub_50e2d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cb94  3b4634                 +cmp eax, dword ptr [esi + 0x34]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(52) /* 0x34 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cb97  7649                   -jbe 0x51cbe2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051cbe2;
    }
L_0x0051cb99:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051cb99  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051cb9c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cb9d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cb9e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cb9f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cba0  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051cba1:
    // 0051cba1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cba3  8a460d                 -mov al, byte ptr [esi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051cba6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051cba7  8d542404               -lea edx, [esp + 4]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051cbab  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051cbad  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051cbaf  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0051cbb1  b8a80f5500             -mov eax, 0x550fa8
    cpu.eax = 5574568 /*0x550fa8*/;
    // 0051cbb6  e8c555ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cbbb  ebca                   -jmp 0x51cb87
    goto L_0x0051cb87;
L_0x0051cbbd:
    // 0051cbbd  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cbc4  7cc1                   -jl 0x51cb87
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051cb87;
    }
    // 0051cbc6  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cbc8  8a460d                 -mov al, byte ptr [esi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051cbcb  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051cbcc  8d542404               -lea edx, [esp + 4]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051cbd0  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051cbd2  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051cbd4  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0051cbd6  b8b00f5500             -mov eax, 0x550fb0
    cpu.eax = 5574576 /*0x550fb0*/;
    // 0051cbdb  e8a055ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cbe0  eba5                   -jmp 0x51cb87
    goto L_0x0051cb87;
L_0x0051cbe2:
    // 0051cbe2  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051cbe4  e87713ffff             -call 0x50df60
    cpu.esp -= 4;
    sub_50df60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cbe9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051cbeb  74ac                   -je 0x51cb99
    if (cpu.flags.zf)
    {
        goto L_0x0051cb99;
    }
    // 0051cbed  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051cbef  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051cbf1  e85a12ffff             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cbf6  eb95                   -jmp 0x51cb8d
    goto L_0x0051cb8d;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_51cc00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051cc00  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051cc01  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051cc02  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051cc03  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051cc04  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051cc07  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051cc09  8d4854                 -lea ecx, [eax + 0x54]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(84) /* 0x54 */);
    // 0051cc0c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051cc0e  e8bd16ffff             -call 0x50e2d0
    cpu.esp -= 4;
    sub_50e2d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cc13  bb7f000000             -mov ebx, 0x7f
    cpu.ebx = 127 /*0x7f*/;
    // 0051cc18  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0051cc1e  e87dfbffff             -call 0x51c7a0
    cpu.esp -= 4;
    sub_51c7a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cc23  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0051cc26  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051cc28  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0051cc2a  e8d118ffff             -call 0x50e500
    cpu.esp -= 4;
    sub_50e500(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cc2f  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051cc31  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051cc33  7410                   -je 0x51cc45
    if (cpu.flags.zf)
    {
        goto L_0x0051cc45;
    }
    // 0051cc35  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cc3c  7d0f                   -jge 0x51cc4d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051cc4d;
    }
    // 0051cc3e  c7470800000000         -mov dword ptr [edi + 8], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
L_0x0051cc45:
    // 0051cc45  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051cc48  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cc49  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cc4a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cc4b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cc4c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051cc4d:
    // 0051cc4d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cc4f  8a460d                 -mov al, byte ptr [esi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051cc52  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051cc53  8d542404               -lea edx, [esp + 4]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051cc57  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051cc59  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051cc5b  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051cc5d  b8b80f5500             -mov eax, 0x550fb8
    cpu.eax = 5574584 /*0x550fb8*/;
    // 0051cc62  e81955ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cc67  c7470800000000         -mov dword ptr [edi + 8], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 0051cc6e  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051cc71  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cc72  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cc73  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cc74  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cc75  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_51cc80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051cc80  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051cc81  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051cc82  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051cc83  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051cc84  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051cc85  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051cc87  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cc8e  7c18                   -jl 0x51cca8
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051cca8;
    }
    // 0051cc90  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cc92  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051cc95  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051cc96  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051cc98  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051cc9a  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051cc9c  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051cc9e  b8c00f5500             -mov eax, 0x550fc0
    cpu.eax = 5574592 /*0x550fc0*/;
    // 0051cca3  e8d854ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051cca8:
    // 0051cca8  b828269f00             -mov eax, 0x9f2628
    cpu.eax = 10429992 /*0x9f2628*/;
    // 0051ccad  8b17                   -mov edx, dword ptr [edi]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi);
    // 0051ccaf  e89c14ffff             -call 0x50e150
    cpu.esp -= 4;
    nfs3hp::ea::qremove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ccb4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ccb6  746e                   -je 0x51cd26
    if (cpu.flags.zf)
    {
        goto L_0x0051cd26;
    }
    // 0051ccb8  8b1dec6d5600           -mov ebx, dword ptr [0x566dec]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
    // 0051ccbe  8b37                   -mov esi, dword ptr [edi]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edi);
    // 0051ccc0  83fb01                 +cmp ebx, 1
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
    // 0051ccc3  7c18                   -jl 0x51ccdd
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051ccdd;
    }
    // 0051ccc5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051ccc7  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051ccca  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051cccb  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051cccd  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051cccf  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051ccd1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051ccd3  b8d00f5500             -mov eax, 0x550fd0
    cpu.eax = 5574608 /*0x550fd0*/;
    // 0051ccd8  e8a354ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051ccdd:
    // 0051ccdd  c7462400000000         -mov dword ptr [esi + 0x24], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
    // 0051cce4  8b5704                 -mov edx, dword ptr [edi + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0051cce7  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051cce9  ff5204                 -call dword ptr [edx + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ccec  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051ccee  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051ccf0  ff5618                 -call dword ptr [esi + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ccf3  b80c269f00             -mov eax, 0x9f260c
    cpu.eax = 10429964 /*0x9f260c*/;
    // 0051ccf8  e85311ffff             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ccfd  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cd04  7d06                   -jge 0x51cd0c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051cd0c;
    }
L_0x0051cd06:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051cd06  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cd07  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cd08  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cd09  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cd0a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cd0b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051cd0c:
    // 0051cd0c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cd0e  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051cd11  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051cd12  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051cd14  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051cd16  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051cd18  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051cd1a  b8dc0f5500             -mov eax, 0x550fdc
    cpu.eax = 5574620 /*0x550fdc*/;
    // 0051cd1f  e85c54ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cd24  ebe0                   -jmp 0x51cd06
    goto L_0x0051cd06;
L_0x0051cd26:
    // 0051cd26  833dec6d560001         +cmp dword ptr [0x566dec], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cd2d  7cd7                   -jl 0x51cd06
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051cd06;
    }
    // 0051cd2f  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051cd32  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051cd33  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051cd35  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051cd37  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051cd39  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051cd3b  b8e80f5500             -mov eax, 0x550fe8
    cpu.eax = 5574632 /*0x550fe8*/;
    // 0051cd40  e83b54ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cd45  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cd46  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cd47  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cd48  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cd49  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cd4a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_51cd50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051cd50  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051cd51  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051cd52  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051cd53  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0051cd56  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051cd58  89542404               -mov dword ptr [esp + 4], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 0051cd5c  89dd                   -mov ebp, ebx
    cpu.ebp = cpu.ebx;
    // 0051cd5e  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0051cd60  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 0051cd62  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0051cd66  8b4704                 -mov eax, dword ptr [edi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0051cd69  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0051cd6b  8a501f                 -mov dl, byte ptr [eax + 0x1f]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(31) /* 0x1f */);
    // 0051cd6e  893424                 -mov dword ptr [esp], esi
    app->getMemory<x86::reg32>(cpu.esp) = cpu.esi;
    // 0051cd71  84d2                   +test dl, dl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & cpu.dl));
    // 0051cd73  7512                   -jne 0x51cd87
    if (!cpu.flags.zf)
    {
        goto L_0x0051cd87;
    }
    // 0051cd75  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0051cd77  8d1429                 -lea edx, [ecx + ebp]
    cpu.edx = x86::reg32(cpu.ecx + cpu.ebp * 1);
L_0x0051cd7a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051cd7a  39d0                   +cmp eax, edx
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
    // 0051cd7c  7309                   -jae 0x51cd87
    if (!cpu.flags.cf)
    {
        goto L_0x0051cd87;
    }
    // 0051cd7e  8038fe                 +cmp byte ptr [eax], 0xfe
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(254 /*0xfe*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051cd81  7404                   -je 0x51cd87
    if (cpu.flags.zf)
    {
        goto L_0x0051cd87;
    }
    // 0051cd83  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0051cd84  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0051cd85  ebf3                   -jmp 0x51cd7a
    goto L_0x0051cd7a;
L_0x0051cd87:
    // 0051cd87  8d042e                 -lea eax, [esi + ebp]
    cpu.eax = x86::reg32(cpu.esi + cpu.ebp * 1);
    // 0051cd8a  8038fe                 +cmp byte ptr [eax], 0xfe
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(254 /*0xfe*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051cd8d  7529                   -jne 0x51cdb8
    if (!cpu.flags.zf)
    {
        goto L_0x0051cdb8;
    }
    // 0051cd8f  8d4e06                 -lea ecx, [esi + 6]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 0051cd92  39d9                   +cmp ecx, ebx
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
    // 0051cd94  7626                   -jbe 0x51cdbc
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051cdbc;
    }
L_0x0051cd96:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051cd96  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051cd99  83c728                 -add edi, 0x28
    (cpu.edi) += x86::reg32(x86::sreg32(40 /*0x28*/));
    // 0051cd9c  39d6                   +cmp esi, edx
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
    // 0051cd9e  0f8677010000           -jbe 0x51cf1b
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051cf1b;
    }
    // 0051cda4  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051cda9  8b0f                   -mov ecx, dword ptr [edi]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edi);
    // 0051cdab  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051cdad  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051cdaf  890f                   -mov dword ptr [edi], ecx
    app->getMemory<x86::reg32>(cpu.edi) = cpu.ecx;
L_0x0051cdb1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051cdb1  83c410                 +add esp, 0x10
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
    // 0051cdb4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cdb5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cdb6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cdb7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051cdb8:
    // 0051cdb8  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051cdba  ebf5                   -jmp 0x51cdb1
    goto L_0x0051cdb1;
L_0x0051cdbc:
    // 0051cdbc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051cdbe  8a5001                 -mov dl, byte ptr [eax + 1]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0051cdc1  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 0051cdc3  3b5708                 +cmp edx, dword ptr [edi + 8]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8) /* 0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cdc6  0f8733010000           -ja 0x51ceff
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051ceff;
    }
    // 0051cdcc  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0051cdce  39d9                   +cmp ecx, ebx
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
    // 0051cdd0  77c4                   -ja 0x51cd96
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051cd96;
    }
    // 0051cdd2  29f1                   -sub ecx, esi
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 0051cdd4  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0051cdd6  e815faffff             -call 0x51c7f0
    cpu.esp -= 4;
    sub_51c7f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cddb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051cddd  0f8414010000           -je 0x51cef7
    if (cpu.flags.zf)
    {
        goto L_0x0051cef7;
    }
    // 0051cde3  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051cde7  83782400               +cmp dword ptr [eax + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cdeb  740c                   -je 0x51cdf9
    if (cpu.flags.zf)
    {
        goto L_0x0051cdf9;
    }
    // 0051cded  8b542408               -mov edx, dword ptr [esp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051cdf1  a1f06d5600             -mov eax, dword ptr [0x566df0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664240) /* 0x566df0 */);
    // 0051cdf6  894224                 -mov dword ptr [edx + 0x24], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(36) /* 0x24 */) = cpu.eax;
L_0x0051cdf9:
    // 0051cdf9  ff4710                 -inc dword ptr [edi + 0x10]
    (app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */))++;
    // 0051cdfc  8a7503                 -mov dh, byte ptr [ebp + 3]
    cpu.dh = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(3) /* 0x3 */);
    // 0051cdff  01ce                   -add esi, ecx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0051ce01  80fe7f                 +cmp dh, 0x7f
    {
        x86::reg8 tmp1 = cpu.dh;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(127 /*0x7f*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051ce04  770c                   -ja 0x51ce12
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051ce12;
    }
    // 0051ce06  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051ce08  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051ce0a  8a5503                 -mov dl, byte ptr [ebp + 3]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(3) /* 0x3 */);
    // 0051ce0d  e83efdffff             -call 0x51cb50
    cpu.esp -= 4;
    sub_51cb50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051ce12:
    // 0051ce12  8a5d01                 -mov bl, byte ptr [ebp + 1]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(1) /* 0x1 */);
    // 0051ce15  890c24                 -mov dword ptr [esp], ecx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ecx;
    // 0051ce18  84db                   +test bl, bl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bl & cpu.bl));
    // 0051ce1a  7408                   -je 0x51ce24
    if (cpu.flags.zf)
    {
        goto L_0x0051ce24;
    }
    // 0051ce1c  8a7d02                 -mov bh, byte ptr [ebp + 2]
    cpu.bh = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(2) /* 0x2 */);
    // 0051ce1f  80ff7f                 +cmp bh, 0x7f
    {
        x86::reg8 tmp1 = cpu.bh;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(127 /*0x7f*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051ce22  7642                   -jbe 0x51ce66
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051ce66;
    }
L_0x0051ce24:
    // 0051ce24  8a4d02                 -mov cl, byte ptr [ebp + 2]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(2) /* 0x2 */);
    // 0051ce27  80f97f                 +cmp cl, 0x7f
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(127 /*0x7f*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051ce2a  0f86b7000000           -jbe 0x51cee7
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051cee7;
    }
    // 0051ce30  80f9ff                 +cmp cl, 0xff
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(255 /*0xff*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051ce33  0f855dffffff           -jne 0x51cd96
    if (!cpu.flags.zf)
    {
        goto L_0x0051cd96;
    }
    // 0051ce39  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ce40  7c18                   -jl 0x51ce5a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051ce5a;
    }
    // 0051ce42  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051ce44  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051ce47  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ce48  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051ce4a  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051ce4c  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051ce4e  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051ce50  b800105500             -mov eax, 0x551000
    cpu.eax = 5574656 /*0x551000*/;
    // 0051ce55  e82653ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051ce5a:
    // 0051ce5a  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051ce5c  e81ffeffff             -call 0x51cc80
    cpu.esp -= 4;
    sub_51cc80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ce61  e930ffffff             -jmp 0x51cd96
    goto L_0x0051cd96;
L_0x0051ce66:
    // 0051ce66  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051ce68  8b4770                 -mov eax, dword ptr [edi + 0x70]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(112) /* 0x70 */);
    // 0051ce6b  88fa                   -mov dl, bh
    cpu.dl = cpu.bh;
    // 0051ce6d  bb7f000000             -mov ebx, 0x7f
    cpu.ebx = 127 /*0x7f*/;
    // 0051ce72  e829f9ffff             -call 0x51c7a0
    cpu.esp -= 4;
    sub_51c7a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ce77  8d5f78                 -lea ebx, [edi + 0x78]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(120) /* 0x78 */);
    // 0051ce7a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051ce7c  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051ce7e  e87d19ffff             -call 0x50e800
    cpu.esp -= 4;
    sub_50e800(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ce83  8944240c               -mov dword ptr [esp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 0051ce87  c6470e00               -mov byte ptr [edi + 0xe], 0
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */) = 0 /*0x0*/;
    // 0051ce8b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ce8d  0f8403ffffff           -je 0x51cd96
    if (cpu.flags.zf)
    {
        goto L_0x0051cd96;
    }
    // 0051ce93  8d500c                 -lea edx, [eax + 0xc]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 0051ce96  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0051ce98  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0051ce9a  e851d6fcff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ce9f  8b54240c               -mov edx, dword ptr [esp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0051cea3  8d8794000000           -lea eax, [edi + 0x94]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(148) /* 0x94 */);
    // 0051cea9  e8f211ffff             -call 0x50e0a0
    cpu.esp -= 4;
    sub_50e0a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ceae  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051ceb2  ff00                   -inc dword ptr [eax]
    (app->getMemory<x86::reg32>(cpu.eax))++;
    // 0051ceb4  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cebb  0f8cd5feffff           -jl 0x51cd96
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051cd96;
    }
    // 0051cec1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cec3  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051cec6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051cec7  8b442410               -mov eax, dword ptr [esp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 0051cecb  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 0051cecd  8d5812                 -lea ebx, [eax + 0x12]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(18) /* 0x12 */);
    // 0051ced0  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051ced2  8d5004                 -lea edx, [eax + 4]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0051ced5  8a480d                 -mov cl, byte ptr [eax + 0xd]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(13) /* 0xd */);
    // 0051ced8  b8f80f5500             -mov eax, 0x550ff8
    cpu.eax = 5574648 /*0x550ff8*/;
    // 0051cedd  e89e52ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cee2  e9affeffff             -jmp 0x51cd96
    goto L_0x0051cd96;
L_0x0051cee7:
    // 0051cee7  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051cee9  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051ceeb  88ca                   -mov dl, cl
    cpu.dl = cpu.cl;
    // 0051ceed  e80efdffff             -call 0x51cc00
    cpu.esp -= 4;
    sub_51cc00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cef2  e99ffeffff             -jmp 0x51cd96
    goto L_0x0051cd96;
L_0x0051cef7:
    // 0051cef7  83c603                 +add esi, 3
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051cefa  e997feffff             -jmp 0x51cd96
    goto L_0x0051cd96;
L_0x0051ceff:
    // 0051ceff  80fafe                 +cmp dl, 0xfe
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(254 /*0xfe*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051cf02  740d                   -je 0x51cf11
    if (cpu.flags.zf)
    {
        goto L_0x0051cf11;
    }
    // 0051cf04  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051cf09  40                     -inc eax
    (cpu.eax)++;
    // 0051cf0a  01c6                   +add esi, eax
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
    // 0051cf0c  e985feffff             -jmp 0x51cd96
    goto L_0x0051cd96;
L_0x0051cf11:
    // 0051cf11  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cf13  40                     -inc eax
    (cpu.eax)++;
    // 0051cf14  01c6                   +add esi, eax
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
    // 0051cf16  e97bfeffff             -jmp 0x51cd96
    goto L_0x0051cd96;
L_0x0051cf1b:
    // 0051cf1b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cf1d  8b0f                   -mov ecx, dword ptr [edi]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edi);
    // 0051cf1f  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051cf21  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051cf23  890f                   -mov dword ptr [edi], ecx
    app->getMemory<x86::reg32>(cpu.edi) = cpu.ecx;
    // 0051cf25  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0051cf28  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cf29  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cf2a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cf2b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_51cf30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051cf30  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051cf31  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051cf32  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051cf33  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051cf34  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051cf35  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051cf36  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0051cf39  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051cf3b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051cf3d  891424                 -mov dword ptr [esp], edx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.edx;
    // 0051cf40  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 0051cf42  89442404               -mov dword ptr [esp + 4], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 0051cf46  8b4704                 -mov eax, dword ptr [edi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0051cf49  8a501f                 -mov dl, byte ptr [eax + 0x1f]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(31) /* 0x1f */);
    // 0051cf4c  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0051cf50  84d2                   +test dl, dl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & cpu.dl));
    // 0051cf52  0f858f000000           -jne 0x51cfe7
    if (!cpu.flags.zf)
    {
        goto L_0x0051cfe7;
    }
L_0x0051cf58:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051cf58  8b4f24                 -mov ecx, dword ptr [edi + 0x24]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */);
    // 0051cf5b  8b4720                 -mov eax, dword ptr [edi + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    // 0051cf5e  39c8                   +cmp eax, ecx
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
    // 0051cf60  7617                   -jbe 0x51cf79
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051cf79;
    }
    // 0051cf62  8b742408               -mov esi, dword ptr [esp + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051cf66  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051cf68  8b571c                 -mov edx, dword ptr [edi + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */);
    // 0051cf6b  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051cf6f  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051cf71  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0051cf73  ff5614                 -call dword ptr [esi + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cf76  014724                 -add dword ptr [edi + 0x24], eax
    (app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */)) += x86::reg32(x86::sreg32(cpu.eax));
L_0x0051cf79:
    // 0051cf79  8b4724                 -mov eax, dword ptr [edi + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */);
    // 0051cf7c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051cf7e  7452                   -je 0x51cfd2
    if (cpu.flags.zf)
    {
        goto L_0x0051cfd2;
    }
    // 0051cf80  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051cf82  31ed                   -xor ebp, ebp
    cpu.ebp ^= x86::reg32(x86::sreg32(cpu.ebp));
L_0x0051cf84:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051cf84  89e2                   -mov edx, esp
    cpu.edx = cpu.esp;
    // 0051cf86  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0051cf88  8b5f1c                 -mov ebx, dword ptr [edi + 0x1c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */);
    // 0051cf8b  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051cf8d  01eb                   -add ebx, ebp
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ebp));
    // 0051cf8f  e8bcfdffff             -call 0x51cd50
    cpu.esp -= 4;
    sub_51cd50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cf94  8b542408               -mov edx, dword ptr [esp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051cf98  807a1f00               +cmp byte ptr [edx + 0x1f], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(31) /* 0x1f */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051cf9c  7516                   -jne 0x51cfb4
    if (!cpu.flags.zf)
    {
        goto L_0x0051cfb4;
    }
    // 0051cf9e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051cfa0  7412                   -je 0x51cfb4
    if (cpu.flags.zf)
    {
        goto L_0x0051cfb4;
    }
    // 0051cfa2  29c6                   -sub esi, eax
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cfa4  01c5                   -add ebp, eax
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051cfa6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051cfa8  740a                   -je 0x51cfb4
    if (cpu.flags.zf)
    {
        goto L_0x0051cfb4;
    }
    // 0051cfaa  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051cfae  83782400               +cmp dword ptr [eax + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cfb2  75d0                   -jne 0x51cf84
    if (!cpu.flags.zf)
    {
        goto L_0x0051cf84;
    }
L_0x0051cfb4:
    // 0051cfb4  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051cfb8  80781f00               +cmp byte ptr [eax + 0x1f], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(31) /* 0x1f */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051cfbc  7514                   -jne 0x51cfd2
    if (!cpu.flags.zf)
    {
        goto L_0x0051cfd2;
    }
    // 0051cfbe  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0051cfc0  7410                   -je 0x51cfd2
    if (cpu.flags.zf)
    {
        goto L_0x0051cfd2;
    }
    // 0051cfc2  8b571c                 -mov edx, dword ptr [edi + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */);
    // 0051cfc5  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0051cfc7  8d042a                 -lea eax, [edx + ebp]
    cpu.eax = x86::reg32(cpu.edx + cpu.ebp * 1);
    // 0051cfca  e821d5fcff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051cfcf  897724                 -mov dword ptr [edi + 0x24], esi
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */) = cpu.esi;
L_0x0051cfd2:
    // 0051cfd2  833c2400               +cmp dword ptr [esp], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051cfd6  741b                   -je 0x51cff3
    if (cpu.flags.zf)
    {
        goto L_0x0051cff3;
    }
    // 0051cfd8  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051cfdd  83c40c                 +add esp, 0xc
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
    // 0051cfe0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cfe1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cfe2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cfe3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cfe4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cfe5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cfe6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051cfe7:
    // 0051cfe7  c7472400000000         -mov dword ptr [edi + 0x24], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
    // 0051cfee  e965ffffff             -jmp 0x51cf58
    goto L_0x0051cf58;
L_0x0051cff3:
    // 0051cff3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051cff5  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0051cff8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cff9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cffa  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cffb  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cffc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cffd  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051cffe  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_51d000(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051d000  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0051d003  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_51d010(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051d010  8b5008                 -mov edx, dword ptr [eax + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 0051d013  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051d015  7e0f                   -jle 0x51d026
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051d026;
    }
    // 0051d017  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051d018  8d4aff                 -lea ecx, [edx - 1]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 0051d01b  894808                 -mov dword ptr [eax + 8], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.ecx;
    // 0051d01e  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0051d020  7e0a                   -jle 0x51d02c
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051d02c;
    }
    // 0051d022  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d024  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d025  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051d026:
    // 0051d026  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051d02b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051d02c:
    // 0051d02c  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051d031  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d032  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_51d040(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051d040  83780800               +cmp dword ptr [eax + 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d044  0f8ea6f8ffff           -jle 0x51c8f0
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
    return sub_51c8f0(app, cpu.handOver());
    }
    // 0051d04a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051d04f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_51d050(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051d050  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051d051  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051d052  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051d053  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051d054  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051d055  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051d056  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0051d058  83ec1c                 -sub esp, 0x1c
    (cpu.esp) -= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 0051d05b  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0051d05e  8b7810                 -mov edi, dword ptr [eax + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0051d061  8b4704                 -mov eax, dword ptr [edi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0051d064  8b15ec6d5600           -mov edx, dword ptr [0x566dec]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
    // 0051d06a  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
    // 0051d06d  83fa05                 +cmp edx, 5
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
    // 0051d070  0f8d68010000           -jge 0x51d1de
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051d1de;
    }
L_0x0051d076:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d076  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0051d079  c7472800000000         -mov dword ptr [edi + 0x28], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(40) /* 0x28 */) = 0 /*0x0*/;
    // 0051d080  83782400               +cmp dword ptr [eax + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d084  0f844b010000           -je 0x51d1d5
    if (cpu.flags.zf)
    {
        goto L_0x0051d1d5;
    }
    // 0051d08a  8d8794000000           -lea eax, [edi + 0x94]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(148) /* 0x94 */);
    // 0051d090  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 0051d093  8d4754                 -lea eax, [edi + 0x54]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(84) /* 0x54 */);
    // 0051d096  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0051d099  8d4778                 -lea eax, [edi + 0x78]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(120) /* 0x78 */);
    // 0051d09c  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 0051d09f  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0051d0a2  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
L_0x0051d0a5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d0a5  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0051d0a8  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0051d0ab  ff5210                 -call dword ptr [edx + 0x10]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d0ae  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d0b0  0f8545010000           -jne 0x51d1fb
    if (!cpu.flags.zf)
    {
        goto L_0x0051d1fb;
    }
L_0x0051d0b6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d0b6  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0051d0b9  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0051d0bc  8b5770                 -mov edx, dword ptr [edi + 0x70]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(112) /* 0x70 */);
    // 0051d0bf  e83c17ffff             -call 0x50e800
    cpu.esp -= 4;
    sub_50e800(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d0c4  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051d0c6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d0c8  0f8485000000           -je 0x51d153
    if (cpu.flags.zf)
    {
        goto L_0x0051d153;
    }
L_0x0051d0ce:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d0ce  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d0d5  7c1d                   -jl 0x51d0f4
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d0f4;
    }
    // 0051d0d7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d0d9  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051d0dc  8d5e12                 -lea ebx, [esi + 0x12]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(18) /* 0x12 */);
    // 0051d0df  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d0e0  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d0e2  8d5604                 -lea edx, [esi + 4]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0051d0e5  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051d0e7  8a4e0d                 -mov cl, byte ptr [esi + 0xd]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051d0ea  b81c105500             -mov eax, 0x55101c
    cpu.eax = 5574684 /*0x55101c*/;
    // 0051d0ef  e88c50ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051d0f4:
    // 0051d0f4  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0051d0f7  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051d0f9  8d5612                 -lea edx, [esi + 0x12]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(18) /* 0x12 */);
    // 0051d0fc  8a5e0d                 -mov bl, byte ptr [esi + 0xd]
    cpu.bl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051d0ff  e8ec55ffff             -call 0x5126f0
    cpu.esp -= 4;
    sub_5126f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d104  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d106  0f84fb000000           -je 0x51d207
    if (cpu.flags.zf)
    {
        goto L_0x0051d207;
    }
    // 0051d10c  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0051d10f  e80c19ffff             -call 0x50ea20
    cpu.esp -= 4;
    sub_50ea20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d114  8b5774                 -mov edx, dword ptr [edi + 0x74]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(116) /* 0x74 */);
    // 0051d117  8d4a01                 -lea ecx, [edx + 1]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 0051d11a  894f74                 -mov dword ptr [edi + 0x74], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(116) /* 0x74 */) = cpu.ecx;
    // 0051d11d  895604                 -mov dword ptr [esi + 4], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 0051d120  ff4770                 -inc dword ptr [edi + 0x70]
    (app->getMemory<x86::reg32>(cpu.edi + x86::reg32(112) /* 0x70 */))++;
    // 0051d123  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051d125  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0051d128  e81319ffff             -call 0x50ea40
    cpu.esp -= 4;
    sub_50ea40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d12d  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0051d130  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051d132  c7460800000000         -mov dword ptr [esi + 8], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 0051d139  e8620fffff             -call 0x50e0a0
    cpu.esp -= 4;
    sub_50e0a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d13e  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0051d141  8b5770                 -mov edx, dword ptr [edi + 0x70]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(112) /* 0x70 */);
    // 0051d144  e8b716ffff             -call 0x50e800
    cpu.esp -= 4;
    sub_50e800(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d149  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051d14b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d14d  0f857bffffff           -jne 0x51d0ce
    if (!cpu.flags.zf)
    {
        goto L_0x0051d0ce;
    }
L_0x0051d153:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d153  837dec00               +cmp dword ptr [ebp - 0x14], 0
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
    // 0051d157  0f8548ffffff           -jne 0x51d0a5
    if (!cpu.flags.zf)
    {
        goto L_0x0051d0a5;
    }
    // 0051d15d  807f0e00               +cmp byte ptr [edi + 0xe], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051d161  0f85d6000000           -jne 0x51d23d
    if (!cpu.flags.zf)
    {
        goto L_0x0051d23d;
    }
    // 0051d167  8d8794000000           -lea eax, [edi + 0x94]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(148) /* 0x94 */);
    // 0051d16d  e85e11ffff             -call 0x50e2d0
    cpu.esp -= 4;
    sub_50e2d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d172  83f8ff                 +cmp eax, -1
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
    // 0051d175  0f84bb000000           -je 0x51d236
    if (cpu.flags.zf)
    {
        goto L_0x0051d236;
    }
    // 0051d17b  3b4770                 +cmp eax, dword ptr [edi + 0x70]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi + x86::reg32(112) /* 0x70 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d17e  0f84b2000000           -je 0x51d236
    if (cpu.flags.zf)
    {
        goto L_0x0051d236;
    }
    // 0051d184  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0051d189:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d189  c6470e01               -mov byte ptr [edi + 0xe], 1
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(14) /* 0xe */) = 1 /*0x1*/;
L_0x0051d18d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d18d  837f2800               +cmp dword ptr [edi + 0x28], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(40) /* 0x28 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d191  0f84ad000000           -je 0x51d244
    if (cpu.flags.zf)
    {
        goto L_0x0051d244;
    }
L_0x0051d197:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d197  833dec6d560004         +cmp dword ptr [0x566dec], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d19e  7c23                   -jl 0x51d1c3
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d1c3;
    }
    // 0051d1a0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d1a2  8b7728                 -mov esi, dword ptr [edi + 0x28]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(40) /* 0x28 */);
    // 0051d1a5  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051d1a8  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d1aa  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d1ab  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051d1ad  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051d1af  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051d1b1  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0051d1b3  0f849c000000           -je 0x51d255
    if (cpu.flags.zf)
    {
        goto L_0x0051d255;
    }
    // 0051d1b9  b82c105500             -mov eax, 0x55102c
    cpu.eax = 5574700 /*0x55102c*/;
L_0x0051d1be:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d1be  e8bd4fffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051d1c3:
    // 0051d1c3  8d4778                 -lea eax, [edi + 0x78]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(120) /* 0x78 */);
    // 0051d1c6  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051d1c8  e8a312ffff             -call 0x50e470
    cpu.esp -= 4;
    nfs3hp::ea::qfindindex(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d1cd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d1cf  0f858a000000           -jne 0x51d25f
    if (!cpu.flags.zf)
    {
        goto L_0x0051d25f;
    }
L_0x0051d1d5:
    // 0051d1d5  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0051d1d7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d1d8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d1d9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d1da  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d1db  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d1dc  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d1dd  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051d1de:
    // 0051d1de  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d1e0  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051d1e3  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d1e4  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d1e6  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051d1e8  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051d1ea  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051d1ec  b810105500             -mov eax, 0x551010
    cpu.eax = 5574672 /*0x551010*/;
    // 0051d1f1  e88a4fffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d1f6  e97bfeffff             -jmp 0x51d076
    goto L_0x0051d076;
L_0x0051d1fb:
    // 0051d1fb  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051d1fd  e82efdffff             -call 0x51cf30
    cpu.esp -= 4;
    sub_51cf30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d202  e9affeffff             -jmp 0x51d0b6
    goto L_0x0051d0b6;
L_0x0051d207:
    // 0051d207  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d20e  7c17                   -jl 0x51d227
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d227;
    }
    // 0051d210  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051d213  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d214  8d5604                 -lea edx, [esi + 4]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0051d217  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d219  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051d21b  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0051d21d  b824105500             -mov eax, 0x551024
    cpu.eax = 5574692 /*0x551024*/;
    // 0051d222  e8594fffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051d227:
    // 0051d227  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0051d22a  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051d22c  e86f0effff             -call 0x50e0a0
    cpu.esp -= 4;
    sub_50e0a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d231  e91dffffff             -jmp 0x51d153
    goto L_0x0051d153;
L_0x0051d236:
    // 0051d236  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051d238  e94cffffff             -jmp 0x51d189
    goto L_0x0051d189;
L_0x0051d23d:
    // 0051d23d  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051d23f  e949ffffff             -jmp 0x51d18d
    goto L_0x0051d18d;
L_0x0051d244:
    // 0051d244  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d246  0f854bffffff           -jne 0x51d197
    if (!cpu.flags.zf)
    {
        goto L_0x0051d197;
    }
    // 0051d24c  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0051d24e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d24f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d250  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d251  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d252  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d253  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d254  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051d255:
    // 0051d255  b834105500             -mov eax, 0x551034
    cpu.eax = 5574708 /*0x551034*/;
    // 0051d25a  e95fffffff             -jmp 0x51d1be
    goto L_0x0051d1be;
L_0x0051d25f:
    // 0051d25f  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0051d261  e88af8ffff             -call 0x51caf0
    cpu.esp -= 4;
    sub_51caf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d266  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0051d268  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d269  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d26a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d26b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d26c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d26d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d26e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_51d270(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051d270  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051d271  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051d272  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051d273  83ec2c                 -sub esp, 0x2c
    (cpu.esp) -= x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 0051d276  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051d278  895c2414               -mov dword ptr [esp + 0x14], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */) = cpu.ebx;
    // 0051d27c  8b6c2414               -mov ebp, dword ptr [esp + 0x14]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */);
    // 0051d280  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051d282  89542410               -mov dword ptr [esp + 0x10], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.edx;
    // 0051d286  895c240c               -mov dword ptr [esp + 0xc], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.ebx;
    // 0051d28a  895c2424               -mov dword ptr [esp + 0x24], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(36) /* 0x24 */) = cpu.ebx;
    // 0051d28e  83fd06                 +cmp ebp, 6
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6 /*0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d291  0f8c47020000           -jl 0x51d4de
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d4de;
    }
    // 0051d297  81fdfa000000           +cmp ebp, 0xfa
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(250 /*0xfa*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d29d  0f8e48020000           -jle 0x51d4eb
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051d4eb;
    }
    // 0051d2a3  bd3c105500             -mov ebp, 0x55103c
    cpu.ebp = 5574716 /*0x55103c*/;
    // 0051d2a8  68fa000000             -push 0xfa
    app->getMemory<x86::reg32>(cpu.esp-4) = 250 /*0xfa*/;
    cpu.esp -= 4;
    // 0051d2ad  b84c105500             -mov eax, 0x55104c
    cpu.eax = 5574732 /*0x55104c*/;
    // 0051d2b2  ba05020000             -mov edx, 0x205
    cpu.edx = 517 /*0x205*/;
    // 0051d2b7  6864105500             -push 0x551064
    app->getMemory<x86::reg32>(cpu.esp-4) = 5574756 /*0x551064*/;
    cpu.esp -= 4;
    // 0051d2bc  892d90215500           -mov dword ptr [0x552190], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebp;
    // 0051d2c2  a394215500             -mov dword ptr [0x552194], eax
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.eax;
    // 0051d2c7  891598215500           -mov dword ptr [0x552198], edx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edx;
    // 0051d2cd  e83e3deeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d2d2  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
L_0x0051d2d5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d2d5  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0051d2d7  7433                   -je 0x51d30c
    if (cpu.flags.zf)
    {
        goto L_0x0051d30c;
    }
    // 0051d2d9  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d2dd  742d                   -je 0x51d30c
    if (cpu.flags.zf)
    {
        goto L_0x0051d30c;
    }
    // 0051d2df  bd3c105500             -mov ebp, 0x55103c
    cpu.ebp = 5574716 /*0x55103c*/;
    // 0051d2e4  b84c105500             -mov eax, 0x55104c
    cpu.eax = 5574732 /*0x55104c*/;
    // 0051d2e9  ba0a020000             -mov edx, 0x20a
    cpu.edx = 522 /*0x20a*/;
    // 0051d2ee  68f4105500             -push 0x5510f4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5574900 /*0x5510f4*/;
    cpu.esp -= 4;
    // 0051d2f3  892d90215500           -mov dword ptr [0x552190], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebp;
    // 0051d2f9  a394215500             -mov dword ptr [0x552194], eax
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.eax;
    // 0051d2fe  891598215500           -mov dword ptr [0x552198], edx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edx;
    // 0051d304  e8073deeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d309  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x0051d30c:
    // 0051d30c  83f902                 +cmp ecx, 2
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
    // 0051d30f  0f8d26020000           -jge 0x51d53b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051d53b;
    }
    // 0051d315  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
L_0x0051d31a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d31a  8b542410               -mov edx, dword ptr [esp + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 0051d31e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d320  8b5c2414               -mov ebx, dword ptr [esp + 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */);
    // 0051d324  8a421e                 -mov al, byte ptr [edx + 0x1e]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(30) /* 0x1e */);
    // 0051d327  39d8                   +cmp eax, ebx
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
    // 0051d329  7e04                   -jle 0x51d32f
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051d32f;
    }
    // 0051d32b  89442414               -mov dword ptr [esp + 0x14], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */) = cpu.eax;
L_0x0051d32f:
    // 0051d32f  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051d334  d3e2                   -shl edx, cl
    cpu.edx <<= cpu.cl % 32;
    // 0051d336  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051d338  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051d33b  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0051d33d  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0051d33f  8d68ff                 -lea ebp, [eax - 1]
    cpu.ebp = x86::reg32(cpu.eax + x86::reg32(-1) /* -0x1 */);
    // 0051d342  bf00010000             -mov edi, 0x100
    cpu.edi = 256 /*0x100*/;
    // 0051d347  8b442410               -mov eax, dword ptr [esp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 0051d34b  897c2408               -mov dword ptr [esp + 8], edi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.edi;
    // 0051d34f  8a501f                 -mov dl, byte ptr [eax + 0x1f]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(31) /* 0x1f */);
    // 0051d352  896c2428               -mov dword ptr [esp + 0x28], ebp
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(40) /* 0x28 */) = cpu.ebp;
    // 0051d356  84d2                   +test dl, dl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & cpu.dl));
    // 0051d358  7508                   -jne 0x51d362
    if (!cpu.flags.zf)
    {
        goto L_0x0051d362;
    }
    // 0051d35a  c744240800020000       -mov dword ptr [esp + 8], 0x200
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = 512 /*0x200*/;
L_0x0051d362:
    // 0051d362  8b542428               -mov edx, dword ptr [esp + 0x28]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(40) /* 0x28 */);
    // 0051d366  8d4501                 -lea eax, [ebp + 1]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(1) /* 0x1 */);
    // 0051d369  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0051d36b  8b442414               -mov eax, dword ptr [esp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */);
    // 0051d36f  83c015                 -add eax, 0x15
    (cpu.eax) += x86::reg32(x86::sreg32(21 /*0x15*/));
    // 0051d372  24fc                   -and al, 0xfc
    cpu.al &= x86::reg8(x86::sreg8(252 /*0xfc*/));
    // 0051d374  0fafc2                 -imul eax, edx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 0051d377  8b542408               -mov edx, dword ptr [esp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051d37b  05b0000000             -add eax, 0xb0
    (cpu.eax) += x86::reg32(x86::sreg32(176 /*0xb0*/));
    // 0051d380  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0051d382  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0051d384  0f84c4010000           -je 0x51d54e
    if (cpu.flags.zf)
    {
        goto L_0x0051d54e;
    }
    // 0051d38a  8b4e10                 -mov ecx, dword ptr [esi + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 0051d38d  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0051d38f  0f85c4010000           -jne 0x51d559
    if (!cpu.flags.zf)
    {
        goto L_0x0051d559;
    }
    // 0051d395  bb3c105500             -mov ebx, 0x55103c
    cpu.ebx = 5574716 /*0x55103c*/;
    // 0051d39a  bf4c105500             -mov edi, 0x55104c
    cpu.edi = 5574732 /*0x55104c*/;
    // 0051d39f  ba2b020000             -mov edx, 0x22b
    cpu.edx = 555 /*0x22b*/;
    // 0051d3a4  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0051d3a9  891d90215500           -mov dword ptr [0x552190], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebx;
    // 0051d3af  893d94215500           -mov dword ptr [0x552194], edi
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.edi;
    // 0051d3b5  891598215500           -mov dword ptr [0x552198], edx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edx;
    // 0051d3bb  8b1df4435600           -mov ebx, dword ptr [0x5643f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 0051d3c1  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051d3c3  b82c115500             -mov eax, 0x55112c
    cpu.eax = 5574956 /*0x55112c*/;
    // 0051d3c8  894c240c               -mov dword ptr [esp + 0xc], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.ecx;
    // 0051d3cc  e84f42fcff             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d3d1  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
L_0x0051d3d3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d3d3  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051d3d5  0f844a030000           -je 0x51d725
    if (cpu.flags.zf)
    {
        goto L_0x0051d725;
    }
    // 0051d3db  8d8fb0000000           -lea ecx, [edi + 0xb0]
    cpu.ecx = x86::reg32(cpu.edi + x86::reg32(176) /* 0xb0 */);
    // 0051d3e1  c70600000000           -mov dword ptr [esi], 0
    app->getMemory<x86::reg32>(cpu.esi) = 0 /*0x0*/;
    // 0051d3e7  c7462400000000         -mov dword ptr [esi + 0x24], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
    // 0051d3ee  8b4618                 -mov eax, dword ptr [esi + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0051d3f1  897e10                 -mov dword ptr [esi + 0x10], edi
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */) = cpu.edi;
    // 0051d3f4  89442404               -mov dword ptr [esp + 4], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 0051d3f8  c74618b0504f00         -mov dword ptr [esi + 0x18], 0x4f50b0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */) = 5198000 /*0x4f50b0*/;
    // 0051d3ff  8b442414               -mov eax, dword ptr [esp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */);
    // 0051d403  8937                   -mov dword ptr [edi], esi
    app->getMemory<x86::reg32>(cpu.edi) = cpu.esi;
    // 0051d405  894708                 -mov dword ptr [edi + 8], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0051d408  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d40a  668b462a               -mov ax, word ptr [esi + 0x2a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(42) /* 0x2a */);
    // 0051d40e  89472c                 -mov dword ptr [edi + 0x2c], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(44) /* 0x2c */) = cpu.eax;
    // 0051d411  8a44240c               -mov al, byte ptr [esp + 0xc]
    cpu.al = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0051d415  88470c                 -mov byte ptr [edi + 0xc], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(12) /* 0xc */) = cpu.al;
    // 0051d418  8b442410               -mov eax, dword ptr [esp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 0051d41c  894704                 -mov dword ptr [edi + 4], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 0051d41f  a124ac5600             -mov eax, dword ptr [0x56ac24]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680164) /* 0x56ac24 */);
    // 0051d424  88470d                 -mov byte ptr [edi + 0xd], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */) = cpu.al;
    // 0051d427  40                     -inc eax
    (cpu.eax)++;
    // 0051d428  a324ac5600             -mov dword ptr [0x56ac24], eax
    app->getMemory<x86::reg32>(x86::reg32(5680164) /* 0x56ac24 */) = cpu.eax;
    // 0051d42d  c7471000000000         -mov dword ptr [edi + 0x10], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */) = 0 /*0x0*/;
    // 0051d434  e8a7defcff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d439  c7473001000000         -mov dword ptr [edi + 0x30], 1
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(48) /* 0x30 */) = 1 /*0x1*/;
    // 0051d440  c7473400000000         -mov dword ptr [edi + 0x34], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(52) /* 0x34 */) = 0 /*0x0*/;
    // 0051d447  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051d449  894714                 -mov dword ptr [edi + 0x14], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 0051d44c  8d4738                 -lea eax, [edi + 0x38]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(56) /* 0x38 */);
    // 0051d44f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051d451  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0051d454  e87708ffff             -call 0x50dcd0
    cpu.esp -= 4;
    sub_50dcd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d459  8d4754                 -lea eax, [edi + 0x54]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(84) /* 0x54 */);
    // 0051d45c  ba00d05100             -mov edx, 0x51d000
    cpu.edx = 5361664 /*0x51d000*/;
    // 0051d461  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0051d463  e86808ffff             -call 0x50dcd0
    cpu.esp -= 4;
    sub_50dcd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d468  ba00d05100             -mov edx, 0x51d000
    cpu.edx = 5361664 /*0x51d000*/;
    // 0051d46d  8d4778                 -lea eax, [edi + 0x78]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(120) /* 0x78 */);
    // 0051d470  c7477001000000         -mov dword ptr [edi + 0x70], 1
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(112) /* 0x70 */) = 1 /*0x1*/;
    // 0051d477  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0051d479  c7477401000000         -mov dword ptr [edi + 0x74], 1
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(116) /* 0x74 */) = 1 /*0x1*/;
    // 0051d480  e84b08ffff             -call 0x50dcd0
    cpu.esp -= 4;
    sub_50dcd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d485  8d8794000000           -lea eax, [edi + 0x94]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(148) /* 0x94 */);
    // 0051d48b  ba00d05100             -mov edx, 0x51d000
    cpu.edx = 5361664 /*0x51d000*/;
    // 0051d490  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0051d492  e83908ffff             -call 0x50dcd0
    cpu.esp -= 4;
    sub_50dcd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d497  8b442414               -mov eax, dword ptr [esp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */);
    // 0051d49b  83c015                 -add eax, 0x15
    (cpu.eax) += x86::reg32(x86::sreg32(21 /*0x15*/));
    // 0051d49e  24fc                   -and al, 0xfc
    cpu.al &= x86::reg8(x86::sreg8(252 /*0xfc*/));
    // 0051d4a0  8b1c24                 -mov ebx, dword ptr [esp]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051d4a3  89442420               -mov dword ptr [esp + 0x20], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(32) /* 0x20 */) = cpu.eax;
L_0x0051d4a7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d4a7  8b442428               -mov eax, dword ptr [esp + 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(40) /* 0x28 */);
    // 0051d4ab  48                     -dec eax
    (cpu.eax)--;
    // 0051d4ac  89442428               -mov dword ptr [esp + 0x28], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(40) /* 0x28 */) = cpu.eax;
    // 0051d4b0  83f8ff                 +cmp eax, -1
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
    // 0051d4b3  0f84a7000000           -je 0x51d560
    if (cpu.flags.zf)
    {
        goto L_0x0051d560;
    }
    // 0051d4b9  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0051d4bb  c70200000000           -mov dword ptr [edx], 0
    app->getMemory<x86::reg32>(cpu.edx) = 0 /*0x0*/;
    // 0051d4c1  8b442420               -mov eax, dword ptr [esp + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(32) /* 0x20 */);
    // 0051d4c5  c7420400000000         -mov dword ptr [edx + 4], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */) = 0 /*0x0*/;
    // 0051d4cc  01c1                   +add ecx, eax
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
    // 0051d4ce  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051d4d0  c7420800000000         -mov dword ptr [edx + 8], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 0051d4d7  e87409ffff             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d4dc  ebc9                   -jmp 0x51d4a7
    goto L_0x0051d4a7;
L_0x0051d4de:
    // 0051d4de  c744241406000000       -mov dword ptr [esp + 0x14], 6
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */) = 6 /*0x6*/;
    // 0051d4e6  e9eafdffff             -jmp 0x51d2d5
    goto L_0x0051d2d5;
L_0x0051d4eb:
    // 0051d4eb  668b5a1c               -mov bx, word ptr [edx + 0x1c]
    cpu.bx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(28) /* 0x1c */);
    // 0051d4ef  6685db                 +test bx, bx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.bx & cpu.bx));
    // 0051d4f2  0f84ddfdffff           -je 0x51d2d5
    if (cpu.flags.zf)
    {
        goto L_0x0051d2d5;
    }
    // 0051d4f8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d4fa  8d5506                 -lea edx, [ebp + 6]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(6) /* 0x6 */);
    // 0051d4fd  6689d8                 -mov ax, bx
    cpu.ax = cpu.bx;
    // 0051d500  39c2                   +cmp edx, eax
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
    // 0051d502  0f86cdfdffff           -jbe 0x51d2d5
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051d2d5;
    }
    // 0051d508  ba3c105500             -mov edx, 0x55103c
    cpu.edx = 5574716 /*0x55103c*/;
    // 0051d50d  bb4c105500             -mov ebx, 0x55104c
    cpu.ebx = 5574732 /*0x55104c*/;
    // 0051d512  bf07020000             -mov edi, 0x207
    cpu.edi = 519 /*0x207*/;
    // 0051d517  68a4105500             -push 0x5510a4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5574820 /*0x5510a4*/;
    cpu.esp -= 4;
    // 0051d51c  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 0051d522  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 0051d528  893d98215500           -mov dword ptr [0x552198], edi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edi;
    // 0051d52e  e8dd3aeeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d533  83c404                 +add esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051d536  e99afdffff             -jmp 0x51d2d5
    goto L_0x0051d2d5;
L_0x0051d53b:
    // 0051d53b  83f907                 +cmp ecx, 7
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
    // 0051d53e  0f8ed6fdffff           -jle 0x51d31a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051d31a;
    }
    // 0051d544  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 0051d549  e9ccfdffff             -jmp 0x51d31a
    goto L_0x0051d31a;
L_0x0051d54e:
    // 0051d54e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051d550  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051d552  83c42c                 +add esp, 0x2c
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(44 /*0x2c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051d555  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d556  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d557  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d558  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051d559:
    // 0051d559  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 0051d55b  e973feffff             -jmp 0x51d3d3
    goto L_0x0051d3d3;
L_0x0051d560:
    // 0051d560  8d4778                 -lea eax, [edi + 0x78]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(120) /* 0x78 */);
    // 0051d563  89442418               -mov dword ptr [esp + 0x18], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 0051d567  8b442414               -mov eax, dword ptr [esp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */);
    // 0051d56b  83c015                 -add eax, 0x15
    (cpu.eax) += x86::reg32(x86::sreg32(21 /*0x15*/));
    // 0051d56e  8944241c               -mov dword ptr [esp + 0x1c], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(28) /* 0x1c */) = cpu.eax;
L_0x0051d572:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d572  8b44241c               -mov eax, dword ptr [esp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(28) /* 0x1c */);
    // 0051d576  24fc                   -and al, 0xfc
    cpu.al &= x86::reg8(x86::sreg8(252 /*0xfc*/));
    // 0051d578  4d                     -dec ebp
    (cpu.ebp)--;
    // 0051d579  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d57b  83fdff                 +cmp ebp, -1
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d57e  7428                   -je 0x51d5a8
    if (cpu.flags.zf)
    {
        goto L_0x0051d5a8;
    }
    // 0051d580  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0051d582  c70200000000           -mov dword ptr [edx], 0
    app->getMemory<x86::reg32>(cpu.edx) = 0 /*0x0*/;
    // 0051d588  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051d58a  8b4774                 -mov eax, dword ptr [edi + 0x74]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(116) /* 0x74 */);
    // 0051d58d  8d5801                 -lea ebx, [eax + 1]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0051d590  895f74                 -mov dword ptr [edi + 0x74], ebx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(116) /* 0x74 */) = cpu.ebx;
    // 0051d593  894204                 -mov dword ptr [edx + 4], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 0051d596  8b442418               -mov eax, dword ptr [esp + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(24) /* 0x18 */);
    // 0051d59a  c7420800000000         -mov dword ptr [edx + 8], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 0051d5a1  e8fa0affff             -call 0x50e0a0
    cpu.esp -= 4;
    sub_50e0a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d5a6  ebca                   -jmp 0x51d572
    goto L_0x0051d572;
L_0x0051d5a8:
    // 0051d5a8  c7472400000000         -mov dword ptr [edi + 0x24], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
    // 0051d5af  89471c                 -mov dword ptr [edi + 0x1c], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */) = cpu.eax;
    // 0051d5b2  894f18                 -mov dword ptr [edi + 0x18], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(24) /* 0x18 */) = cpu.ecx;
    // 0051d5b5  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051d5b9  8b15f06d5600           -mov edx, dword ptr [0x566df0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5664240) /* 0x566df0 */);
    // 0051d5bf  894720                 -mov dword ptr [edi + 0x20], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */) = cpu.eax;
    // 0051d5c2  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051d5c4  752e                   -jne 0x51d5f4
    if (!cpu.flags.zf)
    {
        goto L_0x0051d5f4;
    }
    // 0051d5c6  b93c105500             -mov ecx, 0x55103c
    cpu.ecx = 5574716 /*0x55103c*/;
    // 0051d5cb  bb4c105500             -mov ebx, 0x55104c
    cpu.ebx = 5574732 /*0x55104c*/;
    // 0051d5d0  bd76020000             -mov ebp, 0x276
    cpu.ebp = 630 /*0x276*/;
    // 0051d5d5  6834115500             -push 0x551134
    app->getMemory<x86::reg32>(cpu.esp-4) = 5574964 /*0x551134*/;
    cpu.esp -= 4;
    // 0051d5da  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 0051d5e0  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 0051d5e6  892d98215500           -mov dword ptr [0x552198], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebp;
    // 0051d5ec  e81f3aeeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d5f1  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x0051d5f4:
    // 0051d5f4  8b4c2410               -mov ecx, dword ptr [esp + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 0051d5f8  ba50d05100             -mov edx, 0x51d050
    cpu.edx = 5361744 /*0x51d050*/;
    // 0051d5fd  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051d5ff  ff11                   -call dword ptr [ecx]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d601  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d603  0f8475010000           -je 0x51d77e
    if (cpu.flags.zf)
    {
        goto L_0x0051d77e;
    }
    // 0051d609  c7462400ca9a3b         -mov dword ptr [esi + 0x24], 0x3b9aca00
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 1000000000 /*0x3b9aca00*/;
    // 0051d610  b828269f00             -mov eax, 0x9f2628
    cpu.eax = 10429992 /*0x9f2628*/;
    // 0051d615  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051d617  e83408ffff             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d61c  833dec6d560001         +cmp dword ptr [0x566dec], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d623  0f8d12010000           -jge 0x51d73b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051d73b;
    }
L_0x0051d629:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d629  31ed                   -xor ebp, ebp
    cpu.ebp ^= x86::reg32(x86::sreg32(cpu.ebp));
L_0x0051d62b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d62b  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0051d62d  e8fe22fcff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d632  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d634  0f841e010000           -je 0x51d758
    if (cpu.flags.zf)
    {
        goto L_0x0051d758;
    }
    // 0051d63a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051d63f  e8dca0fcff             -call 0x4e7720
    cpu.esp -= 4;
    nfs3hp::port::waitSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d644  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d646  7c0d                   -jl 0x51d655
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d655;
    }
L_0x0051d648:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d648  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051d64a  ff5614                 -call dword ptr [esi + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d64d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d64f  0f8412010000           -je 0x51d767
    if (cpu.flags.zf)
    {
        goto L_0x0051d767;
    }
L_0x0051d655:
    // 0051d655  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x0051d65a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d65a  89542424               -mov dword ptr [esp + 0x24], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(36) /* 0x24 */) = cpu.edx;
    // 0051d65e  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051d660  7420                   -je 0x51d682
    if (cpu.flags.zf)
    {
        goto L_0x0051d682;
    }
    // 0051d662  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d669  7c17                   -jl 0x51d682
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d682;
    }
    // 0051d66b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d66d  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051d670  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d671  89e9                   -mov ecx, ebp
    cpu.ecx = cpu.ebp;
    // 0051d673  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 0051d675  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051d676  89ea                   -mov edx, ebp
    cpu.edx = cpu.ebp;
    // 0051d678  b868115500             -mov eax, 0x551168
    cpu.eax = 5575016 /*0x551168*/;
    // 0051d67d  e8fe4affff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051d682:
    // 0051d682  3b6e24                 +cmp ebp, dword ptr [esi + 0x24]
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d685  7407                   -je 0x51d68e
    if (cpu.flags.zf)
    {
        goto L_0x0051d68e;
    }
    // 0051d687  c7462400ca9a3b         -mov dword ptr [esi + 0x24], 0x3b9aca00
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 1000000000 /*0x3b9aca00*/;
L_0x0051d68e:
    // 0051d68e  3b6e24                 +cmp ebp, dword ptr [esi + 0x24]
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d691  740d                   -je 0x51d6a0
    if (cpu.flags.zf)
    {
        goto L_0x0051d6a0;
    }
    // 0051d693  8b5710                 -mov edx, dword ptr [edi + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */);
    // 0051d696  39d5                   +cmp ebp, edx
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
    // 0051d698  7506                   -jne 0x51d6a0
    if (!cpu.flags.zf)
    {
        goto L_0x0051d6a0;
    }
    // 0051d69a  3b542424               +cmp edx, dword ptr [esp + 0x24]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(36) /* 0x24 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d69e  748b                   -je 0x51d62b
    if (cpu.flags.zf)
    {
        goto L_0x0051d62b;
    }
L_0x0051d6a0:
    // 0051d6a0  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d6a4  0f85c4000000           -jne 0x51d76e
    if (!cpu.flags.zf)
    {
        goto L_0x0051d76e;
    }
L_0x0051d6aa:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d6aa  8b4624                 -mov eax, dword ptr [esi + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
    // 0051d6ad  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d6af  751f                   -jne 0x51d6d0
    if (!cpu.flags.zf)
    {
        goto L_0x0051d6d0;
    }
    // 0051d6b1  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d6b8  7c16                   -jl 0x51d6d0
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d6d0;
    }
    // 0051d6ba  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051d6bd  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d6be  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d6c0  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051d6c2  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051d6c4  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051d6c6  b878115500             -mov eax, 0x551178
    cpu.eax = 5575032 /*0x551178*/;
    // 0051d6cb  e8b04affff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051d6d0:
    // 0051d6d0  c7462400000000         -mov dword ptr [esi + 0x24], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
    // 0051d6d7  8b542410               -mov edx, dword ptr [esp + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 0051d6db  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051d6dd  ff5204                 -call dword ptr [edx + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051d6e0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d6e0  837e2400               +cmp dword ptr [esi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d6e4  0f84be000000           -je 0x51d7a8
    if (cpu.flags.zf)
    {
        goto L_0x0051d7a8;
    }
    // 0051d6ea  8b5c2424               -mov ebx, dword ptr [esp + 0x24]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(36) /* 0x24 */);
    // 0051d6ee  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051d6f0  0f85b2000000           -jne 0x51d7a8
    if (!cpu.flags.zf)
    {
        goto L_0x0051d7a8;
    }
    // 0051d6f6  a1f06d5600             -mov eax, dword ptr [0x566df0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5664240) /* 0x566df0 */);
    // 0051d6fb  894624                 -mov dword ptr [esi + 0x24], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = cpu.eax;
    // 0051d6fe  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051d702  8b15ec6d5600           -mov edx, dword ptr [0x566dec]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
    // 0051d708  894618                 -mov dword ptr [esi + 0x18], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 0051d70b  83fa01                 +cmp edx, 1
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
    // 0051d70e  7c15                   -jl 0x51d725
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d725;
    }
    // 0051d710  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d712  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051d715  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d716  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d718  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051d719  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051d71b  b898115500             -mov eax, 0x551198
    cpu.eax = 5575064 /*0x551198*/;
    // 0051d720  e85b4affff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051d725:
    // 0051d725  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051d727  0f84c9000000           -je 0x51d7f6
    if (cpu.flags.zf)
    {
        goto L_0x0051d7f6;
    }
    // 0051d72d  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051d732  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051d734  83c42c                 -add esp, 0x2c
    (cpu.esp) += x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 0051d737  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d738  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d739  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d73a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051d73b:
    // 0051d73b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d73d  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051d740  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d741  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d743  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051d745  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051d747  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051d749  b85c115500             -mov eax, 0x55115c
    cpu.eax = 5575004 /*0x55115c*/;
    // 0051d74e  e82d4affff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d753  e9d1feffff             -jmp 0x51d629
    goto L_0x0051d629;
L_0x0051d758:
    // 0051d758  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051d75d  e87e21fcff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d762  e9e1feffff             -jmp 0x51d648
    goto L_0x0051d648;
L_0x0051d767:
    // 0051d767  89ea                   -mov edx, ebp
    cpu.edx = cpu.ebp;
    // 0051d769  e9ecfeffff             -jmp 0x51d65a
    goto L_0x0051d65a;
L_0x0051d76e:
    // 0051d76e  837c242400             +cmp dword ptr [esp + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d773  0f8531ffffff           -jne 0x51d6aa
    if (!cpu.flags.zf)
    {
        goto L_0x0051d6aa;
    }
    // 0051d779  e962ffffff             -jmp 0x51d6e0
    goto L_0x0051d6e0;
L_0x0051d77e:
    // 0051d77e  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d785  0f8c55ffffff           -jl 0x51d6e0
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d6e0;
    }
    // 0051d78b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d78d  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051d790  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d791  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d793  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051d795  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051d797  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051d799  b888115500             -mov eax, 0x551188
    cpu.eax = 5575048 /*0x551188*/;
    // 0051d79e  e8dd49ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d7a3  e938ffffff             -jmp 0x51d6e0
    goto L_0x0051d6e0;
L_0x0051d7a8:
    // 0051d7a8  833dec6d560001         +cmp dword ptr [0x566dec], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d7af  7c18                   -jl 0x51d7c9
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d7c9;
    }
    // 0051d7b1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d7b3  8a470d                 -mov al, byte ptr [edi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(13) /* 0xd */);
    // 0051d7b6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d7b7  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d7b9  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051d7bb  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051d7bd  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051d7bf  b8ac115500             -mov eax, 0x5511ac
    cpu.eax = 5575084 /*0x5511ac*/;
    // 0051d7c4  e8b749ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051d7c9:
    // 0051d7c9  b828269f00             -mov eax, 0x9f2628
    cpu.eax = 10429992 /*0x9f2628*/;
    // 0051d7ce  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051d7d0  e87b09ffff             -call 0x50e150
    cpu.esp -= 4;
    nfs3hp::ea::qremove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d7d5  b80c269f00             -mov eax, 0x9f260c
    cpu.eax = 10429964 /*0x9f260c*/;
    // 0051d7da  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051d7dc  e86f09ffff             -call 0x50e150
    cpu.esp -= 4;
    nfs3hp::ea::qremove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d7e1  837c240c00             +cmp dword ptr [esp + 0xc], 0
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
    // 0051d7e6  740e                   -je 0x51d7f6
    if (cpu.flags.zf)
    {
        goto L_0x0051d7f6;
    }
    // 0051d7e8  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051d7ea  e8a140fcff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d7ef  c7461000000000         -mov dword ptr [esi + 0x10], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */) = 0 /*0x0*/;
L_0x0051d7f6:
    // 0051d7f6  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051d7f8  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051d7fa  83c42c                 -add esp, 0x2c
    (cpu.esp) += x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 0051d7fd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d7fe  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d7ff  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d800  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_51d810(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (commSendTrace(app, cpu.sync())) /* port: native (tools/apply_native_vertices.py) */
    {
        cpu.reload();
        return;
    }
    // 0051d810  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051d811  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051d812  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051d813  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051d814  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 0051d816  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d818  7406                   -je 0x51d820
    if (cpu.flags.zf)
    {
        goto L_0x0051d820;
    }
    // 0051d81a  83782400               +cmp dword ptr [eax + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d81e  7507                   -jne 0x51d827
    if (!cpu.flags.zf)
    {
        goto L_0x0051d827;
    }
L_0x0051d820:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d820  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d822  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d823  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d824  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d825  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d826  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051d827:
    // 0051d827  8b7010                 -mov esi, dword ptr [eax + 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0051d82a  8d4638                 -lea eax, [esi + 0x38]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(56) /* 0x38 */);
    // 0051d82d  e82e07ffff             -call 0x50df60
    cpu.esp -= 4;
    sub_50df60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d832  8b6e08                 -mov ebp, dword ptr [esi + 8]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 0051d835  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051d837  39e9                   +cmp ecx, ebp
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d839  762d                   -jbe 0x51d868
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051d868;
    }
    // 0051d83b  b83c105500             -mov eax, 0x55103c
    cpu.eax = 5574716 /*0x55103c*/;
    // 0051d840  bbc0115500             -mov ebx, 0x5511c0
    cpu.ebx = 5575104 /*0x5511c0*/;
    // 0051d845  bdba020000             -mov ebp, 0x2ba
    cpu.ebp = 698 /*0x2ba*/;
    // 0051d84a  68d4115500             -push 0x5511d4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5575124 /*0x5511d4*/;
    cpu.esp -= 4;
    // 0051d84f  a390215500             -mov dword ptr [0x552190], eax
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.eax;
    // 0051d854  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 0051d85a  892d98215500           -mov dword ptr [0x552198], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebp;
    // 0051d860  e8ab37eeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d865  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x0051d868:
    // 0051d868  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051d86a  7526                   -jne 0x51d892
    if (!cpu.flags.zf)
    {
        goto L_0x0051d892;
    }
    // 0051d86c  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d873  7cab                   -jl 0x51d820
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d820;
    }
    // 0051d875  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d877  8a460d                 -mov al, byte ptr [esi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051d87a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d87b  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 0051d87d  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051d87f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051d881  b85c125500             -mov eax, 0x55125c
    cpu.eax = 5575260 /*0x55125c*/;
    // 0051d886  e8f548ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d88b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d88d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d88e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d88f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d890  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d891  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051d892:
    // 0051d892  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 0051d894  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051d896  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051d898  e8f3efffff             -call 0x51c890
    cpu.esp -= 4;
    sub_51c890(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d89d  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051d89f  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051d8a1  e84af0ffff             -call 0x51c8f0
    cpu.esp -= 4;
    sub_51c8f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d8a6  8d4654                 -lea eax, [esi + 0x54]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(84) /* 0x54 */);
    // 0051d8a9  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0051d8ab  e8f007ffff             -call 0x50e0a0
    cpu.esp -= 4;
    sub_50e0a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d8b0  837e3000               +cmp dword ptr [esi + 0x30], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(48) /* 0x30 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d8b4  752e                   -jne 0x51d8e4
    if (!cpu.flags.zf)
    {
        goto L_0x0051d8e4;
    }
    // 0051d8b6  b93c105500             -mov ecx, 0x55103c
    cpu.ecx = 5574716 /*0x55103c*/;
    // 0051d8bb  bbc0115500             -mov ebx, 0x5511c0
    cpu.ebx = 5575104 /*0x5511c0*/;
    // 0051d8c0  bec6020000             -mov esi, 0x2c6
    cpu.esi = 710 /*0x2c6*/;
    // 0051d8c5  6808125500             -push 0x551208
    app->getMemory<x86::reg32>(cpu.esp-4) = 5575176 /*0x551208*/;
    cpu.esp -= 4;
    // 0051d8ca  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 0051d8d0  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 0051d8d6  893598215500           -mov dword ptr [0x552198], esi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.esi;
    // 0051d8dc  e82f37eeff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d8e1  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x0051d8e4:
    // 0051d8e4  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051d8e9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d8ea  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d8eb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d8ec  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d8ed  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51d8f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051d8f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051d8f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051d8f2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051d8f3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051d8f4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051d8f5  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051d8f7  8b15ec6d5600           -mov edx, dword ptr [0x566dec]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
    // 0051d8fd  8b7010                 -mov esi, dword ptr [eax + 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0051d900  83fa05                 +cmp edx, 5
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
    // 0051d903  7d6a                   -jge 0x51d96f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051d96f;
    }
L_0x0051d905:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d905  ff4f24                 -dec dword ptr [edi + 0x24]
    (app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */))--;
    // 0051d908  8b5f24                 -mov ebx, dword ptr [edi + 0x24]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */);
    // 0051d90b  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051d90d  7526                   -jne 0x51d935
    if (!cpu.flags.zf)
    {
        goto L_0x0051d935;
    }
    // 0051d90f  833dec6d560003         +cmp dword ptr [0x566dec], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d916  7c15                   -jl 0x51d92d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d92d;
    }
    // 0051d918  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d91a  8a460d                 -mov al, byte ptr [esi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051d91d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d91e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d920  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051d921  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051d923  b874125500             -mov eax, 0x551274
    cpu.eax = 5575284 /*0x551274*/;
    // 0051d928  e85348ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051d92d:
    // 0051d92d  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051d92f  ff571c                 -call dword ptr [edi + 0x1c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(28) /* 0x1c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d932  894724                 -mov dword ptr [edi + 0x24], eax
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */) = cpu.eax;
L_0x0051d935:
    // 0051d935  837f2400               +cmp dword ptr [edi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d939  7551                   -jne 0x51d98c
    if (!cpu.flags.zf)
    {
        goto L_0x0051d98c;
    }
L_0x0051d93b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d93b  833dec6d560002         +cmp dword ptr [0x566dec], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051d942  7c19                   -jl 0x51d95d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051d95d;
    }
    // 0051d944  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d946  8a460d                 -mov al, byte ptr [esi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051d949  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d94a  8d5724                 -lea edx, [edi + 0x24]
    cpu.edx = x86::reg32(cpu.edi + x86::reg32(36) /* 0x24 */);
    // 0051d94d  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d94f  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 0051d951  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051d953  b880125500             -mov eax, 0x551280
    cpu.eax = 5575296 /*0x551280*/;
    // 0051d958  e82348ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051d95d:
    // 0051d95d  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051d95f  e81cf3ffff             -call 0x51cc80
    cpu.esp -= 4;
    sub_51cc80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051d964:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051d964  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051d969  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d96a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d96b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d96c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d96d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d96e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051d96f:
    // 0051d96f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051d971  8a460d                 -mov al, byte ptr [esi + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0051d974  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051d975  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051d977  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051d979  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051d97b  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051d97d  b868125500             -mov eax, 0x551268
    cpu.eax = 5575272 /*0x551268*/;
    // 0051d982  e8f947ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d987  e979ffffff             -jmp 0x51d905
    goto L_0x0051d905;
L_0x0051d98c:
    // 0051d98c  8b5604                 -mov edx, dword ptr [esi + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0051d98f  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051d991  ff5208                 -call dword ptr [edx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d994  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d996  74a3                   -je 0x51d93b
    if (cpu.flags.zf)
    {
        goto L_0x0051d93b;
    }
    // 0051d998  8d7e54                 -lea edi, [esi + 0x54]
    cpu.edi = x86::reg32(cpu.esi + x86::reg32(84) /* 0x54 */);
    // 0051d99b  ba10d05100             -mov edx, 0x51d010
    cpu.edx = 5361680 /*0x51d010*/;
    // 0051d9a0  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0051d9a2  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051d9a4  e80710ffff             -call 0x50e9b0
    cpu.esp -= 4;
    sub_50e9b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d9a9  ba40d05100             -mov edx, 0x51d040
    cpu.edx = 5361728 /*0x51d040*/;
    // 0051d9ae  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0051d9b0  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051d9b2  e8290fffff             -call 0x50e8e0
    cpu.esp -= 4;
    sub_50e8e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d9b7  8b462c                 -mov eax, dword ptr [esi + 0x2c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(44) /* 0x2c */);
    // 0051d9ba  8d50ff                 -lea edx, [eax - 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(-1) /* -0x1 */);
    // 0051d9bd  89562c                 -mov dword ptr [esi + 0x2c], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(44) /* 0x2c */) = cpu.edx;
    // 0051d9c0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d9c2  7fa0                   -jg 0x51d964
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0051d964;
    }
    // 0051d9c4  bbfb000000             -mov ebx, 0xfb
    cpu.ebx = 251 /*0xfb*/;
    // 0051d9c9  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051d9cb  8b4618                 -mov eax, dword ptr [esi + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0051d9ce  e87df0ffff             -call 0x51ca50
    cpu.esp -= 4;
    sub_51ca50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051d9d3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051d9d8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d9d9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d9da  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d9db  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d9dc  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051d9dd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51d9e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051d9e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051d9e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051d9e2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051d9e3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051d9e4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051d9e5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051d9e6  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051d9e8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051d9ea  0f84b8000000           -je 0x51daa8
    if (cpu.flags.zf)
    {
        goto L_0x0051daa8;
    }
    // 0051d9f0  8b5010                 -mov edx, dword ptr [eax + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0051d9f3  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051d9f5  0f84ad000000           -je 0x51daa8
    if (cpu.flags.zf)
    {
        goto L_0x0051daa8;
    }
    // 0051d9fb  8b0dec6d5600           -mov ecx, dword ptr [0x566dec]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5664236) /* 0x566dec */);
    // 0051da01  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0051da03  8b6a04                 -mov ebp, dword ptr [edx + 4]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 0051da06  83f901                 +cmp ecx, 1
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
    // 0051da09  7d4c                   -jge 0x51da57
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051da57;
    }
L_0x0051da0b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051da0b  b828269f00             -mov eax, 0x9f2628
    cpu.eax = 10429992 /*0x9f2628*/;
    // 0051da10  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0051da12  e83907ffff             -call 0x50e150
    cpu.esp -= 4;
    nfs3hp::ea::qremove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051da17  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051da19  0f8492000000           -je 0x51dab1
    if (cpu.flags.zf)
    {
        goto L_0x0051dab1;
    }
    // 0051da1f  837f2400               +cmp dword ptr [edi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051da23  7467                   -je 0x51da8c
    if (cpu.flags.zf)
    {
        goto L_0x0051da8c;
    }
    // 0051da25  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 0051da2a  c7472400000000         -mov dword ptr [edi + 0x24], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
L_0x0051da31:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051da31  49                     -dec ecx
    (cpu.ecx)--;
    // 0051da32  83f9ff                 +cmp ecx, -1
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051da35  7446                   -je 0x51da7d
    if (cpu.flags.zf)
    {
        goto L_0x0051da7d;
    }
    // 0051da37  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051da39  ff5508                 -call dword ptr [ebp + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051da3c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051da3e  743d                   -je 0x51da7d
    if (cpu.flags.zf)
    {
        goto L_0x0051da7d;
    }
    // 0051da40  bbff000000             -mov ebx, 0xff
    cpu.ebx = 255 /*0xff*/;
    // 0051da45  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0051da47  8b4618                 -mov eax, dword ptr [esi + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0051da4a  e801f0ffff             -call 0x51ca50
    cpu.esp -= 4;
    sub_51ca50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051da4f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051da51  741e                   -je 0x51da71
    if (cpu.flags.zf)
    {
        goto L_0x0051da71;
    }
    // 0051da53  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 0051da55  ebda                   -jmp 0x51da31
    goto L_0x0051da31;
L_0x0051da57:
    // 0051da57  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051da59  8a420d                 -mov al, byte ptr [edx + 0xd]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(13) /* 0xd */);
    // 0051da5c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051da5d  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051da5f  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051da61  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051da63  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0051da65  b88c125500             -mov eax, 0x55128c
    cpu.eax = 5575308 /*0x55128c*/;
    // 0051da6a  e81147ffff             -call 0x512180
    cpu.esp -= 4;
    sub_512180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051da6f  eb9a                   -jmp 0x51da0b
    goto L_0x0051da0b;
L_0x0051da71:
    // 0051da71  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051da76  e8652cfdff             -call 0x4f06e0
    cpu.esp -= 4;
    sub_4f06e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051da7b  ebb4                   -jmp 0x51da31
    goto L_0x0051da31;
L_0x0051da7d:
    // 0051da7d  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 0051da82  e8592cfdff             -call 0x4f06e0
    cpu.esp -= 4;
    sub_4f06e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051da87  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051da89  ff5504                 -call dword ptr [ebp + 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051da8c:
    // 0051da8c  b80c269f00             -mov eax, 0x9f260c
    cpu.eax = 10429964 /*0x9f260c*/;
    // 0051da91  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0051da93  e8b803ffff             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051da98:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051da98  b80c269f00             -mov eax, 0x9f260c
    cpu.eax = 10429964 /*0x9f260c*/;
    // 0051da9d  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0051da9f  e8ac06ffff             -call 0x50e150
    cpu.esp -= 4;
    nfs3hp::ea::qremove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051daa4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051daa6  751e                   -jne 0x51dac6
    if (!cpu.flags.zf)
    {
        goto L_0x0051dac6;
    }
L_0x0051daa8:
    // 0051daa8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051daaa  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051daab  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051daac  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051daad  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051daae  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051daaf  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051dab0  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051dab1:
    // 0051dab1  837f2400               +cmp dword ptr [edi + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051dab5  74e1                   -je 0x51da98
    if (cpu.flags.zf)
    {
        goto L_0x0051da98;
    }
    // 0051dab7  689c125500             -push 0x55129c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5575324 /*0x55129c*/;
    cpu.esp -= 4;
    // 0051dabc  e82f2efcff             -call 0x4e08f0
    cpu.esp -= 4;
    nfs3hp::ea::monoPrintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051dac1  83c404                 +add esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051dac4  ebd2                   -jmp 0x51da98
    goto L_0x0051da98;
L_0x0051dac6:
    // 0051dac6  8d4638                 -lea eax, [esi + 0x38]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(56) /* 0x38 */);
    // 0051dac9  e81203ffff             -call 0x50dde0
    cpu.esp -= 4;
    nfs3hp::ea::qfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051dace  8d4654                 -lea eax, [esi + 0x54]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(84) /* 0x54 */);
    // 0051dad1  e80a03ffff             -call 0x50dde0
    cpu.esp -= 4;
    nfs3hp::ea::qfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051dad6  8d4678                 -lea eax, [esi + 0x78]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(120) /* 0x78 */);
    // 0051dad9  e80203ffff             -call 0x50dde0
    cpu.esp -= 4;
    nfs3hp::ea::qfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051dade  8d8694000000           -lea eax, [esi + 0x94]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(148) /* 0x94 */);
    // 0051dae4  e8f702ffff             -call 0x50dde0
    cpu.esp -= 4;
    nfs3hp::ea::qfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051dae9  8b4614                 -mov eax, dword ptr [esi + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 0051daec  e86fd8fcff             -call 0x4eb360
    cpu.esp -= 4;
    nfs3hp::ea::freeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051daf1  807e0c00               +cmp byte ptr [esi + 0xc], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(12) /* 0xc */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051daf5  740e                   -je 0x51db05
    if (cpu.flags.zf)
    {
        goto L_0x0051db05;
    }
    // 0051daf7  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051daf9  e8923dfcff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051dafe  c7471000000000         -mov dword ptr [edi + 0x10], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */) = 0 /*0x0*/;
L_0x0051db05:
    // 0051db05  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051db0a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051db0b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051db0c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051db0d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051db0e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051db0f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051db10  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51db20: native, nfs3hp::dead::stub_0x51db20 */

/* sub_51dd50: native, nfs3hp::dead::stub_0x51dd50 */

/* sub_51ddf0: native, nfs3hp::dead::stub_0x51ddf0 */

}
