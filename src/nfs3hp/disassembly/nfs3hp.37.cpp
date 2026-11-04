#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_native_movie.py): defined in native_movie.cpp.
bool madMacroblock(win32::WinApplication* app, x86::CPU& cpu);
// Port (tools/apply_native_movie.py): defined in native_movie.cpp.
bool madBeginFrame(win32::WinApplication* app, x86::CPU& cpu);

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4f0880(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0880  d1fb                   +sar ebx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 004f0882  e9e52c0100             -jmp 0x50356c
    return nfs3hp::dead::stub_0x50356c(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f0890(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0890  01db                   +add ebx, ebx
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
    // 004f0892  e9d52c0100             -jmp 0x50356c
    return nfs3hp::dead::stub_0x50356c(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f08a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f08a0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f08a1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f08a3  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 004f08aa  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004f08ac  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004f08ae  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f08b0  e8b72c0100             -call 0x50356c
    cpu.esp -= 4;
    nfs3hp::dead::stub_0x50356c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f08b5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f08b6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f08c0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f08c0  c1e302                 +shl ebx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004f08c3  e9a42c0100             -jmp 0x50356c
    return nfs3hp::dead::stub_0x50356c(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4f08d0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f08d0  d1fb                   +sar ebx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 004f08d2  e989cc0100             -jmp 0x50d560
    return sub_50d560(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f08e0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f08e0  01db                   +add ebx, ebx
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
    // 004f08e2  e979cc0100             -jmp 0x50d560
    return sub_50d560(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f08f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f08f0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f08f1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f08f3  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 004f08fa  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004f08fc  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004f08fe  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f0900  e85bcc0100             -call 0x50d560
    cpu.esp -= 4;
    sub_50d560(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0905  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0906  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f0910(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0910  c1e302                 +shl ebx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004f0913  e948cc0100             -jmp 0x50d560
    return sub_50d560(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4f0920(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0920  d1fb                   +sar ebx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 004f0922  e959290100             -jmp 0x503280
    return nfs3hp::dead::stub_0x503280(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f0930(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0930  01db                   +add ebx, ebx
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
    // 004f0932  e949290100             -jmp 0x503280
    return nfs3hp::dead::stub_0x503280(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f0940(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0940  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f0941  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f0943  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 004f094a  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004f094c  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004f094e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f0950  e82b290100             -call 0x503280
    cpu.esp -= 4;
    nfs3hp::dead::stub_0x503280(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0955  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0956  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f0960(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0960  c1e302                 +shl ebx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004f0963  e918290100             -jmp 0x503280
    return nfs3hp::dead::stub_0x503280(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4f0970(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0970  d1fb                   +sar ebx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 004f0972  e96d2a0100             -jmp 0x5033e4
    return nfs3hp::mix::copySwapped(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f0980(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0980  01db                   +add ebx, ebx
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
    // 004f0982  e95d2a0100             -jmp 0x5033e4
    return nfs3hp::mix::copySwapped(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f0990(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0990  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f0991  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f0993  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 004f099a  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004f099c  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004f099e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f09a0  e83f2a0100             -call 0x5033e4
    cpu.esp -= 4;
    nfs3hp::mix::copySwapped(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f09a5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f09a6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4f09b0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f09b0  c1e302                 +shl ebx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004f09b3  e92c2a0100             -jmp 0x5033e4
    return nfs3hp::mix::copySwapped(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4f09c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f09c0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f09c1  833d7052560000         +cmp dword ptr [0x565270], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5657200) /* 0x565270 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f09c8  7d13                   -jge 0x4f09dd
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004f09dd;
    }
    // 004f09ca  833db443560000         +cmp dword ptr [0x5643b4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653428) /* 0x5643b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f09d1  7427                   -je 0x4f09fa
    if (cpu.flags.zf)
    {
        goto L_0x004f09fa;
    }
    // 004f09d3  c7057052560003000000   -mov dword ptr [0x565270], 3
    app->getMemory<x86::reg32>(x86::reg32(5657200) /* 0x565270 */) = 3 /*0x3*/;
L_0x004f09dd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f09dd  8b0d70525600           -mov ecx, dword ptr [0x565270]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5657200) /* 0x565270 */);
    // 004f09e3  8b0c8d74525600         -mov ecx, dword ptr [ecx*4 + 0x565274]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5657204) /* 0x565274 */ + cpu.ecx * 4);
    // 004f09ea  e8c1a60100             -call 0x50b0b0
    cpu.esp -= 4;
    sub_50b0b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f09ef  833d7052560003         +cmp dword ptr [0x565270], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5657200) /* 0x565270 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f09f6  740e                   -je 0x4f0a06
    if (cpu.flags.zf)
    {
        goto L_0x004f0a06;
    }
    // 004f09f8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f09f9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f09fa:
    // 004f09fa  c7057052560001000000   -mov dword ptr [0x565270], 1
    app->getMemory<x86::reg32>(x86::reg32(5657200) /* 0x565270 */) = 1 /*0x1*/;
    // 004f0a04  ebd7                   -jmp 0x4f09dd
    goto L_0x004f09dd;
L_0x004f0a06:
    // 004f0a06  0f77                   -emms 
    
    // 004f0a08  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0a09  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4f0a10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0a10  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f0a11  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f0a12  8b580c                 -mov ebx, dword ptr [eax + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004f0a15  8b500c                 -mov edx, dword ptr [eax + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004f0a18  c1e304                 -shl ebx, 4
    cpu.ebx <<= 4 /*0x4*/ % 32;
    // 004f0a1b  c1e214                 -shl edx, 0x14
    cpu.edx <<= 20 /*0x14*/ % 32;
    // 004f0a1e  c1fb14                 -sar ebx, 0x14
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (20 /*0x14*/ % 32));
    // 004f0a21  c1fa14                 -sar edx, 0x14
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (20 /*0x14*/ % 32));
    // 004f0a24  e897ffffff             -call 0x4f09c0
    cpu.esp -= 4;
    sub_4f09c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0a29  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0a2a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0a2b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4f0a30: native, nfs3hp::dead::stub_0x4f0a30 */

/* sub_4f0a50: native, nfs3hp::dead::stub_0x4f0a50 */

/* sub_4f0a70: native, nfs3hp::dead::stub_0x4f0a70 */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4f0a90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0a90  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f0a91  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f0a92  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004f0a94  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004f0a96  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004f0a98  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004f0a9a  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 004f0a9f  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004f0aa1  e8eae6ffff             -call 0x4ef190
    cpu.esp -= 4;
    sub_4ef190(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0aa6  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f0aa8  8b4028                 -mov eax, dword ptr [eax + 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(40) /* 0x28 */);
    // 004f0aab  8b5108                 -mov edx, dword ptr [ecx + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 004f0aae  0fafd0                 -imul edx, eax
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 004f0ab1  8b4120                 -mov eax, dword ptr [ecx + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */);
    // 004f0ab4  8b5c240c               -mov ebx, dword ptr [esp + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 004f0ab8  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004f0abb  e861fcfeff             -call 0x4e0721
    cpu.esp -= 4;
    nfs3hp::fillWord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0ac0  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f0ac2  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004f0ac4  8b4120                 -mov eax, dword ptr [ecx + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */);
    // 004f0ac7  e864d00100             -call 0x50db30
    cpu.esp -= 4;
    sub_50db30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0acc  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f0ace  e82df4ffff             -call 0x4eff00
    cpu.esp -= 4;
    sub_4eff00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0ad3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0ad4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0ad5  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* sub_4f0ae0: native, nfs3hp::watcom::tolower */

/* align: skip 0x00 0x00 */
void Application::sub_4f0af0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0af0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f0af1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f0af2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f0af3  8b0ddc0b9f00           -mov ecx, dword ptr [0x9f0bdc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10423260) /* 0x9f0bdc */);
    // 004f0af9  39c8                   +cmp eax, ecx
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
    // 004f0afb  7e4c                   -jle 0x4f0b49
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004f0b49;
    }
L_0x004f0afd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0afd  8b1dd80b9f00           -mov ebx, dword ptr [0x9f0bd8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10423256) /* 0x9f0bd8 */);
    // 004f0b03  890ddc0b9f00           -mov dword ptr [0x9f0bdc], ecx
    app->getMemory<x86::reg32>(x86::reg32(10423260) /* 0x9f0bdc */) = cpu.ecx;
    // 004f0b09  39da                   +cmp edx, ebx
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
    // 004f0b0b  7e40                   -jle 0x4f0b4d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004f0b4d;
    }
    // 004f0b0d  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
L_0x004f0b0f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0b0f  8b742414               -mov esi, dword ptr [esp + 0x14]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */);
    // 004f0b13  8b3dd00b9f00           -mov edi, dword ptr [0x9f0bd0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10423248) /* 0x9f0bd0 */);
    // 004f0b19  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004f0b1b  890dd80b9f00           -mov dword ptr [0x9f0bd8], ecx
    app->getMemory<x86::reg32>(x86::reg32(10423256) /* 0x9f0bd8 */) = cpu.ecx;
    // 004f0b21  39f8                   +cmp eax, edi
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
    // 004f0b23  7d02                   -jge 0x4f0b27
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004f0b27;
    }
    // 004f0b25  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
L_0x004f0b27:
    // 004f0b27  8b6c2418               -mov ebp, dword ptr [esp + 0x18]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(24) /* 0x18 */);
    // 004f0b2b  a3d00b9f00             -mov dword ptr [0x9f0bd0], eax
    app->getMemory<x86::reg32>(x86::reg32(10423248) /* 0x9f0bd0 */) = cpu.eax;
    // 004f0b30  a1d40b9f00             -mov eax, dword ptr [0x9f0bd4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423252) /* 0x9f0bd4 */);
    // 004f0b35  01ea                   -add edx, ebp
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebp));
    // 004f0b37  39c2                   +cmp edx, eax
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
    // 004f0b39  7d02                   -jge 0x4f0b3d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004f0b3d;
    }
    // 004f0b3b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
L_0x004f0b3d:
    // 004f0b3d  8915d40b9f00           -mov dword ptr [0x9f0bd4], edx
    app->getMemory<x86::reg32>(x86::reg32(10423252) /* 0x9f0bd4 */) = cpu.edx;
    // 004f0b43  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0b44  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0b45  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0b46  c21000                 -ret 0x10
    cpu.esp += 4+16 /*0x10*/;
    return;
L_0x004f0b49:
    // 004f0b49  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f0b4b  ebb0                   -jmp 0x4f0afd
    goto L_0x004f0afd;
L_0x004f0b4d:
    // 004f0b4d  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004f0b4f  ebbe                   -jmp 0x4f0b0f
    goto L_0x004f0b0f;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4f0b60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0b60  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f0b61  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f0b62  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f0b63  81ec10010000           -sub esp, 0x110
    (cpu.esp) -= x86::reg32(x86::sreg32(272 /*0x110*/));
    // 004f0b69  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004f0b6b  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004f0b6d  89dd                   -mov ebp, ebx
    cpu.ebp = cpu.ebx;
    // 004f0b6f  a1f8715600             -mov eax, dword ptr [0x5671f8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5665272) /* 0x5671f8 */);
    // 004f0b74  8984240c010000         -mov dword ptr [esp + 0x10c], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(268) /* 0x10c */) = cpu.eax;
    // 004f0b7b  a1fc715600             -mov eax, dword ptr [0x5671fc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5665276) /* 0x5671fc */);
    // 004f0b80  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f0b82  89842404010000         -mov dword ptr [esp + 0x104], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */) = cpu.eax;
    // 004f0b89  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f0b8b  e8c0fcffff             -call 0x4f0850
    cpu.esp -= 4;
    sub_4f0850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0b90  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f0b92  8915dc0b9f00           -mov dword ptr [0x9f0bdc], edx
    app->getMemory<x86::reg32>(x86::reg32(10423260) /* 0x9f0bdc */) = cpu.edx;
    // 004f0b98  8915d80b9f00           -mov dword ptr [0x9f0bd8], edx
    app->getMemory<x86::reg32>(x86::reg32(10423256) /* 0x9f0bd8 */) = cpu.edx;
    // 004f0b9e  8915d00b9f00           -mov dword ptr [0x9f0bd0], edx
    app->getMemory<x86::reg32>(x86::reg32(10423248) /* 0x9f0bd0 */) = cpu.edx;
    // 004f0ba4  8915d40b9f00           -mov dword ptr [0x9f0bd4], edx
    app->getMemory<x86::reg32>(x86::reg32(10423252) /* 0x9f0bd4 */) = cpu.edx;
    // 004f0baa  81fe00010000           +cmp esi, 0x100
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(256 /*0x100*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f0bb0  7e05                   -jle 0x4f0bb7
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004f0bb7;
    }
    // 004f0bb2  be00010000             -mov esi, 0x100
    cpu.esi = 256 /*0x100*/;
L_0x004f0bb7:
    // 004f0bb7  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f0bb9  30e4                   -xor ah, ah
    cpu.ah ^= x86::reg8(x86::sreg8(cpu.ah));
    // 004f0bbb  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004f0bbd  882434                 -mov byte ptr [esp + esi], ah
    app->getMemory<x86::reg8>(cpu.esp + cpu.esi * 1) = cpu.ah;
    // 004f0bc0  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 004f0bc2  bef00a4f00             -mov esi, 0x4f0af0
    cpu.esi = 5180144 /*0x4f0af0*/;
    // 004f0bc7  e86402ffff             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0bcc  8a1424                 -mov dl, byte ptr [esp]
    cpu.dl = app->getMemory<x86::reg8>(cpu.esp);
    // 004f0bcf  a188725600             -mov eax, dword ptr [0x567288]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5665416) /* 0x567288 */);
    // 004f0bd4  8b3d8c725600           -mov edi, dword ptr [0x56728c]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5665420) /* 0x56728c */);
    // 004f0bda  8b1d90725600           -mov ebx, dword ptr [0x567290]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5665424) /* 0x567290 */);
    // 004f0be0  89842408010000         -mov dword ptr [esp + 0x108], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(264) /* 0x108 */) = cpu.eax;
    // 004f0be7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f0be9  893588725600           -mov dword ptr [0x567288], esi
    app->getMemory<x86::reg32>(x86::reg32(5665416) /* 0x567288 */) = cpu.esi;
    // 004f0bef  a38c725600             -mov dword ptr [0x56728c], eax
    app->getMemory<x86::reg32>(x86::reg32(5665420) /* 0x56728c */) = cpu.eax;
    // 004f0bf4  a390725600             -mov dword ptr [0x567290], eax
    app->getMemory<x86::reg32>(x86::reg32(5665424) /* 0x567290 */) = cpu.eax;
    // 004f0bf9  84d2                   +test dl, dl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & cpu.dl));
    // 004f0bfb  7407                   -je 0x4f0c04
    if (cpu.flags.zf)
    {
        goto L_0x004f0c04;
    }
    // 004f0bfd  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 004f0bff  e84cefffff             -call 0x4efb50
    cpu.esp -= 4;
    sub_4efb50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f0c04:
    // 004f0c04  a1dc0b9f00             -mov eax, dword ptr [0x9f0bdc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423260) /* 0x9f0bdc */);
    // 004f0c09  894500                 -mov dword ptr [ebp], eax
    app->getMemory<x86::reg32>(cpu.ebp) = cpu.eax;
    // 004f0c0c  a1d80b9f00             -mov eax, dword ptr [0x9f0bd8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423256) /* 0x9f0bd8 */);
    // 004f0c11  8901                   -mov dword ptr [ecx], eax
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.eax;
    // 004f0c13  a1d00b9f00             -mov eax, dword ptr [0x9f0bd0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423248) /* 0x9f0bd0 */);
    // 004f0c18  8b0ddc0b9f00           -mov ecx, dword ptr [0x9f0bdc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10423260) /* 0x9f0bdc */);
    // 004f0c1e  8b35f8715600           -mov esi, dword ptr [0x5671f8]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5665272) /* 0x5671f8 */);
    // 004f0c24  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004f0c26  39f0                   +cmp eax, esi
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
    // 004f0c28  7d02                   -jge 0x4f0c2c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004f0c2c;
    }
    // 004f0c2a  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
L_0x004f0c2c:
    // 004f0c2c  8b8c2420010000         -mov ecx, dword ptr [esp + 0x120]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(288) /* 0x120 */);
    // 004f0c33  8b35d40b9f00           -mov esi, dword ptr [0x9f0bd4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10423252) /* 0x9f0bd4 */);
    // 004f0c39  8b2dd80b9f00           -mov ebp, dword ptr [0x9f0bd8]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(10423256) /* 0x9f0bd8 */);
    // 004f0c3f  8901                   -mov dword ptr [ecx], eax
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.eax;
    // 004f0c41  8b842424010000         -mov eax, dword ptr [esp + 0x124]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(292) /* 0x124 */);
    // 004f0c48  29ee                   -sub esi, ebp
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 004f0c4a  8b942404010000         -mov edx, dword ptr [esp + 0x104]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f0c51  8930                   -mov dword ptr [eax], esi
    app->getMemory<x86::reg32>(cpu.eax) = cpu.esi;
    // 004f0c53  8b84240c010000         -mov eax, dword ptr [esp + 0x10c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(268) /* 0x10c */);
    // 004f0c5a  e8f1fbffff             -call 0x4f0850
    cpu.esp -= 4;
    sub_4f0850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0c5f  8b842408010000         -mov eax, dword ptr [esp + 0x108]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(264) /* 0x108 */);
    // 004f0c66  893d8c725600           -mov dword ptr [0x56728c], edi
    app->getMemory<x86::reg32>(x86::reg32(5665420) /* 0x56728c */) = cpu.edi;
    // 004f0c6c  891d90725600           -mov dword ptr [0x567290], ebx
    app->getMemory<x86::reg32>(x86::reg32(5665424) /* 0x567290 */) = cpu.ebx;
    // 004f0c72  a388725600             -mov dword ptr [0x567288], eax
    app->getMemory<x86::reg32>(x86::reg32(5665416) /* 0x567288 */) = cpu.eax;
    // 004f0c77  81c410010000           -add esp, 0x110
    (cpu.esp) += x86::reg32(x86::sreg32(272 /*0x110*/));
    // 004f0c7d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0c7e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0c7f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0c80  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4f0c90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0c90  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f0c91  8b742408               -mov esi, dword ptr [esp + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 004f0c95  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f0c96  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f0c97  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 004f0c99  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 004f0c9b  ba00010000             -mov edx, 0x100
    cpu.edx = 256 /*0x100*/;
    // 004f0ca0  e8bbfeffff             -call 0x4f0b60
    cpu.esp -= 4;
    sub_4f0b60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0ca5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0ca6  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4f0cb0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0cb0  3b5004                 +cmp edx, dword ptr [eax + 4]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f0cb3  750b                   -jne 0x4f0cc0
    if (!cpu.flags.zf)
    {
    return sub_4f0cc0(app, cpu.handOver());
    }
    // 004f0cb5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004f0cba  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4f0cc0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0cc0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f0cc2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4f0cd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0cd0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f0cd1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f0cd2  8b15e00b9f00           -mov edx, dword ptr [0x9f0be0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0cd8  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004f0cda  7434                   -je 0x4f0d10
    if (cpu.flags.zf)
    {
        goto L_0x004f0d10;
    }
    // 004f0cdc  8b4a18                 -mov ecx, dword ptr [edx + 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(24) /* 0x18 */);
    // 004f0cdf  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004f0ce1  7407                   -je 0x4f0cea
    if (cpu.flags.zf)
    {
        goto L_0x004f0cea;
    }
    // 004f0ce3  c7412000000000         -mov dword ptr [ecx + 0x20], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */) = 0 /*0x0*/;
L_0x004f0cea:
    // 004f0cea  8b15e00b9f00           -mov edx, dword ptr [0x9f0be0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0cf0  8b12                   -mov edx, dword ptr [edx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f0cf2  b8e40b9f00             -mov eax, 0x9f0be4
    cpu.eax = 10423268 /*0x9f0be4*/;
    // 004f0cf7  e824eefeff             -call 0x4dfb20
    cpu.esp -= 4;
    nfs3hp::ea::postEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0cfc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004f0cfe:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0cfe  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0d03  3b10                   +cmp edx, dword ptr [eax]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f0d05  7409                   -je 0x4f0d10
    if (cpu.flags.zf)
    {
        goto L_0x004f0d10;
    }
    // 004f0d07  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004f0d09  7405                   -je 0x4f0d10
    if (cpu.flags.zf)
    {
        goto L_0x004f0d10;
    }
    // 004f0d0b  3b4818                 +cmp ecx, dword ptr [eax + 0x18]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f0d0e  7403                   -je 0x4f0d13
    if (cpu.flags.zf)
    {
        goto L_0x004f0d13;
    }
L_0x004f0d10:
    // 004f0d10  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0d11  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0d12  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f0d13:
    // 004f0d13  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004f0d15  e8c6ebfeff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0d1a  ebe2                   -jmp 0x4f0cfe
    goto L_0x004f0cfe;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4f0d20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0d20  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f0d21  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0d26  83c054                 -add eax, 0x54
    (cpu.eax) += x86::reg32(x86::sreg32(84 /*0x54*/));
    // 004f0d29  e8a2d40100             -call 0x50e1d0
    cpu.esp -= 4;
    nfs3hp::ea::qlength(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0d2e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f0d30  770f                   -ja 0x4f0d41
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004f0d41;
    }
L_0x004f0d32:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0d32  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f0d34  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0d35  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 004f0d3b  8d5200                 -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
    // 004f0d3e  8bdb                   -mov ebx, ebx
    cpu.ebx = cpu.ebx;
    // 004f0d40  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f0d41:
    // 004f0d41  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0d46  83c054                 -add eax, 0x54
    (cpu.eax) += x86::reg32(x86::sreg32(84 /*0x54*/));
    // 004f0d49  e8a2d20100             -call 0x50dff0
    cpu.esp -= 4;
    sub_50dff0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0d4e  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f0d50  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f0d52  74de                   -je 0x4f0d32
    if (cpu.flags.zf)
    {
        goto L_0x004f0d32;
    }
L_0x004f0d54:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0d54  8b5118                 -mov edx, dword ptr [ecx + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */);
    // 004f0d57  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004f0d59  740a                   -je 0x4f0d65
    if (cpu.flags.zf)
    {
        goto L_0x004f0d65;
    }
    // 004f0d5b  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0d60  3b5014                 +cmp edx, dword ptr [eax + 0x14]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f0d63  752c                   -jne 0x4f0d91
    if (!cpu.flags.zf)
    {
        goto L_0x004f0d91;
    }
L_0x004f0d65:
    // 004f0d65  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0d6a  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004f0d6c  83c01c                 -add eax, 0x1c
    (cpu.eax) += x86::reg32(x86::sreg32(28 /*0x1c*/));
L_0x004f0d6f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0d6f  e8dcd00100             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0d74  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 004f0d77  ff5124                 -call dword ptr [ecx + 0x24]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0d7a  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0d7f  83c054                 -add eax, 0x54
    (cpu.eax) += x86::reg32(x86::sreg32(84 /*0x54*/));
    // 004f0d82  e869d20100             -call 0x50dff0
    cpu.esp -= 4;
    sub_50dff0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0d87  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f0d89  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f0d8b  75c7                   -jne 0x4f0d54
    if (!cpu.flags.zf)
    {
        goto L_0x004f0d54;
    }
    // 004f0d8d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f0d8f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0d90  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f0d91:
    // 004f0d91  83c038                 +add eax, 0x38
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(56 /*0x38*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f0d94  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004f0d96  ebd7                   -jmp 0x4f0d6f
    goto L_0x004f0d6f;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4f0da0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0da0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f0da1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f0da2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f0da3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f0da4  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004f0da6  8d3400                 -lea esi, [eax + eax]
    cpu.esi = x86::reg32(cpu.eax + cpu.eax * 1);
    // 004f0da9  e8c2820000             -call 0x4f9070
    cpu.esp -= 4;
    nfs3hp::ea::surfaceLocked(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0dae  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f0db0  0f8504010000           -jne 0x4f0eba
    if (!cpu.flags.zf)
    {
        goto L_0x004f0eba;
    }
L_0x004f0db6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0db6  833de00b9f0000         +cmp dword ptr [0x9f0be0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f0dbd  0f852a010000           -jne 0x4f0eed
    if (!cpu.flags.zf)
    {
        goto L_0x004f0eed;
    }
L_0x004f0dc3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0dc3  c7059021550080c65400   -mov dword ptr [0x552190], 0x54c680
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = 5555840 /*0x54c680*/;
    // 004f0dcd  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004f0dd4  ba8cc65400             -mov edx, 0x54c68c
    cpu.edx = 5555852 /*0x54c68c*/;
    // 004f0dd9  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004f0ddb  b9c8000000             -mov ecx, 0xc8
    cpu.ecx = 200 /*0xc8*/;
    // 004f0de0  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004f0de3  890d98215500           -mov dword ptr [0x552198], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ecx;
    // 004f0de9  8d888c000000           -lea ecx, [eax + 0x8c]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(140) /* 0x8c */);
    // 004f0def  891594215500           -mov dword ptr [0x552194], edx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.edx;
    // 004f0df5  b8c8c65400             -mov eax, 0x54c6c8
    cpu.eax = 5555912 /*0x54c6c8*/;
    // 004f0dfa  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004f0dfc  e81f08ffff             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0e01  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004f0e03  a3e00b9f00             -mov dword ptr [0x9f0be0], eax
    app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */) = cpu.eax;
    // 004f0e08  e8fff8feff             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0e0d  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004f0e0f  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0e14  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f0e16  894014                 -mov dword ptr [eax + 0x14], eax
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 004f0e19  83c01c                 -add eax, 0x1c
    (cpu.eax) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 004f0e1c  e8afce0100             -call 0x50dcd0
    cpu.esp -= 4;
    sub_50dcd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0e21  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0e26  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004f0e28  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f0e2a  83c038                 -add eax, 0x38
    (cpu.eax) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 004f0e2d  e89ece0100             -call 0x50dcd0
    cpu.esp -= 4;
    sub_50dcd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0e32  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0e37  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004f0e39  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f0e3b  83c054                 -add eax, 0x54
    (cpu.eax) += x86::reg32(x86::sreg32(84 /*0x54*/));
    // 004f0e3e  e88dce0100             -call 0x50dcd0
    cpu.esp -= 4;
    sub_50dcd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0e43  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0e48  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004f0e4a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f0e4c  83c070                 -add eax, 0x70
    (cpu.eax) += x86::reg32(x86::sreg32(112 /*0x70*/));
    // 004f0e4f  e87cce0100             -call 0x50dcd0
    cpu.esp -= 4;
    sub_50dcd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0e54  8b0de00b9f00           -mov ecx, dword ptr [0x9f0be0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0e5a  81c18c000000           -add ecx, 0x8c
    (cpu.ecx) += x86::reg32(x86::sreg32(140 /*0x8c*/));
L_0x004f0e60:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0e60  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0e65  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004f0e67  83c01c                 -add eax, 0x1c
    (cpu.eax) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 004f0e6a  e8e1cf0100             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0e6f  83c128                 +add ecx, 0x28
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(40 /*0x28*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f0e72  4e                     +dec esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004f0e73  75eb                   -jne 0x4f0e60
    if (!cpu.flags.zf)
    {
        goto L_0x004f0e60;
    }
    // 004f0e75  bb03000000             -mov ebx, 3
    cpu.ebx = 3 /*0x3*/;
    // 004f0e7a  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 004f0e7f  b8200d4f00             -mov eax, 0x4f0d20
    cpu.eax = 5180704 /*0x4f0d20*/;
    // 004f0e84  b9ffffffff             -mov ecx, 0xffffffff
    cpu.ecx = 4294967295 /*0xffffffff*/;
    // 004f0e89  e89266ffff             -call 0x4e7520
    cpu.esp -= 4;
    nfs3hp::port::addSystemTask(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0e8e  68e40b9f00             -push 0x9f0be4
    app->getMemory<x86::reg32>(cpu.esp-4) = 10423268 /*0x9f0be4*/;
    cpu.esp -= 4;
    // 004f0e93  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004f0e98  b820184f00             -mov eax, 0x4f1820
    cpu.eax = 5183520 /*0x4f1820*/;
    // 004f0e9d  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f0e9f  e8fce8feff             -call 0x4df7a0
    cpu.esp -= 4;
    nfs3hp::ea::createThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0ea4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f0ea6  744f                   -je 0x4f0ef7
    if (cpu.flags.zf)
    {
        goto L_0x004f0ef7;
    }
L_0x004f0ea8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0ea8  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004f0eaa  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0eaf  8b28                   -mov ebp, dword ptr [eax]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.eax);
    // 004f0eb1  39e9                   +cmp ecx, ebp
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
    // 004f0eb3  7472                   -je 0x4f0f27
    if (cpu.flags.zf)
    {
        goto L_0x004f0f27;
    }
    // 004f0eb5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0eb6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0eb7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0eb8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0eb9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f0eba:
    // 004f0eba  ba80c65400             -mov edx, 0x54c680
    cpu.edx = 5555840 /*0x54c680*/;
    // 004f0ebf  b98cc65400             -mov ecx, 0x54c68c
    cpu.ecx = 5555852 /*0x54c68c*/;
    // 004f0ec4  bfc4000000             -mov edi, 0xc4
    cpu.edi = 196 /*0xc4*/;
    // 004f0ec9  6898c65400             -push 0x54c698
    app->getMemory<x86::reg32>(cpu.esp-4) = 5555864 /*0x54c698*/;
    cpu.esp -= 4;
    // 004f0ece  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 004f0ed4  890d94215500           -mov dword ptr [0x552194], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ecx;
    // 004f0eda  893d98215500           -mov dword ptr [0x552198], edi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edi;
    // 004f0ee0  e82b01f1ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0ee5  83c404                 +add esp, 4
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
    // 004f0ee8  e9c9feffff             -jmp 0x4f0db6
    goto L_0x004f0db6;
L_0x004f0eed:
    // 004f0eed  e84e000000             -call 0x4f0f40
    cpu.esp -= 4;
    sub_4f0f40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0ef2  e9ccfeffff             -jmp 0x4f0dc3
    goto L_0x004f0dc3;
L_0x004f0ef7:
    // 004f0ef7  bb80c65400             -mov ebx, 0x54c680
    cpu.ebx = 5555840 /*0x54c680*/;
    // 004f0efc  be8cc65400             -mov esi, 0x54c68c
    cpu.esi = 5555852 /*0x54c68c*/;
    // 004f0f01  bfde000000             -mov edi, 0xde
    cpu.edi = 222 /*0xde*/;
    // 004f0f06  68d4c65400             -push 0x54c6d4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5555924 /*0x54c6d4*/;
    cpu.esp -= 4;
    // 004f0f0b  891d90215500           -mov dword ptr [0x552190], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebx;
    // 004f0f11  893594215500           -mov dword ptr [0x552194], esi
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.esi;
    // 004f0f17  893d98215500           -mov dword ptr [0x552198], edi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edi;
    // 004f0f1d  e8ee00f1ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0f22  83c404                 +add esp, 4
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
    // 004f0f25  eb81                   -jmp 0x4f0ea8
    goto L_0x004f0ea8;
L_0x004f0f27:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0f27  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 004f0f29  e8b2e9feff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0f2e  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0f33  8b28                   -mov ebp, dword ptr [eax]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.eax);
    // 004f0f35  39e9                   +cmp ecx, ebp
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
    // 004f0f37  74ee                   -je 0x4f0f27
    if (cpu.flags.zf)
    {
        goto L_0x004f0f27;
    }
    // 004f0f39  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f3a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f3b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f3c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f3d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4f0f40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f0f40  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f0f41  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f0f42  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f0f43  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f0f44  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f0f45  e826810000             -call 0x4f9070
    cpu.esp -= 4;
    nfs3hp::ea::surfaceLocked(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0f4a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f0f4c  7510                   -jne 0x4f0f5e
    if (!cpu.flags.zf)
    {
        goto L_0x004f0f5e;
    }
    // 004f0f4e  8b35e00b9f00           -mov esi, dword ptr [0x9f0be0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0f54  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004f0f56  7544                   -jne 0x4f0f9c
    if (!cpu.flags.zf)
    {
        goto L_0x004f0f9c;
    }
    // 004f0f58  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f59  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f5a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f5b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f5c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f5d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f0f5e:
    // 004f0f5e  ba80c65400             -mov edx, 0x54c680
    cpu.edx = 5555840 /*0x54c680*/;
    // 004f0f63  b904c75400             -mov ecx, 0x54c704
    cpu.ecx = 5555972 /*0x54c704*/;
    // 004f0f68  bb08010000             -mov ebx, 0x108
    cpu.ebx = 264 /*0x108*/;
    // 004f0f6d  6810c75400             -push 0x54c710
    app->getMemory<x86::reg32>(cpu.esp-4) = 5555984 /*0x54c710*/;
    cpu.esp -= 4;
    // 004f0f72  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 004f0f78  890d94215500           -mov dword ptr [0x552194], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ecx;
    // 004f0f7e  891d98215500           -mov dword ptr [0x552198], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebx;
    // 004f0f84  e88700f1ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0f89  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f0f8c  8b35e00b9f00           -mov esi, dword ptr [0x9f0be0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0f92  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004f0f94  7506                   -jne 0x4f0f9c
    if (!cpu.flags.zf)
    {
        goto L_0x004f0f9c;
    }
    // 004f0f96  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f97  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f98  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f99  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f9a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f0f9b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f0f9c:
    // 004f0f9c  c7460401000000         -mov dword ptr [esi + 4], 1
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = 1 /*0x1*/;
    // 004f0fa3  e828fdffff             -call 0x4f0cd0
    cpu.esp -= 4;
    sub_4f0cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0fa8  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004f0faa:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0faa  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0faf  3b10                   +cmp edx, dword ptr [eax]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f0fb1  7409                   -je 0x4f0fbc
    if (cpu.flags.zf)
    {
        goto L_0x004f0fbc;
    }
    // 004f0fb3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004f0fb5  e826e9feff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0fba  ebee                   -jmp 0x4f0faa
    goto L_0x004f0faa;
L_0x004f0fbc:
    // 004f0fbc  83c070                 -add eax, 0x70
    (cpu.eax) += x86::reg32(x86::sreg32(112 /*0x70*/));
    // 004f0fbf  e89ccf0100             -call 0x50df60
    cpu.esp -= 4;
    sub_50df60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0fc4  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f0fc6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f0fc8  743a                   -je 0x4f1004
    if (cpu.flags.zf)
    {
        goto L_0x004f1004;
    }
L_0x004f0fca:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f0fca  8b6a1c                 -mov ebp, dword ptr [edx + 0x1c]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */);
    // 004f0fcd  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 004f0fcf  7407                   -je 0x4f0fd8
    if (cpu.flags.zf)
    {
        goto L_0x004f0fd8;
    }
    // 004f0fd1  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 004f0fd3  e8b808ffff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f0fd8:
    // 004f0fd8  8b4218                 -mov eax, dword ptr [edx + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(24) /* 0x18 */);
    // 004f0fdb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f0fdd  7405                   -je 0x4f0fe4
    if (cpu.flags.zf)
    {
        goto L_0x004f0fe4;
    }
    // 004f0fdf  e8ac08ffff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f0fe4:
    // 004f0fe4  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0fe9  83c01c                 -add eax, 0x1c
    (cpu.eax) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 004f0fec  e85fce0100             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0ff1  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f0ff6  83c070                 -add eax, 0x70
    (cpu.eax) += x86::reg32(x86::sreg32(112 /*0x70*/));
    // 004f0ff9  e862cf0100             -call 0x50df60
    cpu.esp -= 4;
    sub_50df60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f0ffe  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f1000  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f1002  75c6                   -jne 0x4f0fca
    if (!cpu.flags.zf)
    {
        goto L_0x004f0fca;
    }
L_0x004f1004:
    // 004f1004  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1009  83c038                 -add eax, 0x38
    (cpu.eax) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 004f100c  e84fcf0100             -call 0x50df60
    cpu.esp -= 4;
    sub_50df60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1011  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f1013  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f1015  743c                   -je 0x4f1053
    if (cpu.flags.zf)
    {
        goto L_0x004f1053;
    }
L_0x004f1017:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1017  8b4a1c                 -mov ecx, dword ptr [edx + 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */);
    // 004f101a  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004f101c  7407                   -je 0x4f1025
    if (cpu.flags.zf)
    {
        goto L_0x004f1025;
    }
    // 004f101e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f1020  e86b08ffff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f1025:
    // 004f1025  8b5a18                 -mov ebx, dword ptr [edx + 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(24) /* 0x18 */);
    // 004f1028  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004f102a  7407                   -je 0x4f1033
    if (cpu.flags.zf)
    {
        goto L_0x004f1033;
    }
    // 004f102c  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004f102e  e85d08ffff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f1033:
    // 004f1033  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1038  83c01c                 -add eax, 0x1c
    (cpu.eax) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 004f103b  e810ce0100             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1040  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1045  83c038                 -add eax, 0x38
    (cpu.eax) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 004f1048  e813cf0100             -call 0x50df60
    cpu.esp -= 4;
    sub_50df60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f104d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f104f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f1051  75c4                   -jne 0x4f1017
    if (!cpu.flags.zf)
    {
        goto L_0x004f1017;
    }
L_0x004f1053:
    // 004f1053  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1058  83c054                 -add eax, 0x54
    (cpu.eax) += x86::reg32(x86::sreg32(84 /*0x54*/));
    // 004f105b  e800cf0100             -call 0x50df60
    cpu.esp -= 4;
    sub_50df60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1060  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f1062  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f1064  743a                   -je 0x4f10a0
    if (cpu.flags.zf)
    {
        goto L_0x004f10a0;
    }
L_0x004f1066:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1066  8b6a1c                 -mov ebp, dword ptr [edx + 0x1c]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */);
    // 004f1069  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 004f106b  7407                   -je 0x4f1074
    if (cpu.flags.zf)
    {
        goto L_0x004f1074;
    }
    // 004f106d  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 004f106f  e81c08ffff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f1074:
    // 004f1074  8b4218                 -mov eax, dword ptr [edx + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(24) /* 0x18 */);
    // 004f1077  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f1079  7405                   -je 0x4f1080
    if (cpu.flags.zf)
    {
        goto L_0x004f1080;
    }
    // 004f107b  e81008ffff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f1080:
    // 004f1080  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1085  83c01c                 -add eax, 0x1c
    (cpu.eax) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 004f1088  e8c3cd0100             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f108d  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1092  83c054                 -add eax, 0x54
    (cpu.eax) += x86::reg32(x86::sreg32(84 /*0x54*/));
    // 004f1095  e8c6ce0100             -call 0x50df60
    cpu.esp -= 4;
    sub_50df60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f109a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f109c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f109e  75c6                   -jne 0x4f1066
    if (!cpu.flags.zf)
    {
        goto L_0x004f1066;
    }
L_0x004f10a0:
    // 004f10a0  b8200d4f00             -mov eax, 0x4f0d20
    cpu.eax = 5180704 /*0x4f0d20*/;
    // 004f10a5  e83665ffff             -call 0x4e75e0
    cpu.esp -= 4;
    nfs3hp::port::removeSystemTask(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f10aa  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f10af  83c01c                 -add eax, 0x1c
    (cpu.eax) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 004f10b2  e829cd0100             -call 0x50dde0
    cpu.esp -= 4;
    nfs3hp::ea::qfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f10b7  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f10bc  83c038                 -add eax, 0x38
    (cpu.eax) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 004f10bf  e81ccd0100             -call 0x50dde0
    cpu.esp -= 4;
    nfs3hp::ea::qfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f10c4  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f10c9  83c070                 -add eax, 0x70
    (cpu.eax) += x86::reg32(x86::sreg32(112 /*0x70*/));
    // 004f10cc  e80fcd0100             -call 0x50dde0
    cpu.esp -= 4;
    nfs3hp::ea::qfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f10d1  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f10d6  83c054                 -add eax, 0x54
    (cpu.eax) += x86::reg32(x86::sreg32(84 /*0x54*/));
    // 004f10d9  e802cd0100             -call 0x50dde0
    cpu.esp -= 4;
    nfs3hp::ea::qfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f10de  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f10e3  8b7008                 -mov esi, dword ptr [eax + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004f10e6  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004f10e8  7407                   -je 0x4f10f1
    if (cpu.flags.zf)
    {
        goto L_0x004f10f1;
    }
    // 004f10ea  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004f10ec  e8af1cffff             -call 0x4e2da0
    cpu.esp -= 4;
    nfs3hp::ea::closeHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f10f1:
    // 004f10f1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f10f2  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f10f7  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004f10f9  e89207ffff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f10fe  893de00b9f00           -mov dword ptr [0x9f0be0], edi
    app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */) = cpu.edi;
    // 004f1104  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f1105  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f1106  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f1107  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f1108  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f1109  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f110a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4f1110: native, nfs3hp::dead::stub_0x4f1110 */

/* sub_4f1230: native, nfs3hp::dead::stub_0x4f1230 */

/* sub_4f1240: native, nfs3hp::dead::stub_0x4f1240 */

/* sub_4f1360: native, nfs3hp::dead::stub_0x4f1360 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4f1370(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f1370  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f1371  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f1372  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f1373  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f1374  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f1377  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 004f1379  833de00b9f0000         +cmp dword ptr [0x9f0be0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f1380  752e                   -jne 0x4f13b0
    if (!cpu.flags.zf)
    {
        goto L_0x004f13b0;
    }
    // 004f1382  b980c65400             -mov ecx, 0x54c680
    cpu.ecx = 5555840 /*0x54c680*/;
    // 004f1387  bba4c75400             -mov ebx, 0x54c7a4
    cpu.ebx = 5556132 /*0x54c7a4*/;
    // 004f138c  be03020000             -mov esi, 0x203
    cpu.esi = 515 /*0x203*/;
    // 004f1391  68b4c75400             -push 0x54c7b4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5556148 /*0x54c7b4*/;
    cpu.esp -= 4;
    // 004f1396  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 004f139c  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 004f13a2  893598215500           -mov dword ptr [0x552198], esi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.esi;
    // 004f13a8  e863fcf0ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f13ad  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x004f13b0:
    // 004f13b0  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f13b5  83c01c                 -add eax, 0x1c
    (cpu.eax) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 004f13b8  e8a3cb0100             -call 0x50df60
    cpu.esp -= 4;
    sub_50df60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f13bd  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004f13bf  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 004f13c2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f13c4  746d                   -je 0x4f1433
    if (cpu.flags.zf)
    {
        goto L_0x004f1433;
    }
    // 004f13c6  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f13c7  c7460800000000         -mov dword ptr [esi + 8], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 004f13ce  c7460c00000000         -mov dword ptr [esi + 0xc], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */) = 0 /*0x0*/;
    // 004f13d5  c7461000000000         -mov dword ptr [esi + 0x10], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */) = 0 /*0x0*/;
    // 004f13dc  c7462002000000         -mov dword ptr [esi + 0x20], 2
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */) = 2 /*0x2*/;
    // 004f13e3  c7462400000000         -mov dword ptr [esi + 0x24], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */) = 0 /*0x0*/;
    // 004f13ea  a184525600             -mov eax, dword ptr [0x565284]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5657220) /* 0x565284 */);
    // 004f13ef  c7461800000000         -mov dword ptr [esi + 0x18], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */) = 0 /*0x0*/;
    // 004f13f6  8b15f4435600           -mov edx, dword ptr [0x5643f4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 004f13fc  894604                 -mov dword ptr [esi + 4], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 004f13ff  40                     -inc eax
    (cpu.eax)++;
    // 004f1400  895614                 -mov dword ptr [esi + 0x14], edx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */) = cpu.edx;
    // 004f1403  a384525600             -mov dword ptr [0x565284], eax
    app->getMemory<x86::reg32>(x86::reg32(5657220) /* 0x565284 */) = cpu.eax;
    // 004f1408  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 004f140a  752f                   -jne 0x4f143b
    if (!cpu.flags.zf)
    {
        goto L_0x004f143b;
    }
    // 004f140c  896e1c                 -mov dword ptr [esi + 0x1c], ebp
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(28) /* 0x1c */) = cpu.ebp;
L_0x004f140f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f140f  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1414  8b542404               -mov edx, dword ptr [esp + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 004f1418  83c070                 -add eax, 0x70
    (cpu.eax) += x86::reg32(x86::sreg32(112 /*0x70*/));
    // 004f141b  e830ca0100             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1420  8b15e00b9f00           -mov edx, dword ptr [0x9f0be0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1426  8b12                   -mov edx, dword ptr [edx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f1428  b8e40b9f00             -mov eax, 0x9f0be4
    cpu.eax = 10423268 /*0x9f0be4*/;
    // 004f142d  e8eee6feff             -call 0x4dfb20
    cpu.esp -= 4;
    nfs3hp::ea::postEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1432  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x004f1433:
    // 004f1433  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f1436  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f1437  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f1438  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f1439  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f143a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f143b:
    // 004f143b  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f143c  bf80c65400             -mov edi, 0x54c680
    cpu.edi = 5555840 /*0x54c680*/;
    // 004f1441  b8a4c75400             -mov eax, 0x54c7a4
    cpu.eax = 5556132 /*0x54c7a4*/;
    // 004f1446  ba14020000             -mov edx, 0x214
    cpu.edx = 532 /*0x214*/;
    // 004f144b  8b1df4435600           -mov ebx, dword ptr [0x5643f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 004f1451  893d90215500           -mov dword ptr [0x552190], edi
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edi;
    // 004f1457  a394215500             -mov dword ptr [0x552194], eax
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.eax;
    // 004f145c  89ef                   -mov edi, ebp
    cpu.edi = cpu.ebp;
    // 004f145e  891598215500           -mov dword ptr [0x552198], edx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edx;
    // 004f1464  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004f1466  49                     -dec ecx
    (cpu.ecx)--;
    // 004f1467  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004f1469  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004f146b  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004f146d  49                     -dec ecx
    (cpu.ecx)--;
    // 004f146e  b880c75400             -mov eax, 0x54c780
    cpu.eax = 5556096 /*0x54c780*/;
    // 004f1473  8d5101                 -lea edx, [ecx + 1]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(1) /* 0x1 */);
    // 004f1476  e8a501ffff             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f147b  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004f147d  89461c                 -mov dword ptr [esi + 0x1c], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(28) /* 0x1c */) = cpu.eax;
    // 004f1480  89ee                   -mov esi, ebp
    cpu.esi = cpu.ebp;
    // 004f1482  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004f1483:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1483  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004f1485  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004f1487  3c00                   +cmp al, 0
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
    // 004f1489  7410                   -je 0x4f149b
    if (cpu.flags.zf)
    {
        goto L_0x004f149b;
    }
    // 004f148b  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004f148e  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004f1491  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004f1494  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004f1497  3c00                   +cmp al, 0
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
    // 004f1499  75e8                   -jne 0x4f1483
    if (!cpu.flags.zf)
    {
        goto L_0x004f1483;
    }
L_0x004f149b:
    // 004f149b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f149c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f149d  e96dffffff             -jmp 0x4f140f
    goto L_0x004f140f;
}

/* sub_4f14b0: native, nfs3hp::dead::stub_0x4f14b0 */

/* sub_4f14f0: native, nfs3hp::dead::stub_0x4f14f0 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4f1500(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f1500  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f1501  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f1502  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f1503  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f1506  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 004f1509  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 004f150b  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 004f150d  833de00b9f0000         +cmp dword ptr [0x9f0be0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f1514  752d                   -jne 0x4f1543
    if (!cpu.flags.zf)
    {
        goto L_0x004f1543;
    }
    // 004f1516  be80c65400             -mov esi, 0x54c680
    cpu.esi = 5555840 /*0x54c680*/;
    // 004f151b  bd20c85400             -mov ebp, 0x54c820
    cpu.ebp = 5556256 /*0x54c820*/;
    // 004f1520  b87d020000             -mov eax, 0x27d
    cpu.eax = 637 /*0x27d*/;
    // 004f1525  683cc85400             -push 0x54c83c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5556284 /*0x54c83c*/;
    cpu.esp -= 4;
    // 004f152a  893590215500           -mov dword ptr [0x552190], esi
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.esi;
    // 004f1530  892d94215500           -mov dword ptr [0x552194], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebp;
    // 004f1536  a398215500             -mov dword ptr [0x552198], eax
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.eax;
    // 004f153b  e8d0faf0ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1540  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x004f1543:
    // 004f1543  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1548  83c01c                 -add eax, 0x1c
    (cpu.eax) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 004f154b  e810ca0100             -call 0x50df60
    cpu.esp -= 4;
    sub_50df60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1550  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f1552  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f1554  7507                   -jne 0x4f155d
    if (!cpu.flags.zf)
    {
        goto L_0x004f155d;
    }
    // 004f1556  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f1559  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f155a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f155b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f155c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f155d:
    // 004f155d  a184525600             -mov eax, dword ptr [0x565284]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5657220) /* 0x565284 */);
    // 004f1562  894104                 -mov dword ptr [ecx + 4], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 004f1565  40                     -inc eax
    (cpu.eax)++;
    // 004f1566  8b35e00b9f00           -mov esi, dword ptr [0x9f0be0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f156c  a384525600             -mov dword ptr [0x565284], eax
    app->getMemory<x86::reg32>(x86::reg32(5657220) /* 0x565284 */) = cpu.eax;
    // 004f1571  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004f1574  c7412003000000         -mov dword ptr [ecx + 0x20], 3
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */) = 3 /*0x3*/;
    // 004f157b  c7411800000000         -mov dword ptr [ecx + 0x18], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */) = 0 /*0x0*/;
    // 004f1582  c7411c00000000         -mov dword ptr [ecx + 0x1c], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */) = 0 /*0x0*/;
    // 004f1589  8b2c24                 -mov ebp, dword ptr [esp]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    // 004f158c  897910                 -mov dword ptr [ecx + 0x10], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */) = cpu.edi;
    // 004f158f  01e8                   -add eax, ebp
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebp));
    // 004f1591  895108                 -mov dword ptr [ecx + 8], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.edx;
    // 004f1594  89410c                 -mov dword ptr [ecx + 0xc], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 004f1597  a1f4435600             -mov eax, dword ptr [0x5643f4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 004f159c  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004f159e  894114                 -mov dword ptr [ecx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 004f15a1  8d4670                 -lea eax, [esi + 0x70]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(112) /* 0x70 */);
    // 004f15a4  895924                 -mov dword ptr [ecx + 0x24], ebx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */) = cpu.ebx;
    // 004f15a7  e8a4c80100             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f15ac  8b15e00b9f00           -mov edx, dword ptr [0x9f0be0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f15b2  8b12                   -mov edx, dword ptr [edx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f15b4  b8e40b9f00             -mov eax, 0x9f0be4
    cpu.eax = 10423268 /*0x9f0be4*/;
    // 004f15b9  e862e5feff             -call 0x4dfb20
    cpu.esp -= 4;
    nfs3hp::ea::postEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f15be  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 004f15c1  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f15c4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f15c5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f15c6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f15c7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4f15d0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f15d0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f15d1  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004f15d3  e828ffffff             -call 0x4f1500
    cpu.esp -= 4;
    sub_4f1500(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f15d8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f15d9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4f15e0: native, nfs3hp::dead::stub_0x4f15e0 */

/* sub_4f15f0: native, nfs3hp::dead::stub_0x4f15f0 */

/* sub_4f1610: native, nfs3hp::dead::stub_0x4f1610 */

/* sub_4f1710: native, nfs3hp::dead::stub_0x4f1710 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4f1790(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f1790  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f1791  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f1792  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f1793  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004f1795  833de00b9f0000         +cmp dword ptr [0x9f0be0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f179c  7532                   -jne 0x4f17d0
    if (!cpu.flags.zf)
    {
        goto L_0x004f17d0;
    }
    // 004f179e  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f179f  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f17a0  b980c65400             -mov ecx, 0x54c680
    cpu.ecx = 5555840 /*0x54c680*/;
    // 004f17a5  beecc85400             -mov esi, 0x54c8ec
    cpu.esi = 5556460 /*0x54c8ec*/;
    // 004f17aa  bf77030000             -mov edi, 0x377
    cpu.edi = 887 /*0x377*/;
    // 004f17af  6800c95400             -push 0x54c900
    app->getMemory<x86::reg32>(cpu.esp-4) = 5556480 /*0x54c900*/;
    cpu.esp -= 4;
    // 004f17b4  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 004f17ba  893594215500           -mov dword ptr [0x552194], esi
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.esi;
    // 004f17c0  893d98215500           -mov dword ptr [0x552198], edi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edi;
    // 004f17c6  e845f8f0ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f17cb  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f17ce  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f17cf  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x004f17d0:
    // 004f17d0  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f17d5  bab00c4f00             -mov edx, 0x4f0cb0
    cpu.edx = 5180592 /*0x4f0cb0*/;
    // 004f17da  83c038                 -add eax, 0x38
    (cpu.eax) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 004f17dd  e8becd0100             -call 0x50e5a0
    cpu.esp -= 4;
    sub_50e5a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f17e2  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f17e4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f17e6  741a                   -je 0x4f1802
    if (cpu.flags.zf)
    {
        goto L_0x004f1802;
    }
    // 004f17e8  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f17ed  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004f17ef  83c01c                 -add eax, 0x1c
    (cpu.eax) += x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 004f17f2  e859c60100             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f17f7  83792000               +cmp dword ptr [ecx + 0x20], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f17fb  7409                   -je 0x4f1806
    if (cpu.flags.zf)
    {
        goto L_0x004f1806;
    }
    // 004f17fd  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x004f1802:
    // 004f1802  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f1803  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f1804  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f1805  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f1806:
    // 004f1806  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 004f180b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f180c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f180d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f180e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4f1820(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 004f1820  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f1821  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f1822  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f1823  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f1824  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f1825  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f1826  81ec08010000           -sub esp, 0x108
    (cpu.esp) -= x86::reg32(x86::sreg32(264 /*0x108*/));
    // 004f182c  e8cfe2feff             -call 0x4dfb00
    cpu.esp -= 4;
    nfs3hp::ea::createEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1831  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f1833  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1838  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 004f183a  c7401800000000         -mov dword ptr [eax + 0x18], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = 0 /*0x0*/;
    // 004f1841  c7400400000000         -mov dword ptr [eax + 4], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = 0 /*0x0*/;
L_0x004f1848:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1848  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f184d  83780400               +cmp dword ptr [eax + 4], 0
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
    // 004f1851  743d                   -je 0x4f1890
    if (cpu.flags.zf)
    {
        goto L_0x004f1890;
    }
    // 004f1853  8b4808                 -mov ecx, dword ptr [eax + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004f1856  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004f1858  0f85c6030000           -jne 0x4f1c24
    if (!cpu.flags.zf)
    {
        goto L_0x004f1c24;
    }
L_0x004f185e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f185e  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1863  c7400800000000         -mov dword ptr [eax + 8], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 004f186a  c7401800000000         -mov dword ptr [eax + 0x18], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = 0 /*0x0*/;
    // 004f1871  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004f1873  e888e3feff             -call 0x4dfc00
    cpu.esp -= 4;
    nfs3hp::ea::closeEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1878  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f187d  c70000000000           -mov dword ptr [eax], 0
    app->getMemory<x86::reg32>(cpu.eax) = 0 /*0x0*/;
    // 004f1883  81c408010000           -add esp, 0x108
    (cpu.esp) += x86::reg32(x86::sreg32(264 /*0x108*/));
    // 004f1889  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f188a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f188b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f188c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f188d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f188e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f188f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f1890:
    // 004f1890  83c070                 -add eax, 0x70
    (cpu.eax) += x86::reg32(x86::sreg32(112 /*0x70*/));
    // 004f1893  e858c70100             -call 0x50dff0
    cpu.esp -= 4;
    sub_50dff0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1898  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 004f189a  89842404010000         -mov dword ptr [esp + 0x104], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */) = cpu.eax;
    // 004f18a1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f18a3  0f846a030000           -je 0x4f1c13
    if (cpu.flags.zf)
    {
        goto L_0x004f1c13;
    }
    // 004f18a9  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f18ae  896818                 -mov dword ptr [eax + 0x18], ebp
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = cpu.ebp;
    // 004f18b1  8b5520                 -mov edx, dword ptr [ebp + 0x20]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */);
    // 004f18b4  4a                     -dec edx
    (cpu.edx)--;
    // 004f18b5  83fa03                 +cmp edx, 3
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
    // 004f18b8  0f8722030000           -ja 0x4f1be0
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004f1be0;
    }
    // 004f18be  ff249510184f00         -jmp dword ptr [edx*4 + 0x4f1810]
    cpu.ip = app->getMemory<x86::reg32>(5183504 + cpu.edx * 4); goto dynamic_jump;
  case 0x004f18c5:
    // 004f18c5  89e7                   -mov edi, esp
    cpu.edi = cpu.esp;
    // 004f18c7  8b751c                 -mov esi, dword ptr [ebp + 0x1c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004f18ca  8d942400010000         -lea edx, [esp + 0x100]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 004f18d1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004f18d2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f18d2  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004f18d4  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004f18d6  3c00                   +cmp al, 0
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
    // 004f18d8  7410                   -je 0x4f18ea
    if (cpu.flags.zf)
    {
        goto L_0x004f18ea;
    }
    // 004f18da  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004f18dd  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004f18e0  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004f18e3  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004f18e6  3c00                   +cmp al, 0
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
    // 004f18e8  75e8                   -jne 0x4f18d2
    if (!cpu.flags.zf)
    {
        goto L_0x004f18d2;
    }
L_0x004f18ea:
    // 004f18ea  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f18eb  8b451c                 -mov eax, dword ptr [ebp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004f18ee  8d4d10                 -lea ecx, [ebp + 0x10]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004f18f1  e89afffeff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f18f6  8d5d0c                 -lea ebx, [ebp + 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 004f18f9  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 004f18fb  c7451c00000000         -mov dword ptr [ebp + 0x1c], 0
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */) = 0 /*0x0*/;
    // 004f1902  e8c90fffff             -call 0x4e28d0
    cpu.esp -= 4;
    nfs3hp::ea::openHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1907  c7451800000000         -mov dword ptr [ebp + 0x18], 0
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */) = 0 /*0x0*/;
    // 004f190e  8bbc2400010000         -mov edi, dword ptr [esp + 0x100]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 004f1915  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004f1917  0f8464010000           -je 0x4f1a81
    if (cpu.flags.zf)
    {
        goto L_0x004f1a81;
    }
    // 004f191d  8b4510                 -mov eax, dword ptr [ebp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004f1920  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f1922  b90e010000             -mov ecx, 0x10e
    cpu.ecx = 270 /*0x10e*/;
    // 004f1927  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004f192a  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004f192c  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f192e  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f1930  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f1932  e8e9de0000             -call 0x4ff820
    cpu.esp -= 4;
    nfs3hp::port::loadBlockSize(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1937  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004f1939  837d0800               +cmp dword ptr [ebp + 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f193d  0f852e010000           -jne 0x4f1a71
    if (!cpu.flags.zf)
    {
        goto L_0x004f1a71;
    }
    // 004f1943  ba80c65400             -mov edx, 0x54c680
    cpu.edx = 5555840 /*0x54c680*/;
    // 004f1948  bb30c95400             -mov ebx, 0x54c930
    cpu.ebx = 5556528 /*0x54c930*/;
    // 004f194d  bea7030000             -mov esi, 0x3a7
    cpu.esi = 935 /*0x3a7*/;
    // 004f1952  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 004f1954  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 004f195a  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 004f1960  8b5d14                 -mov ebx, dword ptr [ebp + 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004f1963  8b5510                 -mov edx, dword ptr [ebp + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004f1966  893598215500           -mov dword ptr [0x552198], esi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.esi;
    // 004f196c  e8affcfeff             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1971  894518                 -mov dword ptr [ebp + 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 004f1974  894508                 -mov dword ptr [ebp + 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */) = cpu.eax;
L_0x004f1977:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1977  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f1979  e8f2dc0000             -call 0x4ff670
    cpu.esp -= 4;
    nfs3hp::port::topUpStream(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f197e  e86dde0000             -call 0x4ff7f0
    cpu.esp -= 4;
    nfs3hp::port::waitTopupStream(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1983  8b842400010000         -mov eax, dword ptr [esp + 0x100]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 004f198a  8b942404010000         -mov edx, dword ptr [esp + 0x104]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1991  e80a27ffff             -call 0x4e40a0
    cpu.esp -= 4;
    nfs3hp::ea::lockHandleDrive(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1996  8b842400010000         -mov eax, dword ptr [esp + 0x100]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 004f199d  8b520c                 -mov edx, dword ptr [edx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    // 004f19a0  8b9c2404010000         -mov ebx, dword ptr [esp + 0x104]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f19a7  e8b414ffff             -call 0x4e2e60
    cpu.esp -= 4;
    nfs3hp::ea::seekHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f19ac  8b942404010000         -mov edx, dword ptr [esp + 0x104]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f19b3  8b842400010000         -mov eax, dword ptr [esp + 0x100]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 004f19ba  8b5b10                 -mov ebx, dword ptr [ebx + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */);
    // 004f19bd  8b5208                 -mov edx, dword ptr [edx + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 004f19c0  e81b14ffff             -call 0x4e2de0
    cpu.esp -= 4;
    nfs3hp::ea::readHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f19c5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f19c7  750e                   -jne 0x4f19d7
    if (!cpu.flags.zf)
    {
        goto L_0x004f19d7;
    }
    // 004f19c9  8b842404010000         -mov eax, dword ptr [esp + 0x104]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f19d0  c7402000000000         -mov dword ptr [eax + 0x20], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(32) /* 0x20 */) = 0 /*0x0*/;
L_0x004f19d7:
    // 004f19d7  8b842400010000         -mov eax, dword ptr [esp + 0x100]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 004f19de  e80d27ffff             -call 0x4e40f0
    cpu.esp -= 4;
    nfs3hp::ea::unlockHandleDrive(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f19e3  8b842400010000         -mov eax, dword ptr [esp + 0x100]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 004f19ea  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004f19ec  e8af13ffff             -call 0x4e2da0
    cpu.esp -= 4;
    nfs3hp::ea::closeHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f19f1  8b842404010000         -mov eax, dword ptr [esp + 0x104]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f19f8  89bc2400010000         -mov dword ptr [esp + 0x100], edi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */) = cpu.edi;
    // 004f19ff  83782000               +cmp dword ptr [eax + 0x20], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(32) /* 0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f1a03  7530                   -jne 0x4f1a35
    if (!cpu.flags.zf)
    {
        goto L_0x004f1a35;
    }
    // 004f1a05  8b942404010000         -mov edx, dword ptr [esp + 0x104]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1a0c  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1a11  8b5218                 -mov edx, dword ptr [edx + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(24) /* 0x18 */);
    // 004f1a14  3b5014                 +cmp edx, dword ptr [eax + 0x14]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f1a17  741c                   -je 0x4f1a35
    if (cpu.flags.zf)
    {
        goto L_0x004f1a35;
    }
    // 004f1a19  8b842404010000         -mov eax, dword ptr [esp + 0x104]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1a20  8b4018                 -mov eax, dword ptr [eax + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 004f1a23  e868fefeff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1a28  8b842404010000         -mov eax, dword ptr [esp + 0x104]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1a2f  897818                 -mov dword ptr [eax + 0x18], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = cpu.edi;
    // 004f1a32  897808                 -mov dword ptr [eax + 8], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.edi;
L_0x004f1a35:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1a35  8b842404010000         -mov eax, dword ptr [esp + 0x104]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1a3c  83782400               +cmp dword ptr [eax + 0x24], 0
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
    // 004f1a40  7444                   -je 0x4f1a86
    if (cpu.flags.zf)
    {
        goto L_0x004f1a86;
    }
L_0x004f1a42:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1a42  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1a47  8b942404010000         -mov edx, dword ptr [esp + 0x104]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1a4e  83c054                 +add eax, 0x54
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(84 /*0x54*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
L_0x004f1a51:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1a51  e8fac30100             -call 0x50de50
    cpu.esp -= 4;
    sub_50de50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f1a56:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1a56  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1a5b  c7401800000000         -mov dword ptr [eax + 0x18], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = 0 /*0x0*/;
    // 004f1a62  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004f1a67  e874defeff             -call 0x4df8e0
    cpu.esp -= 4;
    nfs3hp::ea::sleepMs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1a6c  e9d7fdffff             -jmp 0x4f1848
    goto L_0x004f1848;
L_0x004f1a71:
    // 004f1a71  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1a76  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 004f1a79  894518                 -mov dword ptr [ebp + 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 004f1a7c  e9f6feffff             -jmp 0x4f1977
    goto L_0x004f1977;
L_0x004f1a81:
    // 004f1a81  897d08                 -mov dword ptr [ebp + 8], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */) = cpu.edi;
    // 004f1a84  ebaf                   -jmp 0x4f1a35
    goto L_0x004f1a35;
L_0x004f1a86:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1a86  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1a8b  8b942404010000         -mov edx, dword ptr [esp + 0x104]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1a92  83c038                 +add eax, 0x38
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(56 /*0x38*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f1a95  ebba                   -jmp 0x4f1a51
    goto L_0x004f1a51;
  case 0x004f1a97:
    // 004f1a97  8b5808                 -mov ebx, dword ptr [eax + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004f1a9a  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004f1a9c  0f857a000000           -jne 0x4f1b1c
    if (!cpu.flags.zf)
    {
        goto L_0x004f1b1c;
    }
L_0x004f1aa2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1aa2  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1aa7  c7400800000000         -mov dword ptr [eax + 8], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 004f1aae  8b842404010000         -mov eax, dword ptr [esp + 0x104]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1ab5  83781c00               +cmp dword ptr [eax + 0x1c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f1ab9  744d                   -je 0x4f1b08
    if (cpu.flags.zf)
    {
        goto L_0x004f1b08;
    }
    // 004f1abb  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f1abd  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f1abf  e85cdd0000             -call 0x4ff820
    cpu.esp -= 4;
    nfs3hp::port::loadBlockSize(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1ac4  e8a7db0000             -call 0x4ff670
    cpu.esp -= 4;
    nfs3hp::port::topUpStream(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1ac9  e822dd0000             -call 0x4ff7f0
    cpu.esp -= 4;
    nfs3hp::port::waitTopupStream(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1ace  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1ad3  8d4810                 -lea ecx, [eax + 0x10]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004f1ad6  8d580c                 -lea ebx, [eax + 0xc]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004f1ad9  8d5008                 -lea edx, [eax + 8]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004f1adc  8b842404010000         -mov eax, dword ptr [esp + 0x104]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1ae3  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004f1ae6  e8e50dffff             -call 0x4e28d0
    cpu.esp -= 4;
    nfs3hp::ea::openHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1aeb  8b842404010000         -mov eax, dword ptr [esp + 0x104]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1af2  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004f1af5  e896fdfeff             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1afa  8b842404010000         -mov eax, dword ptr [esp + 0x104]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1b01  c7401c00000000         -mov dword ptr [eax + 0x1c], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */) = 0 /*0x0*/;
L_0x004f1b08:
    // 004f1b08  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1b0d  8b942404010000         -mov edx, dword ptr [esp + 0x104]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1b14  83c01c                 +add eax, 0x1c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(28 /*0x1c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f1b17  e935ffffff             -jmp 0x4f1a51
    goto L_0x004f1a51;
L_0x004f1b1c:
    // 004f1b1c  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004f1b1e  e87d12ffff             -call 0x4e2da0
    cpu.esp -= 4;
    nfs3hp::ea::closeHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1b23  e97affffff             -jmp 0x4f1aa2
    goto L_0x004f1aa2;
  case 0x004f1b28:
    // 004f1b28  8b5008                 -mov edx, dword ptr [eax + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004f1b2b  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004f1b2d  7519                   -jne 0x4f1b48
    if (!cpu.flags.zf)
    {
        goto L_0x004f1b48;
    }
    // 004f1b2f  895520                 -mov dword ptr [ebp + 0x20], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */) = cpu.edx;
L_0x004f1b32:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f1b32  8b842404010000         -mov eax, dword ptr [esp + 0x104]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1b39  83782400               +cmp dword ptr [eax + 0x24], 0
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
    // 004f1b3d  0f8443ffffff           -je 0x4f1a86
    if (cpu.flags.zf)
    {
        goto L_0x004f1a86;
    }
    // 004f1b43  e9fafeffff             -jmp 0x4f1a42
    goto L_0x004f1a42;
L_0x004f1b48:
    // 004f1b48  8b4510                 -mov eax, dword ptr [ebp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004f1b4b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f1b4d  b90e010000             -mov ecx, 0x10e
    cpu.ecx = 270 /*0x10e*/;
    // 004f1b52  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004f1b55  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004f1b57  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004f1b59  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f1b5b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f1b5d  e8bedc0000             -call 0x4ff820
    cpu.esp -= 4;
    nfs3hp::port::loadBlockSize(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1b62  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004f1b64  e807db0000             -call 0x4ff670
    cpu.esp -= 4;
    nfs3hp::port::topUpStream(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1b69  e882dc0000             -call 0x4ff7f0
    cpu.esp -= 4;
    nfs3hp::port::waitTopupStream(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1b6e  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1b73  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004f1b76  e82525ffff             -call 0x4e40a0
    cpu.esp -= 4;
    nfs3hp::ea::lockHandleDrive(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1b7b  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1b80  8b550c                 -mov edx, dword ptr [ebp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 004f1b83  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004f1b86  e8d512ffff             -call 0x4e2e60
    cpu.esp -= 4;
    nfs3hp::ea::seekHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1b8b  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1b90  8b5d10                 -mov ebx, dword ptr [ebp + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004f1b93  8b5508                 -mov edx, dword ptr [ebp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 004f1b96  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004f1b99  e84212ffff             -call 0x4e2de0
    cpu.esp -= 4;
    nfs3hp::ea::readHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1b9e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f1ba0  7503                   -jne 0x4f1ba5
    if (!cpu.flags.zf)
    {
        goto L_0x004f1ba5;
    }
    // 004f1ba2  894520                 -mov dword ptr [ebp + 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */) = cpu.eax;
L_0x004f1ba5:
    // 004f1ba5  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1baa  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004f1bad  e83e25ffff             -call 0x4e40f0
    cpu.esp -= 4;
    nfs3hp::ea::unlockHandleDrive(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1bb2  e97bffffff             -jmp 0x4f1b32
    goto L_0x004f1b32;
  case 0x004f1bb7:
    // 004f1bb7  8b6808                 -mov ebp, dword ptr [eax + 8]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004f1bba  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 004f1bbc  7407                   -je 0x4f1bc5
    if (cpu.flags.zf)
    {
        goto L_0x004f1bc5;
    }
    // 004f1bbe  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 004f1bc0  e8db11ffff             -call 0x4e2da0
    cpu.esp -= 4;
    nfs3hp::ea::closeHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f1bc5:
    // 004f1bc5  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1bca  83c01c                 +add eax, 0x1c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(28 /*0x1c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f1bcd  8b942404010000         -mov edx, dword ptr [esp + 0x104]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 004f1bd4  c740ec00000000         -mov dword ptr [eax - 0x14], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-20) /* -0x14 */) = 0 /*0x0*/;
    // 004f1bdb  e971feffff             -jmp 0x4f1a51
    goto L_0x004f1a51;
L_0x004f1be0:
    // 004f1be0  bb80c65400             -mov ebx, 0x54c680
    cpu.ebx = 5555840 /*0x54c680*/;
    // 004f1be5  be30c95400             -mov esi, 0x54c930
    cpu.esi = 5556528 /*0x54c930*/;
    // 004f1bea  bffb030000             -mov edi, 0x3fb
    cpu.edi = 1019 /*0x3fb*/;
    // 004f1bef  6840c95400             -push 0x54c940
    app->getMemory<x86::reg32>(cpu.esp-4) = 5556544 /*0x54c940*/;
    cpu.esp -= 4;
    // 004f1bf4  891d90215500           -mov dword ptr [0x552190], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebx;
    // 004f1bfa  893594215500           -mov dword ptr [0x552194], esi
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.esi;
    // 004f1c00  893d98215500           -mov dword ptr [0x552198], edi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edi;
    // 004f1c06  e805f4f0ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1c0b  83c404                 +add esp, 4
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
    // 004f1c0e  e943feffff             -jmp 0x4f1a56
    goto L_0x004f1a56;
L_0x004f1c13:
    // 004f1c13  a1e00b9f00             -mov eax, dword ptr [0x9f0be0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423264) /* 0x9f0be0 */);
    // 004f1c18  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004f1c1a  e851dffeff             -call 0x4dfb70
    cpu.esp -= 4;
    nfs3hp::ea::waitEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1c1f  e924fcffff             -jmp 0x4f1848
    goto L_0x004f1848;
L_0x004f1c24:
    // 004f1c24  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f1c26  e87511ffff             -call 0x4e2da0
    cpu.esp -= 4;
    nfs3hp::ea::closeHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f1c2b  e92efcffff             -jmp 0x4f185e
    goto L_0x004f185e;
  default:
    NFS2_ASSERT(false);
  }
}

/* sub_4f1c30: native, nfs3hp::watcom::floor */

/* sub_4f1c70: native, nfs3hp::removed::processorChecked */

/* sub_4f1ea0: native, nfs3hp::dead::stub_0x4f1ea0 */

/* sub_4f1ef0: native, nfs3hp::removed::processorSpeed */

/* sub_4f20b0: native, nfs3hp::dead::stub_0x4f20b0 */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4f2170(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2170  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f2171  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2172  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f2173  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f2174  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f2175  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f2176  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004f2178  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004f217b  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004f217e  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f2180  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004f2183  8bbdf8ffffff           -mov edi, dword ptr [ebp - 8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004f2189  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004f218b  8b35a4c17900           -mov esi, dword ptr [0x79c1a4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
L_0x004f2191:
    app->unlockContext(cpu);
    win32::Thread::sleep(0);
    app->lockContext(cpu);
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2191  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f2193  b8aaaa5555             -mov eax, 0x5555aaaa
    cpu.eax = 1431677610 /*0x5555aaaa*/;
    // 004f2198  bb21000000             -mov ebx, 0x21
    cpu.ebx = 33 /*0x21*/;
    // 004f219d  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004f219f  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 004f21a4  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004f21a6  41                     -inc ecx
    (cpu.ecx)++;
    // 004f21a7  39f8                   +cmp eax, edi
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
    // 004f21a9  7ce6                   -jl 0x4f2191
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004f2191;
    }
    // 004f21ab  898dfcffffff           -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 004f21b1  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004f21b4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004f21b6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f21b7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f21b8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f21b9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f21ba  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f21bb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f21bc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4f21c0: native, nfs3hp::dead::stub_0x4f21c0 */

/* sub_4f22b0: native, nfs3hp::watcom::time */

/* align: skip 0x00 */
void Application::sub_4f22e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f22e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f22e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f22e2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f22e3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f22e4  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004f22e7  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004f22e9  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004f22eb  b890010000             -mov eax, 0x190
    cpu.eax = 400 /*0x190*/;
    // 004f22f0  89e1                   -mov ecx, esp
    cpu.ecx = cpu.esp;
    // 004f22f2  8d5c2404               -lea ebx, [esp + 4]
    cpu.ebx = x86::reg32(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 004f22f6  8d542408               -lea edx, [esp + 8]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 004f22fa  e8c1d40000             -call 0x4ff7c0
    cpu.esp -= 4;
    nfs3hp::port::topUpBeforeLoad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f22ff  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004f2301  e8ca05ffff             -call 0x4e28d0
    cpu.esp -= 4;
    nfs3hp::ea::openHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2306  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 004f2309  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004f230b  750c                   -jne 0x4f2319
    if (!cpu.flags.zf)
    {
        goto L_0x004f2319;
    }
    // 004f230d  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004f230f  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004f2311  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004f2314  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2315  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2316  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2317  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2318  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f2319:
    // 004f2319  b9c2010000             -mov ecx, 0x1c2
    cpu.ecx = 450 /*0x1c2*/;
    // 004f231e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004f2320  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004f2323  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004f2325  0590010000             -add eax, 0x190
    (cpu.eax) += x86::reg32(x86::sreg32(400 /*0x190*/));
    // 004f232a  e891d40000             -call 0x4ff7c0
    cpu.esp -= 4;
    nfs3hp::port::topUpBeforeLoad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f232f  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004f2331  8b1c24                 -mov ebx, dword ptr [esp]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    // 004f2334  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 004f2338  e8a30affff             -call 0x4e2de0
    cpu.esp -= 4;
    nfs3hp::ea::readHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f233d  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 004f2340  a3f8435600             -mov dword ptr [0x5643f8], eax
    app->getMemory<x86::reg32>(x86::reg32(5653496) /* 0x5643f8 */) = cpu.eax;
    // 004f2345  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 004f2349  e8520affff             -call 0x4e2da0
    cpu.esp -= 4;
    nfs3hp::ea::closeHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f234e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004f2350  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004f2353  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2354  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2355  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2356  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2357  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4f2360: native, nfs3hp::dead::stub_0x4f2360 */

/* sub_4f2380: native, nfs3hp::load::fixedSine */

/* sub_4f23f0: native, nfs3hp::load::fixedCosine */

/* align: skip 0x00 */
void Application::sub_4f2460(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2460  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f2461  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f2463  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f2465  7504                   -jne 0x4f246b
    if (!cpu.flags.zf)
    {
        goto L_0x004f246b;
    }
L_0x004f2467:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2467  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f2469  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f246a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f246b:
    // 004f246b  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004f246d  83e07f                 -and eax, 0x7f
    cpu.eax &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 004f2470  8a80546d5600           -mov al, byte ptr [eax + 0x566d54]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5664084) /* 0x566d54 */);
    // 004f2476  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f2478  741d                   -je 0x4f2497
    if (cpu.flags.zf)
    {
        goto L_0x004f2497;
    }
L_0x004f247a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f247a  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004f247c  74e9                   -je 0x4f2467
    if (cpu.flags.zf)
    {
        goto L_0x004f2467;
    }
    // 004f247e  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2480  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004f2485  83f87c                 +cmp eax, 0x7c
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
    // 004f2488  744b                   -je 0x4f24d5
    if (cpu.flags.zf)
    {
        goto L_0x004f24d5;
    }
    // 004f248a  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004f248c  c1f808                 -sar eax, 8
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (8 /*0x8*/ % 32));
    // 004f248f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f2491  7447                   -je 0x4f24da
    if (cpu.flags.zf)
    {
        goto L_0x004f24da;
    }
    // 004f2493  01c2                   +add edx, eax
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
    // 004f2495  ebe3                   -jmp 0x4f247a
    goto L_0x004f247a;
L_0x004f2497:
    // 004f2497  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f2498  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2499  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f249a  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004f249c  b9d4ca5400             -mov ecx, 0x54cad4
    cpu.ecx = 5556948 /*0x54cad4*/;
    // 004f24a1  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004f24a6  bbe4ca5400             -mov ebx, 0x54cae4
    cpu.ebx = 5556964 /*0x54cae4*/;
    // 004f24ab  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004f24ac  be31000000             -mov esi, 0x31
    cpu.esi = 49 /*0x31*/;
    // 004f24b1  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 004f24b7  68f8ca5400             -push 0x54caf8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5556984 /*0x54caf8*/;
    cpu.esp -= 4;
    // 004f24bc  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 004f24c2  893598215500           -mov dword ptr [0x552198], esi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.esi;
    // 004f24c8  e843ebf0ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f24cd  83c408                 +add esp, 8
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
    // 004f24d0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f24d1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f24d2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f24d3  eba5                   -jmp 0x4f247a
    goto L_0x004f247a;
L_0x004f24d5:
    // 004f24d5  8d4208                 -lea eax, [edx + 8]
    cpu.eax = x86::reg32(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 004f24d8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f24d9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f24da:
    // 004f24da  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004f24dc  eb9c                   -jmp 0x4f247a
    goto L_0x004f247a;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4f24e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f24e0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f24e1  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f24e3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f24e5  7504                   -jne 0x4f24eb
    if (!cpu.flags.zf)
    {
        goto L_0x004f24eb;
    }
L_0x004f24e7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f24e7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f24e9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f24ea  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f24eb:
    // 004f24eb  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004f24ed  83e07f                 -and eax, 0x7f
    cpu.eax &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 004f24f0  8a80546d5600           -mov al, byte ptr [eax + 0x566d54]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5664084) /* 0x566d54 */);
    // 004f24f6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f24f8  741d                   -je 0x4f2517
    if (cpu.flags.zf)
    {
        goto L_0x004f2517;
    }
L_0x004f24fa:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f24fa  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004f24fc  74e9                   -je 0x4f24e7
    if (cpu.flags.zf)
    {
        goto L_0x004f24e7;
    }
    // 004f24fe  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2500  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004f2505  83f86f                 +cmp eax, 0x6f
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
    // 004f2508  744b                   -je 0x4f2555
    if (cpu.flags.zf)
    {
        goto L_0x004f2555;
    }
    // 004f250a  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004f250c  c1f808                 -sar eax, 8
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (8 /*0x8*/ % 32));
    // 004f250f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f2511  7447                   -je 0x4f255a
    if (cpu.flags.zf)
    {
        goto L_0x004f255a;
    }
    // 004f2513  01c2                   +add edx, eax
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
    // 004f2515  ebe3                   -jmp 0x4f24fa
    goto L_0x004f24fa;
L_0x004f2517:
    // 004f2517  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f2518  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2519  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f251a  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004f251c  b9d4ca5400             -mov ecx, 0x54cad4
    cpu.ecx = 5556948 /*0x54cad4*/;
    // 004f2521  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004f2526  bb28cb5400             -mov ebx, 0x54cb28
    cpu.ebx = 5557032 /*0x54cb28*/;
    // 004f252b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004f252c  be69000000             -mov esi, 0x69
    cpu.esi = 105 /*0x69*/;
    // 004f2531  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 004f2537  6838cb5400             -push 0x54cb38
    app->getMemory<x86::reg32>(cpu.esp-4) = 5557048 /*0x54cb38*/;
    cpu.esp -= 4;
    // 004f253c  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 004f2542  893598215500           -mov dword ptr [0x552198], esi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.esi;
    // 004f2548  e8c3eaf0ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f254d  83c408                 +add esp, 8
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
    // 004f2550  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2551  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2552  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2553  eba5                   -jmp 0x4f24fa
    goto L_0x004f24fa;
L_0x004f2555:
    // 004f2555  8d4208                 -lea eax, [edx + 8]
    cpu.eax = x86::reg32(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 004f2558  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2559  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f255a:
    // 004f255a  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004f255c  eb9c                   -jmp 0x4f24fa
    goto L_0x004f24fa;
}

/* sub_4f2560: native, nfs3hp::ea::unpackQuietly */

/* sub_4f2620: native, nfs3hp::ea::unpack */

/* sub_4f26a0: native, nfs3hp::ea::unpackSizeQuietly */

/* sub_4f2710: native, nfs3hp::ea::unpacksize */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4f2790(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2790  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2791  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f2792  2eff1568455300         -call dword ptr cs:[0x534568]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457256) /* 0x534568 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2799  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f279a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f279b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4f27a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f27a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f27a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f27a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f27a3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f27a4  e857d3feff             -call 0x4dfb00
    cpu.esp -= 4;
    nfs3hp::ea::createEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f27a9  8b15340c9f00           -mov edx, dword ptr [0x9f0c34]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */);
    // 004f27af  a3ac525600             -mov dword ptr [0x5652ac], eax
    app->getMemory<x86::reg32>(x86::reg32(5657260) /* 0x5652ac */) = cpu.eax;
    // 004f27b4  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004f27b6  7455                   -je 0x4f280d
    if (cpu.flags.zf)
    {
        goto L_0x004f280d;
    }
L_0x004f27b8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f27b8  a1ac525600             -mov eax, dword ptr [0x5652ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5657260) /* 0x5652ac */);
    // 004f27bd  e8aed3feff             -call 0x4dfb70
    cpu.esp -= 4;
    nfs3hp::ea::waitEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f27c2  8b1d340c9f00           -mov ebx, dword ptr [0x9f0c34]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */);
    // 004f27c8  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004f27ca  7438                   -je 0x4f2804
    if (cpu.flags.zf)
    {
        goto L_0x004f2804;
    }
    // 004f27cc  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f27cd  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f27d3  8b3d000c9f00           -mov edi, dword ptr [0x9f0c00]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10423296) /* 0x9f0c00 */);
    // 004f27d9  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f27db  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004f27dd  7419                   -je 0x4f27f8
    if (cpu.flags.zf)
    {
        goto L_0x004f27f8;
    }
L_0x004f27df:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f27df  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004f27e1  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f27e4  ff90000c9f00           -call dword ptr [eax + 0x9f0c00]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10423296) /* 0x9f0c00 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f27ea  83fa20                 +cmp edx, 0x20
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
    // 004f27ed  7d09                   -jge 0x4f27f8
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004f27f8;
    }
    // 004f27ef  83ba000c9f0000         +cmp dword ptr [edx + 0x9f0c00], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(10423296) /* 0x9f0c00 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f27f6  75e7                   -jne 0x4f27df
    if (!cpu.flags.zf)
    {
        goto L_0x004f27df;
    }
L_0x004f27f8:
    // 004f27f8  a1340c9f00             -mov eax, dword ptr [0x9f0c34]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */);
    // 004f27fd  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004f27fe  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f2804:
    // 004f2804  833d340c9f0000         +cmp dword ptr [0x9f0c34], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f280b  75ab                   -jne 0x4f27b8
    if (!cpu.flags.zf)
    {
        goto L_0x004f27b8;
    }
L_0x004f280d:
    // 004f280d  a1ac525600             -mov eax, dword ptr [0x5652ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5657260) /* 0x5652ac */);
    // 004f2812  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f2814  e8e7d3feff             -call 0x4dfc00
    cpu.esp -= 4;
    nfs3hp::ea::closeEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2819  8915ac525600           -mov dword ptr [0x5652ac], edx
    app->getMemory<x86::reg32>(x86::reg32(5657260) /* 0x5652ac */) = cpu.edx;
    // 004f281f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2820  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2821  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2822  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2823  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4f2830: native, nfs3hp::ea::stopLowTimer */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4f2890(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2890  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f2891  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2892  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f2893  b830284f00             -mov eax, 0x4f2830
    cpu.eax = 5187632 /*0x4f2830*/;
    // 004f2898  e8db010000             -call 0x4f2a78
    cpu.esp -= 4;
    sub_4f2a78(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f289d  e83e8affff             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f28a2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004f28a3  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 004f28a8  a3340c9f00             -mov dword ptr [0x9f0c34], eax
    app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */) = cpu.eax;
    // 004f28ad  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f28b3  68200c9f00             -push 0x9f0c20
    app->getMemory<x86::reg32>(cpu.esp-4) = 10423328 /*0x9f0c20*/;
    cpu.esp -= 4;
    // 004f28b8  b9ffffffff             -mov ecx, 0xffffffff
    cpu.ecx = 4294967295 /*0xffffffff*/;
    // 004f28bd  b8a0274f00             -mov eax, 0x4f27a0
    cpu.eax = 5187488 /*0x4f27a0*/;
    // 004f28c2  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f28c4  e8d7cefeff             -call 0x4df7a0
    cpu.esp -= 4;
    nfs3hp::ea::createThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f28c9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004f28cb  7548                   -jne 0x4f2915
    if (!cpu.flags.zf)
    {
        goto L_0x004f2915;
    }
    // 004f28cd  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f28ce  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f28cf  bbd8cb5400             -mov ebx, 0x54cbd8
    cpu.ebx = 5557208 /*0x54cbd8*/;
    // 004f28d4  bee8cb5400             -mov esi, 0x54cbe8
    cpu.esi = 5557224 /*0x54cbe8*/;
    // 004f28d9  a1340c9f00             -mov eax, dword ptr [0x9f0c34]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */);
    // 004f28de  bf4d000000             -mov edi, 0x4d
    cpu.edi = 77 /*0x4d*/;
    // 004f28e3  e8788affff             -call 0x4eb360
    cpu.esp -= 4;
    nfs3hp::ea::freeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f28e8  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004f28ea  68f8cb5400             -push 0x54cbf8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5557240 /*0x54cbf8*/;
    cpu.esp -= 4;
    // 004f28ef  890d340c9f00           -mov dword ptr [0x9f0c34], ecx
    app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */) = cpu.ecx;
    // 004f28f5  891d90215500           -mov dword ptr [0x552190], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebx;
    // 004f28fb  893594215500           -mov dword ptr [0x552194], esi
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.esi;
    // 004f2901  893d98215500           -mov dword ptr [0x552198], edi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edi;
    // 004f2907  e804e7f0ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f290c  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f290f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2910  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2911  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2912  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2913  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2914  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f2915:
    // 004f2915  8b15340c9f00           -mov edx, dword ptr [0x9f0c34]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */);
    // 004f291b  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f291c  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2922  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2923  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2924  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2925  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4f2930(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2930  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f2931  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2932  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f2933  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f2934  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004f2936  8b15340c9f00           -mov edx, dword ptr [0x9f0c34]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */);
    // 004f293c  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 004f2941  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004f2943  744d                   -je 0x4f2992
    if (cpu.flags.zf)
    {
        goto L_0x004f2992;
    }
L_0x004f2945:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2945  8b0d340c9f00           -mov ecx, dword ptr [0x9f0c34]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */);
    // 004f294b  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f294c  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2952  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004f2954  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004f2956:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2956  83b8000c9f0000         +cmp dword ptr [eax + 0x9f0c00], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10423296) /* 0x9f0c00 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f295d  7504                   -jne 0x4f2963
    if (!cpu.flags.zf)
    {
        goto L_0x004f2963;
    }
    // 004f295f  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004f2961  7c36                   -jl 0x4f2999
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004f2999;
    }
L_0x004f2963:
    // 004f2963  3bb0000c9f00           +cmp esi, dword ptr [eax + 0x9f0c00]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10423296) /* 0x9f0c00 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f2969  7432                   -je 0x4f299d
    if (cpu.flags.zf)
    {
        goto L_0x004f299d;
    }
L_0x004f296b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f296b  42                     -inc edx
    (cpu.edx)++;
    // 004f296c  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f296f  83fa08                 +cmp edx, 8
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
    // 004f2972  7ce2                   -jl 0x4f2956
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004f2956;
    }
    // 004f2974  83fbff                 +cmp ebx, -1
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
    // 004f2977  7407                   -je 0x4f2980
    if (cpu.flags.zf)
    {
        goto L_0x004f2980;
    }
    // 004f2979  89349d000c9f00         -mov dword ptr [ebx*4 + 0x9f0c00], esi
    app->getMemory<x86::reg32>(x86::reg32(10423296) /* 0x9f0c00 */ + cpu.ebx * 4) = cpu.esi;
L_0x004f2980:
    // 004f2980  8b15340c9f00           -mov edx, dword ptr [0x9f0c34]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */);
    // 004f2986  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f2987  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f298d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f298e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f298f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2990  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2991  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f2992:
    // 004f2992  e8f9feffff             -call 0x4f2890
    cpu.esp -= 4;
    sub_4f2890(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2997  ebac                   -jmp 0x4f2945
    goto L_0x004f2945;
L_0x004f2999:
    // 004f2999  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 004f299b  ebce                   -jmp 0x4f296b
    goto L_0x004f296b;
L_0x004f299d:
    // 004f299d  a1340c9f00             -mov eax, dword ptr [0x9f0c34]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10423348) /* 0x9f0c34 */);
    // 004f29a2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004f29a3  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f29a9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f29aa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f29ab  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f29ac  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f29ad  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4f29b0: native, nfs3hp::ea::removeLowTimer */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4f2a60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2a60  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f2a61  8b15ac525600           -mov edx, dword ptr [0x5652ac]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5657260) /* 0x5652ac */);
    // 004f2a67  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004f2a69  7502                   -jne 0x4f2a6d
    if (!cpu.flags.zf)
    {
        goto L_0x004f2a6d;
    }
    // 004f2a6b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2a6c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f2a6d:
    // 004f2a6d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004f2a6f  e8bcd0feff             -call 0x4dfb30
    cpu.esp -= 4;
    nfs3hp::ea::signalEvent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2a74  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2a75  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 */
void Application::sub_4f2a78(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2a78  e9a3510000             -jmp 0x4f7c20
    return nfs3hp::watcom::atexit(app, cpu.handOver());
}

/* align: skip  */
void Application::sub_4f2a7d(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 004f2a7d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f2a7f  e880d2feff             -call 0x4dfd04
    cpu.esp -= 4;
    nfs3hp::watcom::exit(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2a84  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a86  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a88  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a8a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a8c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a8e  0000                   +add byte ptr [eax], al
    {
        auto tmp1 = app->getMemory<x86::reg8>(cpu.eax);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.al));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f2a90  142b                   -adc al, 0x2b
    (cpu.al) += x86::reg8(x86::sreg8(43 /*0x2b*/) + cpu.flags.cf);
    // 004f2a92  4f                     -dec edi
    (cpu.edi)--;
    // 004f2a93  00c3                   -add bl, al
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a95  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004f2a98  b82b4f00ad             -mov eax, 0xad004f2b
    cpu.eax = 2902478635 /*0xad004f2b*/;
    // 004f2a9d  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004f2aa0  a22b4f0097             -mov byte ptr [0x97004f2b], al
    app->getMemory<x86::reg8>(x86::reg32(2533379883) /* 0x97004f2b */) = cpu.al;
    // 004f2aa5  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004f2aa8  8c2b                   -mov word ptr [ebx], gs
    app->getMemory<x86::reg16>(cpu.ebx) = cpu.gs;
    // 004f2aaa  4f                     -dec edi
    (cpu.edi)--;
    // 004f2aab  00812b4f0076           -add byte ptr [ecx + 0x76004f2b], al
    (app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(1979731755) /* 0x76004f2b */)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2ab1  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004f2ab4  6b2b4f                 -imul ebp, dword ptr [ebx], 0x4f
    cpu.ebp = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx))) * x86::sreg64(x86::sreg32(79 /*0x4f*/)));
    // 004f2ab7  00602b                 -add byte ptr [eax + 0x2b], ah
    (app->getMemory<x86::reg8>(cpu.eax + x86::reg32(43) /* 0x2b */)) += x86::reg8(x86::sreg8(cpu.ah));
    // 004f2aba  4f                     -dec edi
    (cpu.edi)--;
    // 004f2abb  00552b                 -add byte ptr [ebp + 0x2b], dl
    (app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(43) /* 0x2b */)) += x86::reg8(x86::sreg8(cpu.dl));
    // 004f2abe  4f                     -dec edi
    (cpu.edi)--;
    // 004f2abf  004a2b                 -add byte ptr [edx + 0x2b], cl
    (app->getMemory<x86::reg8>(cpu.edx + x86::reg32(43) /* 0x2b */)) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f2ac2  4f                     -dec edi
    (cpu.edi)--;
    // 004f2ac3  003f                   -add byte ptr [edi], bh
    (app->getMemory<x86::reg8>(cpu.edi)) += x86::reg8(x86::sreg8(cpu.bh));
    // 004f2ac5  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004f2ac8  342b                   -xor al, 0x2b
    cpu.al ^= x86::reg8(x86::sreg8(43 /*0x2b*/));
    // 004f2aca  4f                     -dec edi
    (cpu.edi)--;
    // 004f2acb  0029                   -add byte ptr [ecx], ch
    (app->getMemory<x86::reg8>(cpu.ecx)) += x86::reg8(x86::sreg8(cpu.ch));
    // 004f2acd  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004f2ad0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f2ad1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f2ad2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f2ad3  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004f2ad5  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004f2ad7  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f2ad9  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004f2adb  7411                   -je 0x4f2aee
    if (cpu.flags.zf)
    {
        goto L_0x004f2aee;
    }
    // 004f2add  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
L_0x004f2adf:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2adf  48                     -dec eax
    (cpu.eax)--;
    // 004f2ae0  83f8ff                 +cmp eax, -1
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
    // 004f2ae3  7409                   -je 0x4f2aee
    if (cpu.flags.zf)
    {
        goto L_0x004f2aee;
    }
    // 004f2ae5  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004f2ae6  8a16                   -mov dl, byte ptr [esi]
    cpu.dl = app->getMemory<x86::reg8>(cpu.esi);
    // 004f2ae8  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004f2ae9  8857ff                 -mov byte ptr [edi - 1], dl
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(-1) /* -0x1 */) = cpu.dl;
    // 004f2aec  ebf1                   -jmp 0x4f2adf
    goto L_0x004f2adf;
