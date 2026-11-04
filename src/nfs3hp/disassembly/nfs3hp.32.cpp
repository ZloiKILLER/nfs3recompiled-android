#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4df310(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004df310  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004df311  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004df312  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004df313  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004df314  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004df316  e865c70000             -call 0x4eba80
    cpu.esp -= 4;
    sub_4eba80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df31b  3b05c0a28c00           +cmp eax, dword ptr [0x8ca2c0]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(9216704) /* 0x8ca2c0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004df321  7518                   -jne 0x4df33b
    if (!cpu.flags.zf)
    {
        goto L_0x004df33b;
    }
    // 004df323  8b0dc8a28c00           -mov ecx, dword ptr [0x8ca2c8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(9216712) /* 0x8ca2c8 */);
    // 004df329  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004df32b  740e                   -je 0x4df33b
    if (cpu.flags.zf)
    {
        goto L_0x004df33b;
    }
    // 004df32d  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004df32e  8b11                   -mov edx, dword ptr [ecx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx);
    // 004df330  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004df332  ff5208                 -call dword ptr [edx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df335  891dc8a28c00           -mov dword ptr [0x8ca2c8], ebx
    app->getMemory<x86::reg32>(x86::reg32(9216712) /* 0x8ca2c8 */) = cpu.ebx;
L_0x004df33b:
    // 004df33b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df33c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df33d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df33e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df33f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4df340(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004df340  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004df341  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004df343  a1d4a28c00             -mov eax, dword ptr [0x8ca2d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216724) /* 0x8ca2d4 */);
    // 004df348  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df349  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4df350: native, nfs3hp::dead::stub_0x4df350 */

/* sub_4df380: native, nfs3hp::dead::stub_0x4df380 */

/* sub_4df390: native, nfs3hp::dead::stub_0x4df390 */

/* sub_4df3a0: native, nfs3hp::dead::stub_0x4df3a0 */

/* sub_4df420: native, nfs3hp::dead::stub_0x4df420 */

/* sub_4df530: native, nfs3hp::dead::stub_0x4df530 */

/* sub_4df570: native, nfs3hp::dead::stub_0x4df570 */

/* sub_4df5c0: native, nfs3hp::dead::stub_0x4df5c0 */

/* sub_4df660: native, nfs3hp::watcom::mem_putc */

/* sub_4df674: native, nfs3hp::watcom::vsprintf */

/* sub_4df690: native, nfs3hp::watcom::sprintf */

/* sub_4df6c0: native, nfs3hp::Message */

/* sub_4df6f0: native, nfs3hp::ea::waitForObjects */

/* sub_4df730: native, nfs3hp::ea::threadStart */

/* sub_4df740: native, nfs3hp::ea::threadsInit */

/* sub_4df7a0: native, nfs3hp::ea::createThread */

/* sub_4df870: native, nfs3hp::ea::removeThread */

/* sub_4df8e0: native, nfs3hp::ea::sleepMs */

/* sub_4df8f0: native, nfs3hp::ea::sleepTicks */

/* sub_4df930: native, nfs3hp::ea::isThread */

/* sub_4df980: native, nfs3hp::ea::threadPriority */

/* sub_4dfa30: native, nfs3hp::ea::setThreadPriority */

/* sub_4dfaf0: native, nfs3hp::ea::exitThread */

/* sub_4dfb00: native, nfs3hp::ea::createEvent */

/* sub_4dfb20: native, nfs3hp::ea::postEvent */

/* sub_4dfb30: native, nfs3hp::ea::signalEvent */

/* sub_4dfb40: native, nfs3hp::ea::pollEvent */

/* sub_4dfb70: native, nfs3hp::ea::waitEvent */

/* sub_4dfb90: native, nfs3hp::ea::waitEventTicks */

/* sub_4dfbf0: native, nfs3hp::ea::waitForAny */

/* sub_4dfc00: native, nfs3hp::ea::closeEvent */

/* sub_4dfc10: native, nfs3hp::ea::createManualEvent */

/* sub_4dfc30: native, nfs3hp::ea::setEvent */

/* sub_4dfc40: native, nfs3hp::ea::resetEvent */

/* sub_4dfc50: native, nfs3hp::ea::pulseEvent */

/* sub_4dfc60: native, nfs3hp::ea::waitManualEvent */

/* sub_4dfc80: native, nfs3hp::ea::setAndCloseEvent */

/* sub_4dfca0: native, nfs3hp::ea::lastErrorText */

/* sub_4dfce0: native, nfs3hp::ea::setLastError */

/* sub_4dfcf0: native, nfs3hp::ea::lastError */