L_0x004f2aee:
    // 004f2aee  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 004f2af0  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004f2af2  89dd                   -mov ebp, ebx
    cpu.ebp = cpu.ebx;
    // 004f2af4  c1f902                 -sar ecx, 2
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (2 /*0x2*/ % 32));
    // 004f2af7  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004f2af9  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 004f2afb  83e303                 -and ebx, 3
    cpu.ebx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 004f2afe  83e60f                 -and esi, 0xf
    cpu.esi &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 004f2b01  c1fd06                 -sar ebp, 6
    cpu.ebp = x86::reg32(x86::sreg32(cpu.ebp) >> (6 /*0x6*/ % 32));
    // 004f2b04  83fe0f                 +cmp esi, 0xf
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(15 /*0xf*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f2b07  0f87c6000000           -ja 0x4f2bd3
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004f2bd3;
    }
    // 004f2b0d  ff24b5902a4f00         -jmp dword ptr [esi*4 + 0x4f2a90]
    cpu.ip = app->getMemory<x86::reg32>(5188240 + cpu.esi * 4); goto dynamic_jump;
  case 0x004f2b14:
L_0x004f2b14:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2b14  4d                     -dec ebp
    (cpu.ebp)--;
    // 004f2b15  83fdff                 +cmp ebp, -1
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
    // 004f2b18  0f84b5000000           -je 0x4f2bd3
    if (cpu.flags.zf)
    {
        goto L_0x004f2bd3;
    }
    // 004f2b1e  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b21  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b23  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b26  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2b29:
    // 004f2b29  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b2c  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b2e  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b31  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b34:
    // 004f2b34  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b37  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b39  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b3c  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b3f:
    // 004f2b3f  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b42  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b44  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b47  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b4a:
    // 004f2b4a  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b4d  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b4f  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b52  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b55:
    // 004f2b55  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b58  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b5a  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b5d  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2b60:
    // 004f2b60  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b63  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b65  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b68  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b6b:
    // 004f2b6b  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b6e  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b70  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b73  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b76:
    // 004f2b76  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b79  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b7b  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b7e  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b81:
    // 004f2b81  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b84  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b86  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b89  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b8c:
    // 004f2b8c  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b8f  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b91  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b94  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b97:
    // 004f2b97  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b9a  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b9c  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b9f  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2ba2:
    // 004f2ba2  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2ba5  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2ba7  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2baa  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2bad:
    // 004f2bad  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2bb0  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2bb2  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2bb5  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2bb8:
    // 004f2bb8  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2bbb  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2bbd  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2bc0  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2bc3:
    // 004f2bc3  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2bc6  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2bc8  83c204                 +add edx, 4
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f2bcb  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 004f2bce  e941ffffff             -jmp 0x4f2b14
    goto L_0x004f2b14;
L_0x004f2bd3:
    // 004f2bd3  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x004f2bd5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2bd5  4b                     -dec ebx
    (cpu.ebx)--;
    // 004f2bd6  83fbff                 +cmp ebx, -1
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
    // 004f2bd9  740d                   -je 0x4f2be8
    if (cpu.flags.zf)
    {
        goto L_0x004f2be8;
    }
    // 004f2bdb  8d0c32                 -lea ecx, [edx + esi]
    cpu.ecx = x86::reg32(cpu.edx + cpu.esi * 1);
    // 004f2bde  8d3c30                 -lea edi, [eax + esi]
    cpu.edi = x86::reg32(cpu.eax + cpu.esi * 1);
    // 004f2be1  8a09                   -mov cl, byte ptr [ecx]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx);
    // 004f2be3  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004f2be4  880f                   -mov byte ptr [edi], cl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.cl;
    // 004f2be6  ebed                   -jmp 0x4f2bd5
    goto L_0x004f2bd5;
L_0x004f2be8:
    // 004f2be8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2be9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2bea  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2beb  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip  */
void Application::sub_4f2ad0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    goto L_entry_0x004f2ad0;
    // 004f2a7d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f2a7f  e880d2feff             -call 0x4dfd04
    cpu.esp -= 4;
    nfs3hp::watcom::exit(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2a84  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a86  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a88  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a8a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a8c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a8e  0000                   +add byte ptr [eax], al
    {
        auto tmp1 = app->getMemory<x86::reg8>(cpu.eax);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.al));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f2a90  142b                   -adc al, 0x2b
    (cpu.al) += x86::reg8(x86::sreg8(43 /*0x2b*/) + cpu.flags.cf);
    // 004f2a92  4f                     -dec edi
    (cpu.edi)--;
    // 004f2a93  00c3                   -add bl, al
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2a95  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004f2a98  b82b4f00ad             -mov eax, 0xad004f2b
    cpu.eax = 2902478635 /*0xad004f2b*/;
    // 004f2a9d  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004f2aa0  a22b4f0097             -mov byte ptr [0x97004f2b], al
    app->getMemory<x86::reg8>(x86::reg32(2533379883) /* 0x97004f2b */) = cpu.al;
    // 004f2aa5  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004f2aa8  8c2b                   -mov word ptr [ebx], gs
    app->getMemory<x86::reg16>(cpu.ebx) = cpu.gs;
    // 004f2aaa  4f                     -dec edi
    (cpu.edi)--;
    // 004f2aab  00812b4f0076           -add byte ptr [ecx + 0x76004f2b], al
    (app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(1979731755) /* 0x76004f2b */)) += x86::reg8(x86::sreg8(cpu.al));
    // 004f2ab1  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004f2ab4  6b2b4f                 -imul ebp, dword ptr [ebx], 0x4f
    cpu.ebp = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx))) * x86::sreg64(x86::sreg32(79 /*0x4f*/)));
    // 004f2ab7  00602b                 -add byte ptr [eax + 0x2b], ah
    (app->getMemory<x86::reg8>(cpu.eax + x86::reg32(43) /* 0x2b */)) += x86::reg8(x86::sreg8(cpu.ah));
    // 004f2aba  4f                     -dec edi
    (cpu.edi)--;
    // 004f2abb  00552b                 -add byte ptr [ebp + 0x2b], dl
    (app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(43) /* 0x2b */)) += x86::reg8(x86::sreg8(cpu.dl));
    // 004f2abe  4f                     -dec edi
    (cpu.edi)--;
    // 004f2abf  004a2b                 -add byte ptr [edx + 0x2b], cl
    (app->getMemory<x86::reg8>(cpu.edx + x86::reg32(43) /* 0x2b */)) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f2ac2  4f                     -dec edi
    (cpu.edi)--;
    // 004f2ac3  003f                   -add byte ptr [edi], bh
    (app->getMemory<x86::reg8>(cpu.edi)) += x86::reg8(x86::sreg8(cpu.bh));
    // 004f2ac5  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004f2ac8  342b                   -xor al, 0x2b
    cpu.al ^= x86::reg8(x86::sreg8(43 /*0x2b*/));
    // 004f2aca  4f                     -dec edi
    (cpu.edi)--;
    // 004f2acb  0029                   -add byte ptr [ecx], ch
    (app->getMemory<x86::reg8>(cpu.ecx)) += x86::reg8(x86::sreg8(cpu.ch));
    // 004f2acd  2b4f00                 -sub ecx, dword ptr [edi]
    (cpu.ecx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
L_entry_0x004f2ad0:
    // 004f2ad0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f2ad1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f2ad2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f2ad3  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004f2ad5  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004f2ad7  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f2ad9  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004f2adb  7411                   -je 0x4f2aee
    if (cpu.flags.zf)
    {
        goto L_0x004f2aee;
    }
    // 004f2add  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
L_0x004f2adf:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2adf  48                     -dec eax
    (cpu.eax)--;
    // 004f2ae0  83f8ff                 +cmp eax, -1
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
    // 004f2ae3  7409                   -je 0x4f2aee
    if (cpu.flags.zf)
    {
        goto L_0x004f2aee;
    }
    // 004f2ae5  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004f2ae6  8a16                   -mov dl, byte ptr [esi]
    cpu.dl = app->getMemory<x86::reg8>(cpu.esi);
    // 004f2ae8  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004f2ae9  8857ff                 -mov byte ptr [edi - 1], dl
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(-1) /* -0x1 */) = cpu.dl;
    // 004f2aec  ebf1                   -jmp 0x4f2adf
    goto L_0x004f2adf;