/* sub_4dfd00: native, nfs3hp::ea::doNothing */

/* sub_4dfd04: native, nfs3hp::watcom::exit */

/* sub_4dfd30: native, nfs3hp::watcom::_exit */

/* sub_4dfd56: native, nfs3hp::watcom::CHP */

/* sub_4dfd80: native, nfs3hp::ea::xRotation */

/* sub_4dfde0: native, nfs3hp::ea::yRotation */

/* sub_4dfe40: native, nfs3hp::ea::zRotation */

/* sub_4dfea0: native, nfs3hp::ea::axisRotation */

/* sub_4dffb0: native, nfs3hp::vertices::addVectors */

/* sub_4e0000: native, nfs3hp::vertices::subtractVectors */

/* sub_4e0050: native, nfs3hp::vertices::scaleVectors */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4e0090(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 004e0090  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004e0093  d902                   -fld dword ptr [edx]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx));
    // 004e0095  d820                   -fsub dword ptr [eax]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax)));
    // 004e0097  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 004e009a  d94204                 -fld dword ptr [edx + 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */));
    // 004e009d  d86004                 -fsub dword ptr [eax + 4]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */)));
    // 004e00a0  d95c2404               -fstp dword ptr [esp + 4]
    app->getMemory<float>(cpu.esp + x86::reg32(4) /* 0x4 */) = float(fp0);
    // 004e00a4  d94208                 -fld dword ptr [edx + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */));
    // 004e00a7  d86008                 -fsub dword ptr [eax + 8]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 004e00aa  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 004e00ac  d95c2408               -fstp dword ptr [esp + 8]
    app->getMemory<float>(cpu.esp + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 004e00b0  e8eb050000             -call 0x4e06a0
    cpu.esp -= 4;
    nfs3hp::vertices::vectorLengthOf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004e00b5  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004e00b8  c3                     -ret 
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fp0;
    cpu.esp += 4;
    return;
}