L_0x004f2aee:
    // 004f2aee  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 004f2af0  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004f2af2  89dd                   -mov ebp, ebx
    cpu.ebp = cpu.ebx;
    // 004f2af4  c1f902                 -sar ecx, 2
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (2 /*0x2*/ % 32));
    // 004f2af7  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004f2af9  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 004f2afb  83e303                 -and ebx, 3
    cpu.ebx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 004f2afe  83e60f                 -and esi, 0xf
    cpu.esi &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 004f2b01  c1fd06                 -sar ebp, 6
    cpu.ebp = x86::reg32(x86::sreg32(cpu.ebp) >> (6 /*0x6*/ % 32));
    // 004f2b04  83fe0f                 +cmp esi, 0xf
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(15 /*0xf*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f2b07  0f87c6000000           -ja 0x4f2bd3
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004f2bd3;
    }
    // 004f2b0d  ff24b5902a4f00         -jmp dword ptr [esi*4 + 0x4f2a90]
    cpu.ip = app->getMemory<x86::reg32>(5188240 + cpu.esi * 4); goto dynamic_jump;
  case 0x004f2b14:
L_0x004f2b14:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2b14  4d                     -dec ebp
    (cpu.ebp)--;
    // 004f2b15  83fdff                 +cmp ebp, -1
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
    // 004f2b18  0f84b5000000           -je 0x4f2bd3
    if (cpu.flags.zf)
    {
        goto L_0x004f2bd3;
    }
    // 004f2b1e  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b21  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b23  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b26  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2b29:
    // 004f2b29  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b2c  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b2e  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b31  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b34:
    // 004f2b34  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b37  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b39  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b3c  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b3f:
    // 004f2b3f  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b42  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b44  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b47  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b4a:
    // 004f2b4a  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b4d  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b4f  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b52  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b55:
    // 004f2b55  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b58  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b5a  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b5d  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2b60:
    // 004f2b60  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b63  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b65  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b68  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b6b:
    // 004f2b6b  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b6e  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b70  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b73  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b76:
    // 004f2b76  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b79  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b7b  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b7e  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b81:
    // 004f2b81  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b84  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b86  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b89  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b8c:
    // 004f2b8c  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b8f  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b91  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b94  8948fc                 -mov dword ptr [eax - 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
  [[fallthrough]];
  case 0x004f2b97:
    // 004f2b97  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b9a  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2b9c  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2b9f  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2ba2:
    // 004f2ba2  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2ba5  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2ba7  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2baa  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2bad:
    // 004f2bad  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2bb0  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2bb2  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2bb5  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2bb8:
    // 004f2bb8  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2bbb  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2bbd  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2bc0  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
  [[fallthrough]];
  case 0x004f2bc3:
    // 004f2bc3  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f2bc6  8b32                   -mov esi, dword ptr [edx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx);
    // 004f2bc8  83c204                 +add edx, 4
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f2bcb  8970fc                 -mov dword ptr [eax - 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 004f2bce  e941ffffff             -jmp 0x4f2b14
    goto L_0x004f2b14;
L_0x004f2bd3:
    // 004f2bd3  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x004f2bd5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2bd5  4b                     -dec ebx
    (cpu.ebx)--;
    // 004f2bd6  83fbff                 +cmp ebx, -1
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
    // 004f2bd9  740d                   -je 0x4f2be8
    if (cpu.flags.zf)
    {
        goto L_0x004f2be8;
    }
    // 004f2bdb  8d0c32                 -lea ecx, [edx + esi]
    cpu.ecx = x86::reg32(cpu.edx + cpu.esi * 1);
    // 004f2bde  8d3c30                 -lea edi, [eax + esi]
    cpu.edi = x86::reg32(cpu.eax + cpu.esi * 1);
    // 004f2be1  8a09                   -mov cl, byte ptr [ecx]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx);
    // 004f2be3  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004f2be4  880f                   -mov byte ptr [edi], cl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.cl;
    // 004f2be6  ebed                   -jmp 0x4f2bd5
    goto L_0x004f2bd5;
L_0x004f2be8:
    // 004f2be8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2be9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2bea  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2beb  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4f2bf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2bf0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2bf1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f2bf2  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f2bf4  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004f2bf6  83fb04                 +cmp ebx, 4
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f2bf9  7d13                   -jge 0x4f2c0e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004f2c0e;
    }
L_0x004f2bfb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2bfb  01da                   -add edx, ebx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004f2bfd  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
L_0x004f2bff:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f2bff  4b                     -dec ebx
    (cpu.ebx)--;
    // 004f2c00  83fbff                 +cmp ebx, -1
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
    // 004f2c03  742a                   -je 0x4f2c2f
    if (cpu.flags.zf)
    {
        goto L_0x004f2c2f;
    }
    // 004f2c05  4a                     +dec edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004f2c06  8a48ff                 -mov cl, byte ptr [eax - 1]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-1) /* -0x1 */);
    // 004f2c09  48                     +dec eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004f2c0a  880a                   -mov byte ptr [edx], cl
    app->getMemory<x86::reg8>(cpu.edx) = cpu.cl;
    // 004f2c0c  ebf1                   -jmp 0x4f2bff
    goto L_0x004f2bff;
L_0x004f2c0e:
    // 004f2c0e  39c2                   +cmp edx, eax
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
    // 004f2c10  7607                   -jbe 0x4f2c19
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004f2c19;
    }
    // 004f2c12  8d0c18                 -lea ecx, [eax + ebx]
    cpu.ecx = x86::reg32(cpu.eax + cpu.ebx * 1);
    // 004f2c15  39ca                   +cmp edx, ecx
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
    // 004f2c17  72e2                   -jb 0x4f2bfb
    if (cpu.flags.cf)
    {
        goto L_0x004f2bfb;
    }
L_0x004f2c19:
    // 004f2c19  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f2c1b  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 004f2c20  83e103                 -and ecx, 3
    cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 004f2c23  29ce                   -sub esi, ecx
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004f2c25  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 004f2c27  83e103                 -and ecx, 3
    cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 004f2c2a  e8a1feffff             -call 0x4f2ad0
    cpu.esp -= 4;
    sub_4f2ad0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f2c2f:
    // 004f2c2f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2c30  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2c31  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4f2c40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2c40  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2c41  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004f2c43  d1fb                   -sar ebx, 1
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (1 /*0x1*/ % 32));
    // 004f2c45  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f2c47  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f2c49  e8a2ffffff             -call 0x4f2bf0
    cpu.esp -= 4;
    sub_4f2bf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2c4e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2c4f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4f2c50(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2c50  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2c51  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004f2c53  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f2c55  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f2c57  e894ffffff             -call 0x4f2bf0
    cpu.esp -= 4;
    sub_4f2bf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2c5c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2c5d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4f2c60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2c60  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2c61  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004f2c63  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f2c65  01db                   -add ebx, ebx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004f2c67  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f2c69  e882ffffff             -call 0x4f2bf0
    cpu.esp -= 4;
    sub_4f2bf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2c6e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2c6f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4f2c70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2c70  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2c71  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f2c72  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f2c74  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004f2c76  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004f2c78  c1e302                 -shl ebx, 2
    cpu.ebx <<= 2 /*0x2*/ % 32;
    // 004f2c7b  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004f2c7d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f2c7f  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004f2c81  e86affffff             -call 0x4f2bf0
    cpu.esp -= 4;
    sub_4f2bf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2c86  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2c87  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2c88  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4f2c90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2c90  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2c91  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004f2c93  c1e302                 -shl ebx, 2
    cpu.ebx <<= 2 /*0x2*/ % 32;
    // 004f2c96  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f2c98  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f2c9a  e851ffffff             -call 0x4f2bf0
    cpu.esp -= 4;
    sub_4f2bf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2c9f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2ca0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4f2cb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f2cb0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f2cb1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f2cb2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f2cb3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f2cb4  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004f2cb6  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004f2cb8  89dd                   -mov ebp, ebx
    cpu.ebp = cpu.ebx;
    // 004f2cba  b9b0525600             -mov ecx, 0x5652b0
    cpu.ecx = 5657264 /*0x5652b0*/;
    // 004f2cbf  e8ec830100             -call 0x50b0b0
    cpu.esp -= 4;
    sub_50b0b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f2cc4  6681660c00f0           -and word ptr [esi + 0xc], 0xf000
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(12) /* 0xc */) &= x86::reg16(x86::sreg16(61440 /*0xf000*/));
    // 004f2cca  81e7ff0f0000           -and edi, 0xfff
    cpu.edi &= x86::reg32(x86::sreg32(4095 /*0xfff*/));
    // 004f2cd0  097e0c                 -or dword ptr [esi + 0xc], edi
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */) |= x86::reg32(x86::sreg32(cpu.edi));
    // 004f2cd3  668b5e0e               -mov bx, word ptr [esi + 0xe]
    cpu.bx = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(14) /* 0xe */);
    // 004f2cd7  81e300f0ffff           -and ebx, 0xfffff000
    cpu.ebx &= x86::reg32(x86::sreg32(4294963200 /*0xfffff000*/));
    // 004f2cdd  81e5ff0f0000           -and ebp, 0xfff
    cpu.ebp &= x86::reg32(x86::sreg32(4095 /*0xfff*/));
    // 004f2ce3  66895e0e               -mov word ptr [esi + 0xe], bx
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(14) /* 0xe */) = cpu.bx;
    // 004f2ce7  c1e510                 -shl ebp, 0x10
    cpu.ebp <<= 16 /*0x10*/ % 32;
    // 004f2cea  096e0c                 -or dword ptr [esi + 0xc], ebp
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */) |= x86::reg32(x86::sreg32(cpu.ebp));
    // 004f2ced  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2cee  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2cef  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2cf0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f2cf1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4f2d00: native, nfs3hp::dead::stub_0x4f2d00 */