/* sub_4e00c0: native, nfs3hp::dead::stub_0x4e00c0 */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4e0110(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 004e0110  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004e0111  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004e0114  8b08                   -mov ecx, dword ptr [eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax);
    // 004e0116  890a                   -mov dword ptr [edx], ecx
    app->getMemory<x86::reg32>(cpu.edx) = cpu.ecx;
    // 004e0118  8b4810                 -mov ecx, dword ptr [eax + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004e011b  894a10                 -mov dword ptr [edx + 0x10], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(16) /* 0x10 */) = cpu.ecx;
    // 004e011e  8b4820                 -mov ecx, dword ptr [eax + 0x20]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(32) /* 0x20 */);
    // 004e0121  894a20                 -mov dword ptr [edx + 0x20], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(32) /* 0x20 */) = cpu.ecx;
    // 004e0124  d9400c                 -fld dword ptr [eax + 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(12) /* 0xc */));
    // 004e0127  8b4804                 -mov ecx, dword ptr [eax + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004e012a  d95a04                 -fstp dword ptr [edx + 4]
    app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */) = float(fp0);
    // 004e012d  894a0c                 -mov dword ptr [edx + 0xc], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */) = cpu.ecx;
    // 004e0130  890c24                 -mov dword ptr [esp], ecx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ecx;
    // 004e0133  d94018                 -fld dword ptr [eax + 0x18]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(24) /* 0x18 */));
    // 004e0136  8b4808                 -mov ecx, dword ptr [eax + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004e0139  d95a08                 -fstp dword ptr [edx + 8]
    app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 004e013c  894a18                 -mov dword ptr [edx + 0x18], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(24) /* 0x18 */) = cpu.ecx;
    // 004e013f  890c24                 -mov dword ptr [esp], ecx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ecx;
    // 004e0142  d9401c                 -fld dword ptr [eax + 0x1c]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(28) /* 0x1c */));
    // 004e0145  8b4814                 -mov ecx, dword ptr [eax + 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 004e0148  d95a14                 -fstp dword ptr [edx + 0x14]
    app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */) = float(fp0);
    // 004e014b  890c24                 -mov dword ptr [esp], ecx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ecx;
    // 004e014e  894a1c                 -mov dword ptr [edx + 0x1c], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */) = cpu.ecx;
    // 004e0151  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004e0154  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004e0155  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4e0160(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
    // 004e0160  d94004                 -fld dword ptr [eax + 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004e0163  d84a08                 -fmul dword ptr [edx + 8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004e0166  d94008                 -fld dword ptr [eax + 8]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004e0169  d80a                   -fmul dword ptr [edx]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx)));
    // 004e016b  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004e016d  d84a04                 -fmul dword ptr [edx + 4]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 004e0170  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004e0172  d94008                 -fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004e0175  d84a04                 -fmul dword ptr [edx + 4]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 004e0178  d900                   -fld dword ptr [eax]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004e017a  d84a08                 -fmul dword ptr [edx + 8]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004e017d  d94004                 -fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004e0180  d80a                   -fmul dword ptr [edx]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx)));
    // 004e0182  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 004e0184  deeb                   -fsubp st(3)
    fp2 = fp::sub(fp2, fp5);
    // 004e0186  deeb                   -fsubp st(3)
    fp1 = fp::sub(fp1, fp4);
    // 004e0188  deeb                   -fsubp st(3)
    fp0 = fp::sub(fp0, fp3);
    // 004e018a  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004e018d  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 004e018f  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 004e0192  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 004e0195  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4e01a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004e01a0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004e01a1  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004e01a4  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 004e01a6  89e3                   -mov ebx, esp
    cpu.ebx = cpu.esp;
    // 004e01a8  e8b3ffffff             -call 0x4e0160
    cpu.esp -= 4;
    sub_4e0160(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004e01ad  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 004e01af  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004e01b1  e80a000000             -call 0x4e01c0
    cpu.esp -= 4;
    nfs3hp::vertices::unitVector(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004e01b6  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004e01b9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004e01ba  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4e01c0: native, nfs3hp::vertices::unitVector */

/* sub_4e01f0: native, nfs3hp::vertices::dotPush */

/* sub_4e0210: native, nfs3hp::vertices::dotProducts */

/* sub_4e0250: native, nfs3hp::dead::stub_0x4e0250 */

/* sub_4e0280: native, nfs3hp::vertices::multiplyMatrices */

/* sub_4e03b0: native, nfs3hp::vertices::rotateVectors */

/* sub_4e0430: native, nfs3hp::vertices::transformPoints */

/* sub_4e04e0: native, nfs3hp::dead::stub_0x4e04e0 */

/* sub_4e05b0: native, nfs3hp::dead::stub_0x4e05b0 */

/* sub_4e0620: native, nfs3hp::dead::stub_0x4e0620 */

/* sub_4e0640: native, nfs3hp::watcom::memset */

/* align: skip  */
void Application::sub_4e0658(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004e0658  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4e0659(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004e0659  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4e0660: native, nfs3hp::dead::stub_0x4e0660 */

/* sub_4e06a0: native, nfs3hp::vertices::vectorLengthOf */

/* sub_4e06c4: native, nfs3hp::dead::stub_0x4e06c4 */

/* sub_4e06d9: native, nfs3hp::watcom::IF_DATAN2 */

/* sub_4e06ee: native, nfs3hp::dead::stub_0x4e06ee */

/* sub_4e06fa: native, nfs3hp::dead::stub_0x4e06fa */

/* sub_4e070c: native, nfs3hp::fillZero */

/* sub_4e0716: native, nfs3hp::fillByte */

/* sub_4e0721: native, nfs3hp::fillWord */

/* sub_4e0820: native, nfs3hp::watcom::strstr */

/* sub_4e08f0: native, nfs3hp::ea::monoPrintf */

/* sub_4e0980: native, nfs3hp::ea::monoPrintfAt */

/* sub_4e0a60: native, nfs3hp::ea::monoWrite */

/* sub_4e0c10: native, nfs3hp::ea::monoScroll */

/* sub_4e0ca0: native, nfs3hp::ea::monoClear */

/* sub_4e0d00: native, nfs3hp::ea::monoSetTop */

/* sub_4e0d50: native, nfs3hp::ea::monoSetAttribute */

/* sub_4e0d60: native, nfs3hp::ea::monoSetTab */

/* sub_4e0d70: native, nfs3hp::ea::monoPage */

/* sub_4e0d80: native, nfs3hp::ea::monoNoCursor */

/* sub_4e0d88: native, nfs3hp::port::eaRandom */

/* sub_4e0dee: native, nfs3hp::ea::seedRandom */

/* sub_4e0e30: native, nfs3hp::watcom::strncpy */

/* sub_4e0e60: native, nfs3hp::ea::fileExists */

/* sub_4e0ec0: native, nfs3hp::ea::loadFile */

/* sub_4e0fe0: native, nfs3hp::ea::loadFileQuietly */

/* sub_4e1000: native, nfs3hp::ea::findFreeBlock */

/* sub_4e1030: native, nfs3hp::ea::findLargestFreeBlock */

/* sub_4e1080: native, nfs3hp::ea::linkFreeBlock */

}