/* sub_4f2d20: native, nfs3hp::dead::stub_0x4f2d20 */

/* sub_4f2d40: native, nfs3hp::dead::stub_0x4f2d40 */

/* sub_4f2d60: native, nfs3hp::dead::stub_0x4f2d60 */

/* sub_4f2d78: native, nfs3hp::watcom::ceil */

/* sub_4f2da0: native, nfs3hp::dead::stub_0x4f2da0 */

/* sub_4f2e20: native, nfs3hp::ea::loadpackadr */

/* sub_4f2f30: native, nfs3hp::dead::stub_0x4f2f30 */

/* sub_4f2f50: native, nfs3hp::port::SNDSTRM_requeststatus */

/* align: skip 0x00 */
void Application::sub_4f30d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f30d0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f30d1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f30d2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f30d3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f30d4  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f30d5  be3f000000             -mov esi, 0x3f
    cpu.esi = 63 /*0x3f*/;
    // 004f30da  bbc0ffffff             -mov ebx, 0xffffffc0
    cpu.ebx = 4294967232 /*0xffffffc0*/;
    // 004f30df  ba80ffffff             -mov edx, 0xffffff80
    cpu.edx = 4294967168 /*0xffffff80*/;
L_0x004f30e4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f30e4  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004f30e6  83fac0                 +cmp edx, -0x40
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-64 /*-0x40*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f30e9  0f8dbd010000           -jge 0x4f32ac
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004f32ac;
    }
    // 004f30ef  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
L_0x004f30f1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f30f1  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004f30f3  81e1ff000000           -and ecx, 0xff
    cpu.ecx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004f30f9  0440                   -add al, 0x40
    (cpu.al) += x86::reg8(x86::sreg8(64 /*0x40*/));
    // 004f30fb  42                     -inc edx
    (cpu.edx)++;
    // 004f30fc  8881380d9f00           -mov byte ptr [ecx + 0x9f0d38], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(10423608) /* 0x9f0d38 */) = cpu.al;
    // 004f3102  83fa7f                 +cmp edx, 0x7f
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(127 /*0x7f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f3105  7cdd                   -jl 0x4f30e4
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004f30e4;
    }
    // 004f3107  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 004f310c  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 004f3111  b91f000000             -mov ecx, 0x1f
    cpu.ecx = 31 /*0x1f*/;
    // 004f3116  891538199f00           -mov dword ptr [0x9f1938], edx
    app->getMemory<x86::reg32>(x86::reg32(10426680) /* 0x9f1938 */) = cpu.edx;
L_0x004f311c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f311c  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f311f  898834199f00           -mov dword ptr [eax + 0x9f1934], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10426676) /* 0x9f1934 */) = cpu.ecx;
    // 004f3125  83f820                 +cmp eax, 0x20
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
    // 004f3128  75f2                   -jne 0x4f311c
    if (!cpu.flags.zf)
    {
        goto L_0x004f311c;
    }
    // 004f312a  bb2f000000             -mov ebx, 0x2f
    cpu.ebx = 47 /*0x2f*/;
    // 004f312f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004f3131:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f3131  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f3134  899854199f00           -mov dword ptr [eax + 0x9f1954], ebx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10426708) /* 0x9f1954 */) = cpu.ebx;
    // 004f313a  83f820                 +cmp eax, 0x20
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
    // 004f313d  75f2                   -jne 0x4f3131
    if (!cpu.flags.zf)
    {
        goto L_0x004f3131;
    }
    // 004f313f  be3f000000             -mov esi, 0x3f
    cpu.esi = 63 /*0x3f*/;
    // 004f3144  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004f3146:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f3146  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f3149  89b0341d9f00           -mov dword ptr [eax + 0x9f1d34], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10427700) /* 0x9f1d34 */) = cpu.esi;
    // 004f314f  3d00020000             +cmp eax, 0x200
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(512 /*0x200*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f3154  75f0                   -jne 0x4f3146
    if (!cpu.flags.zf)
    {
        goto L_0x004f3146;
    }
    // 004f3156  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004f315b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f315b  8b83d8525600           -mov eax, dword ptr [ebx + 0x5652d8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5657304) /* 0x5652d8 */);
    // 004f3161  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004f3163  8b8be0525600           -mov ecx, dword ptr [ebx + 0x5652e0]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5657312) /* 0x5652e0 */);
    // 004f3169  81e600fc0000           -and esi, 0xfc00
    cpu.esi &= x86::reg32(x86::sreg32(64512 /*0xfc00*/));
    // 004f316f  25ff030000             -and eax, 0x3ff
    cpu.eax &= x86::reg32(x86::sreg32(1023 /*0x3ff*/));
    // 004f3174  c1e606                 -shl esi, 6
    cpu.esi <<= 6 /*0x6*/ % 32;
    // 004f3177  c1e016                 -shl eax, 0x16
    cpu.eax <<= 22 /*0x16*/ % 32;
    // 004f317a  8b93d4525600           -mov edx, dword ptr [ebx + 0x5652d4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5657300) /* 0x5652d4 */);
    // 004f3180  09f0                   -or eax, esi
    cpu.eax |= x86::reg32(x86::sreg32(cpu.esi));
    // 004f3182  f6c5fc                 +test ch, 0xfc
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ch & 252 /*0xfc*/));
    // 004f3185  0f8431010000           -je 0x4f32bc
    if (cpu.flags.zf)
    {
        goto L_0x004f32bc;
    }
    // 004f318b  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 004f318d  b909000000             -mov ecx, 9
    cpu.ecx = 9 /*0x9*/;
    // 004f3192  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004f3197  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004f3199  09d0                   -or eax, edx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.edx));
    // 004f319b  d3e7                   -shl edi, cl
    cpu.edi <<= cpu.cl % 32;
    // 004f319d  c1fe07                 -sar esi, 7
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (7 /*0x7*/ % 32));
    // 004f31a0  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 004f31a2  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004f31a4  7e1b                   -jle 0x4f31c1
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004f31c1;
    }
    // 004f31a6  c1e602                 -shl esi, 2
    cpu.esi <<= 2 /*0x2*/ % 32;
    // 004f31a9  8d0cbd00000000         -lea ecx, [edi*4]
    cpu.ecx = x86::reg32(cpu.edi * 4);
    // 004f31b0  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004f31b2  01f1                   -add ecx, esi
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.esi));
L_0x004f31b4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f31b4  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f31b7  898234199f00           -mov dword ptr [edx + 0x9f1934], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(10426676) /* 0x9f1934 */) = cpu.eax;
    // 004f31bd  39ca                   +cmp edx, ecx
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
    // 004f31bf  7cf3                   -jl 0x4f31b4
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004f31b4;
    }
L_0x004f31c1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f31c1  83c310                 -add ebx, 0x10
    (cpu.ebx) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004f31c4  81fbf0050000           +cmp ebx, 0x5f0
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1520 /*0x5f0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f31ca  758f                   -jne 0x4f315b
    if (!cpu.flags.zf)
    {
        goto L_0x004f315b;
    }
    // 004f31cc  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x004f31ce:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f31ce  8b83c8585600           -mov eax, dword ptr [ebx + 0x5658c8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5658824) /* 0x5658c8 */);
    // 004f31d4  8b93c4585600           -mov edx, dword ptr [ebx + 0x5658c4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5658820) /* 0x5658c4 */);
    // 004f31da  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004f31dc  8b8bd0585600           -mov ecx, dword ptr [ebx + 0x5658d0]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5658832) /* 0x5658d0 */);
    // 004f31e2  81e600fc0000           -and esi, 0xfc00
    cpu.esi &= x86::reg32(x86::sreg32(64512 /*0xfc00*/));
    // 004f31e8  25ff030000             -and eax, 0x3ff
    cpu.eax &= x86::reg32(x86::sreg32(1023 /*0x3ff*/));
    // 004f31ed  c1e606                 -shl esi, 6
    cpu.esi <<= 6 /*0x6*/ % 32;
    // 004f31f0  c1e016                 -shl eax, 0x16
    cpu.eax <<= 22 /*0x16*/ % 32;
    // 004f31f3  83c208                 -add edx, 8
    (cpu.edx) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004f31f6  09f0                   -or eax, esi
    cpu.eax |= x86::reg32(x86::sreg32(cpu.esi));
    // 004f31f8  f6c580                 +test ch, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ch & 128 /*0x80*/));
    // 004f31fb  0f85fe000000           -jne 0x4f32ff
    if (!cpu.flags.zf)
    {
        goto L_0x004f32ff;
    }
    // 004f3201  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 004f3203  83ea09                 -sub edx, 9
    (cpu.edx) -= x86::reg32(x86::sreg32(9 /*0x9*/));
    // 004f3206  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 004f320b  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004f3210  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004f3212  c1fe07                 -sar esi, 7
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (7 /*0x7*/ % 32));
    // 004f3215  d3e7                   -shl edi, cl
    cpu.edi <<= cpu.cl % 32;
    // 004f3217  09d0                   -or eax, edx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.edx));
    // 004f3219  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 004f321b  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004f321d  7e1b                   -jle 0x4f323a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004f323a;
    }
    // 004f321f  c1e602                 -shl esi, 2
    cpu.esi <<= 2 /*0x2*/ % 32;
    // 004f3222  8d0cbd00000000         -lea ecx, [edi*4]
    cpu.ecx = x86::reg32(cpu.edi * 4);
    // 004f3229  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004f322b  01f1                   -add ecx, esi
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.esi));
L_0x004f322d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f322d  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f3230  898234119f00           -mov dword ptr [edx + 0x9f1134], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(10424628) /* 0x9f1134 */) = cpu.eax;
    // 004f3236  39ca                   +cmp edx, ecx
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
    // 004f3238  7cf3                   -jl 0x4f322d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004f322d;
    }
L_0x004f323a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f323a  83c310                 -add ebx, 0x10
    (cpu.ebx) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004f323d  81fb00080000           +cmp ebx, 0x800
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2048 /*0x800*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f3243  7589                   -jne 0x4f31ce
    if (!cpu.flags.zf)
    {
        goto L_0x004f31ce;
    }
    // 004f3245  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004f324a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f324c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f324d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x004f324e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f324e  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f3251  89b834109f00           -mov dword ptr [eax + 0x9f1034], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10424372) /* 0x9f1034 */) = cpu.edi;
    // 004f3257  3d80000000             +cmp eax, 0x80
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
    // 004f325c  75f0                   -jne 0x4f324e
    if (!cpu.flags.zf)
    {
        goto L_0x004f324e;
    }
    // 004f325e  ba00004000             -mov edx, 0x400000
    cpu.edx = 4194304 /*0x400000*/;
    // 004f3263  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004f3265:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f3265  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004f3267  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f326a  80c906                 -or cl, 6
    cpu.cl |= x86::reg8(x86::sreg8(6 /*0x6*/));
    // 004f326d  81c200004000           -add edx, 0x400000
    (cpu.edx) += x86::reg32(x86::sreg32(4194304 /*0x400000*/));
    // 004f3273  8988b4109f00           -mov dword ptr [eax + 0x9f10b4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10424500) /* 0x9f10b4 */) = cpu.ecx;
    // 004f3279  83f840                 +cmp eax, 0x40
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
    // 004f327c  75e7                   -jne 0x4f3265
    if (!cpu.flags.zf)
    {
        goto L_0x004f3265;
    }
    // 004f327e  ba000000fc             -mov edx, 0xfc000000
    cpu.edx = 4227858432 /*0xfc000000*/;
    // 004f3283  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004f3285:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f3285  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004f3287  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f328a  80c906                 -or cl, 6
    cpu.cl |= x86::reg8(x86::sreg8(6 /*0x6*/));
    // 004f328d  81c200004000           -add edx, 0x400000
    (cpu.edx) += x86::reg32(x86::sreg32(4194304 /*0x400000*/));
    // 004f3293  8988f4109f00           -mov dword ptr [eax + 0x9f10f4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10424564) /* 0x9f10f4 */) = cpu.ecx;
    // 004f3299  83f840                 +cmp eax, 0x40
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
    // 004f329c  75e7                   -jne 0x4f3285
    if (!cpu.flags.zf)
    {
        goto L_0x004f3285;
    }
    // 004f329e  c70538259f0001000000   -mov dword ptr [0x9f2538], 1
    app->getMemory<x86::reg32>(x86::reg32(10429752) /* 0x9f2538 */) = 1 /*0x1*/;
    // 004f32a8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f32a9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f32aa  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f32ab  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f32ac:
    // 004f32ac  83fa3f                 +cmp edx, 0x3f
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(63 /*0x3f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f32af  0f8e3cfeffff           -jle 0x4f30f1
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004f30f1;
    }
    // 004f32b5  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004f32b7  e935feffff             -jmp 0x4f30f1
    goto L_0x004f30f1;
L_0x004f32bc:
    // 004f32bc  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 004f32be  83ea06                 -sub edx, 6
    (cpu.edx) -= x86::reg32(x86::sreg32(6 /*0x6*/));
    // 004f32c1  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 004f32c6  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004f32cb  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004f32cd  c1fe02                 -sar esi, 2
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (2 /*0x2*/ % 32));
    // 004f32d0  d3e7                   -shl edi, cl
    cpu.edi <<= cpu.cl % 32;
    // 004f32d2  09d0                   -or eax, edx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.edx));
    // 004f32d4  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 004f32d6  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004f32d8  0f8ee3feffff           -jle 0x4f31c1
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004f31c1;
    }
    // 004f32de  c1e602                 -shl esi, 2
    cpu.esi <<= 2 /*0x2*/ % 32;
    // 004f32e1  8d0cbd00000000         -lea ecx, [edi*4]
    cpu.ecx = x86::reg32(cpu.edi * 4);
    // 004f32e8  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004f32ea  01f1                   -add ecx, esi
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.esi));
L_0x004f32ec:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f32ec  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f32ef  898234159f00           -mov dword ptr [edx + 0x9f1534], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(10425652) /* 0x9f1534 */) = cpu.eax;
    // 004f32f5  39ca                   +cmp edx, ecx
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
    // 004f32f7  0f8dc4feffff           -jge 0x4f31c1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004f31c1;
    }
    // 004f32fd  ebed                   -jmp 0x4f32ec
    goto L_0x004f32ec;
L_0x004f32ff:
    // 004f32ff  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 004f3301  83ea06                 -sub edx, 6
    (cpu.edx) -= x86::reg32(x86::sreg32(6 /*0x6*/));
    // 004f3304  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 004f3309  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004f330e  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004f3310  c1fe0a                 -sar esi, 0xa
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (10 /*0xa*/ % 32));
    // 004f3313  d3e7                   -shl edi, cl
    cpu.edi <<= cpu.cl % 32;
    // 004f3315  09d0                   -or eax, edx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.edx));
    // 004f3317  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 004f3319  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004f331b  0f8e19ffffff           -jle 0x4f323a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004f323a;
    }
    // 004f3321  c1e602                 -shl esi, 2
    cpu.esi <<= 2 /*0x2*/ % 32;
    // 004f3324  8d0cbd00000000         -lea ecx, [edi*4]
    cpu.ecx = x86::reg32(cpu.edi * 4);
    // 004f332b  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004f332d  01f1                   -add ecx, esi
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.esi));
L_0x004f332f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f332f  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f3332  898234159f00           -mov dword ptr [edx + 0x9f1534], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(10425652) /* 0x9f1534 */) = cpu.eax;
    // 004f3338  39ca                   +cmp edx, ecx
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
    // 004f333a  0f8dfafeffff           -jge 0x4f323a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004f323a;
    }
    // 004f3340  ebed                   -jmp 0x4f332f
    goto L_0x004f332f;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4f3350(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f3350  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f3351  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f3352  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f3353  8b1544259f00           -mov edx, dword ptr [0x9f2544]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10429764) /* 0x9f2544 */);
    // 004f3359  8b353c259f00           -mov esi, dword ptr [0x9f253c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10429756) /* 0x9f253c */);
    // 004f335f  88c1                   -mov cl, al
    cpu.cl = cpu.al;
    // 004f3361  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004f3363  d3e6                   -shl esi, cl
    cpu.esi <<= cpu.cl % 32;
    // 004f3365  83fa10                 +cmp edx, 0x10
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
    // 004f3368  7c10                   -jl 0x4f337a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004f337a;
    }
    // 004f336a  89353c259f00           -mov dword ptr [0x9f253c], esi
    app->getMemory<x86::reg32>(x86::reg32(10429756) /* 0x9f253c */) = cpu.esi;
    // 004f3370  891544259f00           -mov dword ptr [0x9f2544], edx
    app->getMemory<x86::reg32>(x86::reg32(10429764) /* 0x9f2544 */) = cpu.edx;
    // 004f3376  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f3377  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f3378  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f3379  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f337a:
    // 004f337a  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f337b  a148259f00             -mov eax, dword ptr [0x9f2548]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10429768) /* 0x9f2548 */);
    // 004f3380  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004f3382  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 004f3387  668b18                 -mov bx, word ptr [eax]
    cpu.bx = app->getMemory<x86::reg16>(cpu.eax);
    // 004f338a  83c002                 -add eax, 2
    (cpu.eax) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004f338d  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004f338f  83c210                 -add edx, 0x10
    (cpu.edx) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004f3392  d3e3                   -shl ebx, cl
    cpu.ebx <<= cpu.cl % 32;
    // 004f3394  a348259f00             -mov dword ptr [0x9f2548], eax
    app->getMemory<x86::reg32>(x86::reg32(10429768) /* 0x9f2548 */) = cpu.eax;
    // 004f3399  09de                   -or esi, ebx
    cpu.esi |= x86::reg32(x86::sreg32(cpu.ebx));
    // 004f339b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f339c  89353c259f00           -mov dword ptr [0x9f253c], esi
    app->getMemory<x86::reg32>(x86::reg32(10429756) /* 0x9f253c */) = cpu.esi;
    // 004f33a2  891544259f00           -mov dword ptr [0x9f2544], edx
    app->getMemory<x86::reg32>(x86::reg32(10429764) /* 0x9f2544 */) = cpu.edx;
    // 004f33a8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f33a9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f33aa  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f33ab  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4f33b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f33b0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004f33b1  a13c259f00             -mov eax, dword ptr [0x9f253c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10429756) /* 0x9f253c */);
    // 004f33b6  c1e81a                 -shr eax, 0x1a
    cpu.eax >>= 26 /*0x1a*/ % 32;
    // 004f33b9  8b148538109f00         -mov edx, dword ptr [eax*4 + 0x9f1038]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10424376) /* 0x9f1038 */ + cpu.eax * 4);
    // 004f33c0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004f33c2  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004f33c7  e884ffffff             -call 0x4f3350
    cpu.esp -= 4;
    sub_4f3350(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f33cc  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004f33ce  c1f816                 -sar eax, 0x16
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (22 /*0x16*/ % 32));
    // 004f33d1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f33d2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4f33e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f33e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004f33e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f33e2  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 004f33e4  c1e302                 -shl ebx, 2
    cpu.ebx <<= 2 /*0x2*/ % 32;
    // 004f33e7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004f33e9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f33e9  8b0d2ccf5600           -mov ecx, dword ptr [0x56cf2c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5689132) /* 0x56cf2c */);
    // 004f33ef  8908                   -mov dword ptr [eax], ecx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.ecx;
    // 004f33f1  894804                 -mov dword ptr [eax + 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.ecx;
    // 004f33f4  894808                 -mov dword ptr [eax + 8], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.ecx;
    // 004f33f7  89480c                 -mov dword ptr [eax + 0xc], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = cpu.ecx;
    // 004f33fa  894810                 -mov dword ptr [eax + 0x10], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */) = cpu.ecx;
    // 004f33fd  894814                 -mov dword ptr [eax + 0x14], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */) = cpu.ecx;
    // 004f3400  894818                 -mov dword ptr [eax + 0x18], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = cpu.ecx;
    // 004f3403  42                     -inc edx
    (cpu.edx)++;
    // 004f3404  89481c                 -mov dword ptr [eax + 0x1c], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */) = cpu.ecx;
    // 004f3407  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004f3409  83fa08                 +cmp edx, 8
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
    // 004f340c  7cdb                   -jl 0x4f33e9
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004f33e9;
    }
    // 004f340e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f340f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f3410  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4f3420(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f3420  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f3421  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f3422  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f3423  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004f3425  89d5                   -mov ebp, edx
    cpu.ebp = cpu.edx;
    // 004f3427  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 004f3429  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004f342b  8b4c2410               -mov ecx, dword ptr [esp + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 004f342f  8d3c9d00000000         -lea edi, [ebx*4]
    cpu.edi = x86::reg32(cpu.ebx * 4);
    // 004f3436  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004f3438  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 004f343a  7453                   -je 0x4f348f
    if (cpu.flags.zf)
    {
        goto L_0x004f348f;
    }
    // 004f343c  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x004f343e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f343e  8a5a02                 -mov bl, byte ptr [edx + 2]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(2) /* 0x2 */);
    // 004f3441  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f3443  885802                 -mov byte ptr [eax + 2], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(2) /* 0x2 */) = cpu.bl;
    // 004f3446  8a5a07                 -mov bl, byte ptr [edx + 7]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(7) /* 0x7 */);
    // 004f3449  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f344b  885806                 -mov byte ptr [eax + 6], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(6) /* 0x6 */) = cpu.bl;
    // 004f344e  8a5a06                 -mov bl, byte ptr [edx + 6]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 004f3451  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f3453  88580a                 -mov byte ptr [eax + 0xa], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10) /* 0xa */) = cpu.bl;
    // 004f3456  8a5a0b                 -mov bl, byte ptr [edx + 0xb]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(11) /* 0xb */);
    // 004f3459  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f345b  88580e                 -mov byte ptr [eax + 0xe], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(14) /* 0xe */) = cpu.bl;
    // 004f345e  8a5a0a                 -mov bl, byte ptr [edx + 0xa]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(10) /* 0xa */);
    // 004f3461  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f3463  885812                 -mov byte ptr [eax + 0x12], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(18) /* 0x12 */) = cpu.bl;
    // 004f3466  8a5a0f                 -mov bl, byte ptr [edx + 0xf]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(15) /* 0xf */);
    // 004f3469  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f346b  885816                 -mov byte ptr [eax + 0x16], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(22) /* 0x16 */) = cpu.bl;
    // 004f346e  8a5a0e                 -mov bl, byte ptr [edx + 0xe]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(14) /* 0xe */);
    // 004f3471  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f3473  88581a                 -mov byte ptr [eax + 0x1a], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(26) /* 0x1a */) = cpu.bl;
    // 004f3476  83c040                 -add eax, 0x40
    (cpu.eax) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 004f3479  8a5a13                 -mov bl, byte ptr [edx + 0x13]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(19) /* 0x13 */);
    // 004f347c  46                     -inc esi
    (cpu.esi)++;
    // 004f347d  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f347f  01fa                   -add edx, edi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f3481  8858de                 -mov byte ptr [eax - 0x22], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-34) /* -0x22 */) = cpu.bl;
    // 004f3484  83fe08                 +cmp esi, 8
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
    // 004f3487  7cb5                   -jl 0x4f343e
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004f343e;
    }
    // 004f3489  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f348a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f348b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f348c  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x004f348f:
    // 004f348f  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x004f3491:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f3491  8a5a03                 -mov bl, byte ptr [edx + 3]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(3) /* 0x3 */);
    // 004f3494  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f3496  885802                 -mov byte ptr [eax + 2], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(2) /* 0x2 */) = cpu.bl;
    // 004f3499  8a5a02                 -mov bl, byte ptr [edx + 2]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(2) /* 0x2 */);
    // 004f349c  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f349e  885806                 -mov byte ptr [eax + 6], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(6) /* 0x6 */) = cpu.bl;
    // 004f34a1  8a5a07                 -mov bl, byte ptr [edx + 7]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(7) /* 0x7 */);
    // 004f34a4  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f34a6  88580a                 -mov byte ptr [eax + 0xa], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10) /* 0xa */) = cpu.bl;
    // 004f34a9  8a5a06                 -mov bl, byte ptr [edx + 6]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 004f34ac  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f34ae  88580e                 -mov byte ptr [eax + 0xe], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(14) /* 0xe */) = cpu.bl;
    // 004f34b1  8a5a0b                 -mov bl, byte ptr [edx + 0xb]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(11) /* 0xb */);
    // 004f34b4  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f34b6  885812                 -mov byte ptr [eax + 0x12], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(18) /* 0x12 */) = cpu.bl;
    // 004f34b9  8a5a0a                 -mov bl, byte ptr [edx + 0xa]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(10) /* 0xa */);
    // 004f34bc  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f34be  885816                 -mov byte ptr [eax + 0x16], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(22) /* 0x16 */) = cpu.bl;
    // 004f34c1  8a5a0f                 -mov bl, byte ptr [edx + 0xf]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(15) /* 0xf */);
    // 004f34c4  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f34c6  88581a                 -mov byte ptr [eax + 0x1a], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(26) /* 0x1a */) = cpu.bl;
    // 004f34c9  83c040                 -add eax, 0x40
    (cpu.eax) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 004f34cc  8a5a0e                 -mov bl, byte ptr [edx + 0xe]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(14) /* 0xe */);
    // 004f34cf  46                     -inc esi
    (cpu.esi)++;
    // 004f34d0  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f34d2  01fa                   -add edx, edi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f34d4  8858de                 -mov byte ptr [eax - 0x22], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-34) /* -0x22 */) = cpu.bl;
    // 004f34d7  83fe08                 +cmp esi, 8
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
    // 004f34da  7cb5                   -jl 0x4f3491
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004f3491;
    }
    // 004f34dc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f34dd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f34de  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f34df  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4f34f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004f34f0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f34f1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f34f2  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004f34f4  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 004f34f6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004f34f8  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004f34fa  8b4c240c               -mov ecx, dword ptr [esp + 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 004f34fe  8d3cdd00000000         -lea edi, [ebx*8]
    cpu.edi = x86::reg32(cpu.ebx * 8);
    // 004f3505  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004f3507  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x004f3509:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f3509  8a18                   -mov bl, byte ptr [eax]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax);
    // 004f350b  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f350d  885a02                 -mov byte ptr [edx + 2], bl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(2) /* 0x2 */) = cpu.bl;
    // 004f3510  8a5804                 -mov bl, byte ptr [eax + 4]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004f3513  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f3515  885a06                 -mov byte ptr [edx + 6], bl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(6) /* 0x6 */) = cpu.bl;
    // 004f3518  8a5808                 -mov bl, byte ptr [eax + 8]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004f351b  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f351d  885a0a                 -mov byte ptr [edx + 0xa], bl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(10) /* 0xa */) = cpu.bl;
    // 004f3520  8a580c                 -mov bl, byte ptr [eax + 0xc]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004f3523  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f3525  885a0e                 -mov byte ptr [edx + 0xe], bl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(14) /* 0xe */) = cpu.bl;
    // 004f3528  8a5810                 -mov bl, byte ptr [eax + 0x10]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004f352b  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f352d  885a12                 -mov byte ptr [edx + 0x12], bl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(18) /* 0x12 */) = cpu.bl;
    // 004f3530  8a5814                 -mov bl, byte ptr [eax + 0x14]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 004f3533  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f3535  885a16                 -mov byte ptr [edx + 0x16], bl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(22) /* 0x16 */) = cpu.bl;
    // 004f3538  8a5818                 -mov bl, byte ptr [eax + 0x18]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 004f353b  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f353d  885a1a                 -mov byte ptr [edx + 0x1a], bl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(26) /* 0x1a */) = cpu.bl;
    // 004f3540  83c220                 -add edx, 0x20
    (cpu.edx) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 004f3543  8a581c                 -mov bl, byte ptr [eax + 0x1c]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004f3546  46                     -inc esi
    (cpu.esi)++;
    // 004f3547  00cb                   -add bl, cl
    (cpu.bl) += x86::reg8(x86::sreg8(cpu.cl));
    // 004f3549  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f354b  885afe                 -mov byte ptr [edx - 2], bl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(-2) /* -0x2 */) = cpu.bl;
    // 004f354e  83fe08                 +cmp esi, 8
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
    // 004f3551  7cb6                   -jl 0x4f3509
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004f3509;
    }
    // 004f3553  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f3554  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f3555  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4f3560(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (madBeginFrame(app, cpu.sync())) /* port: native (tools/apply_native_movie.py) */
    {
        cpu.reload();
        return;
    }
    // 004f3560  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f3561  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f3563  833d38259f0000         +cmp dword ptr [0x9f2538], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10429752) /* 0x9f2538 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f356a  0f8486000000           -je 0x4f35f6
    if (cpu.flags.zf)
    {
        goto L_0x004f35f6;
    }
L_0x004f3570:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f3570  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f3571  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f3572  bf20000000             -mov edi, 0x20
    cpu.edi = 32 /*0x20*/;
    // 004f3577  83c104                 -add ecx, 4
    (cpu.ecx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f357a  891540259f00           -mov dword ptr [0x9f2540], edx
    app->getMemory<x86::reg32>(x86::reg32(10429760) /* 0x9f2540 */) = cpu.edx;
    // 004f3580  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004f3582  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004f3584  8b152cd05600           -mov edx, dword ptr [0x56d02c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5689388) /* 0x56d02c */);
    // 004f358a  893d44259f00           -mov dword ptr [0x9f2544], edi
    app->getMemory<x86::reg32>(x86::reg32(10429764) /* 0x9f2544 */) = cpu.edi;
    // 004f3590  668b71fc               -mov si, word ptr [ecx - 4]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(-4) /* -0x4 */);
    // 004f3594  668b41fe               -mov ax, word ptr [ecx - 2]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(-2) /* -0x2 */);
    // 004f3598  c1e610                 -shl esi, 0x10
    cpu.esi <<= 16 /*0x10*/ % 32;
    // 004f359b  890d48259f00           -mov dword ptr [0x9f2548], ecx
    app->getMemory<x86::reg32>(x86::reg32(10429768) /* 0x9f2548 */) = cpu.ecx;
    // 004f35a1  09c6                   -or esi, eax
    cpu.esi |= x86::reg32(x86::sreg32(cpu.eax));
    // 004f35a3  a1c4605600             -mov eax, dword ptr [0x5660c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5660868) /* 0x5660c4 */);
    // 004f35a8  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 004f35ad  c1e00f                 -shl eax, 0xf
    cpu.eax <<= 15 /*0xf*/ % 32;
    // 004f35b0  89353c259f00           -mov dword ptr [0x9f253c], esi
    app->getMemory<x86::reg32>(x86::reg32(10429756) /* 0x9f253c */) = cpu.esi;
    // 004f35b6  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004f35b8  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004f35bb  c1e810                 +shr eax, 0x10
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
    // 004f35be  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004f35c0  a3380c9f00             -mov dword ptr [0x9f0c38], eax
    app->getMemory<x86::reg32>(x86::reg32(10423352) /* 0x9f0c38 */) = cpu.eax;
    // 004f35c5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f35c6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x004f35c7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f35c7  8b81c4605600           -mov eax, dword ptr [ecx + 0x5660c4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5660868) /* 0x5660c4 */);
    // 004f35cd  0fafc3                 -imul eax, ebx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 004f35d0  8b912cd05600           -mov edx, dword ptr [ecx + 0x56d02c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5689388) /* 0x56d02c */);
    // 004f35d6  c1e00c                 -shl eax, 0xc
    cpu.eax <<= 12 /*0xc*/ % 32;
    // 004f35d9  83c104                 -add ecx, 4
    (cpu.ecx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004f35dc  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004f35de  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004f35e1  c1e810                 +shr eax, 0x10
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
    // 004f35e4  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004f35e6  8981340c9f00           -mov dword ptr [ecx + 0x9f0c34], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(10423348) /* 0x9f0c34 */) = cpu.eax;
    // 004f35ec  81f900010000           +cmp ecx, 0x100
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(256 /*0x100*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004f35f2  75d3                   -jne 0x4f35c7
    if (!cpu.flags.zf)
    {
        goto L_0x004f35c7;
    }
    // 004f35f4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f35f5  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f35f6:
    // 004f35f6  e8d5faffff             -call 0x4f30d0
    cpu.esp -= 4;
    sub_4f30d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f35fb  e970ffffff             -jmp 0x4f3570
    goto L_0x004f3570;
}

/* align: skip  */
void Application::sub_4f3600(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (madMacroblock(app, cpu.sync())) /* port: native (tools/apply_native_movie.py) */
    {
        cpu.reload();
        return;
    }
    // 004f3600  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004f3601  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004f3602  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004f3603  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004f3606  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 004f3608  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004f360a  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 004f360c  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004f360d  8b1540259f00           -mov edx, dword ptr [0x9f2540]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10429760) /* 0x9f2540 */);
    // 004f3613  d1ff                   -sar edi, 1
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (1 /*0x1*/ % 32));
    // 004f3615  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004f3617  0f843c020000           -je 0x4f3859
    if (cpu.flags.zf)
    {
        goto L_0x004f3859;
    }
    // 004f361d  8a253f259f00           -mov ah, byte ptr [0x9f253f]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(10429759) /* 0x9f253f */);
    // 004f3623  f6c4c0                 +test ah, 0xc0
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 192 /*0xc0*/));
    // 004f3626  0f8536020000           -jne 0x4f3862
    if (!cpu.flags.zf)
    {
        goto L_0x004f3862;
    }
    // 004f362c  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 004f3631  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004f3633  e818fdffff             -call 0x4f3350
    cpu.esp -= 4;
    sub_4f3350(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3638  895c2408               -mov dword ptr [esp + 8], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.ebx;
L_0x004f363c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f363c  e807d80100             -call 0x510e48
    cpu.esp -= 4;
    sub_510e48(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3641  83f801                 +cmp eax, 1
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
    // 004f3644  0f8598020000           -jne 0x4f38e2
    if (!cpu.flags.zf)
    {
        goto L_0x004f38e2;
    }
    // 004f364a  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 004f364f  b838219f00             -mov eax, 0x9f2138
    cpu.eax = 10428728 /*0x9f2138*/;
    // 004f3654  e887fdffff             -call 0x4f33e0
    cpu.esp -= 4;
    sub_4f33e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f3659:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f3659  f644240802             +test byte ptr [esp + 8], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.esp + x86::reg32(8) /* 0x8 */) & 2 /*0x2*/));
    // 004f365e  0f85a6020000           -jne 0x4f390a
    if (!cpu.flags.zf)
    {
        goto L_0x004f390a;
    }
    // 004f3664  e8dfd70100             -call 0x510e48
    cpu.esp -= 4;
    sub_510e48(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3669  83f801                 +cmp eax, 1
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
    // 004f366c  0f8584020000           -jne 0x4f38f6
    if (!cpu.flags.zf)
    {
        goto L_0x004f38f6;
    }
    // 004f3672  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 004f3677  b858219f00             -mov eax, 0x9f2158
    cpu.eax = 10428760 /*0x9f2158*/;
    // 004f367c  e85ffdffff             -call 0x4f33e0
    cpu.esp -= 4;
    sub_4f33e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f3681:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f3681  f644240804             +test byte ptr [esp + 8], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.esp + x86::reg32(8) /* 0x8 */) & 4 /*0x4*/));
    // 004f3686  0f85b3020000           -jne 0x4f393f
    if (!cpu.flags.zf)
    {
        goto L_0x004f393f;
    }
    // 004f368c  e8b7d70100             -call 0x510e48
    cpu.esp -= 4;
    sub_510e48(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3691  83f801                 +cmp eax, 1
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
    // 004f3694  0f8591020000           -jne 0x4f392b
    if (!cpu.flags.zf)
    {
        goto L_0x004f392b;
    }
    // 004f369a  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 004f369f  b838239f00             -mov eax, 0x9f2338
    cpu.eax = 10429240 /*0x9f2338*/;
    // 004f36a4  e837fdffff             -call 0x4f33e0
    cpu.esp -= 4;
    sub_4f33e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f36a9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f36a9  f644240808             +test byte ptr [esp + 8], 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.esp + x86::reg32(8) /* 0x8 */) & 8 /*0x8*/));
    // 004f36ae  0f85c4020000           -jne 0x4f3978
    if (!cpu.flags.zf)
    {
        goto L_0x004f3978;
    }
    // 004f36b4  e88fd70100             -call 0x510e48
    cpu.esp -= 4;
    sub_510e48(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f36b9  83f801                 +cmp eax, 1
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
    // 004f36bc  0f85a2020000           -jne 0x4f3964
    if (!cpu.flags.zf)
    {
        goto L_0x004f3964;
    }
    // 004f36c2  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 004f36c7  b858239f00             -mov eax, 0x9f2358
    cpu.eax = 10429272 /*0x9f2358*/;
    // 004f36cc  e80ffdffff             -call 0x4f33e0
    cpu.esp -= 4;
    sub_4f33e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f36d1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f36d1  f644240810             +test byte ptr [esp + 8], 0x10
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.esp + x86::reg32(8) /* 0x8 */) & 16 /*0x10*/));
    // 004f36d6  0f85d8020000           -jne 0x4f39b4
    if (!cpu.flags.zf)
    {
        goto L_0x004f39b4;
    }
    // 004f36dc  e867d70100             -call 0x510e48
    cpu.esp -= 4;
    sub_510e48(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f36e1  83f801                 +cmp eax, 1
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
    // 004f36e4  0f85b6020000           -jne 0x4f39a0
    if (!cpu.flags.zf)
    {
        goto L_0x004f39a0;
    }
    // 004f36ea  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 004f36ef  b8380e9f00             -mov eax, 0x9f0e38
    cpu.eax = 10423864 /*0x9f0e38*/;
    // 004f36f4  e8e7fcffff             -call 0x4f33e0
    cpu.esp -= 4;
    sub_4f33e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f36f9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f36f9  f644240820             +test byte ptr [esp + 8], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.esp + x86::reg32(8) /* 0x8 */) & 32 /*0x20*/));
    // 004f36fe  0f85e5020000           -jne 0x4f39e9
    if (!cpu.flags.zf)
    {
        goto L_0x004f39e9;
    }
    // 004f3704  e83fd70100             -call 0x510e48
    cpu.esp -= 4;
    sub_510e48(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3709  83f801                 +cmp eax, 1
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
    // 004f370c  0f85c3020000           -jne 0x4f39d5
    if (!cpu.flags.zf)
    {
        goto L_0x004f39d5;
    }
    // 004f3712  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 004f3717  b8380f9f00             -mov eax, 0x9f0f38
    cpu.eax = 10424120 /*0x9f0f38*/;
    // 004f371c  e8bffcffff             -call 0x4f33e0
    cpu.esp -= 4;
    sub_4f33e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004f3721:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f3721  ba380e9f00             -mov edx, 0x9f0e38
    cpu.edx = 10423864 /*0x9f0e38*/;
    // 004f3726  b838219f00             -mov eax, 0x9f2138
    cpu.eax = 10428728 /*0x9f2138*/;
    // 004f372b  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f372d  e8c6d80100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3732  c1e702                 -shl edi, 2
    cpu.edi <<= 2 /*0x2*/ % 32;
    // 004f3735  ba380e9f00             -mov edx, 0x9f0e38
    cpu.edx = 10423864 /*0x9f0e38*/;
    // 004f373a  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f373c  b878219f00             -mov eax, 0x9f2178
    cpu.eax = 10428792 /*0x9f2178*/;
    // 004f3741  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f3743  e8b0d80100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3748  ba580e9f00             -mov edx, 0x9f0e58
    cpu.edx = 10423896 /*0x9f0e58*/;
    // 004f374d  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f374f  b8b8219f00             -mov eax, 0x9f21b8
    cpu.eax = 10428856 /*0x9f21b8*/;
    // 004f3754  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f3756  e89dd80100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f375b  ba580e9f00             -mov edx, 0x9f0e58
    cpu.edx = 10423896 /*0x9f0e58*/;
    // 004f3760  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f3762  b8f8219f00             -mov eax, 0x9f21f8
    cpu.eax = 10428920 /*0x9f21f8*/;
    // 004f3767  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f3769  e88ad80100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f376e  ba780e9f00             -mov edx, 0x9f0e78
    cpu.edx = 10423928 /*0x9f0e78*/;
    // 004f3773  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f3775  b838229f00             -mov eax, 0x9f2238
    cpu.eax = 10428984 /*0x9f2238*/;
    // 004f377a  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f377c  e877d80100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3781  ba780e9f00             -mov edx, 0x9f0e78
    cpu.edx = 10423928 /*0x9f0e78*/;
    // 004f3786  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f3788  b878229f00             -mov eax, 0x9f2278
    cpu.eax = 10429048 /*0x9f2278*/;
    // 004f378d  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f378f  e864d80100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3794  ba980e9f00             -mov edx, 0x9f0e98
    cpu.edx = 10423960 /*0x9f0e98*/;
    // 004f3799  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f379b  b8b8229f00             -mov eax, 0x9f22b8
    cpu.eax = 10429112 /*0x9f22b8*/;
    // 004f37a0  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f37a2  e851d80100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f37a7  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f37a9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f37aa  ba980e9f00             -mov edx, 0x9f0e98
    cpu.edx = 10423960 /*0x9f0e98*/;
    // 004f37af  b8f8229f00             -mov eax, 0x9f22f8
    cpu.eax = 10429176 /*0x9f22f8*/;
    // 004f37b4  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f37b6  e83dd80100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f37bb  bab80e9f00             -mov edx, 0x9f0eb8
    cpu.edx = 10423992 /*0x9f0eb8*/;
    // 004f37c0  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f37c2  b838239f00             -mov eax, 0x9f2338
    cpu.eax = 10429240 /*0x9f2338*/;
    // 004f37c7  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f37c9  e82ad80100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f37ce  bab80e9f00             -mov edx, 0x9f0eb8
    cpu.edx = 10423992 /*0x9f0eb8*/;
    // 004f37d3  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f37d5  b878239f00             -mov eax, 0x9f2378
    cpu.eax = 10429304 /*0x9f2378*/;
    // 004f37da  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f37dc  e817d80100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f37e1  bad80e9f00             -mov edx, 0x9f0ed8
    cpu.edx = 10424024 /*0x9f0ed8*/;
    // 004f37e6  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f37e8  b8b8239f00             -mov eax, 0x9f23b8
    cpu.eax = 10429368 /*0x9f23b8*/;
    // 004f37ed  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f37ef  e804d80100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f37f4  bad80e9f00             -mov edx, 0x9f0ed8
    cpu.edx = 10424024 /*0x9f0ed8*/;
    // 004f37f9  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f37fb  b8f8239f00             -mov eax, 0x9f23f8
    cpu.eax = 10429432 /*0x9f23f8*/;
    // 004f3800  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f3802  e8f1d70100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3807  baf80e9f00             -mov edx, 0x9f0ef8
    cpu.edx = 10424056 /*0x9f0ef8*/;
    // 004f380c  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f380e  b838249f00             -mov eax, 0x9f2438
    cpu.eax = 10429496 /*0x9f2438*/;
    // 004f3813  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f3815  e8ded70100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f381a  baf80e9f00             -mov edx, 0x9f0ef8
    cpu.edx = 10424056 /*0x9f0ef8*/;
    // 004f381f  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f3821  b878249f00             -mov eax, 0x9f2478
    cpu.eax = 10429560 /*0x9f2478*/;
    // 004f3826  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f3828  e8cbd70100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f382d  ba180f9f00             -mov edx, 0x9f0f18
    cpu.edx = 10424088 /*0x9f0f18*/;
    // 004f3832  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004f3834  b8b8249f00             -mov eax, 0x9f24b8
    cpu.eax = 10429624 /*0x9f24b8*/;
    // 004f3839  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004f383b  e8b8d70100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3840  ba180f9f00             -mov edx, 0x9f0f18
    cpu.edx = 10424088 /*0x9f0f18*/;
    // 004f3845  b8f8249f00             -mov eax, 0x9f24f8
    cpu.eax = 10429688 /*0x9f24f8*/;
    // 004f384a  8d1c3e                 -lea ebx, [esi + edi]
    cpu.ebx = x86::reg32(cpu.esi + cpu.edi * 1);
    // 004f384d  e8a6d70100             -call 0x510ff8
    cpu.esp -= 4;
    sub_510ff8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3852  83c408                 +add esp, 8
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
    // 004f3855  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f3856  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f3857  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004f3858  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004f3859:
    // 004f3859  89542408               -mov dword ptr [esp + 8], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.edx;
    // 004f385d  e9dafdffff             -jmp 0x4f363c
    goto L_0x004f363c;
L_0x004f3862:
    // 004f3862  f6c480                 +test ah, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 128 /*0x80*/));
    // 004f3865  7463                   -je 0x4f38ca
    if (cpu.flags.zf)
    {
        goto L_0x004f38ca;
    }
    // 004f3867  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004f386c  b9ff030000             -mov ecx, 0x3ff
    cpu.ecx = 1023 /*0x3ff*/;
    // 004f3871  e8dafaffff             -call 0x4f3350
    cpu.esp -= 4;
    sub_4f3350(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3876  894c2408               -mov dword ptr [esp + 8], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.ecx;
L_0x004f387a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004f387a  e831fbffff             -call 0x4f33b0
    cpu.esp -= 4;
    sub_4f33b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f387f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004f3881  e82afbffff             -call 0x4f33b0
    cpu.esp -= 4;
    sub_4f33b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3886  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004f3888  0fafcf                 -imul ecx, edi
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(cpu.edi)));
    // 004f388b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004f388d  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004f388f  83e201                 -and edx, 1
    cpu.edx &= x86::reg32(x86::sreg32(1 /*0x1*/));
    // 004f3892  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004f3894  89542404               -mov dword ptr [esp + 4], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 004f3898  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004f389b  8a742408               -mov dh, byte ptr [esp + 8]
    cpu.dh = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 004f389f  01c5                   -add ebp, eax
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.eax));
    // 004f38a1  f6c601                 +test dh, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dh & 1 /*0x1*/));
    // 004f38a4  0f8492fdffff           -je 0x4f363c
    if (cpu.flags.zf)
    {
        goto L_0x004f363c;
    }
    // 004f38aa  e801fbffff             -call 0x4f33b0
    cpu.esp -= 4;
    sub_4f33b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f38af  b938219f00             -mov ecx, 0x9f2138
    cpu.ecx = 10428728 /*0x9f2138*/;
    // 004f38b4  83e840                 +sub eax, 0x40
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f38b7  8b542404               -mov edx, dword ptr [esp + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 004f38bb  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004f38bc  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004f38be  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 004f38c0  e85bfbffff             -call 0x4f3420
    cpu.esp -= 4;
    sub_4f3420(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f38c5  e98ffdffff             -jmp 0x4f3659
    goto L_0x004f3659;
L_0x004f38ca:
    // 004f38ca  a13c259f00             -mov eax, dword ptr [0x9f253c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10429756) /* 0x9f253c */);
    // 004f38cf  c1e818                 +shr eax, 0x18
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
    // 004f38d2  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004f38d6  b808000000             -mov eax, 8
    cpu.eax = 8 /*0x8*/;
    // 004f38db  e870faffff             -call 0x4f3350
    cpu.esp -= 4;
    sub_4f3350(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f38e0  eb98                   -jmp 0x4f387a
    goto L_0x004f387a;
L_0x004f38e2:
    // 004f38e2  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 004f38e7  b838219f00             -mov eax, 0x9f2138
    cpu.eax = 10428728 /*0x9f2138*/;
    // 004f38ec  e8ccd30100             -call 0x510cbd
    cpu.esp -= 4;
    sub_510cbd(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f38f1  e963fdffff             -jmp 0x4f3659
    goto L_0x004f3659;
L_0x004f38f6:
    // 004f38f6  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 004f38fb  b858219f00             -mov eax, 0x9f2158
    cpu.eax = 10428760 /*0x9f2158*/;
    // 004f3900  e8b8d30100             -call 0x510cbd
    cpu.esp -= 4;
    sub_510cbd(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3905  e977fdffff             -jmp 0x4f3681
    goto L_0x004f3681;
L_0x004f390a:
    // 004f390a  e8a1faffff             -call 0x4f33b0
    cpu.esp -= 4;
    sub_4f33b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f390f  b958219f00             -mov ecx, 0x9f2158
    cpu.ecx = 10428760 /*0x9f2158*/;
    // 004f3914  83e840                 +sub eax, 0x40
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f3917  8b542404               -mov edx, dword ptr [esp + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 004f391b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004f391c  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004f391e  8d4510                 -lea eax, [ebp + 0x10]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004f3921  e8fafaffff             -call 0x4f3420
    cpu.esp -= 4;
    sub_4f3420(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3926  e956fdffff             -jmp 0x4f3681
    goto L_0x004f3681;
L_0x004f392b:
    // 004f392b  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 004f3930  b838239f00             -mov eax, 0x9f2338
    cpu.eax = 10429240 /*0x9f2338*/;
    // 004f3935  e883d30100             -call 0x510cbd
    cpu.esp -= 4;
    sub_510cbd(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f393a  e96afdffff             -jmp 0x4f36a9
    goto L_0x004f36a9;
L_0x004f393f:
    // 004f393f  e86cfaffff             -call 0x4f33b0
    cpu.esp -= 4;
    sub_4f33b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3944  83e840                 -sub eax, 0x40
    (cpu.eax) -= x86::reg32(x86::sreg32(64 /*0x40*/));
    // 004f3947  b938239f00             -mov ecx, 0x9f2338
    cpu.ecx = 10429240 /*0x9f2338*/;
    // 004f394c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004f394d  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004f394f  8b542408               -mov edx, dword ptr [esp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 004f3953  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 004f3956  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004f3958  01e8                   +add eax, ebp
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f395a  e8c1faffff             -call 0x4f3420
    cpu.esp -= 4;
    sub_4f3420(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f395f  e945fdffff             -jmp 0x4f36a9
    goto L_0x004f36a9;
L_0x004f3964:
    // 004f3964  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 004f3969  b858239f00             -mov eax, 0x9f2358
    cpu.eax = 10429272 /*0x9f2358*/;
    // 004f396e  e84ad30100             -call 0x510cbd
    cpu.esp -= 4;
    sub_510cbd(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3973  e959fdffff             -jmp 0x4f36d1
    goto L_0x004f36d1;
L_0x004f3978:
    // 004f3978  e833faffff             -call 0x4f33b0
    cpu.esp -= 4;
    sub_4f33b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f397d  83e840                 -sub eax, 0x40
    (cpu.eax) -= x86::reg32(x86::sreg32(64 /*0x40*/));
    // 004f3980  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004f3981  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004f3983  b958239f00             -mov ecx, 0x9f2358
    cpu.ecx = 10429272 /*0x9f2358*/;
    // 004f3988  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 004f398b  8b542408               -mov edx, dword ptr [esp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 004f398f  01e8                   -add eax, ebp
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebp));
    // 004f3991  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004f3993  83c010                 +add eax, 0x10
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
    // 004f3996  e885faffff             -call 0x4f3420
    cpu.esp -= 4;
    sub_4f3420(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f399b  e931fdffff             -jmp 0x4f36d1
    goto L_0x004f36d1;
L_0x004f39a0:
    // 004f39a0  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 004f39a5  b8380e9f00             -mov eax, 0x9f0e38
    cpu.eax = 10423864 /*0x9f0e38*/;
    // 004f39aa  e80ed30100             -call 0x510cbd
    cpu.esp -= 4;
    sub_510cbd(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f39af  e945fdffff             -jmp 0x4f36f9
    goto L_0x004f36f9;
L_0x004f39b4:
    // 004f39b4  e8f7f9ffff             -call 0x4f33b0
    cpu.esp -= 4;
    sub_4f33b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f39b9  b9380e9f00             -mov ecx, 0x9f0e38
    cpu.ecx = 10423864 /*0x9f0e38*/;
    // 004f39be  83e840                 +sub eax, 0x40
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004f39c1  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004f39c6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004f39c7  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004f39c9  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 004f39cb  e820fbffff             -call 0x4f34f0
    cpu.esp -= 4;
    sub_4f34f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f39d0  e924fdffff             -jmp 0x4f36f9
    goto L_0x004f36f9;
L_0x004f39d5:
    // 004f39d5  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 004f39da  b8380f9f00             -mov eax, 0x9f0f38
    cpu.eax = 10424120 /*0x9f0f38*/;
    // 004f39df  e8d9d20100             -call 0x510cbd
    cpu.esp -= 4;
    sub_510cbd(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f39e4  e938fdffff             -jmp 0x4f3721
    goto L_0x004f3721;
L_0x004f39e9:
    // 004f39e9  e8c2f9ffff             -call 0x4f33b0
    cpu.esp -= 4;
    sub_4f33b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f39ee  b9380f9f00             -mov ecx, 0x9f0f38
    cpu.ecx = 10424120 /*0x9f0f38*/;
    // 004f39f3  83e840                 -sub eax, 0x40
    (cpu.eax) -= x86::reg32(x86::sreg32(64 /*0x40*/));
    // 004f39f6  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004f39f8  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004f39f9  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004f39fb  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 004f39fd  e8eefaffff             -call 0x4f34f0
    cpu.esp -= 4;
    sub_4f34f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004f3a02  e91afdffff             -jmp 0x4f3721
    goto L_0x004f3721;
}

/* sub_4f3a10: native, nfs3hp::port::streamMemory */

}
