#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4c7a60: native, nfs3hp::decompiled::setDepthOffsets */

/* align: skip  */
void Application::sub_4c7b10(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7b10  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7b11  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7b13  e848ffffff             -call 0x4c7a60
    cpu.esp -= 4;
    nfs3hp::decompiled::setDepthOffsets(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7b18  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7b19  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4c7b20(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7b20  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7b21  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7b23  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7b24  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4c7b30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7b30  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7b31  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7b33  833d1401560000         +cmp dword ptr [0x560114], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7b3a  750a                   -jne 0x4c7b46
    if (!cpu.flags.zf)
    {
        goto L_0x004c7b46;
    }
    // 004c7b3c  e89f370200             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7b41  a314015600             -mov dword ptr [0x560114], eax
    app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */) = cpu.eax;
L_0x004c7b46:
    // 004c7b46  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7b47  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4c7b50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7b50  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7b51  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7b52  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7b53  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7b55  8b1514015600           -mov edx, dword ptr [0x560114]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c7b5b  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7b5c  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7b62  baa0000000             -mov edx, 0xa0
    cpu.edx = 160 /*0xa0*/;
    // 004c7b67  b870d07d00             -mov eax, 0x7dd070
    cpu.eax = 8245360 /*0x7dd070*/;
    // 004c7b6c  e89b8b0100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7b71  ba60000000             -mov edx, 0x60
    cpu.edx = 96 /*0x60*/;
    // 004c7b76  b810d07d00             -mov eax, 0x7dd010
    cpu.eax = 8245264 /*0x7dd010*/;
    // 004c7b7b  e88c8b0100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7b80  ba8c000000             -mov edx, 0x8c
    cpu.edx = 140 /*0x8c*/;
    // 004c7b85  b8a0d27d00             -mov eax, 0x7dd2a0
    cpu.eax = 8245920 /*0x7dd2a0*/;
    // 004c7b8a  e87d8b0100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7b8f  ba90010000             -mov edx, 0x190
    cpu.edx = 400 /*0x190*/;
    // 004c7b94  b810d17d00             -mov eax, 0x7dd110
    cpu.eax = 8245520 /*0x7dd110*/;
    // 004c7b99  e86e8b0100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7b9e  8b0d14015600           -mov ecx, dword ptr [0x560114]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c7ba4  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7ba5  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7bab  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7bac  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7bad  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7bae  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4c7bb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7bb0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7bb1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7bb2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7bb4  a114015600             -mov eax, dword ptr [0x560114]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c7bb9  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c7bbb  e8a0370200             -call 0x4eb360
    cpu.esp -= 4;
    nfs3hp::ea::freeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7bc0  891514015600           -mov dword ptr [0x560114], edx
    app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */) = cpu.edx;
    // 004c7bc6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7bc7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7bc8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4c7bd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7bd0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c7bd1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7bd2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7bd3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c7bd4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7bd5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7bd6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7bd8  83ec20                 -sub esp, 0x20
    (cpu.esp) -= x86::reg32(x86::sreg32(32 /*0x20*/));
    // 004c7bdb  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004c7bdd  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7be4  0f8f32010000           -jg 0x4c7d1c
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c7d1c;
    }
    // 004c7bea  8b0db0d36f00           -mov ecx, dword ptr [0x6fd3b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 004c7bf0  83f901                 +cmp ecx, 1
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
    // 004c7bf3  0f8423010000           -je 0x4c7d1c
    if (cpu.flags.zf)
    {
        goto L_0x004c7d1c;
    }
    // 004c7bf9  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c7bfb  0f841b010000           -je 0x4c7d1c
    if (cpu.flags.zf)
    {
        goto L_0x004c7d1c;
    }
    // 004c7c01  833d0cd56f0000         +cmp dword ptr [0x6fd50c], 0
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
    // 004c7c08  0f840e010000           -je 0x4c7d1c
    if (cpu.flags.zf)
    {
        goto L_0x004c7d1c;
    }
    // 004c7c0e  c645e00b               -mov byte ptr [ebp - 0x20], 0xb
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = 11 /*0xb*/;
    // 004c7c12  a078227a00             -mov al, byte ptr [0x7a2278]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(8004216) /* 0x7a2278 */);
    // 004c7c17  8845e1                 -mov byte ptr [ebp - 0x1f], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-31) /* -0x1f */) = cpu.al;
    // 004c7c1a  8d45e8                 -lea eax, [ebp - 0x18]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004c7c1d  895de4                 -mov dword ptr [ebp - 0x1c], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.ebx;
    // 004c7c20  e8eb060000             -call 0x4c8310
    cpu.esp -= 4;
    sub_4c8310(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7c25  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 004c7c2a  8d55fc                 -lea edx, [ebp - 4]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c7c2d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004c7c30  e80becffff             -call 0x4c6840
    cpu.esp -= 4;
    nfs3hp::controls::takenControls(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7c35  8a45ff                 -mov al, byte ptr [ebp - 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-1) /* -0x1 */);
    // 004c7c38  668b15da227a00         -mov dx, word ptr [0x7a22da]
    cpu.dx = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
    // 004c7c3f  8845ec                 -mov byte ptr [ebp - 0x14], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.al;
    // 004c7c42  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 004c7c45  7437                   -je 0x4c7c7e
    if (cpu.flags.zf)
    {
        goto L_0x004c7c7e;
    }
    // 004c7c47  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c7c49:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c7c49  8b1520d56f00           -mov edx, dword ptr [0x6fd520]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 004c7c4f  39d0                   +cmp eax, edx
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
    // 004c7c51  7d13                   -jge 0x4c7c66
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c7c66;
    }
    // 004c7c53  8b148588fb5e00         -mov edx, dword ptr [eax*4 + 0x5efb88]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6224776) /* 0x5efb88 */ + cpu.eax * 4);
    // 004c7c5a  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c7c5b  668b5208               -mov dx, word ptr [edx + 8]
    cpu.dx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 004c7c5f  66895445ec             -mov word ptr [ebp + eax*2 - 0x14], dx
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-20) /* -0x14 */ + cpu.eax * 2) = cpu.dx;
    // 004c7c64  ebe3                   -jmp 0x4c7c49
    goto L_0x004c7c49;
L_0x004c7c66:
    // 004c7c66  b807000000             -mov eax, 7
    cpu.eax = 7 /*0x7*/;
    // 004c7c6b  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c7c6d  bb1c000000             -mov ebx, 0x1c
    cpu.ebx = 28 /*0x1c*/;
    // 004c7c72  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 004c7c74  8d55e0                 -lea edx, [ebp - 0x20]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004c7c77  29c3                   +sub ebx, eax
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c7c79  e978000000             -jmp 0x4c7cf6
    goto L_0x004c7cf6;
L_0x004c7c7e:
    // 004c7c7e  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004c7c80  c1fa05                 -sar edx, 5
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (5 /*0x5*/ % 32));
    // 004c7c83  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 004c7c88  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004c7c8a  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004c7c8d  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004c7c8f  8b3d14015600           -mov edi, dword ptr [0x560114]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c7c95  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7c96  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004c7c98  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7c9e  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004c7ca0  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c7ca7  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c7ca9  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c7cab  891c8520d17d00         -mov dword ptr [eax*4 + 0x7dd120], ebx
    app->getMemory<x86::reg32>(x86::reg32(8245536) /* 0x7dd120 */ + cpu.eax * 4) = cpu.ebx;
L_0x004c7cb2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c7cb2  3b1520d56f00           +cmp edx, dword ptr [0x6fd520]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7cb8  7d27                   -jge 0x4c7ce1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c7ce1;
    }
    // 004c7cba  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004c7cc1  8d1c06                 -lea ebx, [esi + eax]
    cpu.ebx = x86::reg32(cpu.esi + cpu.eax * 1);
    // 004c7cc4  c1e302                 -shl ebx, 2
    cpu.ebx <<= 2 /*0x2*/ % 32;
    // 004c7cc7  8d0412                 -lea eax, [edx + edx]
    cpu.eax = x86::reg32(cpu.edx + cpu.edx * 1);
    // 004c7cca  8b0c9588fb5e00         -mov ecx, dword ptr [edx*4 + 0x5efb88]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(6224776) /* 0x5efb88 */ + cpu.edx * 4);
    // 004c7cd1  01c3                   +add ebx, eax
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
    // 004c7cd3  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 004c7cd7  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c7cd8  66898310d17d00         -mov word ptr [ebx + 0x7dd110], ax
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(8245520) /* 0x7dd110 */) = cpu.ax;
    // 004c7cdf  ebd1                   -jmp 0x4c7cb2
    goto L_0x004c7cb2;
L_0x004c7ce1:
    // 004c7ce1  8b1514015600           -mov edx, dword ptr [0x560114]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c7ce7  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7ce8  bb0e000000             -mov ebx, 0xe
    cpu.ebx = 14 /*0xe*/;
    // 004c7ced  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7cf3  8d55e0                 -lea edx, [ebp - 0x20]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-32) /* -0x20 */);
L_0x004c7cf6:
    // 004c7cf6  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c7cf8  e8e3e3fdff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7cfd  a1f4d46f00             -mov eax, dword ptr [0x6fd4f4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 004c7d02  8b0485c8fa5e00         -mov eax, dword ptr [eax*4 + 0x5efac8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */ + cpu.eax * 4);
    // 004c7d09  83b8c402000003         +cmp dword ptr [eax + 0x2c4], 3
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
    // 004c7d10  750a                   -jne 0x4c7d1c
    if (!cpu.flags.zf)
    {
        goto L_0x004c7d1c;
    }
    // 004c7d12  a1f4d46f00             -mov eax, dword ptr [0x6fd4f4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 004c7d17  e834010000             -call 0x4c7e50
    cpu.esp -= 4;
    sub_4c7e50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c7d1c:
    // 004c7d1c  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c7d1e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7d1f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7d20  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7d21  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7d22  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7d23  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7d24  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_4c7d30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7d30  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7d31  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c7d32  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7d33  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7d34  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7d36  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004c7d39  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004c7d3b  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 004c7d3d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c7d3f  8a4201                 -mov al, byte ptr [edx + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 004c7d42  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004c7d45  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004c7d48  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004c7d4a  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c7d4d  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c7d4f  8b04857cd07d00         -mov eax, dword ptr [eax*4 + 0x7dd07c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8245372) /* 0x7dd07c */ + cpu.eax * 4);
    // 004c7d56  3b4104                 +cmp eax, dword ptr [ecx + 4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7d59  0f8fe7000000           -jg 0x4c7e46
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c7e46;
    }
    // 004c7d5f  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 004c7d67  7432                   -je 0x4c7d9b
    if (cpu.flags.zf)
    {
        goto L_0x004c7d9b;
    }
    // 004c7d69  833de4227a0002         +cmp dword ptr [0x7a22e4], 2
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
    // 004c7d70  7429                   -je 0x4c7d9b
    if (cpu.flags.zf)
    {
        goto L_0x004c7d9b;
    }
    // 004c7d72  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
L_0x004c7d77:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c7d77  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 004c7d7c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004c7d7f  39c6                   +cmp esi, eax
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
    // 004c7d81  7d18                   -jge 0x4c7d9b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c7d9b;
    }
    // 004c7d83  3b75f8                 +cmp esi, dword ptr [ebp - 8]
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
    // 004c7d86  7410                   -je 0x4c7d98
    if (cpu.flags.zf)
    {
        goto L_0x004c7d98;
    }
    // 004c7d88  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004c7d8a  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004c7d8c  8b04b5f0217a00         -mov eax, dword ptr [esi*4 + 0x7a21f0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004080) /* 0x7a21f0 */ + cpu.esi * 4);
    // 004c7d93  e848e3fdff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c7d98:
    // 004c7d98  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c7d99  ebdc                   -jmp 0x4c7d77
    goto L_0x004c7d77;
L_0x004c7d9b:
    // 004c7d9b  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c7d9e  8d1c9500000000         -lea ebx, [edx*4]
    cpu.ebx = x86::reg32(cpu.edx * 4);
    // 004c7da5  01d3                   -add ebx, edx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c7da7  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c7daa  c1e302                 -shl ebx, 2
    cpu.ebx <<= 2 /*0x2*/ % 32;
    // 004c7dad  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004c7db0  89837cd07d00           -mov dword ptr [ebx + 0x7dd07c], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8245372) /* 0x7dd07c */) = cpu.eax;
    // 004c7db6  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c7db9  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c7dbc  83c008                 -add eax, 8
    (cpu.eax) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004c7dbf  e88c050000             -call 0x4c8350
    cpu.esp -= 4;
    sub_4c8350(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7dc4  8b1514015600           -mov edx, dword ptr [0x560114]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c7dca  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7dcb  8d75f0                 -lea esi, [ebp - 0x10]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c7dce  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7dd4  8dbb70d07d00           -lea edi, [ebx + 0x7dd070]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(8245360) /* 0x7dd070 */);
    // 004c7dda  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c7ddd  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c7dde  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c7ddf  8a400c                 -mov al, byte ptr [eax + 0xc]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004c7de2  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004c7de7  888378d07d00           -mov byte ptr [ebx + 0x7dd078], al
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(8245368) /* 0x7dd078 */) = cpu.al;
    // 004c7ded  898b80d07d00           -mov dword ptr [ebx + 0x7dd080], ecx
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8245376) /* 0x7dd080 */) = cpu.ecx;
    // 004c7df3  837df800               +cmp dword ptr [ebp - 8], 0
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
    // 004c7df7  7540                   -jne 0x4c7e39
    if (!cpu.flags.zf)
    {
        goto L_0x004c7e39;
    }
    // 004c7df9  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004c7dfb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c7dfb  3b1520d56f00           +cmp edx, dword ptr [0x6fd520]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7e01  7d36                   -jge 0x4c7e39
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c7e39;
    }
    // 004c7e03  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c7e0a  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c7e0d  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c7e0f  8b5b04                 -mov ebx, dword ptr [ebx + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 004c7e12  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c7e15  891c85a0d27d00         -mov dword ptr [eax*4 + 0x7dd2a0], ebx
    app->getMemory<x86::reg32>(x86::reg32(8245920) /* 0x7dd2a0 */ + cpu.eax * 4) = cpu.ebx;
    // 004c7e1c  8d1c12                 -lea ebx, [edx + edx]
    cpu.ebx = x86::reg32(cpu.edx + cpu.edx * 1);
    // 004c7e1f  01fb                   -add ebx, edi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004c7e21  668b5b0e               -mov bx, word ptr [ebx + 0xe]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(14) /* 0xe */);
    // 004c7e25  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004c7e27  66891c85a4d27d00       -mov word ptr [eax*4 + 0x7dd2a4], bx
    app->getMemory<x86::reg16>(x86::reg32(8245924) /* 0x7dd2a4 */ + cpu.eax * 4) = cpu.bx;
    // 004c7e2f  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c7e30  890c85b0d27d00         -mov dword ptr [eax*4 + 0x7dd2b0], ecx
    app->getMemory<x86::reg32>(x86::reg32(8245936) /* 0x7dd2b0 */ + cpu.eax * 4) = cpu.ecx;
    // 004c7e37  ebc2                   -jmp 0x4c7dfb
    goto L_0x004c7dfb;
L_0x004c7e39:
    // 004c7e39  8b1d14015600           -mov ebx, dword ptr [0x560114]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c7e3f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c7e40  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c7e46:
    // 004c7e46  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c7e48  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7e49  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7e4a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7e4b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7e4c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4c7e50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7e50  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c7e51  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7e52  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7e53  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7e55  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004c7e58  b40d                   -mov ah, 0xd
    cpu.ah = 13 /*0xd*/;
    // 004c7e5a  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 004c7e5f  8d55fc                 -lea edx, [ebp - 4]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c7e62  8865fc                 -mov byte ptr [ebp - 4], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ah;
    // 004c7e65  8845fd                 -mov byte ptr [ebp - 3], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-3) /* -0x3 */) = cpu.al;
    // 004c7e68  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c7e6a  e871e2fdff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7e6f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c7e71  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7e72  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7e73  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7e74  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_4c7e80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7e80  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7e81  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7e83  8a5201                 -mov dl, byte ptr [edx + 1]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 004c7e86  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004c7e8c  8b1495c8fa5e00         -mov edx, dword ptr [edx*4 + 0x5efac8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */ + cpu.edx * 4);
    // 004c7e93  c782c402000003000000   -mov dword ptr [edx + 0x2c4], 3
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(708) /* 0x2c4 */) = 3 /*0x3*/;
    // 004c7e9d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7e9e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4c7ea0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7ea0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c7ea1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7ea2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7ea3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c7ea4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7ea5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7ea7  81ecc4000000           -sub esp, 0xc4
    (cpu.esp) -= x86::reg32(x86::sreg32(196 /*0xc4*/));
    // 004c7ead  81ed3e010000           -sub ebp, 0x13e
    (cpu.ebp) -= x86::reg32(x86::sreg32(318 /*0x13e*/));
    // 004c7eb3  833d0cd56f0000         +cmp dword ptr [0x6fd50c], 0
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
    // 004c7eba  0f84bc000000           -je 0x4c7f7c
    if (cpu.flags.zf)
    {
        goto L_0x004c7f7c;
    }
    // 004c7ec0  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 004c7ec7  0f8eaf000000           -jle 0x4c7f7c
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c7f7c;
    }
    // 004c7ecd  ba22000000             -mov edx, 0x22
    cpu.edx = 34 /*0x22*/;
    // 004c7ed2  b85a000000             -mov eax, 0x5a
    cpu.eax = 90 /*0x5a*/;
    // 004c7ed7  e834ebfdff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7edc  c6457a0c               -mov byte ptr [ebp + 0x7a], 0xc
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(122) /* 0x7a */) = 12 /*0xc*/;
    // 004c7ee0  a0f4d46f00             -mov al, byte ptr [0x6fd4f4]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 004c7ee5  88457b                 -mov byte ptr [ebp + 0x7b], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(123) /* 0x7b */) = cpu.al;
    // 004c7ee8  a1f4d46f00             -mov eax, dword ptr [0x6fd4f4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 004c7eed  bbc0000000             -mov ebx, 0xc0
    cpu.ebx = 192 /*0xc0*/;
    // 004c7ef2  8b048548fa5e00         -mov eax, dword ptr [eax*4 + 0x5efa48]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6224456) /* 0x5efa48 */ + cpu.eax * 4);
    // 004c7ef9  8d557e                 -lea edx, [ebp + 0x7e]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004c7efc  0524020000             -add eax, 0x224
    (cpu.eax) += x86::reg32(x86::sreg32(548 /*0x224*/));
    // 004c7f01  e8ea250200             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7f06  bbc4000000             -mov ebx, 0xc4
    cpu.ebx = 196 /*0xc4*/;
    // 004c7f0b  8d557a                 -lea edx, [ebp + 0x7a]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004c7f0e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c7f10  e8cbe1fdff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7f15  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 004c7f1d  744e                   -je 0x4c7f6d
    if (cpu.flags.zf)
    {
        goto L_0x004c7f6d;
    }
    // 004c7f1f  8b0d14d56f00           -mov ecx, dword ptr [0x6fd514]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */);
L_0x004c7f25:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c7f25  3b0d10d56f00           +cmp ecx, dword ptr [0x6fd510]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7f2b  7d40                   -jge 0x4c7f6d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c7f6d;
    }
    // 004c7f2d  884d7b                 -mov byte ptr [ebp + 0x7b], cl
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(123) /* 0x7b */) = cpu.cl;
    // 004c7f30  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004c7f37  8bb0c8fa5e00           -mov esi, dword ptr [eax + 0x5efac8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6224584) /* 0x5efac8 */);
    // 004c7f3d  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004c7f3f  7429                   -je 0x4c7f6a
    if (cpu.flags.zf)
    {
        goto L_0x004c7f6a;
    }
    // 004c7f41  bbc0000000             -mov ebx, 0xc0
    cpu.ebx = 192 /*0xc0*/;
    // 004c7f46  8d557e                 -lea edx, [ebp + 0x7e]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004c7f49  8d8624020000           -lea eax, [esi + 0x224]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(548) /* 0x224 */);
    // 004c7f4f  e89c250200             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7f54  bbc4000000             -mov ebx, 0xc4
    cpu.ebx = 196 /*0xc4*/;
    // 004c7f59  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c7f5b  8d557a                 -lea edx, [ebp + 0x7a]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004c7f5e  e8cdf60100             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7f63  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004c7f65  e876e1fdff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c7f6a:
    // 004c7f6a  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c7f6b  ebb8                   -jmp 0x4c7f25
    goto L_0x004c7f25;
L_0x004c7f6d:
    // 004c7f6d  ba23000000             -mov edx, 0x23
    cpu.edx = 35 /*0x23*/;
    // 004c7f72  b85a000000             -mov eax, 0x5a
    cpu.eax = 90 /*0x5a*/;
    // 004c7f77  e894eafdff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c7f7c:
    // 004c7f7c  8da53e010000           -lea esp, [ebp + 0x13e]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(318) /* 0x13e */);
    // 004c7f82  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7f83  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7f84  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7f85  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7f86  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7f87  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4c7f90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7f90  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7f91  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7f92  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7f94  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004c7f96  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c7f98  8a4201                 -mov al, byte ptr [edx + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 004c7f9b  8b1485c8fa5e00         -mov edx, dword ptr [eax*4 + 0x5efac8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */ + cpu.eax * 4);
    // 004c7fa2  bbc0000000             -mov ebx, 0xc0
    cpu.ebx = 192 /*0xc0*/;
    // 004c7fa7  81c224020000           -add edx, 0x224
    (cpu.edx) += x86::reg32(x86::sreg32(548 /*0x224*/));
    // 004c7fad  8d4104                 -lea eax, [ecx + 4]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 004c7fb0  e83b250200             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7fb5  66833dda227a0000       +cmp word ptr [0x7a22da], 0
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
    // 004c7fbd  740e                   -je 0x4c7fcd
    if (cpu.flags.zf)
    {
        goto L_0x004c7fcd;
    }
    // 004c7fbf  bbc4000000             -mov ebx, 0xc4
    cpu.ebx = 196 /*0xc4*/;
    // 004c7fc4  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004c7fc6  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c7fc8  e813e1fdff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c7fcd:
    // 004c7fcd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7fce  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7fcf  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4c7fd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7fd0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c7fd1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7fd2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7fd3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c7fd4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7fd5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7fd6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7fd8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004c7fda  3b05a4fd5e00           +cmp eax, dword ptr [0x5efda4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7fe0  7c48                   -jl 0x4c802a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c802a;
    }
    // 004c7fe2  8b0d14015600           -mov ecx, dword ptr [0x560114]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c7fe8  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7fe9  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7fef  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004c7ff1  eb05                   -jmp 0x4c7ff8
    goto L_0x004c7ff8;
L_0x004c7ff3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c7ff3  83fa14                 +cmp edx, 0x14
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7ff6  7d25                   -jge 0x4c801d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c801d;
    }
L_0x004c7ff8:
    // 004c7ff8  8b35a4fd5e00           -mov esi, dword ptr [0x5efda4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */);
    // 004c7ffe  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c8000  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004c8002  8d3400                 -lea esi, [eax + eax]
    cpu.esi = x86::reg32(cpu.eax + cpu.eax * 1);
    // 004c8005  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004c8007  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c800e  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c8010  31d1                   +xor ecx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004c8012  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c8013  66898c8610d17d00       -mov word ptr [esi + eax*4 + 0x7dd110], cx
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(8245520) /* 0x7dd110 */ + cpu.eax * 4) = cpu.cx;
    // 004c801b  ebd6                   -jmp 0x4c7ff3
    goto L_0x004c7ff3;
L_0x004c801d:
    // 004c801d  8b3d14015600           -mov edi, dword ptr [0x560114]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c8023  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c8024  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c802a:
    // 004c802a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c802b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c802c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c802d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c802e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c802f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8030  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4c8040(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c8040  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c8041  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c8042  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c8043  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c8044  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c8045  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c8047  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004c804a  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004c804c  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004c804e  8b0d14015600           -mov ecx, dword ptr [0x560114]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c8054  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004c8056  8b15a4fd5e00           -mov edx, dword ptr [0x5efda4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */);
    // 004c805c  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c805d  29d7                   -sub edi, edx
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c805f  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c8065  3b35a4fd5e00           +cmp esi, dword ptr [0x5efda4]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c806b  7d74                   -jge 0x4c80e1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c80e1;
    }
    // 004c806d  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004c806f  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c8076  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c8078  8b148510d07d00         -mov edx, dword ptr [eax*4 + 0x7dd010]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8245264) /* 0x7dd010 */ + cpu.eax * 4);
    // 004c807f  895308                 -mov dword ptr [ebx + 8], edx
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */) = cpu.edx;
    // 004c8082  668b148514d07d00       -mov dx, word ptr [eax*4 + 0x7dd014]
    cpu.dx = app->getMemory<x86::reg16>(x86::reg32(8245268) /* 0x7dd014 */ + cpu.eax * 4);
    // 004c808a  66895304               -mov word ptr [ebx + 4], dx
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(4) /* 0x4 */) = cpu.dx;
    // 004c808e  8b14b5c8fa5e00         -mov edx, dword ptr [esi*4 + 0x5efac8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */ + cpu.esi * 4);
    // 004c8095  668b4b04               -mov cx, word ptr [ebx + 4]
    cpu.cx = app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 004c8099  668b5208               -mov dx, word ptr [edx + 8]
    cpu.dx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 004c809d  c7431001000000         -mov dword ptr [ebx + 0x10], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */) = 1 /*0x1*/;
    // 004c80a4  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c80a6  6689530c               -mov word ptr [ebx + 0xc], dx
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(12) /* 0xc */) = cpu.dx;
    // 004c80aa  8a048518d07d00         -mov al, byte ptr [eax*4 + 0x7dd018]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(8245272) /* 0x7dd018 */ + cpu.eax * 4);
    // 004c80b1  c0e004                 -shl al, 4
    cpu.al <<= 4 /*0x4*/ % 32;
    // 004c80b4  8b14b5c8fa5e00         -mov edx, dword ptr [esi*4 + 0x5efac8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */ + cpu.esi * 4);
    // 004c80bb  c0e807                 -shr al, 7
    cpu.al >>= 7 /*0x7*/ % 32;
    // 004c80be  88820e050000           -mov byte ptr [edx + 0x50e], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1294) /* 0x50e */) = cpu.al;
    // 004c80c4  668b7304               -mov si, word ptr [ebx + 4]
    cpu.si = app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 004c80c8  6685f6                 +test si, si
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.si & cpu.si));
    // 004c80cb  0f85cc000000           -jne 0x4c819d
    if (!cpu.flags.zf)
    {
        goto L_0x004c819d;
    }
    // 004c80d1  c7430800000000         -mov dword ptr [ebx + 8], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 004c80d8  6689730c               -mov word ptr [ebx + 0xc], si
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(12) /* 0xc */) = cpu.si;
    // 004c80dc  e9bc000000             -jmp 0x4c819d
    goto L_0x004c819d;
L_0x004c80e1:
    // 004c80e1  8d0cbd00000000         -lea ecx, [edi*4]
    cpu.ecx = x86::reg32(cpu.edi * 4);
    // 004c80e8  01f9                   -add ecx, edi
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004c80ea  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 004c80ed  8b81a0d27d00           -mov eax, dword ptr [ecx + 0x7dd2a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8245920) /* 0x7dd2a0 */);
    // 004c80f3  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004c80f5  8903                   -mov dword ptr [ebx], eax
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.eax;
    // 004c80f7  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c80f9  7514                   -jne 0x4c810f
    if (!cpu.flags.zf)
    {
        goto L_0x004c810f;
    }
    // 004c80fb  8b1514015600           -mov edx, dword ptr [0x560114]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c8101  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c8102  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c8108  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004c810a  e9a0000000             -jmp 0x4c81af
    goto L_0x004c81af;
L_0x004c810f:
    // 004c810f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004c8111  b814000000             -mov eax, 0x14
    cpu.eax = 20 /*0x14*/;
    // 004c8116  c1fa05                 -sar edx, 5
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (5 /*0x5*/ % 32));
    // 004c8119  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004c811c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004c811e  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004c8121  f77dfc                 -idiv dword ptr [ebp - 4]
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004c8124  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c812b  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c812d  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c8130  3bb020d17d00           +cmp esi, dword ptr [eax + 0x7dd120]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8245536) /* 0x7dd120 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c8136  7417                   -je 0x4c814f
    if (cpu.flags.zf)
    {
        goto L_0x004c814f;
    }
    // 004c8138  8b0d14015600           -mov ecx, dword ptr [0x560114]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c813e  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c813f  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c8145  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c8147  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c8149  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c814a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c814b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c814c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c814d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c814e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c814f:
    // 004c814f  8d143f                 -lea edx, [edi + edi]
    cpu.edx = x86::reg32(cpu.edi + cpu.edi * 1);
    // 004c8152  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c8154  6683b810d17d0000       +cmp word ptr [eax + 0x7dd110], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8245520) /* 0x7dd110 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c815c  7517                   -jne 0x4c8175
    if (!cpu.flags.zf)
    {
        goto L_0x004c8175;
    }
    // 004c815e  8b1d14015600           -mov ebx, dword ptr [0x560114]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c8164  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c8165  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c816b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c816d  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c816f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8170  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8171  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8172  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8173  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8174  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c8175:
    // 004c8175  668b91a4d27d00         -mov dx, word ptr [ecx + 0x7dd2a4]
    cpu.dx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8245924) /* 0x7dd2a4 */);
    // 004c817c  66895304               -mov word ptr [ebx + 4], dx
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(4) /* 0x4 */) = cpu.dx;
    // 004c8180  668b8010d17d00         -mov ax, word ptr [eax + 0x7dd110]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8245520) /* 0x7dd110 */);
    // 004c8187  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c8189  6689430c               -mov word ptr [ebx + 0xc], ax
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(12) /* 0xc */) = cpu.ax;
    // 004c818d  8bb1b0d27d00           -mov esi, dword ptr [ecx + 0x7dd2b0]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8245936) /* 0x7dd2b0 */);
    // 004c8193  46                     -inc esi
    (cpu.esi)++;
    // 004c8194  89b1b0d27d00           -mov dword ptr [ecx + 0x7dd2b0], esi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8245936) /* 0x7dd2b0 */) = cpu.esi;
    // 004c819a  897310                 -mov dword ptr [ebx + 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */) = cpu.esi;
L_0x004c819d:
    // 004c819d  8b3d14015600           -mov edi, dword ptr [0x560114]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c81a3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c81a4  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c81aa  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x004c81af:
    // 004c81af  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c81b1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c81b2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c81b3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c81b4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c81b5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c81b6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4c81c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c81c0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c81c1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c81c2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c81c4  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c81cb  7f21                   -jg 0x4c81ee
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c81ee;
    }
    // 004c81cd  8b0db0d36f00           -mov ecx, dword ptr [0x6fd3b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 004c81d3  83f901                 +cmp ecx, 1
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
    // 004c81d6  7416                   -je 0x4c81ee
    if (cpu.flags.zf)
    {
        goto L_0x004c81ee;
    }
    // 004c81d8  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c81da  7412                   -je 0x4c81ee
    if (cpu.flags.zf)
    {
        goto L_0x004c81ee;
    }
    // 004c81dc  833d0cd56f0000         +cmp dword ptr [0x6fd50c], 0
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
    // 004c81e3  7409                   -je 0x4c81ee
    if (cpu.flags.zf)
    {
        goto L_0x004c81ee;
    }
    // 004c81e5  a801                   +test al, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 1 /*0x1*/));
    // 004c81e7  7405                   -je 0x4c81ee
    if (cpu.flags.zf)
    {
        goto L_0x004c81ee;
    }
    // 004c81e9  e862000000             -call 0x4c8250
    cpu.esp -= 4;
    sub_4c8250(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c81ee:
    // 004c81ee  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c81ef  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c81f0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4c8200: native, nfs3hp::dead::stub_0x4c8200 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4c8250(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c8250  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c8251  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c8252  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c8253  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c8254  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c8255  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c8256  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c8258  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004c825b  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004c825e  8b1514015600           -mov edx, dword ptr [0x560114]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c8264  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c8265  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c826b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004c826d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c826d  3b1514d56f00           +cmp edx, dword ptr [0x6fd514]
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
    // 004c8273  0f8d7e000000           -jge 0x4c82f7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c82f7;
    }
    // 004c8279  3b15f4d46f00           +cmp edx, dword ptr [0x6fd4f4]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c827f  7470                   -je 0x4c82f1
    if (cpu.flags.zf)
    {
        goto L_0x004c82f1;
    }
    // 004c8281  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c8288  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c828a  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c828d  83b880d07d0000         +cmp dword ptr [eax + 0x7dd080], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8245376) /* 0x7dd080 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c8294  745b                   -je 0x4c82f1
    if (cpu.flags.zf)
    {
        goto L_0x004c82f1;
    }
    // 004c8296  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c8299  8b807cd07d00           -mov eax, dword ptr [eax + 0x7dd07c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8245372) /* 0x7dd07c */);
    // 004c829f  29c1                   -sub ecx, eax
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004c82a1  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c82a3  7d12                   -jge 0x4c82b7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c82b7;
    }
    // 004c82a5  8b0dac367d00           -mov ecx, dword ptr [0x7d36ac]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205996) /* 0x7d36ac */);
    // 004c82ab  83c120                 -add ecx, 0x20
    (cpu.ecx) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 004c82ae  39c1                   +cmp ecx, eax
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
    // 004c82b0  7d05                   -jge 0x4c82b7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c82b7;
    }
    // 004c82b2  e88959f6ff             -call 0x42dc40
    cpu.esp -= 4;
    nfs3hp::raceload::wakeRaceControls(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c82b7:
    // 004c82b7  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c82be  8d0c02                 -lea ecx, [edx + eax]
    cpu.ecx = x86::reg32(cpu.edx + cpu.eax * 1);
    // 004c82c1  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c82c8  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 004c82cb  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c82cd  8db170d07d00           -lea esi, [ecx + 0x7dd070]
    cpu.esi = x86::reg32(cpu.ecx + x86::reg32(8245360) /* 0x7dd070 */);
    // 004c82d3  8d3c8510d07d00         -lea edi, [eax*4 + 0x7dd010]
    cpu.edi = x86::reg32(x86::reg32(8245264) /* 0x7dd010 */ + cpu.eax * 4);
    // 004c82da  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c82db  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c82dc  8a9978d07d00           -mov bl, byte ptr [ecx + 0x7dd078]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(8245368) /* 0x7dd078 */);
    // 004c82e2  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 004c82e4  881c8518d07d00         -mov byte ptr [eax*4 + 0x7dd018], bl
    app->getMemory<x86::reg8>(x86::reg32(8245272) /* 0x7dd018 */ + cpu.eax * 4) = cpu.bl;
    // 004c82eb  89b980d07d00           -mov dword ptr [ecx + 0x7dd080], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8245376) /* 0x7dd080 */) = cpu.edi;
L_0x004c82f1:
    // 004c82f1  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c82f2  e976ffffff             -jmp 0x4c826d
    goto L_0x004c826d;
L_0x004c82f7:
    // 004c82f7  a114015600             -mov eax, dword ptr [0x560114]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5636372) /* 0x560114 */);
    // 004c82fc  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004c82fd  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c8303  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c8305  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8306  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8307  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8308  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8309  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c830a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c830b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4c8310(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 004c8310  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c8311  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c8312  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c8313  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c8315  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004c8318  8b15f4d46f00           -mov edx, dword ptr [0x6fd4f4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 004c831e  8b0c9548fa5e00         -mov ecx, dword ptr [edx*4 + 0x5efa48]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(6224456) /* 0x5efa48 */ + cpu.edx * 4);
    // 004c8325  d981c0070000           -fld dword ptr [ecx + 0x7c0]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(1984) /* 0x7c0 */));
    // 004c832b  e8267a0100             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004c8330  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004c8333  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c8336  668908                 -mov word ptr [eax], cx
    app->getMemory<x86::reg16>(cpu.eax) = cpu.cx;
    // 004c8339  8b149548fa5e00         -mov edx, dword ptr [edx*4 + 0x5efa48]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6224456) /* 0x5efa48 */ + cpu.edx * 4);
    // 004c8340  668b5208               -mov dx, word ptr [edx + 8]
    cpu.dx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 004c8344  66895002               -mov word ptr [eax + 2], dx
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(2) /* 0x2 */) = cpu.dx;
    // 004c8348  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c834a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c834b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c834c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c834d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4c8350(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 004c8350  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c8351  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c8353  df00                   -fild word ptr [eax]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax)));
    // 004c8355  d91a                   -fstp dword ptr [edx]
    app->getMemory<float>(cpu.edx) = float(fp0);
    // 004c8357  668b4002               -mov ax, word ptr [eax + 2]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(2) /* 0x2 */);
    // 004c835b  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 004c8360  894204                 -mov dword ptr [edx + 4], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 004c8363  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8364  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4c8370: native, nfs3hp::port::speechInit */

/* sub_4c8440: native, nfs3hp::port::speechFree */

/* align: skip 0x8b 0xc0 */
void Application::sub_4c8470(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c8470  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c8471  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c8472  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c8473  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c8474  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c8475  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c8477  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004c8479  a1acd46f00             -mov eax, dword ptr [0x6fd4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 004c847e  8b0485a00a5600         -mov eax, dword ptr [eax*4 + 0x560aa0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5638816) /* 0x560aa0 */ + cpu.eax * 4);
    // 004c8485  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c8487  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c8489  0f84ae000000           -je 0x4c853d
    if (cpu.flags.zf)
    {
        goto L_0x004c853d;
    }
    // 004c848f  3b08                   +cmp ecx, dword ptr [eax]
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
    // 004c8491  7d1c                   -jge 0x4c84af
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c84af;
    }
    // 004c8493  8b580c                 -mov ebx, dword ptr [eax + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004c8496  891a                   -mov dword ptr [edx], ebx
    app->getMemory<x86::reg32>(cpu.edx) = cpu.ebx;
    // 004c8498  8b18                   -mov ebx, dword ptr [eax]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax);
    // 004c849a  29cb                   +sub ebx, ecx
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c849c  895a04                 -mov dword ptr [edx + 4], ebx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */) = cpu.ebx;
    // 004c849f  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004c84a2  894208                 -mov dword ptr [edx + 8], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004c84a5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c84aa  e98e000000             -jmp 0x4c853d
    goto L_0x004c853d;
L_0x004c84af:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c84af  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004c84b1  0f8584000000           -jne 0x4c853b
    if (!cpu.flags.zf)
    {
        goto L_0x004c853b;
    }
    // 004c84b7  8338ff                 +cmp dword ptr [eax], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c84ba  7507                   -jne 0x4c84c3
    if (!cpu.flags.zf)
    {
        goto L_0x004c84c3;
    }
    // 004c84bc  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004c84c1  ebec                   -jmp 0x4c84af
    goto L_0x004c84af;
L_0x004c84c3:
    // 004c84c3  8b7010                 -mov esi, dword ptr [eax + 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004c84c6  83feff                 +cmp esi, -1
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
    // 004c84c9  744d                   -je 0x4c8518
    if (cpu.flags.zf)
    {
        goto L_0x004c8518;
    }
    // 004c84cb  8b7804                 -mov edi, dword ptr [eax + 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004c84ce  39f9                   +cmp ecx, edi
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
    // 004c84d0  7e41                   -jle 0x4c8513
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c8513;
    }
    // 004c84d2  39f1                   +cmp ecx, esi
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
    // 004c84d4  7d3d                   -jge 0x4c8513
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c8513;
    }
    // 004c84d6  8b5810                 -mov ebx, dword ptr [eax + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004c84d9  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 004c84db  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c84dd  29fe                   -sub esi, edi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004c84df  39de                   +cmp esi, ebx
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
    // 004c84e1  7d12                   -jge 0x4c84f5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c84f5;
    }
    // 004c84e3  8b580c                 -mov ebx, dword ptr [eax + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004c84e6  891a                   -mov dword ptr [edx], ebx
    app->getMemory<x86::reg32>(cpu.edx) = cpu.ebx;
    // 004c84e8  8b5804                 -mov ebx, dword ptr [eax + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004c84eb  29d9                   +sub ecx, ebx
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
    // 004c84ed  894a04                 -mov dword ptr [edx + 4], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */) = cpu.ecx;
    // 004c84f0  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004c84f3  eb10                   -jmp 0x4c8505
    goto L_0x004c8505;
L_0x004c84f5:
    // 004c84f5  8b581c                 -mov ebx, dword ptr [eax + 0x1c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004c84f8  891a                   -mov dword ptr [edx], ebx
    app->getMemory<x86::reg32>(cpu.edx) = cpu.ebx;
    // 004c84fa  8b5810                 -mov ebx, dword ptr [eax + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004c84fd  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c84ff  895a04                 -mov dword ptr [edx + 4], ebx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */) = cpu.ebx;
    // 004c8502  8b4018                 -mov eax, dword ptr [eax + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
L_0x004c8505:
    // 004c8505  894208                 -mov dword ptr [edx + 8], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004c8508  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c850d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c850e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c850f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8510  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8511  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8512  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c8513:
    // 004c8513  83c010                 +add eax, 0x10
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
    // 004c8516  eb97                   -jmp 0x4c84af
    goto L_0x004c84af;
L_0x004c8518:
    // 004c8518  3b4804                 +cmp ecx, dword ptr [eax + 4]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c851b  7e1e                   -jle 0x4c853b
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c853b;
    }
    // 004c851d  8b580c                 -mov ebx, dword ptr [eax + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004c8520  891a                   -mov dword ptr [edx], ebx
    app->getMemory<x86::reg32>(cpu.edx) = cpu.ebx;
    // 004c8522  8b5804                 -mov ebx, dword ptr [eax + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004c8525  29d9                   -sub ecx, ebx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c8527  894a04                 -mov dword ptr [edx + 4], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */) = cpu.ecx;
    // 004c852a  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004c852d  894208                 -mov dword ptr [edx + 8], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004c8530  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c8535  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8536  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8537  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8538  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8539  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c853a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c853b:
    // 004c853b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c853d:
    // 004c853d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c853e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c853f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8540  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8541  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8542  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4c8550(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c8550  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c8551  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c8552  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c8553  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c8555  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004c8557  a1acd46f00             -mov eax, dword ptr [0x6fd4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 004c855c  8b0485a00a5600         -mov eax, dword ptr [eax*4 + 0x560aa0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5638816) /* 0x560aa0 */ + cpu.eax * 4);
    // 004c8563  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c8565  7426                   -je 0x4c858d
    if (cpu.flags.zf)
    {
        goto L_0x004c858d;
    }
L_0x004c8567:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c8567  8b18                   -mov ebx, dword ptr [eax]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax);
    // 004c8569  83fbff                 +cmp ebx, -1
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
    // 004c856c  7e1f                   -jle 0x4c858d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c858d;
    }
    // 004c856e  39d9                   +cmp ecx, ebx
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
    // 004c8570  7c16                   -jl 0x4c8588
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c8588;
    }
    // 004c8572  3b4804                 +cmp ecx, dword ptr [eax + 4]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c8575  7d11                   -jge 0x4c8588
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c8588;
    }
    // 004c8577  8b480c                 -mov ecx, dword ptr [eax + 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004c857a  890a                   -mov dword ptr [edx], ecx
    app->getMemory<x86::reg32>(cpu.edx) = cpu.ecx;
    // 004c857c  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004c857f  894208                 -mov dword ptr [edx + 8], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004c8582  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c8584  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8585  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8586  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8587  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c8588:
    // 004c8588  83c010                 +add eax, 0x10
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
    // 004c858b  ebda                   -jmp 0x4c8567
    goto L_0x004c8567;
L_0x004c858d:
    // 004c858d  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 004c8592  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8593  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8594  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c8595  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4c85a0: native, nfs3hp::port::speechLine */

/* sub_4c86d0: native, nfs3hp::port::speechTick */

/* sub_4c8810: native, nfs3hp::port::radioTick */

/* sub_4c89b0: native, nfs3hp::port::radioStart */

/* sub_4c8ac0: native, nfs3hp::port::speechSaid */

/* sub_4c8b30: native, nfs3hp::port::copRadio */

/* sub_4c8ca0: native, nfs3hp::port::queueSpeechLine */

/* sub_4c8d10: native, nfs3hp::port::coDriverTick */

/* sub_4c8f90: native, nfs3hp::port::queueRaceLine */

/* sub_4c9010: native, nfs3hp::port::raceEventLine */

/* sub_4c9090: native, nfs3hp::port::gapLine */

/* sub_4c91a0: native, nfs3hp::port::lapTimeLine */

/* sub_4c9240: native, nfs3hp::snd::descriptionLoad */

/* sub_4c9340: native, nfs3hp::port::descriptionSpeak */

/* sub_4c9380: native, nfs3hp::port::descriptionTick */

/* sub_4c9490: native, nfs3hp::port::descriptionStop */

/* sub_4c94e0: native, nfs3hp::frontend::infoSpeak */

/* sub_4c9590: native, nfs3hp::frontend::infoStopSpeaking */

/* sub_4c95e0: native, nfs3hp::port::spokenStatus */

/* sub_4c95f0: native, nfs3hp::port::descriptionStart */

/* sub_4c9860: native, nfs3hp::port::descriptionService */

/* sub_4c9890: native, nfs3hp::port::raceVoiceLoad */

/* sub_4c9990: native, nfs3hp::port::carLineSpeak */

/* sub_4c99d0: native, nfs3hp::port::spokenStop */

/* sub_4c99f0: native, nfs3hp::port::spokenStatus */

/* sub_4c9a00: native, nfs3hp::port::raceVoiceFree */

/* align: skip 0x00 0x00 0x00 */
/* data blob: 000000000000803f000000008d442000 */
void Application::sub_4c9a80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c9a80  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c9a81  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c9a82  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c9a83  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c9a84  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c9a85  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c9a87  8b3548d95d00           -mov esi, dword ptr [0x5dd948]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 004c9a8d  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004c9a8f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004c9a91  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 004c9a93  8d14dd00000000         -lea edx, [ebx*8]
    cpu.edx = x86::reg32(cpu.ebx * 8);
    // 004c9a9a  01da                   -add edx, ebx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004c9a9c  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004c9a9f  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004c9aa1  01f2                   -add edx, esi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004c9aa3  31f3                   -xor ebx, esi
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004c9aa5  8a5a13                 -mov bl, byte ptr [edx + 0x13]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(19) /* 0x13 */);
    // 004c9aa8  c1fb04                 -sar ebx, 4
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (4 /*0x4*/ % 32));
    // 004c9aab  bf06000000             -mov edi, 6
    cpu.edi = 6 /*0x6*/;
    // 004c9ab0  83e30f                 -and ebx, 0xf
    cpu.ebx &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 004c9ab3  29df                   -sub edi, ebx
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c9ab5  39f8                   +cmp eax, edi
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
    // 004c9ab7  7413                   -je 0x4c9acc
    if (cpu.flags.zf)
    {
        goto L_0x004c9acc;
    }
    // 004c9ab9  8a5a13                 -mov bl, byte ptr [edx + 0x13]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(19) /* 0x13 */);
    // 004c9abc  80e30f                 -and bl, 0xf
    cpu.bl &= x86::reg8(x86::sreg8(15 /*0xf*/));
    // 004c9abf  81e3ff000000           -and ebx, 0xff
    cpu.ebx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004c9ac5  83c307                 -add ebx, 7
    (cpu.ebx) += x86::reg32(x86::sreg32(7 /*0x7*/));
    // 004c9ac8  39d8                   +cmp eax, ebx
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
    // 004c9aca  7552                   -jne 0x4c9b1e
    if (!cpu.flags.zf)
    {
        goto L_0x004c9b1e;
    }
L_0x004c9acc:
    // 004c9acc  8d14cd00000000         -lea edx, [ecx*8]
    cpu.edx = x86::reg32(cpu.ecx * 8);
    // 004c9ad3  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004c9ad5  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004c9ad8  83f807                 +cmp eax, 7
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
    // 004c9adb  7d25                   -jge 0x4c9b02
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c9b02;
    }
    // 004c9add  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004c9adf  8a541a17               -mov dl, byte ptr [edx + ebx + 0x17]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(23) /* 0x17 */ + cpu.ebx * 1);
    // 004c9ae3  bb07000000             -mov ebx, 7
    cpu.ebx = 7 /*0x7*/;
    // 004c9ae8  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004c9aea  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004c9af0  8d43ff                 -lea eax, [ebx - 1]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(-1) /* -0x1 */);
    // 004c9af3  c1e20f                 -shl edx, 0xf
    cpu.edx <<= 15 /*0xf*/ % 32;
    // 004c9af6  f7d8                   -neg eax
    cpu.eax = ~cpu.eax + 1;
    // 004c9af8  0fafc2                 -imul eax, edx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004c9afb  2d00000200             +sub eax, 0x20000
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(131072 /*0x20000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c9b00  eb68                   -jmp 0x4c9b6a
    goto L_0x004c9b6a;
L_0x004c9b02:
    // 004c9b02  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004c9b04  8a541a1b               -mov dl, byte ptr [edx + ebx + 0x1b]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(27) /* 0x1b */ + cpu.ebx * 1);
    // 004c9b08  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004c9b0e  83e807                 -sub eax, 7
    (cpu.eax) -= x86::reg32(x86::sreg32(7 /*0x7*/));
    // 004c9b11  c1e20f                 -shl edx, 0xf
    cpu.edx <<= 15 /*0xf*/ % 32;
    // 004c9b14  0fafc2                 -imul eax, edx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004c9b17  0500000200             +add eax, 0x20000
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(131072 /*0x20000*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c9b1c  eb4c                   -jmp 0x4c9b6a
    goto L_0x004c9b6a;
L_0x004c9b1e:
    // 004c9b1e  83f807                 +cmp eax, 7
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
    // 004c9b21  7d28                   -jge 0x4c9b4b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c9b4b;
    }
    // 004c9b23  8a5217                 -mov dl, byte ptr [edx + 0x17]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(23) /* 0x17 */);
    // 004c9b26  bb07000000             -mov ebx, 7
    cpu.ebx = 7 /*0x7*/;
    // 004c9b2b  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004c9b2d  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004c9b33  4b                     -dec ebx
    (cpu.ebx)--;
    // 004c9b34  c1e20f                 -shl edx, 0xf
    cpu.edx <<= 15 /*0xf*/ % 32;
    // 004c9b37  f7db                   -neg ebx
    cpu.ebx = ~cpu.ebx + 1;
    // 004c9b39  0fafda                 -imul ebx, edx
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004c9b3c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004c9b3e  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004c9b41  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c9b43  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004c9b45  29c3                   +sub ebx, eax
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c9b47  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c9b49  eb1f                   -jmp 0x4c9b6a
    goto L_0x004c9b6a;
L_0x004c9b4b:
    // 004c9b4b  8a521b                 -mov dl, byte ptr [edx + 0x1b]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(27) /* 0x1b */);
    // 004c9b4e  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004c9b54  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004c9b56  c1e20f                 -shl edx, 0xf
    cpu.edx <<= 15 /*0xf*/ % 32;
    // 004c9b59  83eb07                 -sub ebx, 7
    (cpu.ebx) -= x86::reg32(x86::sreg32(7 /*0x7*/));
    // 004c9b5c  0fafda                 -imul ebx, edx
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004c9b5f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004c9b61  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004c9b64  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c9b66  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004c9b68  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
L_0x004c9b6a:
    // 004c9b6a  8d14cd00000000         -lea edx, [ecx*8]
    cpu.edx = x86::reg32(cpu.ecx * 8);
    // 004c9b71  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004c9b73  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004c9b75  8b549320               -mov edx, dword ptr [ebx + edx*4 + 0x20]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(32) /* 0x20 */ + cpu.edx * 4);
    // 004c9b79  81ea00000200           -sub edx, 0x20000
    (cpu.edx) -= x86::reg32(x86::sreg32(131072 /*0x20000*/));
    // 004c9b7f  39d0                   +cmp eax, edx
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
    // 004c9b81  7e02                   -jle 0x4c9b85
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c9b85;
    }
    // 004c9b83  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x004c9b85:
    // 004c9b85  8d14cd00000000         -lea edx, [ecx*8]
    cpu.edx = x86::reg32(cpu.ecx * 8);
    // 004c9b8c  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c9b8e  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 004c9b91  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004c9b93  8b54111c               -mov edx, dword ptr [ecx + edx + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */ + cpu.edx * 1);
    // 004c9b97  f7da                   -neg edx
    cpu.edx = ~cpu.edx + 1;
    // 004c9b99  81c200000200           -add edx, 0x20000
    (cpu.edx) += x86::reg32(x86::sreg32(131072 /*0x20000*/));
    // 004c9b9f  39d0                   +cmp eax, edx
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
    // 004c9ba1  7d02                   -jge 0x4c9ba5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c9ba5;
    }
    // 004c9ba3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x004c9ba5:
    // 004c9ba5  893548d95d00           -mov dword ptr [0x5dd948], esi
    app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */) = cpu.esi;
    // 004c9bab  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9bac  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9bad  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9bae  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9baf  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9bb0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4c9bc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3;
    // 004c9bc0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c9bc1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c9bc2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c9bc3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c9bc5  81ecbc000000           -sub esp, 0xbc
    (cpu.esp) -= x86::reg32(x86::sreg32(188 /*0xbc*/));
    // 004c9bcb  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 004c9bd1  897572                 -mov dword ptr [ebp + 0x72], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.esi;
    // 004c9bd4  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004c9bd6  895576                 -mov dword ptr [ebp + 0x76], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.edx;
    // 004c9bd9  895d6e                 -mov dword ptr [ebp + 0x6e], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.ebx;
    // 004c9bdc  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004c9bdf  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004c9be1  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c9be4  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 004c9be7  8b557a                 -mov edx, dword ptr [ebp + 0x7a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004c9bea  a148d95d00             -mov eax, dword ptr [0x5dd948]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 004c9bef  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c9bf1  8d7d3e                 -lea edi, [ebp + 0x3e]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(62) /* 0x3e */);
    // 004c9bf4  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004c9bf6  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c9bf7  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c9bf8  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c9bf9  8b5015                 -mov edx, dword ptr [eax + 0x15]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(21) /* 0x15 */);
    // 004c9bfc  c1fa18                 -sar edx, 0x18
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (24 /*0x18*/ % 32));
    // 004c9bff  c1e209                 -shl edx, 9
    cpu.edx <<= 9 /*0x9*/ % 32;
    // 004c9c02  89554a                 -mov dword ptr [ebp + 0x4a], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */) = cpu.edx;
    // 004c9c05  8b5016                 -mov edx, dword ptr [eax + 0x16]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(22) /* 0x16 */);
    // 004c9c08  c1fa18                 -sar edx, 0x18
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (24 /*0x18*/ % 32));
    // 004c9c0b  8b4017                 -mov eax, dword ptr [eax + 0x17]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(23) /* 0x17 */);
    // 004c9c0e  c1e209                 -shl edx, 9
    cpu.edx <<= 9 /*0x9*/ % 32;
    // 004c9c11  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 004c9c14  89554e                 -mov dword ptr [ebp + 0x4e], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */) = cpu.edx;
    // 004c9c17  c1e009                 -shl eax, 9
    cpu.eax <<= 9 /*0x9*/ % 32;
    // 004c9c1a  8b5576                 -mov edx, dword ptr [ebp + 0x76]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004c9c1d  894552                 -mov dword ptr [ebp + 0x52], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */) = cpu.eax;
    // 004c9c20  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004c9c22  e859feffff             -call 0x4c9a80
    cpu.esp -= 4;
    sub_4c9a80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c9c27  8b554a                 -mov edx, dword ptr [ebp + 0x4a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 004c9c2a  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004c9c2c  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004c9c2e  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004c9c31  c1e810                 +shr eax, 0x10
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
    // 004c9c34  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004c9c36  8b554e                 -mov edx, dword ptr [ebp + 0x4e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */);
    // 004c9c39  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004c9c3b  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c9c3d  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004c9c3f  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004c9c42  c1e810                 +shr eax, 0x10
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
    // 004c9c45  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004c9c47  8b5552                 -mov edx, dword ptr [ebp + 0x52]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 004c9c4a  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004c9c4c  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c9c4e  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004c9c50  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004c9c53  c1e810                 +shr eax, 0x10
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
    // 004c9c56  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004c9c58  8b553e                 -mov edx, dword ptr [ebp + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */);
    // 004c9c5b  01f2                   -add edx, esi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004c9c5d  895556                 -mov dword ptr [ebp + 0x56], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */) = cpu.edx;
    // 004c9c60  8b5542                 -mov edx, dword ptr [ebp + 0x42]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */);
    // 004c9c63  01fa                   -add edx, edi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004c9c65  89555a                 -mov dword ptr [ebp + 0x5a], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */) = cpu.edx;
    // 004c9c68  8b5546                 -mov edx, dword ptr [ebp + 0x46]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 004c9c6b  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004c9c6d  8b5d76                 -mov ebx, dword ptr [ebp + 0x76]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004c9c70  89555e                 -mov dword ptr [ebp + 0x5e], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */) = cpu.edx;
    // 004c9c73  83fb07                 +cmp ebx, 7
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c9c76  7d0f                   -jge 0x4c9c87
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c9c87;
    }
    // 004c9c78  8b757a                 -mov esi, dword ptr [ebp + 0x7a]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004c9c7b  a148d95d00             -mov eax, dword ptr [0x5dd948]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 004c9c80  01f0                   +add eax, esi
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c9c82  8a4017                 -mov al, byte ptr [eax + 0x17]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(23) /* 0x17 */);
    // 004c9c85  eb0d                   -jmp 0x4c9c94
    goto L_0x004c9c94;
L_0x004c9c87:
    // 004c9c87  8b7d7a                 -mov edi, dword ptr [ebp + 0x7a]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004c9c8a  a148d95d00             -mov eax, dword ptr [0x5dd948]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 004c9c8f  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004c9c91  8a401b                 -mov al, byte ptr [eax + 0x1b]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(27) /* 0x1b */);
L_0x004c9c94:
    // 004c9c94  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004c9c99  c1e00f                 -shl eax, 0xf
    cpu.eax <<= 15 /*0xf*/ % 32;
    // 004c9c9c  ba00800100             -mov edx, 0x18000
    cpu.edx = 98304 /*0x18000*/;
    // 004c9ca1  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004c9ca3  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004c9ca6  c1e810                 +shr eax, 0x10
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
    // 004c9ca9  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004c9cab  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004c9cad  837d6e00               +cmp dword ptr [ebp + 0x6e], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c9cb1  7502                   -jne 0x4c9cb5
    if (!cpu.flags.zf)
    {
        goto L_0x004c9cb5;
    }
    // 004c9cb3  f7da                   -neg edx
    cpu.edx = ~cpu.edx + 1;
L_0x004c9cb5:
    // 004c9cb5  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004c9cb7  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004c9cba  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c9cbc  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004c9cbe  8b554a                 -mov edx, dword ptr [ebp + 0x4a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 004c9cc1  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004c9cc3  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004c9cc5  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004c9cc8  c1e810                 +shr eax, 0x10
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
    // 004c9ccb  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004c9ccd  8b554e                 -mov edx, dword ptr [ebp + 0x4e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */);
    // 004c9cd0  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004c9cd2  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c9cd4  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004c9cd6  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004c9cd9  c1e810                 +shr eax, 0x10
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
    // 004c9cdc  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004c9cde  8b5552                 -mov edx, dword ptr [ebp + 0x52]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 004c9ce1  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004c9ce3  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c9ce5  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004c9ce7  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004c9cea  c1e810                 +shr eax, 0x10
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
    // 004c9ced  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004c9cef  8b5556                 -mov edx, dword ptr [ebp + 0x56]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 004c9cf2  8b5d5a                 -mov ebx, dword ptr [ebp + 0x5a]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */);
    // 004c9cf5  01fa                   -add edx, edi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004c9cf7  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004c9cf9  8b755e                 -mov esi, dword ptr [ebp + 0x5e]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 004c9cfc  895556                 -mov dword ptr [ebp + 0x56], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */) = cpu.edx;
    // 004c9cff  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 004c9d01  895d5a                 -mov dword ptr [ebp + 0x5a], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */) = cpu.ebx;
    // 004c9d04  89755e                 -mov dword ptr [ebp + 0x5e], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */) = cpu.esi;
    // 004c9d07  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c9d0c  8d4556                 -lea eax, [ebp + 0x56]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 004c9d0f  8d75c6                 -lea esi, [ebp - 0x3a]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-58) /* -0x3a */);
    // 004c9d12  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 004c9d14  e83769f5ff             -call 0x420650
    cpu.esp -= 4;
    nfs3hp::trackdata::placeOnTrack(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c9d19  8b4556                 -mov eax, dword ptr [ebp + 0x56]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 004c9d1c  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004c9d1f  db457e                 -fild dword ptr [ebp + 0x7e]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004c9d22  8b455a                 -mov eax, dword ptr [ebp + 0x5a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */);
    // 004c9d25  dd0558035400           -fld qword ptr [0x540358]
    fp1 = x86::Float(app->getMemory<double>(x86::reg32(5505880) /* 0x540358 */));
    // 004c9d2b  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004c9d2d  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004c9d2f  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004c9d32  8b455e                 -mov eax, dword ptr [ebp + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 004c9d35  db457e                 -fild dword ptr [ebp + 0x7e]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004c9d38  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004c9d3b  d8ca                   -fmul st(2)
    fp2 = fp::mul(fp2, x86::Float(fp0));
    // 004c9d3d  db457e                 -fild dword ptr [ebp + 0x7e]
    fp3 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004c9d40  decb                   -fmulp st(3)
    fp0 = fp::mul(fp0, fp3);
    // 004c9d42  8b7d72                 -mov edi, dword ptr [ebp + 0x72]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 004c9d45  8d7562                 -lea esi, [ebp + 0x62]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 004c9d48  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004c9d4a  d95d62                 -fstp dword ptr [ebp + 0x62]
    app->getMemory<float>(cpu.ebp + x86::reg32(98) /* 0x62 */) = float(fp2);
    // 004c9d4d  d95d66                 -fstp dword ptr [ebp + 0x66]
    app->getMemory<float>(cpu.ebp + x86::reg32(102) /* 0x66 */) = float(fp1);
    // 004c9d50  d95d6a                 -fstp dword ptr [ebp + 0x6a]
    app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */) = float(fp0);
    // 004c9d53  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c9d54  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c9d55  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c9d56  8b4572                 -mov eax, dword ptr [ebp + 0x72]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 004c9d59  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004c9d5f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9d60  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9d61  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9d62  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4c9d70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c9d70  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c9d71  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c9d72  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c9d74  ba80020000             -mov edx, 0x280
    cpu.edx = 640 /*0x280*/;
    // 004c9d79  b850d87d00             -mov eax, 0x7dd850
    cpu.eax = 8247376 /*0x7dd850*/;
    // 004c9d7e  e889690100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c9d83  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9d84  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9d85  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4c9d90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c9d90  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c9d91  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c9d92  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c9d94  ba80020000             -mov edx, 0x280
    cpu.edx = 640 /*0x280*/;
    // 004c9d99  b850d87d00             -mov eax, 0x7dd850
    cpu.eax = 8247376 /*0x7dd850*/;
    // 004c9d9e  e869690100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c9da3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9da4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9da5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4c9db0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c9db0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c9db1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c9db2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c9db4  ba80020000             -mov edx, 0x280
    cpu.edx = 640 /*0x280*/;
    // 004c9db9  b850d87d00             -mov eax, 0x7dd850
    cpu.eax = 8247376 /*0x7dd850*/;
    // 004c9dbe  e849690100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c9dc3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9dc4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c9dc5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4c9dd0: native, nfs3hp::frontend::simSirenOn */

/* sub_4c9f50: native, nfs3hp::frontend::simSirenOff */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4c9f70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  x86::reg16 fsw = 0;
    // 004c9f70  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c9f71  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c9f72  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c9f73  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c9f74  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c9f75  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c9f76  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c9f78  81ec98000000           -sub esp, 0x98
    (cpu.esp) -= x86::reg32(x86::sreg32(152 /*0x98*/));
    // 004c9f7e  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 004c9f84  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004c9f87  8d756a                 -lea esi, [ebp + 0x6a]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 004c9f8a  e8e11effff             -call 0x4bbe70
    cpu.esp -= 4;
    sub_4bbe70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c9f8f  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004c9f91  e88a1fffff             -call 0x4bbf20
    cpu.esp -= 4;
    sub_4bbf20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c9f96  895572                 -mov dword ptr [ebp + 0x72], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.edx;
    // 004c9f99  e97b010000             -jmp 0x4ca119
    goto L_0x004ca119;
L_0x004c9f9e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c9f9e  8b1584367d00           -mov edx, dword ptr [0x7d3684]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004c9fa4  8bb09cd87d00           -mov esi, dword ptr [eax + 0x7dd89c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8247452) /* 0x7dd89c */);
    // 004c9faa  39f2                   +cmp edx, esi
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c9fac  7d0c                   -jge 0x4c9fba
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c9fba;
    }
    // 004c9fae  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004c9fb0  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c9fb2  a810                   +test al, 0x10
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 16 /*0x10*/));
    // 004c9fb4  0f8556010000           -jne 0x4ca110
    if (!cpu.flags.zf)
    {
        goto L_0x004ca110;
    }
L_0x004c9fba:
    // 004c9fba  8d45ea                 -lea eax, [ebp - 0x16]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-22) /* -0x16 */);
    // 004c9fbd  8b5572                 -mov edx, dword ptr [ebp + 0x72]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 004c9fc0  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004c9fc1  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c9fc8  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c9fca  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004c9fcd  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004c9fd0  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 004c9fd3  b850d87d00             -mov eax, 0x7dd850
    cpu.eax = 8247376 /*0x7dd850*/;
    // 004c9fd8  8b557a                 -mov edx, dword ptr [ebp + 0x7a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004c9fdb  8b5d7e                 -mov ebx, dword ptr [ebp + 0x7e]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004c9fde  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c9fe0  83c138                 -add ecx, 0x38
    (cpu.ecx) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 004c9fe3  8d5004                 -lea edx, [eax + 4]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004c9fe6  83c344                 +add ebx, 0x44
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(68 /*0x44*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c9fe9  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 004c9fee  e8cd54ffff             -call 0x4bf4c0
    cpu.esp -= 4;
    nfs3hp::vertices::objectVertices(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c9ff3  d945f2                 +fld dword ptr [ebp - 0xe]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-14) /* -0xe */));
    // 004c9ff6  dc1d78035400           +fcomp qword ptr [0x540378]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505912) /* 0x540378 */)));
    // 004c9ffc  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004c9ffe  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004c9fff  0f870b010000           -ja 0x4ca110
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004ca110;
    }
    // 004ca005  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004ca007  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ca009  8a551e                 -mov dl, byte ptr [ebp + 0x1e]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(30) /* 0x1e */);
    // 004ca00c  8a45fe                 -mov al, byte ptr [ebp - 2]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-2) /* -0x2 */);
    // 004ca00f  21c2                   -and edx, eax
    cpu.edx &= x86::reg32(x86::sreg32(cpu.eax));
    // 004ca011  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ca013  8a453e                 -mov al, byte ptr [ebp + 0x3e]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(62) /* 0x3e */);
    // 004ca016  21d0                   -and eax, edx
    cpu.eax &= x86::reg32(x86::sreg32(cpu.edx));
    // 004ca018  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004ca01a  8a555e                 -mov dl, byte ptr [ebp + 0x5e]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 004ca01d  85d0                   +test eax, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.edx));
    // 004ca01f  0f85eb000000           -jne 0x4ca110
    if (!cpu.flags.zf)
    {
        goto L_0x004ca110;
    }
    // 004ca025  baa0000000             -mov edx, 0xa0
    cpu.edx = 160 /*0xa0*/;
    // 004ca02a  8d4576                 -lea eax, [ebp + 0x76]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004ca02d  e84e1fffff             -call 0x4bbf80
    cpu.esp -= 4;
    sub_4bbf80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ca032  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004ca034  0f84d6000000           -je 0x4ca110
    if (cpu.flags.zf)
    {
        goto L_0x004ca110;
    }
    // 004ca03a  8b5576                 -mov edx, dword ptr [ebp + 0x76]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004ca03d  bb0ad7a33c             -mov ebx, 0x3ca3d70a
    cpu.ebx = 1017370378 /*0x3ca3d70a*/;
    // 004ca042  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004ca044  beffffffff             -mov esi, 0xffffffff
    cpu.esi = 4294967295 /*0xffffffff*/;
    // 004ca049  894d02                 -mov dword ptr [ebp + 2], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(2) /* 0x2 */) = cpu.ecx;
    // 004ca04c  895d06                 -mov dword ptr [ebp + 6], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(6) /* 0x6 */) = cpu.ebx;
    // 004ca04f  8975fa                 -mov dword ptr [ebp - 6], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.esi;
    // 004ca052  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004ca054  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 004ca059  8d75ea                 -lea esi, [ebp - 0x16]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-22) /* -0x16 */);
    // 004ca05c  8d7820                 -lea edi, [eax + 0x20]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(32) /* 0x20 */);
    // 004ca05f  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004ca062  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004ca064  8b8098d87d00           -mov eax, dword ptr [eax + 0x7dd898]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8247448) /* 0x7dd898 */);
    // 004ca06a  894522                 -mov dword ptr [ebp + 0x22], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(34) /* 0x22 */) = cpu.eax;
    // 004ca06d  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 004ca072  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 004ca077  89451a                 -mov dword ptr [ebp + 0x1a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(26) /* 0x1a */) = cpu.eax;
    // 004ca07a  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004ca07d  895d26                 -mov dword ptr [ebp + 0x26], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(38) /* 0x26 */) = cpu.ebx;
    // 004ca080  8d750a                 -lea esi, [ebp + 0xa]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(10) /* 0xa */);
    // 004ca083  8d7840                 -lea edi, [eax + 0x40]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 004ca086  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 004ca08b  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004ca08d  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004ca090  b90000803f             -mov ecx, 0x3f800000
    cpu.ecx = 1065353216 /*0x3f800000*/;
    // 004ca095  8d752a                 -lea esi, [ebp + 0x2a]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(42) /* 0x2a */);
    // 004ca098  8b8098d87d00           -mov eax, dword ptr [eax + 0x7dd898]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8247448) /* 0x7dd898 */);
    // 004ca09e  895d3a                 -mov dword ptr [ebp + 0x3a], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(58) /* 0x3a */) = cpu.ebx;
    // 004ca0a1  894542                 -mov dword ptr [ebp + 0x42], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */) = cpu.eax;
    // 004ca0a4  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004ca0a7  894d46                 -mov dword ptr [ebp + 0x46], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */) = cpu.ecx;
    // 004ca0aa  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 004ca0af  8d7860                 -lea edi, [eax + 0x60]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(96) /* 0x60 */);
    // 004ca0b2  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004ca0b4  bf48e17a3f             -mov edi, 0x3f7ae148
    cpu.edi = 1065017672 /*0x3f7ae148*/;
    // 004ca0b9  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 004ca0be  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004ca0c1  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004ca0c3  895d5a                 -mov dword ptr [ebp + 0x5a], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */) = cpu.ebx;
    // 004ca0c6  897562                 -mov dword ptr [ebp + 0x62], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */) = cpu.esi;
    // 004ca0c9  897d66                 -mov dword ptr [ebp + 0x66], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */) = cpu.edi;
    // 004ca0cc  8d754a                 -lea esi, [ebp + 0x4a]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 004ca0cf  8db880000000           -lea edi, [eax + 0x80]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(128) /* 0x80 */);
    // 004ca0d5  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004ca0d7  66c742040000           -mov word ptr [edx + 4], 0
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(4) /* 0x4 */) = 0 /*0x0*/;
    // 004ca0dd  66c742060a00           -mov word ptr [edx + 6], 0xa
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(6) /* 0x6 */) = 10 /*0xa*/;
    // 004ca0e3  c74218d0da7d00         -mov dword ptr [edx + 0x18], 0x7ddad0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(24) /* 0x18 */) = 8248016 /*0x7ddad0*/;
    // 004ca0ea  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004ca0ed  83c020                 -add eax, 0x20
    (cpu.eax) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 004ca0f0  894208                 -mov dword ptr [edx + 8], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004ca0f3  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004ca0f6  83c040                 -add eax, 0x40
    (cpu.eax) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 004ca0f9  89420c                 -mov dword ptr [edx + 0xc], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 004ca0fc  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004ca0ff  83c060                 -add eax, 0x60
    (cpu.eax) += x86::reg32(x86::sreg32(96 /*0x60*/));
    // 004ca102  894210                 -mov dword ptr [edx + 0x10], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 004ca105  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004ca108  0580000000             -add eax, 0x80
    (cpu.eax) += x86::reg32(x86::sreg32(128 /*0x80*/));
    // 004ca10d  894214                 -mov dword ptr [edx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */) = cpu.eax;
L_0x004ca110:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca110  ff4572                 -inc dword ptr [ebp + 0x72]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */))++;
    // 004ca113  837d7208               +cmp dword ptr [ebp + 0x72], 8
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca117  7d1e                   -jge 0x4ca137
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004ca137;
    }
L_0x004ca119:
    // 004ca119  8b5572                 -mov edx, dword ptr [ebp + 0x72]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 004ca11c  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004ca123  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004ca125  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004ca128  83b850d87d0000         +cmp dword ptr [eax + 0x7dd850], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8247376) /* 0x7dd850 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca12f  0f8569feffff           -jne 0x4c9f9e
    if (!cpu.flags.zf)
    {
        goto L_0x004c9f9e;
    }
    // 004ca135  ebd9                   -jmp 0x4ca110
    goto L_0x004ca110;
L_0x004ca137:
    // 004ca137  8d756a                 -lea esi, [ebp + 0x6a]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 004ca13a  8d556a                 -lea edx, [ebp + 0x6a]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 004ca13d  e87e1effff             -call 0x4bbfc0
    cpu.esp -= 4;
    sub_4bbfc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ca142  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004ca145  ff35e0cf7d00           -push dword ptr [0x7dcfe0]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(x86::reg32(8245216) /* 0x7dcfe0 */);
    cpu.esp -= 4;
    // 004ca14b  e8101fffff             -call 0x4bc060
    cpu.esp -= 4;
    sub_4bc060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ca150  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004ca156  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca157  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca158  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca159  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca15a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca15b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca15c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4ca160(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
  x86::reg16 fsw = 0;
    // 004ca160  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004ca161  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004ca162  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004ca163  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004ca164  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004ca165  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004ca167  83ec7c                 -sub esp, 0x7c
    (cpu.esp) -= x86::reg32(x86::sreg32(124 /*0x7c*/));
    // 004ca16a  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 004ca16d  8955e8                 -mov dword ptr [ebp - 0x18], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.edx;
    // 004ca170  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004ca172  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 004ca175  8955f0                 -mov dword ptr [ebp - 0x10], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edx;
    // 004ca178  e9d2020000             -jmp 0x4ca44f
    goto L_0x004ca44f;
L_0x004ca17d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca17d  8b1584367d00           -mov edx, dword ptr [0x7d3684]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004ca183  3b909cd87d00           +cmp edx, dword ptr [eax + 0x7dd89c]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8247452) /* 0x7dd89c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca189  0f8cb7020000           -jl 0x4ca446
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004ca446;
    }
    // 004ca18f  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004ca192  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004ca195  48                     -dec eax
    (cpu.eax)--;
    // 004ca196  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004ca198  7d06                   -jge 0x4ca1a0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004ca1a0;
    }
    // 004ca19a  030540d95d00           -add eax, dword ptr [0x5dd940]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
L_0x004ca1a0:
    // 004ca1a0  8945c4                 -mov dword ptr [ebp - 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.eax;
    // 004ca1a3  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004ca1a6  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004ca1a9  8b0d40d95d00           -mov ecx, dword ptr [0x5dd940]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 004ca1af  8945c8                 -mov dword ptr [ebp - 0x38], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.eax;
    // 004ca1b2  8d5001                 -lea edx, [eax + 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 004ca1b5  39ca                   +cmp edx, ecx
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
    // 004ca1b7  7c02                   -jl 0x4ca1bb
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004ca1bb;
    }
    // 004ca1b9  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
L_0x004ca1bb:
    // 004ca1bb  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004ca1be  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004ca1c5  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004ca1c7  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004ca1ca  8b75c4                 -mov esi, dword ptr [ebp - 0x3c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 004ca1cd  8b8884d87d00           -mov ecx, dword ptr [eax + 0x7dd884]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8247428) /* 0x7dd884 */);
    // 004ca1d3  8955cc                 -mov dword ptr [ebp - 0x34], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.edx;
    // 004ca1d6  39f1                   +cmp ecx, esi
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
    // 004ca1d8  740d                   -je 0x4ca1e7
    if (cpu.flags.zf)
    {
        goto L_0x004ca1e7;
    }
    // 004ca1da  3b4dc8                 +cmp ecx, dword ptr [ebp - 0x38]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca1dd  7408                   -je 0x4ca1e7
    if (cpu.flags.zf)
    {
        goto L_0x004ca1e7;
    }
    // 004ca1df  39ca                   +cmp edx, ecx
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
    // 004ca1e1  0f855f020000           -jne 0x4ca446
    if (!cpu.flags.zf)
    {
        goto L_0x004ca446;
    }
L_0x004ca1e7:
    // 004ca1e7  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004ca1ea  8d5584                 -lea edx, [ebp - 0x7c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-124) /* -0x7c */);
    // 004ca1ed  05c0000000             -add eax, 0xc0
    (cpu.eax) += x86::reg32(x86::sreg32(192 /*0xc0*/));
    // 004ca1f2  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004ca1f4  e8175f0100             -call 0x4e0110
    cpu.esp -= 4;
    sub_4e0110(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ca1f9  895df4                 -mov dword ptr [ebp - 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;
    // 004ca1fc  e966010000             -jmp 0x4ca367
    goto L_0x004ca367;
L_0x004ca201:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca201  83f904                 +cmp ecx, 4
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca204  0f8d2a010000           -jge 0x4ca334
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004ca334;
    }
L_0x004ca20a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca20a  8d348d00000000         -lea esi, [ecx*4]
    cpu.esi = x86::reg32(cpu.ecx * 4);
    // 004ca211  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ca213  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004ca216  89442ea8               -mov dword ptr [esi + ebp - 0x58], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(-88) /* -0x58 */ + cpu.ebp * 1) = cpu.eax;
    // 004ca21a  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004ca221  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004ca223  bb50d87d00             -mov ebx, 0x7dd850
    cpu.ebx = 8247376 /*0x7dd850*/;
    // 004ca228  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004ca22b  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004ca22d  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004ca234  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004ca236  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 004ca23d  8d4304                 -lea eax, [ebx + 4]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 004ca240  8d5ddc                 -lea ebx, [ebp - 0x24]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004ca243  01d0                   +add eax, edx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004ca245  8d5584                 -lea edx, [ebp - 0x7c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-124) /* -0x7c */);
    // 004ca248  d900                   +fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004ca24a  d80a                   +fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 004ca24c  d900                   +fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004ca24e  d84a04                 +fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 004ca251  d900                   +fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004ca253  d84a08                 +fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004ca256  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004ca258  d94004                 +fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004ca25b  d84a0c                 +fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 004ca25e  d94004                 +fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004ca261  d84a10                 +fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 004ca264  d94004                 +fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004ca267  d84a14                 +fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 004ca26a  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 004ca26c  dec3                   +faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 004ca26e  dec3                   +faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 004ca270  dec3                   +faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 004ca272  d94008                 +fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004ca275  d84a18                 +fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 004ca278  d94008                 +fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004ca27b  d84a1c                 +fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 004ca27e  d94008                 +fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004ca281  d84a20                 +fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 004ca284  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 004ca286  dec3                   +faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 004ca288  dec3                   +faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 004ca28a  dec3                   +faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 004ca28c  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004ca28f  d91b                   +fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 004ca291  d95b04                 +fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 004ca294  d95b08                 +fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 004ca297  d945dc                 +fld dword ptr [ebp - 0x24]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */));
    // 004ca29a  d945e0                 +fld dword ptr [ebp - 0x20]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */));
    // 004ca29d  d945e4                 +fld dword ptr [ebp - 0x1c]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */));
    // 004ca2a0  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004ca2a2  d845d0                 +fadd dword ptr [ebp - 0x30]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */)));
    // 004ca2a5  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004ca2a7  d845d4                 +fadd dword ptr [ebp - 0x2c]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */)));
    // 004ca2aa  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004ca2ac  d845d8                 +fadd dword ptr [ebp - 0x28]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */)));
    // 004ca2af  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004ca2b1  d95ddc                 +fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp2);
    // 004ca2b4  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004ca2b6  d95de0                 +fstp dword ptr [ebp - 0x20]
    app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = float(fp1);
    // 004ca2b9  d95de4                 +fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp0);
    // 004ca2bc  d945dc                 +fld dword ptr [ebp - 0x24]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */));
    // 004ca2bf  dc1d88035400           +fcomp qword ptr [0x540388]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505928) /* 0x540388 */)));
    // 004ca2c5  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ca2c7  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ca2c8  7605                   -jbe 0x4ca2cf
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ca2cf;
    }
    // 004ca2ca  804c2ea802             +or byte ptr [esi + ebp - 0x58], 2
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.esi + x86::reg32(-88) /* -0x58 */ + cpu.ebp * 1) |= x86::reg8(x86::sreg8(2 /*0x2*/))));
L_0x004ca2cf:
    // 004ca2cf  d945e0                 +fld dword ptr [ebp - 0x20]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */));
    // 004ca2d2  dc1d88035400           +fcomp qword ptr [0x540388]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505928) /* 0x540388 */)));
    // 004ca2d8  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ca2da  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ca2db  7605                   -jbe 0x4ca2e2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ca2e2;
    }
    // 004ca2dd  804c8da808             +or byte ptr [ebp + ecx*4 - 0x58], 8
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-88) /* -0x58 */ + cpu.ecx * 4) |= x86::reg8(x86::sreg8(8 /*0x8*/))));
L_0x004ca2e2:
    // 004ca2e2  d945e4                 +fld dword ptr [ebp - 0x1c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */));
    // 004ca2e5  dc1d88035400           +fcomp qword ptr [0x540388]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505928) /* 0x540388 */)));
    // 004ca2eb  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ca2ed  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ca2ee  7605                   -jbe 0x4ca2f5
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ca2f5;
    }
    // 004ca2f0  804c8da820             +or byte ptr [ebp + ecx*4 - 0x58], 0x20
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-88) /* -0x58 */ + cpu.ecx * 4) |= x86::reg8(x86::sreg8(32 /*0x20*/))));
L_0x004ca2f5:
    // 004ca2f5  d945dc                 +fld dword ptr [ebp - 0x24]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */));
    // 004ca2f8  dc1d90035400           +fcomp qword ptr [0x540390]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505936) /* 0x540390 */)));
    // 004ca2fe  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ca300  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ca301  7305                   -jae 0x4ca308
    if (!cpu.flags.cf)
    {
        goto L_0x004ca308;
    }
    // 004ca303  804c8da801             +or byte ptr [ebp + ecx*4 - 0x58], 1
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-88) /* -0x58 */ + cpu.ecx * 4) |= x86::reg8(x86::sreg8(1 /*0x1*/))));
L_0x004ca308:
    // 004ca308  d945e0                 +fld dword ptr [ebp - 0x20]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */));
    // 004ca30b  dc1d90035400           +fcomp qword ptr [0x540390]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505936) /* 0x540390 */)));
    // 004ca311  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ca313  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ca314  7305                   -jae 0x4ca31b
    if (!cpu.flags.cf)
    {
        goto L_0x004ca31b;
    }
    // 004ca316  804c8da804             +or byte ptr [ebp + ecx*4 - 0x58], 4
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-88) /* -0x58 */ + cpu.ecx * 4) |= x86::reg8(x86::sreg8(4 /*0x4*/))));
L_0x004ca31b:
    // 004ca31b  d945e4                 +fld dword ptr [ebp - 0x1c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */));
    // 004ca31e  dc1d90035400           +fcomp qword ptr [0x540390]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505936) /* 0x540390 */)));
    // 004ca324  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ca326  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ca327  7305                   -jae 0x4ca32e
    if (!cpu.flags.cf)
    {
        goto L_0x004ca32e;
    }
    // 004ca329  804c8da810             +or byte ptr [ebp + ecx*4 - 0x58], 0x10
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-88) /* -0x58 */ + cpu.ecx * 4) |= x86::reg8(x86::sreg8(16 /*0x10*/))));
L_0x004ca32e:
    // 004ca32e  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004ca32f  e9cdfeffff             -jmp 0x4ca201
    goto L_0x004ca201;
L_0x004ca334:
    // 004ca334  8b45a8                 -mov eax, dword ptr [ebp - 0x58]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */);
    // 004ca337  8b55ac                 -mov edx, dword ptr [ebp - 0x54]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 004ca33a  8b4db0                 -mov ecx, dword ptr [ebp - 0x50]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 004ca33d  21d0                   -and eax, edx
    cpu.eax &= x86::reg32(x86::sreg32(cpu.edx));
    // 004ca33f  8b5db4                 -mov ebx, dword ptr [ebp - 0x4c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 004ca342  21c8                   -and eax, ecx
    cpu.eax &= x86::reg32(x86::sreg32(cpu.ecx));
    // 004ca344  85d8                   +test eax, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.ebx));
    // 004ca346  7512                   -jne 0x4ca35a
    if (!cpu.flags.zf)
    {
        goto L_0x004ca35a;
    }
    // 004ca348  8a4df4                 -mov cl, byte ptr [ebp - 0xc]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004ca34b  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004ca350  8b75f0                 -mov esi, dword ptr [ebp - 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004ca353  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 004ca355  09c6                   -or esi, eax
    cpu.esi |= x86::reg32(x86::sreg32(cpu.eax));
    // 004ca357  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
L_0x004ca35a:
    // 004ca35a  ff45f4                 -inc dword ptr [ebp - 0xc]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */))++;
    // 004ca35d  837df404               +cmp dword ptr [ebp - 0xc], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca361  0f8dcc000000           -jge 0x4ca433
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004ca433;
    }
L_0x004ca367:
    // 004ca367  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004ca36a  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004ca36d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004ca36f  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004ca372  8b7dec                 -mov edi, dword ptr [ebp - 0x14]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004ca375  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004ca377  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004ca379  8d7db8                 -lea edi, [ebp - 0x48]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 004ca37c  8db004030000           -lea esi, [eax + 0x304]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(772) /* 0x304 */);
    // 004ca382  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004ca383  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004ca384  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004ca385  8b45b8                 -mov eax, dword ptr [ebp - 0x48]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 004ca388  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004ca38b  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004ca38e  8b45bc                 -mov eax, dword ptr [ebp - 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 004ca391  dd0580035400           -fld qword ptr [0x540380]
    fp1 = x86::Float(app->getMemory<double>(x86::reg32(5505920) /* 0x540380 */));
    // 004ca397  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004ca399  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004ca39b  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004ca39e  8b45c0                 -mov eax, dword ptr [ebp - 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004ca3a1  db45fc                 -fild dword ptr [ebp - 4]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004ca3a4  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004ca3a7  d8ca                   -fmul st(2)
    fp2 = fp::mul(fp2, x86::Float(fp0));
    // 004ca3a9  db45fc                 -fild dword ptr [ebp - 4]
    fp3 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004ca3ac  decb                   -fmulp st(3)
    fp0 = fp::mul(fp0, fp3);
    // 004ca3ae  8d5dd0                 -lea ebx, [ebp - 0x30]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 004ca3b1  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004ca3b3  d95dd0                 -fstp dword ptr [ebp - 0x30]
    app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = float(fp2);
    // 004ca3b6  d95dd4                 -fstp dword ptr [ebp - 0x2c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = float(fp1);
    // 004ca3b9  8a65d3                 -mov ah, byte ptr [ebp - 0x2d]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-45) /* -0x2d */);
    // 004ca3bc  8a55d7                 -mov dl, byte ptr [ebp - 0x29]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-41) /* -0x29 */);
    // 004ca3bf  80f480                 -xor ah, 0x80
    cpu.ah ^= x86::reg8(x86::sreg8(128 /*0x80*/));
    // 004ca3c2  80f280                 -xor dl, 0x80
    cpu.dl ^= x86::reg8(x86::sreg8(128 /*0x80*/));
    // 004ca3c5  d95dd8                 -fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fp0);
    // 004ca3c8  8855d7                 -mov byte ptr [ebp - 0x29], dl
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-41) /* -0x29 */) = cpu.dl;
    // 004ca3cb  8a75db                 -mov dh, byte ptr [ebp - 0x25]
    cpu.dh = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-37) /* -0x25 */);
    // 004ca3ce  8865d3                 -mov byte ptr [ebp - 0x2d], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-45) /* -0x2d */) = cpu.ah;
    // 004ca3d1  80f680                 -xor dh, 0x80
    cpu.dh ^= x86::reg8(x86::sreg8(128 /*0x80*/));
    // 004ca3d4  8d45d0                 -lea eax, [ebp - 0x30]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 004ca3d7  8875db                 -mov byte ptr [ebp - 0x25], dh
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-37) /* -0x25 */) = cpu.dh;
    // 004ca3da  8d5584                 -lea edx, [ebp - 0x7c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-124) /* -0x7c */);
    // 004ca3dd  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004ca3df  d900                   +fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004ca3e1  d80a                   +fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 004ca3e3  d900                   +fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004ca3e5  d84a04                 +fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 004ca3e8  d900                   +fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004ca3ea  d84a08                 +fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004ca3ed  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004ca3ef  d94004                 +fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004ca3f2  d84a0c                 +fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 004ca3f5  d94004                 +fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004ca3f8  d84a10                 +fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 004ca3fb  d94004                 +fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004ca3fe  d84a14                 +fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 004ca401  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 004ca403  dec3                   +faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 004ca405  dec3                   +faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 004ca407  dec3                   +faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 004ca409  d94008                 +fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004ca40c  d84a18                 +fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 004ca40f  d94008                 +fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004ca412  d84a1c                 +fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 004ca415  d94008                 +fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004ca418  d84a20                 +fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 004ca41b  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 004ca41d  dec3                   +faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 004ca41f  dec3                   +faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 004ca421  dec3                   +faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 004ca423  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004ca426  d91b                   +fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 004ca428  d95b04                 +fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 004ca42b  d95b08                 +fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 004ca42e  e9d7fdffff             -jmp 0x4ca20a
    goto L_0x004ca20a;
L_0x004ca433:
    // 004ca433  837df000               +cmp dword ptr [ebp - 0x10], 0
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
    // 004ca437  740d                   -je 0x4ca446
    if (cpu.flags.zf)
    {
        goto L_0x004ca446;
    }
    // 004ca439  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004ca43c  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004ca43f  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
    // 004ca441  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004ca444  eb29                   -jmp 0x4ca46f
    goto L_0x004ca46f;
L_0x004ca446:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca446  ff45f8                 -inc dword ptr [ebp - 8]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */))++;
    // 004ca449  837df808               +cmp dword ptr [ebp - 8], 8
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca44d  7d1e                   -jge 0x4ca46d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004ca46d;
    }
L_0x004ca44f:
    // 004ca44f  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004ca452  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004ca459  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004ca45b  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004ca45e  83b850d87d0000         +cmp dword ptr [eax + 0x7dd850], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8247376) /* 0x7dd850 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca465  0f8512fdffff           -jne 0x4ca17d
    if (!cpu.flags.zf)
    {
        goto L_0x004ca17d;
    }
    // 004ca46b  ebd9                   -jmp 0x4ca446
    goto L_0x004ca446;
L_0x004ca46d:
    // 004ca46d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004ca46f:
    // 004ca46f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004ca471  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca472  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca473  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca474  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca475  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca476  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4ca480(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ca480  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca482  803f00                 -cmp byte ptr [edi], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edi);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004ca485  00803f9a9959           -add byte ptr [eax + 0x59999a3f], al
    (app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1503238719) /* 0x59999a3f */)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca48b  3f                     -aas 
    NFS2_ASSERT(false);
    // 004ca48c  3333                   -xor esi, dword ptr [ebx]
    cpu.esi ^= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx)));
    // 004ca48e  333f                   -xor edi, dword ptr [edi]
    cpu.edi ^= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004ca490  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca492  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca494  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca496  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca498  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca49a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca49c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca49e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca4a0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca4a2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca4a4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca4a6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca4a8  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 004ca4ae  8bd2                   -mov edx, edx
    cpu.edx = cpu.edx;
    // 004ca4b0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004ca4b1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004ca4b2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004ca4b3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004ca4b4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004ca4b6  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004ca4b8  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004ca4ba  eb13                   -jmp 0x4ca4cf
    goto L_0x004ca4cf;
L_0x004ca4bc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca4bc  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004ca4be  7404                   -je 0x4ca4c4
    if (cpu.flags.zf)
    {
        goto L_0x004ca4c4;
    }
    // 004ca4c0  3b18                   +cmp ebx, dword ptr [eax]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca4c2  7f02                   -jg 0x4ca4c6
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004ca4c6;
    }
L_0x004ca4c4:
    // 004ca4c4  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x004ca4c6:
    // 004ca4c6  41                     -inc ecx
    (cpu.ecx)++;
    // 004ca4c7  81f958020000           +cmp ecx, 0x258
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(600 /*0x258*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca4cd  7d18                   -jge 0x4ca4e7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004ca4e7;
    }
L_0x004ca4cf:
    // 004ca4cf  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004ca4d1  c1e207                 -shl edx, 7
    cpu.edx <<= 7 /*0x7*/ % 32;
    // 004ca4d4  81c2d0df7d00           -add edx, 0x7ddfd0
    (cpu.edx) += x86::reg32(x86::sreg32(8249296 /*0x7ddfd0*/));
    // 004ca4da  8b1a                   -mov ebx, dword ptr [edx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx);
    // 004ca4dc  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004ca4de  75dc                   -jne 0x4ca4bc
    if (!cpu.flags.zf)
    {
        goto L_0x004ca4bc;
    }
    // 004ca4e0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004ca4e2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4e3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4e4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4e5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4e6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ca4e7:
    // 004ca4e7  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004ca4e9  7505                   -jne 0x4ca4f0
    if (!cpu.flags.zf)
    {
        goto L_0x004ca4f0;
    }
    // 004ca4eb  b8d0df7d00             -mov eax, 0x7ddfd0
    cpu.eax = 8249296 /*0x7ddfd0*/;
L_0x004ca4f0:
    // 004ca4f0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4f1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4f2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4f3  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4f4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4ca4b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x004ca4b0;
    // 004ca480  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca482  803f00                 -cmp byte ptr [edi], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edi);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004ca485  00803f9a9959           -add byte ptr [eax + 0x59999a3f], al
    (app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1503238719) /* 0x59999a3f */)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca48b  3f                     -aas 
    NFS2_ASSERT(false);
    // 004ca48c  3333                   -xor esi, dword ptr [ebx]
    cpu.esi ^= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx)));
    // 004ca48e  333f                   -xor edi, dword ptr [edi]
    cpu.edi ^= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi)));
    // 004ca490  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca492  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca494  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca496  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca498  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca49a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca49c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca49e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca4a0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca4a2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca4a4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca4a6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004ca4a8  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 004ca4ae  8bd2                   -mov edx, edx
    cpu.edx = cpu.edx;
L_entry_0x004ca4b0:
    // 004ca4b0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004ca4b1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004ca4b2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004ca4b3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004ca4b4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004ca4b6  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004ca4b8  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004ca4ba  eb13                   -jmp 0x4ca4cf
    goto L_0x004ca4cf;
L_0x004ca4bc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca4bc  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004ca4be  7404                   -je 0x4ca4c4
    if (cpu.flags.zf)
    {
        goto L_0x004ca4c4;
    }
    // 004ca4c0  3b18                   +cmp ebx, dword ptr [eax]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca4c2  7f02                   -jg 0x4ca4c6
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004ca4c6;
    }
L_0x004ca4c4:
    // 004ca4c4  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x004ca4c6:
    // 004ca4c6  41                     -inc ecx
    (cpu.ecx)++;
    // 004ca4c7  81f958020000           +cmp ecx, 0x258
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(600 /*0x258*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca4cd  7d18                   -jge 0x4ca4e7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004ca4e7;
    }
L_0x004ca4cf:
    // 004ca4cf  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004ca4d1  c1e207                 -shl edx, 7
    cpu.edx <<= 7 /*0x7*/ % 32;
    // 004ca4d4  81c2d0df7d00           -add edx, 0x7ddfd0
    (cpu.edx) += x86::reg32(x86::sreg32(8249296 /*0x7ddfd0*/));
    // 004ca4da  8b1a                   -mov ebx, dword ptr [edx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx);
    // 004ca4dc  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004ca4de  75dc                   -jne 0x4ca4bc
    if (!cpu.flags.zf)
    {
        goto L_0x004ca4bc;
    }
    // 004ca4e0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004ca4e2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4e3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4e4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4e5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4e6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ca4e7:
    // 004ca4e7  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004ca4e9  7505                   -jne 0x4ca4f0
    if (!cpu.flags.zf)
    {
        goto L_0x004ca4f0;
    }
    // 004ca4eb  b8d0df7d00             -mov eax, 0x7ddfd0
    cpu.eax = 8249296 /*0x7ddfd0*/;
L_0x004ca4f0:
    // 004ca4f0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4f1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4f2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4f3  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca4f4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_4ca500(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5, fp6;
  x86::reg16 fsw = 0;
    // 004ca500  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004ca501  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004ca502  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004ca503  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004ca504  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004ca505  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004ca506  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004ca508  83ec58                 -sub esp, 0x58
    (cpu.esp) -= x86::reg32(x86::sreg32(88 /*0x58*/));
    // 004ca50b  8b7528                 -mov esi, dword ptr [ebp + 0x28]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(40) /* 0x28 */);
    // 004ca50e  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004ca510  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004ca512  c1e207                 -shl edx, 7
    cpu.edx <<= 7 /*0x7*/ % 32;
    // 004ca515  8b82c80b5600           -mov eax, dword ptr [edx + 0x560bc8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(5639112) /* 0x560bc8 */);
    // 004ca51b  8b0d30bc6f00           -mov ecx, dword ptr [0x6fbc30]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7322672) /* 0x6fbc30 */);
    // 004ca521  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
    // 004ca524  83f901                 +cmp ecx, 1
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
    // 004ca527  7c07                   -jl 0x4ca530
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004ca530;
    }
    // 004ca529  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004ca52b  e915030000             -jmp 0x4ca845
    goto L_0x004ca845;
L_0x004ca530:
    // 004ca530  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004ca532  7413                   -je 0x4ca547
    if (cpu.flags.zf)
    {
        goto L_0x004ca547;
    }
    // 004ca534  83bacc0b560000         +cmp dword ptr [edx + 0x560bcc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(5639116) /* 0x560bcc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca53b  740a                   -je 0x4ca547
    if (cpu.flags.zf)
    {
        goto L_0x004ca547;
    }
    // 004ca53d  833c9da0c4790000       +cmp dword ptr [ebx*4 + 0x79c4a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7980192) /* 0x79c4a0 */ + cpu.ebx * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca545  740d                   -je 0x4ca554
    if (cpu.flags.zf)
    {
        goto L_0x004ca554;
    }
L_0x004ca547:
    // 004ca547  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ca549  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004ca54b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca54c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca54d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca54e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca54f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca550  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca551  c21000                 -ret 0x10
    cpu.esp += 4+16 /*0x10*/;
    return;
L_0x004ca554:
    // 004ca554  833d30bc6f0001         +cmp dword ptr [0x6fbc30], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7322672) /* 0x6fbc30 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca55b  7c0d                   -jl 0x4ca56a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004ca56a;
    }
    // 004ca55d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ca55f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004ca561  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca562  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca563  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca564  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca565  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca566  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca567  c21000                 -ret 0x10
    cpu.esp += 4+16 /*0x10*/;
    return;
L_0x004ca56a:
    // 004ca56a  f782e00b5600ffffff7f   +test dword ptr [edx + 0x560be0], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(5639136) /* 0x560be0 */) & 2147483647 /*0x7fffffff*/));
    // 004ca574  7414                   -je 0x4ca58a
    if (cpu.flags.zf)
    {
        goto L_0x004ca58a;
    }
    // 004ca576  e8b50c0200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca57b  d89ae00b5600           +fcomp dword ptr [edx + 0x560be0]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(5639136) /* 0x560be0 */)));
    // 004ca581  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ca583  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ca584  0f87b6020000           -ja 0x4ca840
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004ca840;
    }
L_0x004ca58a:
    // 004ca58a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004ca58c  8955ec                 -mov dword ptr [ebp - 0x14], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edx;
L_0x004ca58f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca58f  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004ca592  3b45dc                 +cmp eax, dword ptr [ebp - 0x24]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca595  0f8da5020000           -jge 0x4ca840
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004ca840;
    }
    // 004ca59b  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004ca59d  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004ca59f  894dd0                 -mov dword ptr [ebp - 0x30], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.ecx;
    // 004ca5a2  eb09                   -jmp 0x4ca5ad
    goto L_0x004ca5ad;
L_0x004ca5a4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca5a4  42                     -inc edx
    (cpu.edx)++;
    // 004ca5a5  81fac8000000           +cmp edx, 0xc8
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(200 /*0xc8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca5ab  7d25                   -jge 0x4ca5d2
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004ca5d2;
    }
L_0x004ca5ad:
    // 004ca5ad  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 004ca5b4  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004ca5b6  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004ca5b9  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004ca5bb  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004ca5bd  c1e009                 -shl eax, 9
    cpu.eax <<= 9 /*0x9*/ % 32;
    // 004ca5c0  c1e706                 -shl edi, 6
    cpu.edi <<= 6 /*0x6*/ % 32;
    // 004ca5c3  05d00b7f00             -add eax, 0x7f0bd0
    (cpu.eax) += x86::reg32(x86::sreg32(8326096 /*0x7f0bd0*/));
    // 004ca5c8  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004ca5ca  8b38                   -mov edi, dword ptr [eax]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax);
    // 004ca5cc  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004ca5ce  75d4                   -jne 0x4ca5a4
    if (!cpu.flags.zf)
    {
        goto L_0x004ca5a4;
    }
    // 004ca5d0  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x004ca5d2:
    // 004ca5d2  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004ca5d4  750d                   -jne 0x4ca5e3
    if (!cpu.flags.zf)
    {
        goto L_0x004ca5e3;
    }
    // 004ca5d6  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ca5d8  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004ca5da  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca5db  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca5dc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca5dd  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca5de  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca5df  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca5e0  c21000                 -ret 0x10
    cpu.esp += 4+16 /*0x10*/;
    return;
L_0x004ca5e3:
    // 004ca5e3  f74524ffffff7f         +test dword ptr [ebp + 0x24], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(36) /* 0x24 */) & 2147483647 /*0x7fffffff*/));
    // 004ca5ea  7408                   -je 0x4ca5f4
    if (cpu.flags.zf)
    {
        goto L_0x004ca5f4;
    }
    // 004ca5ec  d94524                 +fld dword ptr [ebp + 0x24]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(36) /* 0x24 */));
    // 004ca5ef  dd5db0                 +fstp qword ptr [ebp - 0x50]
    app->getMemory<double>(cpu.ebp + x86::reg32(-80) /* -0x50 */) = double(fp0);
    // 004ca5f2  eb10                   -jmp 0x4ca604
    goto L_0x004ca604;
L_0x004ca5f4:
    // 004ca5f4  ba0f001000             -mov edx, 0x10000f
    cpu.edx = 1048591 /*0x10000f*/;
    // 004ca5f9  bf1000f03e             -mov edi, 0x3ef00010
    cpu.edi = 1055916048 /*0x3ef00010*/;
    // 004ca5fe  8955b0                 -mov dword ptr [ebp - 0x50], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */) = cpu.edx;
    // 004ca601  897db4                 -mov dword ptr [ebp - 0x4c], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = cpu.edi;
L_0x004ca604:
    // 004ca604  dd45b0                 -fld qword ptr [ebp - 0x50]
    fp0 = x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-80) /* -0x50 */));
    // 004ca607  d9451c                 -fld dword ptr [ebp + 0x1c]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(28) /* 0x1c */));
    // 004ca60a  d9e8                   -fld1 
    fp2 = 1.0;
    // 004ca60c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004ca60e  def2                   -fdivrp st(2)
    fp0 = fp::div(fp2, x86::Float(fp0));
    // 004ca610  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004ca612  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004ca615  d94520                 -fld dword ptr [ebp + 0x20]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(32) /* 0x20 */));
    // 004ca618  8b80d80b5600           -mov eax, dword ptr [eax + 0x560bd8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5639128) /* 0x560bd8 */);
    // 004ca61e  deca                   -fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 004ca620  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 004ca623  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp1);
    // 004ca626  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 004ca629  a9ffffff7f             +test eax, 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483647 /*0x7fffffff*/));
    // 004ca62e  7434                   -je 0x4ca664
    if (cpu.flags.zf)
    {
        goto L_0x004ca664;
    }
    // 004ca630  e8fb0b0200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca635  d84df0                 -fmul dword ptr [ebp - 0x10]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 004ca638  dc0d98035400           -fmul qword ptr [0x540398]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505944) /* 0x540398 */)));
    // 004ca63e  d945f0                 -fld dword ptr [ebp - 0x10]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */));
    // 004ca641  dd5da8                 -fstp qword ptr [ebp - 0x58]
    app->getMemory<double>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = double(fp1);
    // 004ca644  dc65a8                 -fsub qword ptr [ebp - 0x58]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-88) /* -0x58 */)));
    // 004ca647  d845f4                 -fadd dword ptr [ebp - 0xc]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 004ca64a  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp0);
    // 004ca64d  e8de0b0200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca652  d84df0                 -fmul dword ptr [ebp - 0x10]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 004ca655  dc0d98035400           -fmul qword ptr [0x540398]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505944) /* 0x540398 */)));
    // 004ca65b  dc65a8                 -fsub qword ptr [ebp - 0x58]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-88) /* -0x58 */)));
    // 004ca65e  d845f8                 -fadd dword ptr [ebp - 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004ca661  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
L_0x004ca664:
    // 004ca664  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 004ca666  eb5f                   -jmp 0x4ca6c7
    goto L_0x004ca6c7;
L_0x004ca668:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca668  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004ca66b  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
L_0x004ca66e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca66e  d945d8                 +fld dword ptr [ebp - 0x28]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */));
    // 004ca671  d85dd4                 +fcomp dword ptr [ebp - 0x2c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */)));
    // 004ca674  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ca676  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ca677  7345                   -jae 0x4ca6be
    if (!cpu.flags.cf)
    {
        goto L_0x004ca6be;
    }
    // 004ca679  d945f8                 +fld dword ptr [ebp - 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004ca67c  d86238                 +fsub dword ptr [edx + 0x38]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(56) /* 0x38 */)));
    // 004ca67f  d9ee                   +fldz 
    fp1 = 0.0;
    // 004ca681  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004ca683  d95de4                 +fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp1);
    // 004ca686  d85de4                 +fcomp dword ptr [ebp - 0x1c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */)));
    // 004ca689  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ca68b  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ca68c  760a                   -jbe 0x4ca698
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ca698;
    }
    // 004ca68e  d945e4                 +fld dword ptr [ebp - 0x1c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */));
    // 004ca691  d9e0                   +fchs 
    fp0 = -fp0;
    // 004ca693  d95dfc                 +fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 004ca696  eb06                   -jmp 0x4ca69e
    goto L_0x004ca69e;
L_0x004ca698:
    // 004ca698  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004ca69b  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
L_0x004ca69e:
    // 004ca69e  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004ca6a1  d85dd4                 +fcomp dword ptr [ebp - 0x2c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */)));
    // 004ca6a4  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ca6a6  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ca6a7  7315                   -jae 0x4ca6be
    if (!cpu.flags.cf)
    {
        goto L_0x004ca6be;
    }
    // 004ca6a9  83fe0a                 +cmp esi, 0xa
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(10 /*0xa*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca6ac  0f8586010000           -jne 0x4ca838
    if (!cpu.flags.zf)
    {
        goto L_0x004ca838;
    }
    // 004ca6b2  837a040b               +cmp dword ptr [edx + 4], 0xb
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(11 /*0xb*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca6b6  0f857c010000           -jne 0x4ca838
    if (!cpu.flags.zf)
    {
        goto L_0x004ca838;
    }
    // 004ca6bc  d13a                   -sar dword ptr [edx], 1
    app->getMemory<x86::reg32>(cpu.edx) = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx)) >> (1 /*0x1*/ % 32));
L_0x004ca6be:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca6be  47                     -inc edi
    (cpu.edi)++;
    // 004ca6bf  81ffc8000000           +cmp edi, 0xc8
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(200 /*0xc8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ca6c5  7d59                   -jge 0x4ca720
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004ca720;
    }
L_0x004ca6c7:
    // 004ca6c7  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 004ca6ce  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004ca6d0  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004ca6d3  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004ca6d5  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004ca6d7  c1e009                 -shl eax, 9
    cpu.eax <<= 9 /*0x9*/ % 32;
    // 004ca6da  c1e206                 -shl edx, 6
    cpu.edx <<= 6 /*0x6*/ % 32;
    // 004ca6dd  05d00b7f00             -add eax, 0x7f0bd0
    (cpu.eax) += x86::reg32(x86::sreg32(8326096 /*0x7f0bd0*/));
    // 004ca6e2  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004ca6e4  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004ca6e6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004ca6e8  74d4                   -je 0x4ca6be
    if (cpu.flags.zf)
    {
        goto L_0x004ca6be;
    }
    // 004ca6ea  d9423c                 +fld dword ptr [edx + 0x3c]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(60) /* 0x3c */));
    // 004ca6ed  dc0da0035400           +fmul qword ptr [0x5403a0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505952) /* 0x5403a0 */)));
    // 004ca6f3  d945f4                 +fld dword ptr [ebp - 0xc]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004ca6f6  d9ee                   +fldz 
    fp2 = 0.0;
    // 004ca6f8  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004ca6fa  d86234                 +fsub dword ptr [edx + 0x34]
    fp2 = fp::sub(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(52) /* 0x34 */)));
    // 004ca6fd  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004ca6ff  d95dd4                 +fstp dword ptr [ebp - 0x2c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = float(fp2);
    // 004ca702  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004ca704  d95de8                 +fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp1);
    // 004ca707  d85de8                 +fcomp dword ptr [ebp - 0x18]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */)));
    // 004ca70a  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ca70c  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ca70d  0f8655ffffff           -jbe 0x4ca668
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ca668;
    }
    // 004ca713  d945e8                 +fld dword ptr [ebp - 0x18]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */));
    // 004ca716  d9e0                   +fchs 
    fp0 = -fp0;
    // 004ca718  d95dd8                 +fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fp0);
    // 004ca71b  e94effffff             -jmp 0x4ca66e
    goto L_0x004ca66e;
L_0x004ca720:
    // 004ca720  8b55d0                 -mov edx, dword ptr [ebp - 0x30]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 004ca723  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004ca725  0f850d010000           -jne 0x4ca838
    if (!cpu.flags.zf)
    {
        goto L_0x004ca838;
    }
    // 004ca72b  89f7                   -mov edi, esi
    cpu.edi = cpu.esi;
    // 004ca72d  c1e707                 -shl edi, 7
    cpu.edi <<= 7 /*0x7*/ % 32;
    // 004ca730  8b87cc0b5600           -mov eax, dword ptr [edi + 0x560bcc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(5639116) /* 0x560bcc */);
    // 004ca736  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004ca738  8945cc                 -mov dword ptr [ebp - 0x34], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.eax;
    // 004ca73b  e848660100             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ca740  f775cc                 -div dword ptr [ebp - 0x34]
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 004ca743  8b87cc0b5600           -mov eax, dword ptr [edi + 0x560bcc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(5639116) /* 0x560bcc */);
    // 004ca749  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004ca74b  897104                 -mov dword ptr [ecx + 4], esi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */) = cpu.esi;
    // 004ca74e  8901                   -mov dword ptr [ecx], eax
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.eax;
    // 004ca750  8b87d40b5600           -mov eax, dword ptr [edi + 0x560bd4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(5639124) /* 0x560bd4 */);
    // 004ca756  8945c4                 -mov dword ptr [ebp - 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.eax;
    // 004ca759  e8d20a0200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca75e  d945c4                 -fld dword ptr [ebp - 0x3c]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-60) /* -0x3c */));
    // 004ca761  d9c0                   -fld st(0)
    fp2 = x86::Float(fp1);
    // 004ca763  dc0da0035400           -fmul qword ptr [0x5403a0]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<double>(x86::reg32(5505952) /* 0x5403a0 */)));
    // 004ca769  deca                   -fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 004ca76b  dee1                   -fsubrp st(1)
    fp0 = fp::sub(fp1, x86::Float(fp0));
    // 004ca76d  d95dc4                 -fstp dword ptr [ebp - 0x3c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = float(fp0);
    // 004ca770  e8bb0a0200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca775  d95dc0                 -fstp dword ptr [ebp - 0x40]
    app->getMemory<float>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = float(fp0);
    // 004ca778  ff75c0                 -push dword ptr [ebp - 0x40]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    cpu.esp -= 4;
    // 004ca77b  e8b0010200             -call 0x4ea930
    cpu.esp -= 4;
    sub_4ea930(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca780  d84dc4                 -fmul dword ptr [ebp - 0x3c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-60) /* -0x3c */)));
    // 004ca783  ff75c0                 -push dword ptr [ebp - 0x40]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    cpu.esp -= 4;
    // 004ca786  d95dc8                 -fstp dword ptr [ebp - 0x38]
    app->getMemory<float>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = float(fp0);
    // 004ca789  e8c2010200             -call 0x4ea950
    cpu.esp -= 4;
    sub_4ea950(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca78e  d84dc4                 -fmul dword ptr [ebp - 0x3c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-60) /* -0x3c */)));
    // 004ca791  d945f4                 -fld dword ptr [ebp - 0xc]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004ca794  d945f8                 -fld dword ptr [ebp - 8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004ca797  d945f4                 -fld dword ptr [ebp - 0xc]
    fp3 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004ca79a  d945f8                 -fld dword ptr [ebp - 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004ca79d  d945f4                 -fld dword ptr [ebp - 0xc]
    fp5 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004ca7a0  d945f8                 -fld dword ptr [ebp - 8]
    fp6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004ca7a3  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004ca7a6  d9cd                   -fxch st(5)
    {
        x86::Float tmp = fp6;
        fp6 = fp1;
        fp1 = tmp;
    }
    // 004ca7a8  d865c8                 -fsub dword ptr [ebp - 0x38]
    fp6 = fp::sub(fp6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-56) /* -0x38 */)));
    // 004ca7ab  894134                 -mov dword ptr [ecx + 0x34], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(52) /* 0x34 */) = cpu.eax;
    // 004ca7ae  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp6;
        fp6 = fp4;
        fp4 = tmp;
    }
    // 004ca7b0  d845c8                 -fadd dword ptr [ebp - 0x38]
    fp6 = fp::add(fp6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-56) /* -0x38 */)));
    // 004ca7b3  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp6;
        fp6 = fp4;
        fp4 = tmp;
    }
    // 004ca7b5  d95908                 -fstp dword ptr [ecx + 8]
    app->getMemory<float>(cpu.ecx + x86::reg32(8) /* 0x8 */) = float(fp6);
    // 004ca7b8  d945f4                 -fld dword ptr [ebp - 0xc]
    fp6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004ca7bb  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp6;
        fp6 = fp4;
        fp4 = tmp;
    }
    // 004ca7bd  d95914                 -fstp dword ptr [ecx + 0x14]
    app->getMemory<float>(cpu.ecx + x86::reg32(20) /* 0x14 */) = float(fp6);
    // 004ca7c0  d945f8                 -fld dword ptr [ebp - 8]
    fp6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004ca7c3  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp6;
        fp6 = fp5;
        fp5 = tmp;
    }
    // 004ca7c5  d845c8                 -fadd dword ptr [ebp - 0x38]
    fp6 = fp::add(fp6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-56) /* -0x38 */)));
    // 004ca7c8  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp6;
        fp6 = fp5;
        fp5 = tmp;
    }
    // 004ca7ca  d865c8                 -fsub dword ptr [ebp - 0x38]
    fp6 = fp::sub(fp6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-56) /* -0x38 */)));
    // 004ca7cd  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp6;
        fp6 = fp5;
        fp5 = tmp;
    }
    // 004ca7cf  d95918                 -fstp dword ptr [ecx + 0x18]
    app->getMemory<float>(cpu.ecx + x86::reg32(24) /* 0x18 */) = float(fp6);
    // 004ca7d2  d95924                 -fstp dword ptr [ecx + 0x24]
    app->getMemory<float>(cpu.ecx + x86::reg32(36) /* 0x24 */) = float(fp5);
    // 004ca7d5  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004ca7d8  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp4;
        fp4 = fp2;
        fp2 = tmp;
    }
    // 004ca7da  d8c4                   -fadd st(4)
    fp4 = fp::add(fp4, x86::Float(fp0));
    // 004ca7dc  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp4;
        fp4 = fp3;
        fp3 = tmp;
    }
    // 004ca7de  d8c4                   -fadd st(4)
    fp4 = fp::add(fp4, x86::Float(fp0));
    // 004ca7e0  894138                 -mov dword ptr [ecx + 0x38], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(56) /* 0x38 */) = cpu.eax;
    // 004ca7e3  8b45c4                 -mov eax, dword ptr [ebp - 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 004ca7e6  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp4;
        fp4 = fp1;
        fp1 = tmp;
    }
    // 004ca7e8  d8e4                   -fsub st(4)
    fp4 = fp::sub(fp4, x86::Float(fp0));
    // 004ca7ea  89413c                 -mov dword ptr [ecx + 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */) = cpu.eax;
    // 004ca7ed  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp4;
        fp4 = fp2;
        fp2 = tmp;
    }
    // 004ca7ef  dee4                   -fsubrp st(4)
    fp0 = fp::sub(fp4, x86::Float(fp0));
    // 004ca7f1  d9590c                 -fstp dword ptr [ecx + 0xc]
    app->getMemory<float>(cpu.ecx + x86::reg32(12) /* 0xc */) = float(fp3);
    // 004ca7f4  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004ca7f6  d95910                 -fstp dword ptr [ecx + 0x10]
    app->getMemory<float>(cpu.ecx + x86::reg32(16) /* 0x10 */) = float(fp2);
    // 004ca7f9  d9591c                 -fstp dword ptr [ecx + 0x1c]
    app->getMemory<float>(cpu.ecx + x86::reg32(28) /* 0x1c */) = float(fp1);
    // 004ca7fc  d95920                 -fstp dword ptr [ecx + 0x20]
    app->getMemory<float>(cpu.ecx + x86::reg32(32) /* 0x20 */) = float(fp0);
    // 004ca7ff  8b87dc0b5600           -mov eax, dword ptr [edi + 0x560bdc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(5639132) /* 0x560bdc */);
    // 004ca805  894128                 -mov dword ptr [ecx + 0x28], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(40) /* 0x28 */) = cpu.eax;
    // 004ca808  8b87d00b5600           -mov eax, dword ptr [edi + 0x560bd0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(5639120) /* 0x560bd0 */);
    // 004ca80e  89412c                 -mov dword ptr [ecx + 0x2c], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(44) /* 0x2c */) = cpu.eax;
    // 004ca811  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004ca818  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004ca81a  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004ca81d  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004ca81f  ba00db7d00             -mov edx, 0x7ddb00
    cpu.edx = 8248064 /*0x7ddb00*/;
    // 004ca824  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004ca827  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004ca829  895130                 -mov dword ptr [ecx + 0x30], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(48) /* 0x30 */) = cpu.edx;
    // 004ca82c  83fe04                 +cmp esi, 4
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
    // 004ca82f  7507                   -jne 0x4ca838
    if (!cpu.flags.zf)
    {
        goto L_0x004ca838;
    }
    // 004ca831  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004ca833  e8d8a7f4ff             -call 0x415010
    cpu.esp -= 4;
    nfs3hp::port::carEffectFifteen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004ca838:
    // 004ca838  ff45ec                 +inc dword ptr [ebp - 0x14]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004ca83b  e94ffdffff             -jmp 0x4ca58f
    goto L_0x004ca58f;
L_0x004ca840:
    // 004ca840  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x004ca845:
    // 004ca845  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004ca847  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca848  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca849  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca84a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca84b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca84c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca84d  c21000                 -ret 0x10
    cpu.esp += 4+16 /*0x10*/;
    return;
}

/* align: skip  */
void Application::sub_4ca850(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5, fp6;
    // 004ca850  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004ca851  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004ca852  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004ca853  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004ca854  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004ca855  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004ca856  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004ca858  83ec2c                 -sub esp, 0x2c
    (cpu.esp) -= x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 004ca85b  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
    // 004ca85e  e84dfcffff             -call 0x4ca4b0
    cpu.esp -= 4;
    sub_4ca4b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ca863  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004ca865  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004ca868  bb640b5600             -mov ebx, 0x560b64
    cpu.ebx = 5639012 /*0x560b64*/;
    // 004ca86d  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004ca870  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004ca872  8b4310                 -mov eax, dword ptr [ebx + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(16) /* 0x10 */);
    // 004ca875  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 004ca878  8b4314                 -mov eax, dword ptr [ebx + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */);
    // 004ca87b  8b3584367d00           -mov esi, dword ptr [0x7d3684]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004ca881  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004ca884  8b4318                 -mov eax, dword ptr [ebx + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */);
    // 004ca887  d1fe                   -sar esi, 1
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (1 /*0x1*/ % 32));
    // 004ca889  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 004ca88c  8b4334                 -mov eax, dword ptr [ebx + 0x34]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(52) /* 0x34 */);
    // 004ca88f  8b7b08                 -mov edi, dword ptr [ebx + 8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 004ca892  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 004ca895  8d043f                 -lea eax, [edi + edi]
    cpu.eax = x86::reg32(cpu.edi + cpu.edi * 1);
    // 004ca898  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004ca89a  83c002                 -add eax, 2
    (cpu.eax) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004ca89d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004ca8a0  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004ca8a3  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004ca8a5  f77df8                 -idiv dword ptr [ebp - 8]
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004ca8a8  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004ca8aa  7507                   -jne 0x4ca8b3
    if (!cpu.flags.zf)
    {
        goto L_0x004ca8b3;
    }
    // 004ca8ac  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004ca8b1  eb02                   -jmp 0x4ca8b5
    goto L_0x004ca8b5;
L_0x004ca8b3:
    // 004ca8b3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004ca8b5:
    // 004ca8b5  8b1530bc6f00           -mov edx, dword ptr [0x6fbc30]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7322672) /* 0x6fbc30 */);
    // 004ca8bb  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
    // 004ca8be  83fa02                 +cmp edx, 2
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
    // 004ca8c1  7507                   -jne 0x4ca8ca
    if (!cpu.flags.zf)
    {
        goto L_0x004ca8ca;
    }
L_0x004ca8c3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ca8c3  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004ca8c5  e97f020000             -jmp 0x4cab49
    goto L_0x004cab49;
L_0x004ca8ca:
    // 004ca8ca  83fa01                 +cmp edx, 1
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
    // 004ca8cd  7504                   -jne 0x4ca8d3
    if (!cpu.flags.zf)
    {
        goto L_0x004ca8d3;
    }
    // 004ca8cf  a801                   +test al, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 1 /*0x1*/));
    // 004ca8d1  75f0                   -jne 0x4ca8c3
    if (!cpu.flags.zf)
    {
        goto L_0x004ca8c3;
    }
L_0x004ca8d3:
    // 004ca8d3  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004ca8d5  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004ca8d7  47                     -inc edi
    (cpu.edi)++;
    // 004ca8d8  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004ca8db  f7ff                   -idiv edi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.edi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004ca8dd  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004ca8df  740d                   -je 0x4ca8ee
    if (cpu.flags.zf)
    {
        goto L_0x004ca8ee;
    }
    // 004ca8e1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ca8e3  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004ca8e5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca8e6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca8e7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca8e8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca8e9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca8ea  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ca8eb  c23400                 -ret 0x34
    cpu.esp += 4+52 /*0x34*/;
    return;
L_0x004ca8ee:
    // 004ca8ee  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004ca8f1  894104                 -mov dword ptr [ecx + 4], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 004ca8f4  8b4304                 -mov eax, dword ptr [ebx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 004ca8f7  8901                   -mov dword ptr [ecx], eax
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.eax;
    // 004ca8f9  8d751c                 -lea esi, [ebp + 0x1c]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004ca8fc  894108                 -mov dword ptr [ecx + 8], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004ca8ff  8a45e4                 -mov al, byte ptr [ebp - 0x1c]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004ca902  8d790c                 -lea edi, [ecx + 0xc]
    cpu.edi = x86::reg32(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 004ca905  88417d                 -mov byte ptr [ecx + 0x7d], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(125) /* 0x7d */) = cpu.al;
    // 004ca908  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004ca909  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004ca90a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004ca90b  e820090200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca910  dc05a8035400           -fadd qword ptr [0x5403a8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505960) /* 0x5403a8 */)));
    // 004ca916  d84b1c                 -fmul dword ptr [ebx + 0x1c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(28) /* 0x1c */)));
    // 004ca919  d9e8                   -fld1 
    fp1 = 1.0;
    // 004ca91b  dec1                   -faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 004ca91d  d84df0                 -fmul dword ptr [ebp - 0x10]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 004ca920  d95df0                 -fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp0);
    // 004ca923  e808090200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca928  dc05a8035400           -fadd qword ptr [0x5403a8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505960) /* 0x5403a8 */)));
    // 004ca92e  d84b1c                 -fmul dword ptr [ebx + 0x1c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(28) /* 0x1c */)));
    // 004ca931  d9e8                   -fld1 
    fp1 = 1.0;
    // 004ca933  dec1                   -faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 004ca935  d84df4                 -fmul dword ptr [ebp - 0xc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 004ca938  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp0);
    // 004ca93b  e8f0080200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca940  d945f4                 -fld dword ptr [ebp - 0xc]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004ca943  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004ca945  dc05a8035400           -fadd qword ptr [0x5403a8]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<double>(x86::reg32(5505960) /* 0x5403a8 */)));
    // 004ca94b  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004ca94d  d84d28                 -fmul dword ptr [ebp + 0x28]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(40) /* 0x28 */)));
    // 004ca950  d9431c                 -fld dword ptr [ebx + 0x1c]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(28) /* 0x1c */));
    // 004ca953  deca                   -fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 004ca955  d9c0                   -fld st(0)
    fp2 = x86::Float(fp1);
    // 004ca957  d9e8                   -fld1 
    fp3 = 1.0;
    // 004ca959  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 004ca95b  d84d2c                 -fmul dword ptr [ebp + 0x2c]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(44) /* 0x2c */)));
    // 004ca95e  d945e8                 -fld dword ptr [ebp - 0x18]
    fp3 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */));
    // 004ca961  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 004ca963  d8cb                   -fmul st(3)
    fp3 = fp::mul(fp3, x86::Float(fp0));
    // 004ca965  d9c2                   -fld st(2)
    fp4 = x86::Float(fp1);
    // 004ca967  d84d30                 -fmul dword ptr [ebp + 0x30]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(48) /* 0x30 */)));
    // 004ca96a  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp4;
        fp4 = fp3;
        fp3 = tmp;
    }
    // 004ca96c  dddc                   -fstp st(4)
    fp0 = x86::Float(fp4);
    // 004ca96e  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 004ca970  d955e8                 -fst dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp3);
    // 004ca973  d84d28                 -fmul dword ptr [ebp + 0x28]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(40) /* 0x28 */)));
    // 004ca976  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp3;
        fp3 = fp1;
        fp1 = tmp;
    }
    // 004ca978  d84d34                 -fmul dword ptr [ebp + 0x34]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(52) /* 0x34 */)));
    // 004ca97b  d9c2                   -fld st(2)
    fp4 = x86::Float(fp1);
    // 004ca97d  d84d38                 -fmul dword ptr [ebp + 0x38]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(56) /* 0x38 */)));
    // 004ca980  d9c3                   -fld st(3)
    fp5 = x86::Float(fp1);
    // 004ca982  d84d3c                 -fmul dword ptr [ebp + 0x3c]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(60) /* 0x3c */)));
    // 004ca985  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp5;
        fp5 = fp4;
        fp4 = tmp;
    }
    // 004ca987  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 004ca989  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp4;
        fp4 = fp1;
        fp1 = tmp;
    }
    // 004ca98b  d84d40                 -fmul dword ptr [ebp + 0x40]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(64) /* 0x40 */)));
    // 004ca98e  d945ec                 -fld dword ptr [ebp - 0x14]
    fp5 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */));
    // 004ca991  d94544                 -fld dword ptr [ebp + 0x44]
    fp6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(68) /* 0x44 */));
    // 004ca994  d8c9                   -fmul st(1)
    fp6 = fp::mul(fp6, x86::Float(fp5));
    // 004ca996  d9cd                   -fxch st(5)
    {
        x86::Float tmp = fp6;
        fp6 = fp1;
        fp1 = tmp;
    }
    // 004ca998  dec6                   -faddp st(6)
    fp0 = fp::add(fp0, fp6);
    // 004ca99a  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp5;
        fp5 = fp4;
        fp4 = tmp;
    }
    // 004ca99c  dec2                   -faddp st(2)
    fp3 = fp::add(fp3, fp5);
    // 004ca99e  d94548                 -fld dword ptr [ebp + 0x48]
    fp5 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(72) /* 0x48 */));
    // 004ca9a1  d8c9                   -fmul st(1)
    fp5 = fp::mul(fp5, x86::Float(fp4));
    // 004ca9a3  d9454c                 -fld dword ptr [ebp + 0x4c]
    fp6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(76) /* 0x4c */));
    // 004ca9a6  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp6;
        fp6 = fp5;
        fp5 = tmp;
    }
    // 004ca9a8  dec6                   -faddp st(6)
    fp0 = fp::add(fp0, fp6);
    // 004ca9aa  dec9                   -fmulp st(1)
    fp4 = fp::mul(fp4, fp5);
    // 004ca9ac  d945f0                 -fld dword ptr [ebp - 0x10]
    fp5 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */));
    // 004ca9af  d84d28                 -fmul dword ptr [ebp + 0x28]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(40) /* 0x28 */)));
    // 004ca9b2  d9cc                   -fxch st(4)
    {
        x86::Float tmp = fp5;
        fp5 = fp1;
        fp1 = tmp;
    }
    // 004ca9b4  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 004ca9b6  dec1                   -faddp st(1)
    fp3 = fp::add(fp3, fp4);
    // 004ca9b8  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 004ca9ba  d95918                 -fstp dword ptr [ecx + 0x18]
    app->getMemory<float>(cpu.ecx + x86::reg32(24) /* 0x18 */) = float(fp3);
    // 004ca9bd  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004ca9bf  dec2                   -faddp st(2)
    fp0 = fp::add(fp0, fp2);
    // 004ca9c1  d95920                 -fstp dword ptr [ecx + 0x20]
    app->getMemory<float>(cpu.ecx + x86::reg32(32) /* 0x20 */) = float(fp1);
    // 004ca9c4  d9591c                 -fstp dword ptr [ecx + 0x1c]
    app->getMemory<float>(cpu.ecx + x86::reg32(28) /* 0x1c */) = float(fp0);
    // 004ca9c7  d94340                 -fld dword ptr [ebx + 0x40]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(64) /* 0x40 */));
    // 004ca9ca  d84b48                 -fmul dword ptr [ebx + 0x48]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(72) /* 0x48 */)));
    // 004ca9cd  d95ddc                 -fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp0);
    // 004ca9d0  e85b080200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca9d5  d84ddc                 -fmul dword ptr [ebp - 0x24]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */)));
    // 004ca9d8  d94340                 -fld dword ptr [ebx + 0x40]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(64) /* 0x40 */));
    // 004ca9db  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004ca9dd  d95ddc                 -fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp1);
    // 004ca9e0  d845dc                 -fadd dword ptr [ebp - 0x24]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */)));
    // 004ca9e3  d95124                 -fst dword ptr [ecx + 0x24]
    app->getMemory<float>(cpu.ecx + x86::reg32(36) /* 0x24 */) = float(fp0);
    // 004ca9e6  d95928                 -fstp dword ptr [ecx + 0x28]
    app->getMemory<float>(cpu.ecx + x86::reg32(40) /* 0x28 */) = float(fp0);
    // 004ca9e9  d94344                 -fld dword ptr [ebx + 0x44]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(68) /* 0x44 */));
    // 004ca9ec  d84d28                 -fmul dword ptr [ebp + 0x28]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(40) /* 0x28 */)));
    // 004ca9ef  d9592c                 -fstp dword ptr [ecx + 0x2c]
    app->getMemory<float>(cpu.ecx + x86::reg32(44) /* 0x2c */) = float(fp0);
    // 004ca9f2  e839080200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ca9f7  d95930                 -fstp dword ptr [ecx + 0x30]
    app->getMemory<float>(cpu.ecx + x86::reg32(48) /* 0x30 */) = float(fp0);
    // 004ca9fa  d94330                 -fld dword ptr [ebx + 0x30]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(48) /* 0x30 */));
    // 004ca9fd  d84d28                 -fmul dword ptr [ebp + 0x28]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(40) /* 0x28 */)));
    // 004caa00  d95934                 -fstp dword ptr [ecx + 0x34]
    app->getMemory<float>(cpu.ecx + x86::reg32(52) /* 0x34 */) = float(fp0);
    // 004caa03  ff7130                 -push dword ptr [ecx + 0x30]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(48) /* 0x30 */);
    cpu.esp -= 4;
    // 004caa06  e845ff0100             -call 0x4ea950
    cpu.esp -= 4;
    sub_4ea950(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004caa0b  ff7130                 -push dword ptr [ecx + 0x30]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(48) /* 0x30 */);
    cpu.esp -= 4;
    // 004caa0e  d95938                 -fstp dword ptr [ecx + 0x38]
    app->getMemory<float>(cpu.ecx + x86::reg32(56) /* 0x38 */) = float(fp0);
    // 004caa11  e81aff0100             -call 0x4ea930
    cpu.esp -= 4;
    sub_4ea930(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004caa16  d9593c                 -fstp dword ptr [ecx + 0x3c]
    app->getMemory<float>(cpu.ecx + x86::reg32(60) /* 0x3c */) = float(fp0);
    // 004caa19  e812080200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004caa1e  d95940                 -fstp dword ptr [ecx + 0x40]
    app->getMemory<float>(cpu.ecx + x86::reg32(64) /* 0x40 */) = float(fp0);
    // 004caa21  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004caa24  8b5320                 -mov edx, dword ptr [ebx + 0x20]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(32) /* 0x20 */);
    // 004caa27  895144                 -mov dword ptr [ecx + 0x44], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(68) /* 0x44 */) = cpu.edx;
    // 004caa2a  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 004caa31  d94324                 -fld dword ptr [ebx + 0x24]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(36) /* 0x24 */));
    // 004caa34  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004caa36  d95948                 -fstp dword ptr [ecx + 0x48]
    app->getMemory<float>(cpu.ecx + x86::reg32(72) /* 0x48 */) = float(fp0);
    // 004caa39  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004caa3c  d94328                 -fld dword ptr [ebx + 0x28]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(40) /* 0x28 */));
    // 004caa3f  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004caa41  b868dd7d00             -mov eax, 0x7ddd68
    cpu.eax = 8248680 /*0x7ddd68*/;
    // 004caa46  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004caa49  c6417e00               -mov byte ptr [ecx + 0x7e], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(126) /* 0x7e */) = 0 /*0x0*/;
    // 004caa4d  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004caa4f  894150                 -mov dword ptr [ecx + 0x50], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(80) /* 0x50 */) = cpu.eax;
    // 004caa52  d9594c                 -fstp dword ptr [ecx + 0x4c]
    app->getMemory<float>(cpu.ecx + x86::reg32(76) /* 0x4c */) = float(fp0);
    // 004caa55  8b434c                 -mov eax, dword ptr [ebx + 0x4c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(76) /* 0x4c */);
    // 004caa58  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004caa5a  894154                 -mov dword ptr [ecx + 0x54], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(84) /* 0x54 */) = cpu.eax;
    // 004caa5d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004caa5f  8a5357                 -mov dl, byte ptr [ebx + 0x57]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(87) /* 0x57 */);
    // 004caa62  8a4356                 -mov al, byte ptr [ebx + 0x56]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(86) /* 0x56 */);
    // 004caa65  09c2                   -or edx, eax
    cpu.edx |= x86::reg32(x86::sreg32(cpu.eax));
    // 004caa67  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004caa69  8a4355                 -mov al, byte ptr [ebx + 0x55]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(85) /* 0x55 */);
    // 004caa6c  09d0                   -or eax, edx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.edx));
    // 004caa6e  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004caa70  8a5354                 -mov dl, byte ptr [ebx + 0x54]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(84) /* 0x54 */);
    // 004caa73  09d0                   +or eax, edx
    cpu.clear_co();
    cpu.set_szp((cpu.eax |= x86::reg32(x86::sreg32(cpu.edx))));
    // 004caa75  0f848a000000           -je 0x4cab05
    if (cpu.flags.zf)
    {
        goto L_0x004cab05;
    }
    // 004caa7b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004caa7d  8a4357                 -mov al, byte ptr [ebx + 0x57]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(87) /* 0x57 */);
    // 004caa80  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004caa83  df45fc                 -fild word ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004caa86  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 004caa89  e8a2070200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004caa8e  d84df8                 -fmul dword ptr [ebp - 8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004caa91  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004caa94  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004caa97  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004caa98  284157                 -sub byte ptr [ecx + 0x57], al
    (app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(87) /* 0x57 */)) -= x86::reg8(x86::sreg8(cpu.al));
    // 004caa9b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004caa9d  8a4356                 -mov al, byte ptr [ebx + 0x56]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(86) /* 0x56 */);
    // 004caaa0  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004caaa3  df45fc                 -fild word ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004caaa6  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 004caaa9  e882070200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004caaae  d84df8                 -fmul dword ptr [ebp - 8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004caab1  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004caab4  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004caab7  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004caab8  8a5156                 -mov dl, byte ptr [ecx + 0x56]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(86) /* 0x56 */);
    // 004caabb  28c2                   -sub dl, al
    (cpu.dl) -= x86::reg8(x86::sreg8(cpu.al));
    // 004caabd  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004caabf  885156                 -mov byte ptr [ecx + 0x56], dl
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(86) /* 0x56 */) = cpu.dl;
    // 004caac2  8a4355                 -mov al, byte ptr [ebx + 0x55]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(85) /* 0x55 */);
    // 004caac5  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004caac8  df45fc                 -fild word ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004caacb  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 004caace  e85d070200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004caad3  d84df8                 -fmul dword ptr [ebp - 8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004caad6  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004caad9  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004caadc  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004caadd  8a7155                 -mov dh, byte ptr [ecx + 0x55]
    cpu.dh = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(85) /* 0x55 */);
    // 004caae0  28c6                   -sub dh, al
    (cpu.dh) -= x86::reg8(x86::sreg8(cpu.al));
    // 004caae2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004caae4  887155                 -mov byte ptr [ecx + 0x55], dh
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(85) /* 0x55 */) = cpu.dh;
    // 004caae7  8a4354                 -mov al, byte ptr [ebx + 0x54]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(84) /* 0x54 */);
    // 004caaea  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004caaed  df45fc                 -fild word ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004caaf0  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 004caaf3  e838070200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004caaf8  d84df8                 -fmul dword ptr [ebp - 8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004caafb  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004caafe  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004cab01  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cab02  284154                 -sub byte ptr [ecx + 0x54], al
    (app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(84) /* 0x54 */)) -= x86::reg8(x86::sreg8(cpu.al));
L_0x004cab05:
    // 004cab05  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004cab08  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004cab0b  8a80c00b5600           -mov al, byte ptr [eax + 0x560bc0]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5639104) /* 0x560bc0 */);
    // 004cab11  c6417800               -mov byte ptr [ecx + 0x78], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(120) /* 0x78 */) = 0 /*0x0*/;
    // 004cab15  c6417900               -mov byte ptr [ecx + 0x79], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(121) /* 0x79 */) = 0 /*0x0*/;
    // 004cab19  c6417a00               -mov byte ptr [ecx + 0x7a], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(122) /* 0x7a */) = 0 /*0x0*/;
    // 004cab1d  c6417b00               -mov byte ptr [ecx + 0x7b], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(123) /* 0x7b */) = 0 /*0x0*/;
    // 004cab21  8b5918                 -mov ebx, dword ptr [ecx + 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */);
    // 004cab24  88417c                 -mov byte ptr [ecx + 0x7c], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(124) /* 0x7c */) = cpu.al;
    // 004cab27  f7c3ffffff7f           +test ebx, 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & 2147483647 /*0x7fffffff*/));
    // 004cab2d  7518                   -jne 0x4cab47
    if (!cpu.flags.zf)
    {
        goto L_0x004cab47;
    }
    // 004cab2f  f7411cffffff7f         +test dword ptr [ecx + 0x1c], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */) & 2147483647 /*0x7fffffff*/));
    // 004cab36  750f                   -jne 0x4cab47
    if (!cpu.flags.zf)
    {
        goto L_0x004cab47;
    }
    // 004cab38  f74120ffffff7f         +test dword ptr [ecx + 0x20], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */) & 2147483647 /*0x7fffffff*/));
    // 004cab3f  7506                   -jne 0x4cab47
    if (!cpu.flags.zf)
    {
        goto L_0x004cab47;
    }
    // 004cab41  c70100000000           -mov dword ptr [ecx], 0
    app->getMemory<x86::reg32>(cpu.ecx) = 0 /*0x0*/;
L_0x004cab47:
    // 004cab47  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
L_0x004cab49:
    // 004cab49  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cab4b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cab4c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cab4d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cab4e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cab4f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cab50  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cab51  c23400                 -ret 0x34
    cpu.esp += 4+52 /*0x34*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4cab60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 004cab60  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cab61  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cab62  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cab63  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cab64  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cab65  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cab66  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cab68  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 004cab6b  8d7de8                 -lea edi, [ebp - 0x18]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004cab6e  be90a44c00             -mov esi, 0x4ca490
    cpu.esi = 5022864 /*0x4ca490*/;
    // 004cab73  8b5534                 -mov edx, dword ptr [ebp + 0x34]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(52) /* 0x34 */);
    // 004cab76  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cab77  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cab78  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cab79  f7c2ffffff7f           +test edx, 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 2147483647 /*0x7fffffff*/));
    // 004cab7f  7516                   -jne 0x4cab97
    if (!cpu.flags.zf)
    {
        goto L_0x004cab97;
    }
    // 004cab81  f74538ffffff7f         +test dword ptr [ebp + 0x38], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(56) /* 0x38 */) & 2147483647 /*0x7fffffff*/));
    // 004cab88  750d                   -jne 0x4cab97
    if (!cpu.flags.zf)
    {
        goto L_0x004cab97;
    }
    // 004cab8a  f7453cffffff7f         +test dword ptr [ebp + 0x3c], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(60) /* 0x3c */) & 2147483647 /*0x7fffffff*/));
    // 004cab91  0f8479000000           -je 0x4cac10
    if (cpu.flags.zf)
    {
        goto L_0x004cac10;
    }
L_0x004cab97:
    // 004cab97  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004cab9a  8d4534                 -lea eax, [ebp + 0x34]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(52) /* 0x34 */);
    // 004cab9d  e85ec9fcff             -call 0x497500
    cpu.esp -= 4;
    sub_497500(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004caba2  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004caba4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004caba4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004caba6  8a859a000000           -mov al, byte ptr [ebp + 0x9a]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(154) /* 0x9a */);
    // 004cabac  39c2                   +cmp edx, eax
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
    // 004cabae  7d60                   -jge 0x4cac10
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004cac10;
    }
    // 004cabb0  8b753c                 -mov esi, dword ptr [ebp + 0x3c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(60) /* 0x3c */);
    // 004cabb3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cabb4  8b7d38                 -mov edi, dword ptr [ebp + 0x38]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(56) /* 0x38 */);
    // 004cabb7  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cabb8  8b4534                 -mov eax, dword ptr [ebp + 0x34]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(52) /* 0x34 */);
    // 004cabbb  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cabbc  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004cabbf  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cabc0  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004cabc3  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cabc4  8b75e8                 -mov esi, dword ptr [ebp - 0x18]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004cabc7  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cabc8  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004cabcb  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cabcc  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cabcf  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cabd0  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004cabd3  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cabd4  e857060200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004cabd9  dc0db0035400           -fmul qword ptr [0x5403b0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505968) /* 0x5403b0 */)));
    // 004cabdf  dc05b8035400           -fadd qword ptr [0x5403b8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505976) /* 0x5403b8 */)));
    // 004cabe5  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004cabe8  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 004cabeb  8b5d30                 -mov ebx, dword ptr [ebp + 0x30]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(48) /* 0x30 */);
    // 004cabee  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cabef  8b752c                 -mov esi, dword ptr [ebp + 0x2c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(44) /* 0x2c */);
    // 004cabf2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cabf3  8b7d28                 -mov edi, dword ptr [ebp + 0x28]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(40) /* 0x28 */);
    // 004cabf6  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cabf7  8b4520                 -mov eax, dword ptr [ebp + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */);
    // 004cabfa  e851fcffff             -call 0x4ca850
    cpu.esp -= 4;
    sub_4ca850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cabff  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004cac01  740a                   -je 0x4cac0d
    if (cpu.flags.zf)
    {
        goto L_0x004cac0d;
    }
    // 004cac03  8b08                   -mov ecx, dword ptr [eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax);
    // 004cac05  c6407e00               -mov byte ptr [eax + 0x7e], 0
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(126) /* 0x7e */) = 0 /*0x0*/;
    // 004cac09  d1f9                   +sar ecx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ecx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 004cac0b  8908                   -mov dword ptr [eax], ecx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.ecx;
L_0x004cac0d:
    // 004cac0d  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004cac0e  eb94                   -jmp 0x4caba4
    goto L_0x004caba4;
L_0x004cac10:
    // 004cac10  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cac12  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cac13  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cac14  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cac15  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cac16  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cac17  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cac18  c28000                 -ret 0x80
    cpu.esp += 4+128 /*0x80*/;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4cac20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
  x86::reg16 fsw = 0;
    // 004cac20  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cac21  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cac22  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cac23  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cac24  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cac25  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cac26  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cac28  83ec54                 +sub esp, 0x54
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(84 /*0x54*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004cac2b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004cac2d  8d7dd0                 -lea edi, [ebp - 0x30]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 004cac30  a1e40c5600             -mov eax, dword ptr [0x560ce4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5639396) /* 0x560ce4 */);
    // 004cac35  be9ca44c00             -mov esi, 0x4ca49c
    cpu.esi = 5022876 /*0x4ca49c*/;
    // 004cac3a  d94528                 +fld dword ptr [ebp + 0x28]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(40) /* 0x28 */));
    // 004cac3d  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004cac40  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cac41  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cac42  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cac43  d81dc0035400           +fcomp dword ptr [0x5403c0]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(5505984) /* 0x5403c0 */)));
    // 004cac49  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004cac4b  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004cac4c  7331                   -jae 0x4cac7f
    if (!cpu.flags.cf)
    {
        goto L_0x004cac7f;
    }
    // 004cac4e  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cac51  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004cac54  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004cac57  d84d28                 -fmul dword ptr [ebp + 0x28]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(40) /* 0x28 */)));
    // 004cac5a  d905c0035400           -fld dword ptr [0x5403c0]
    fp1 = x86::Float(app->getMemory<float>(x86::reg32(5505984) /* 0x5403c0 */));
    // 004cac60  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004cac62  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp1);
    // 004cac65  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004cac68  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004cac6b  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cac6c  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004cac6f  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004cac72  d87df4                 -fdivr dword ptr [ebp - 0xc]
    fp0 = fp::div(x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)), fp0);
    // 004cac75  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004cac78  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004cac7b  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cac7c  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
L_0x004cac7f:
    // 004cac7f  8b81a4000000           -mov eax, dword ptr [ecx + 0xa4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(164) /* 0xa4 */);
    // 004cac85  8945c4                 -mov dword ptr [ebp - 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.eax;
    // 004cac88  8b81a8000000           -mov eax, dword ptr [ecx + 0xa8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(168) /* 0xa8 */);
    // 004cac8e  8945c8                 -mov dword ptr [ebp - 0x38], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.eax;
    // 004cac91  8b81ac000000           -mov eax, dword ptr [ecx + 0xac]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(172) /* 0xac */);
    // 004cac97  8945cc                 -mov dword ptr [ebp - 0x34], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.eax;
    // 004cac9a  8b451c                 -mov eax, dword ptr [ebp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004cac9d  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
    // 004caca0  8b4520                 -mov eax, dword ptr [ebp + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */);
    // 004caca3  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
    // 004caca6  8b4524                 -mov eax, dword ptr [ebp + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(36) /* 0x24 */);
    // 004caca9  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
    // 004cacac  8d7de8                 -lea edi, [ebp - 0x18]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004cacaf  8db1a4000000           -lea esi, [ecx + 0xa4]
    cpu.esi = x86::reg32(cpu.ecx + x86::reg32(164) /* 0xa4 */);
    // 004cacb5  8d45e8                 -lea eax, [ebp - 0x18]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004cacb8  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cacb9  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cacba  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cacbb  e830c6fcff             -call 0x4972f0
    cpu.esp -= 4;
    nfs3hp::vertices::normalise(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004cacc0  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004cacc3  8d55dc                 -lea edx, [ebp - 0x24]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004cacc6  8945b8                 -mov dword ptr [ebp - 0x48], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = cpu.eax;
    // 004cacc9  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004caccc  8d9998000000           -lea ebx, [ecx + 0x98]
    cpu.ebx = x86::reg32(cpu.ecx + x86::reg32(152) /* 0x98 */);
    // 004cacd2  8945bc                 -mov dword ptr [ebp - 0x44], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = cpu.eax;
    // 004cacd5  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004cacd8  8d4de8                 -lea ecx, [ebp - 0x18]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004cacdb  8945c0                 -mov dword ptr [ebp - 0x40], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.eax;
    // 004cacde  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004cace3  ddd8                   -fstp st(0)
    fp0 = x86::Float(fp0);
    // 004cace5  e816530100             -call 0x4e0000
    cpu.esp -= 4;
    nfs3hp::vertices::subtractVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cacea  f745e8ffffff7f         +test dword ptr [ebp - 0x18], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) & 2147483647 /*0x7fffffff*/));
    // 004cacf1  7512                   -jne 0x4cad05
    if (!cpu.flags.zf)
    {
        goto L_0x004cad05;
    }
    // 004cacf3  f745ecffffff7f         +test dword ptr [ebp - 0x14], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) & 2147483647 /*0x7fffffff*/));
    // 004cacfa  7509                   -jne 0x4cad05
    if (!cpu.flags.zf)
    {
        goto L_0x004cad05;
    }
    // 004cacfc  f745f0ffffff7f         +test dword ptr [ebp - 0x10], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) & 2147483647 /*0x7fffffff*/));
    // 004cad03  740a                   -je 0x4cad0f
    if (cpu.flags.zf)
    {
        goto L_0x004cad0f;
    }
L_0x004cad05:
    // 004cad05  8d45e8                 -lea eax, [ebp - 0x18]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004cad08  e8e3c5fcff             -call 0x4972f0
    cpu.esp -= 4;
    nfs3hp::vertices::normalise(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004cad0d  ddd8                   -fstp st(0)
    fp0 = x86::Float(fp0);
L_0x004cad0f:
    // 004cad0f  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004cad12  8945d0                 -mov dword ptr [ebp - 0x30], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.eax;
    // 004cad15  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004cad18  8945d4                 -mov dword ptr [ebp - 0x2c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.eax;
    // 004cad1b  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004cad1e  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004cad20  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
L_0x004cad23:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004cad23  3b5df8                 +cmp ebx, dword ptr [ebp - 8]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cad26  0f8dc6000000           -jge 0x4cadf2
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004cadf2;
    }
    // 004cad2c  e8ff040200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004cad31  dc0dc8035400           -fmul qword ptr [0x5403c8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505992) /* 0x5403c8 */)));
    // 004cad37  d845c4                 -fadd dword ptr [ebp - 0x3c]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-60) /* -0x3c */)));
    // 004cad3a  dc05d0035400           -fadd qword ptr [0x5403d0]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(x86::reg32(5506000) /* 0x5403d0 */)));
    // 004cad40  d95dac                 -fstp dword ptr [ebp - 0x54]
    app->getMemory<float>(cpu.ebp + x86::reg32(-84) /* -0x54 */) = float(fp0);
    // 004cad43  e8e8040200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004cad48  dc0dc8035400           -fmul qword ptr [0x5403c8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505992) /* 0x5403c8 */)));
    // 004cad4e  d845c8                 -fadd dword ptr [ebp - 0x38]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-56) /* -0x38 */)));
    // 004cad51  dc05d0035400           -fadd qword ptr [0x5403d0]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(x86::reg32(5506000) /* 0x5403d0 */)));
    // 004cad57  d95db0                 -fstp dword ptr [ebp - 0x50]
    app->getMemory<float>(cpu.ebp + x86::reg32(-80) /* -0x50 */) = float(fp0);
    // 004cad5a  e8d1040200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004cad5f  dc0dc8035400           -fmul qword ptr [0x5403c8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505992) /* 0x5403c8 */)));
    // 004cad65  d845cc                 -fadd dword ptr [ebp - 0x34]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-52) /* -0x34 */)));
    // 004cad68  dc05d0035400           -fadd qword ptr [0x5403d0]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(x86::reg32(5506000) /* 0x5403d0 */)));
    // 004cad6e  d95db4                 -fstp dword ptr [ebp - 0x4c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = float(fp0);
    // 004cad71  8b7db4                 -mov edi, dword ptr [ebp - 0x4c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 004cad74  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cad75  8b45b0                 -mov eax, dword ptr [ebp - 0x50]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 004cad78  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cad79  8b55ac                 -mov edx, dword ptr [ebp - 0x54]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 004cad7c  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cad7d  8b4dd8                 -mov ecx, dword ptr [ebp - 0x28]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 004cad80  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cad81  8b75d4                 -mov esi, dword ptr [ebp - 0x2c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 004cad84  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cad85  8b7dd0                 -mov edi, dword ptr [ebp - 0x30]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 004cad88  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cad89  8b45c0                 -mov eax, dword ptr [ebp - 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004cad8c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cad8d  8b55bc                 -mov edx, dword ptr [ebp - 0x44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 004cad90  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cad91  8b4db8                 -mov ecx, dword ptr [ebp - 0x48]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 004cad94  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cad95  e896040200             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004cad9a  dc0dd8035400           -fmul qword ptr [0x5403d8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5506008) /* 0x5403d8 */)));
    // 004cada0  dc05e0035400           -fadd qword ptr [0x5403e0]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(x86::reg32(5506016) /* 0x5403e0 */)));
    // 004cada6  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004cada9  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 004cadac  8b75e4                 -mov esi, dword ptr [ebp - 0x1c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004cadaf  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cadb0  8b7de0                 -mov edi, dword ptr [ebp - 0x20]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004cadb3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cadb4  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004cadb7  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cadb8  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 004cadbd  e88efaffff             -call 0x4ca850
    cpu.esp -= 4;
    sub_4ca850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cadc2  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004cadc4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004cadc6  7424                   -je 0x4cadec
    if (cpu.flags.zf)
    {
        goto L_0x004cadec;
    }
    // 004cadc8  8b30                   -mov esi, dword ptr [eax]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax);
    // 004cadca  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004cadcc  d1fe                   -sar esi, 1
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (1 /*0x1*/ % 32));
    // 004cadce  e8b55f0100             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cadd3  f7f6                   -div esi
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = cpu.esi;
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 004cadd5  8b31                   -mov esi, dword ptr [ecx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ecx);
    // 004cadd7  29d6                   -sub esi, edx
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004cadd9  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004caddb  8931                   -mov dword ptr [ecx], esi
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.esi;
    // 004caddd  be03000000             -mov esi, 3
    cpu.esi = 3 /*0x3*/;
    // 004cade2  e8a15f0100             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cade7  f7f6                   +div esi
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = cpu.esi;
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 004cade9  88517e                 -mov byte ptr [ecx + 0x7e], dl
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(126) /* 0x7e */) = cpu.dl;
L_0x004cadec:
    // 004cadec  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004caded  e931ffffff             -jmp 0x4cad23
    goto L_0x004cad23;
L_0x004cadf2:
    // 004cadf2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cadf4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cadf5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cadf6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cadf7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cadf8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cadf9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cadfa  c21000                 -ret 0x10
    cpu.esp += 4+16 /*0x10*/;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4cae00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 004cae00  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cae01  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cae02  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cae03  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cae04  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cae06  83ec34                 -sub esp, 0x34
    (cpu.esp) -= x86::reg32(x86::sreg32(52 /*0x34*/));
    // 004cae09  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004cae0b  e8785f0100             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cae10  a801                   +test al, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 1 /*0x1*/));
    // 004cae12  7407                   -je 0x4cae1b
    if (cpu.flags.zf)
    {
        goto L_0x004cae1b;
    }
    // 004cae14  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004cae19  eb05                   -jmp 0x4cae20
    goto L_0x004cae20;
L_0x004cae1b:
    // 004cae1b  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
L_0x004cae20:
    // 004cae20  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004cae22  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004cae24  746b                   -je 0x4cae91
    if (cpu.flags.zf)
    {
        goto L_0x004cae91;
    }
    // 004cae26  83fa0a                 +cmp edx, 0xa
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
    // 004cae29  7509                   -jne 0x4cae34
    if (!cpu.flags.zf)
    {
        goto L_0x004cae34;
    }
    // 004cae2b  f605583a7a0006         +test byte ptr [0x7a3a58], 6
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 6 /*0x6*/));
    // 004cae32  755d                   -jne 0x4cae91
    if (!cpu.flags.zf)
    {
        goto L_0x004cae91;
    }
L_0x004cae34:
    // 004cae34  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004cae36  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cae37  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cae38  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cae39  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004cae3c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cae3d  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
    // 004cae40  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cae41  895df0                 -mov dword ptr [ebp - 0x10], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ebx;
    // 004cae44  895dd8                 -mov dword ptr [ebp - 0x28], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.ebx;
    // 004cae47  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cae48  895dcc                 -mov dword ptr [ebp - 0x34], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.ebx;
    // 004cae4b  895df4                 -mov dword ptr [ebp - 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;
    // 004cae4e  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cae4f  895ddc                 -mov dword ptr [ebp - 0x24], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.ebx;
    // 004cae52  895dd0                 -mov dword ptr [ebp - 0x30], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.ebx;
    // 004cae55  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cae56  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 004cae59  8b4518                 -mov eax, dword ptr [ebp + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 004cae5c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cae5d  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 004cae60  8b451c                 -mov eax, dword ptr [ebp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004cae63  680000803f             -push 0x3f800000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1065353216 /*0x3f800000*/;
    cpu.esp -= 4;
    // 004cae68  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 004cae6b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cae6c  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004cae6f  895de0                 -mov dword ptr [ebp - 0x20], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.ebx;
    // 004cae72  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cae73  895dd4                 -mov dword ptr [ebp - 0x2c], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.ebx;
    // 004cae76  8b5de4                 -mov ebx, dword ptr [ebp - 0x1c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004cae79  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cae7a  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004cae7c  e8cff9ffff             -call 0x4ca850
    cpu.esp -= 4;
    sub_4ca850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cae81  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004cae83  740c                   -je 0x4cae91
    if (cpu.flags.zf)
    {
        goto L_0x004cae91;
    }
    // 004cae85  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 004cae88  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004cae8b  d84834                 -fmul dword ptr [eax + 0x34]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(52) /* 0x34 */)));
    // 004cae8e  d95834                 -fstp dword ptr [eax + 0x34]
    app->getMemory<float>(cpu.eax + x86::reg32(52) /* 0x34 */) = float(fp0);
L_0x004cae91:
    // 004cae91  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cae93  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cae94  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cae95  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cae96  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cae97  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4caea0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3;
    // 004caea0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004caea1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004caea2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caea3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004caea4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004caea5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004caea6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004caea8  83ec44                 -sub esp, 0x44
    (cpu.esp) -= x86::reg32(x86::sreg32(68 /*0x44*/));
    // 004caeab  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004caead  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004caeaf  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caeb0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caeb1  8d7dec                 -lea edi, [ebp - 0x14]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004caeb4  8b451c                 -mov eax, dword ptr [ebp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004caeb7  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caeb8  8db1a4000000           -lea esi, [ecx + 0xa4]
    cpu.esi = x86::reg32(cpu.ecx + x86::reg32(164) /* 0xa4 */);
    // 004caebe  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004caec0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caec1  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
    // 004caec4  8955bc                 -mov dword ptr [ebp - 0x44], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = cpu.edx;
    // 004caec7  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caec8  8955c8                 -mov dword ptr [ebp - 0x38], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.edx;
    // 004caecb  8955d4                 -mov dword ptr [ebp - 0x2c], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.edx;
    // 004caece  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caecf  8955c0                 -mov dword ptr [ebp - 0x40], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.edx;
    // 004caed2  8955cc                 -mov dword ptr [ebp - 0x34], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.edx;
    // 004caed5  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caed6  8955d8                 -mov dword ptr [ebp - 0x28], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.edx;
    // 004caed9  8955c4                 -mov dword ptr [ebp - 0x3c], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.edx;
    // 004caedc  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caedd  8955d0                 -mov dword ptr [ebp - 0x30], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.edx;
    // 004caee0  8b4520                 -mov eax, dword ptr [ebp + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */);
    // 004caee3  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caee4  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
    // 004caee7  8b4524                 -mov eax, dword ptr [ebp + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(36) /* 0x24 */);
    // 004caeea  680000803f             -push 0x3f800000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1065353216 /*0x3f800000*/;
    cpu.esp -= 4;
    // 004caeef  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 004caef2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004caef3  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004caef6  8955dc                 -mov dword ptr [ebp - 0x24], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.edx;
    // 004caef9  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004caefa  8b55e0                 -mov edx, dword ptr [ebp - 0x20]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004caefd  b809000000             -mov eax, 9
    cpu.eax = 9 /*0x9*/;
    // 004caf02  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caf03  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004caf04  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004caf05  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004caf06  e845f9ffff             -call 0x4ca850
    cpu.esp -= 4;
    sub_4ca850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004caf0b  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004caf0c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004caf0d  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004caf0e  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004caf0f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004caf10  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004caf11  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004caf12  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004caf13  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004caf14  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004caf17  680000803f             -push 0x3f800000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1065353216 /*0x3f800000*/;
    cpu.esp -= 4;
    // 004caf1c  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caf1d  8b5de4                 -mov ebx, dword ptr [ebp - 0x1c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004caf20  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004caf21  8b75e0                 -mov esi, dword ptr [ebp - 0x20]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004caf24  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004caf25  b807000000             -mov eax, 7
    cpu.eax = 7 /*0x7*/;
    // 004caf2a  e821f9ffff             -call 0x4ca850
    cpu.esp -= 4;
    sub_4ca850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004caf2f  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004caf34  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004caf36  bb30000000             -mov ebx, 0x30
    cpu.ebx = 48 /*0x30*/;
    // 004caf3b  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004caf3e  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004caf40  83fa08                 +cmp edx, 8
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
    // 004caf43  7d21                   -jge 0x4caf66
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004caf66;
    }
    // 004caf45  e83e5e0100             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004caf4a  a801                   +test al, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 1 /*0x1*/));
    // 004caf4c  7418                   -je 0x4caf66
    if (cpu.flags.zf)
    {
        goto L_0x004caf66;
    }
    // 004caf4e  8b7d24                 -mov edi, dword ptr [ebp + 0x24]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(36) /* 0x24 */);
    // 004caf51  680000a041             -push 0x41a00000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1101004800 /*0x41a00000*/;
    cpu.esp -= 4;
    // 004caf56  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004caf57  8b4520                 -mov eax, dword ptr [ebp + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */);
    // 004caf5a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004caf5b  8b551c                 -mov edx, dword ptr [ebp + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004caf5e  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004caf5f  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004caf61  e8bafcffff             -call 0x4cac20
    cpu.esp -= 4;
    sub_4cac20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004caf66:
    // 004caf66  e81d5e0100             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004caf6b  a807                   +test al, 7
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 7 /*0x7*/));
    // 004caf6d  0f8488000000           -je 0x4caffb
    if (cpu.flags.zf)
    {
        goto L_0x004caffb;
    }
    // 004caf73  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004caf78  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004caf7a  b930000000             -mov ecx, 0x30
    cpu.ecx = 48 /*0x30*/;
    // 004caf7f  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004caf82  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004caf84  d905f0035400           -fld dword ptr [0x5403f0]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5506032) /* 0x5403f0 */));
    // 004caf8a  d945ec                 -fld dword ptr [ebp - 0x14]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */));
    // 004caf8d  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004caf8f  d945f0                 -fld dword ptr [ebp - 0x10]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */));
    // 004caf92  d8ca                   -fmul st(2)
    fp2 = fp::mul(fp2, x86::Float(fp0));
    // 004caf94  d945f4                 -fld dword ptr [ebp - 0xc]
    fp3 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004caf97  decb                   -fmulp st(3)
    fp0 = fp::mul(fp0, fp3);
    // 004caf99  d95dd8                 -fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fp2);
    // 004caf9c  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004caf9e  d95ddc                 -fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp1);
    // 004cafa1  8b4ddc                 -mov ecx, dword ptr [ebp - 0x24]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004cafa4  8b5dd8                 -mov ebx, dword ptr [ebp - 0x28]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 004cafa7  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cafa8  d95dd4                 -fstp dword ptr [ebp - 0x2c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = float(fp0);
    // 004cafab  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cafac  8b75d4                 -mov esi, dword ptr [ebp - 0x2c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 004cafaf  8b7dc4                 -mov edi, dword ptr [ebp - 0x3c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 004cafb2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cafb3  8b45c0                 -mov eax, dword ptr [ebp - 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004cafb6  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004cafb9  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cafba  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004cafbd  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cafbe  8b55bc                 -mov edx, dword ptr [ebp - 0x44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 004cafc1  8b4dd0                 -mov ecx, dword ptr [ebp - 0x30]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 004cafc4  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cafc5  dc0de8035400           -fmul qword ptr [0x5403e8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5506024) /* 0x5403e8 */)));
    // 004cafcb  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cafcc  8b5dcc                 -mov ebx, dword ptr [ebp - 0x34]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 004cafcf  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cafd0  8b75c8                 -mov esi, dword ptr [ebp - 0x38]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 004cafd3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cafd4  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 004cafd7  8b7de8                 -mov edi, dword ptr [ebp - 0x18]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004cafda  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 004cafdd  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cafde  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004cafe1  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cafe2  8b55e0                 -mov edx, dword ptr [ebp - 0x20]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004cafe5  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cafe6  b808000000             -mov eax, 8
    cpu.eax = 8 /*0x8*/;
    // 004cafeb  e860f8ffff             -call 0x4ca850
    cpu.esp -= 4;
    sub_4ca850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004caff0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004caff2  7407                   -je 0x4caffb
    if (cpu.flags.zf)
    {
        goto L_0x004caffb;
    }
    // 004caff4  c7404000000000         -mov dword ptr [eax + 0x40], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = 0 /*0x0*/;
L_0x004caffb:
    // 004caffb  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004caffd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004caffe  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cafff  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb000  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb001  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb002  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb003  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4cb010(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5, fp6;
  x86::reg16 fsw = 0;
    // 004cb010  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cb011  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cb012  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cb013  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cb014  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cb015  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cb017  83ec68                 -sub esp, 0x68
    (cpu.esp) -= x86::reg32(x86::sreg32(104 /*0x68*/));
    // 004cb01a  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004cb01c  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 004cb01e  8b5524                 -mov edx, dword ptr [ebp + 0x24]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(36) /* 0x24 */);
    // 004cb021  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004cb023  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004cb026  be0000803f             -mov esi, 0x3f800000
    cpu.esi = 1065353216 /*0x3f800000*/;
    // 004cb02b  8b80640b5600           -mov eax, dword ptr [eax + 0x560b64]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5639012) /* 0x560b64 */);
    // 004cb031  8975f8                 -mov dword ptr [ebp - 8], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.esi;
    // 004cb034  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 004cb037  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004cb039  0f84d6010000           -je 0x4cb215
    if (cpu.flags.zf)
    {
        goto L_0x004cb215;
    }
    // 004cb03f  83fa0a                 +cmp edx, 0xa
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
    // 004cb042  750d                   -jne 0x4cb051
    if (!cpu.flags.zf)
    {
        goto L_0x004cb051;
    }
    // 004cb044  f605583a7a0006         +test byte ptr [0x7a3a58], 6
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 6 /*0x6*/));
    // 004cb04b  0f85c4010000           -jne 0x4cb215
    if (!cpu.flags.zf)
    {
        goto L_0x004cb215;
    }
L_0x004cb051:
    // 004cb051  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004cb053  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004cb056  f780700b5600ffffff7f   +test dword ptr [eax + 0x560b70], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5639024) /* 0x560b70 */) & 2147483647 /*0x7fffffff*/));
    // 004cb060  0f848a000000           -je 0x4cb0f0
    if (cpu.flags.zf)
    {
        goto L_0x004cb0f0;
    }
    // 004cb066  d9ee                   +fldz 
    fp0 = 0.0;
    // 004cb068  d899b8000000           +fcomp dword ptr [ecx + 0xb8]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(184) /* 0xb8 */)));
    // 004cb06e  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004cb070  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004cb071  730b                   -jae 0x4cb07e
    if (!cpu.flags.cf)
    {
        goto L_0x004cb07e;
    }
    // 004cb073  8b81b8000000           -mov eax, dword ptr [ecx + 0xb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(184) /* 0xb8 */);
    // 004cb079  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 004cb07c  eb0b                   -jmp 0x4cb089
    goto L_0x004cb089;
L_0x004cb07e:
    // 004cb07e  d981b8000000           -fld dword ptr [ecx + 0xb8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(184) /* 0xb8 */));
    // 004cb084  d9e0                   -fchs 
    fp0 = -fp0;
    // 004cb086  d95df0                 -fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp0);
L_0x004cb089:
    // 004cb089  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004cb08b  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004cb08e  d945f0                 -fld dword ptr [ebp - 0x10]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */));
    // 004cb091  d8b0700b5600           -fdiv dword ptr [eax + 0x560b70]
    fp0 = fp::div(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(5639024) /* 0x560b70 */)));
    // 004cb097  d95dfc                 -fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 004cb09a  83fa0c                 +cmp edx, 0xc
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(12 /*0xc*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cb09d  741e                   -je 0x4cb0bd
    if (cpu.flags.zf)
    {
        goto L_0x004cb0bd;
    }
    // 004cb09f  83b99405000000         +cmp dword ptr [ecx + 0x594], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(1428) /* 0x594 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cb0a6  7415                   -je 0x4cb0bd
    if (cpu.flags.zf)
    {
        goto L_0x004cb0bd;
    }
    // 004cb0a8  83fb01                 +cmp ebx, 1
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
    // 004cb0ab  7e10                   -jle 0x4cb0bd
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004cb0bd;
    }
    // 004cb0ad  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004cb0b0  d9c0                   +fld st(0)
    fp1 = x86::Float(fp0);
    // 004cb0b2  dc05f8035400           +fadd qword ptr [0x5403f8]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<double>(x86::reg32(5506040) /* 0x5403f8 */)));
    // 004cb0b8  ddd9                   +fstp st(1)
    fp0 = x86::Float(fp1);
    // 004cb0ba  d95dfc                 +fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
L_0x004cb0bd:
    // 004cb0bd  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004cb0c0  d9e8                   +fld1 
    fp1 = 1.0;
    // 004cb0c2  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004cb0c4  dd5de4                 +fstp qword ptr [ebp - 0x1c]
    app->getMemory<double>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = double(fp1);
    // 004cb0c7  dc5de4                 +fcomp qword ptr [ebp - 0x1c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-28) /* -0x1c */)));
    // 004cb0ca  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004cb0cc  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004cb0cd  760e                   -jbe 0x4cb0dd
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004cb0dd;
    }
    // 004cb0cf  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004cb0d2  8945d4                 -mov dword ptr [ebp - 0x2c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.eax;
    // 004cb0d5  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004cb0d8  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
    // 004cb0db  eb0d                   -jmp 0x4cb0ea
    goto L_0x004cb0ea;
L_0x004cb0dd:
    // 004cb0dd  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004cb0df  bf0000f03f             -mov edi, 0x3ff00000
    cpu.edi = 1072693248 /*0x3ff00000*/;
    // 004cb0e4  8975d4                 -mov dword ptr [ebp - 0x2c], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.esi;
    // 004cb0e7  897dd8                 -mov dword ptr [ebp - 0x28], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.edi;
L_0x004cb0ea:
    // 004cb0ea  dd45d4                 -fld qword ptr [ebp - 0x2c]
    fp0 = x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-44) /* -0x2c */));
    // 004cb0ed  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
L_0x004cb0f0:
    // 004cb0f0  dd0500045400           -fld qword ptr [0x540400]
    fp0 = x86::Float(app->getMemory<double>(x86::reg32(5506048) /* 0x540400 */));
    // 004cb0f6  db4518                 -fild dword ptr [ebp + 0x18]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */)));
    // 004cb0f9  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004cb0fb  db451c                 -fild dword ptr [ebp + 0x1c]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */)));
    // 004cb0fe  d8ca                   -fmul st(2)
    fp2 = fp::mul(fp2, x86::Float(fp0));
    // 004cb100  db4520                 -fild dword ptr [ebp + 0x20]
    fp3 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */)));
    // 004cb103  decb                   -fmulp st(3)
    fp0 = fp::mul(fp0, fp3);
    // 004cb105  d90508045400           -fld dword ptr [0x540408]
    fp3 = x86::Float(app->getMemory<float>(x86::reg32(5506056) /* 0x540408 */));
    // 004cb10b  d981a4000000           -fld dword ptr [ecx + 0xa4]
    fp4 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(164) /* 0xa4 */));
    // 004cb111  d8c9                   -fmul st(1)
    fp4 = fp::mul(fp4, x86::Float(fp3));
    // 004cb113  d981a8000000           -fld dword ptr [ecx + 0xa8]
    fp5 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(168) /* 0xa8 */));
    // 004cb119  d8ca                   -fmul st(2)
    fp5 = fp::mul(fp5, x86::Float(fp3));
    // 004cb11b  d981ac000000           -fld dword ptr [ecx + 0xac]
    fp6 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(172) /* 0xac */));
    // 004cb121  decb                   -fmulp st(3)
    fp3 = fp::mul(fp3, fp6);
    // 004cb123  8d7dc8                 -lea edi, [ebp - 0x38]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 004cb126  8db1a4000000           -lea esi, [ecx + 0xa4]
    cpu.esi = x86::reg32(cpu.ecx + x86::reg32(164) /* 0xa4 */);
    // 004cb12c  8d45c8                 -lea eax, [ebp - 0x38]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 004cb12f  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cb130  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cb131  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cb132  d9cc                   -fxch st(4)
    {
        x86::Float tmp = fp5;
        fp5 = fp1;
        fp1 = tmp;
    }
    // 004cb134  d95d98                 -fstp dword ptr [ebp - 0x68]
    app->getMemory<float>(cpu.ebp + x86::reg32(-104) /* -0x68 */) = float(fp5);
    // 004cb137  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp4;
        fp4 = fp2;
        fp2 = tmp;
    }
    // 004cb139  d95d9c                 -fstp dword ptr [ebp - 0x64]
    app->getMemory<float>(cpu.ebp + x86::reg32(-100) /* -0x64 */) = float(fp4);
    // 004cb13c  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 004cb13e  d95da0                 -fstp dword ptr [ebp - 0x60]
    app->getMemory<float>(cpu.ebp + x86::reg32(-96) /* -0x60 */) = float(fp3);
    // 004cb141  d95da4                 -fstp dword ptr [ebp - 0x5c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-92) /* -0x5c */) = float(fp2);
    // 004cb144  d95da8                 -fstp dword ptr [ebp - 0x58]
    app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = float(fp1);
    // 004cb147  d95dac                 -fstp dword ptr [ebp - 0x54]
    app->getMemory<float>(cpu.ebp + x86::reg32(-84) /* -0x54 */) = float(fp0);
    // 004cb14a  e8a1c1fcff             -call 0x4972f0
    cpu.esp -= 4;
    nfs3hp::vertices::normalise(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004cb14f  8b45c8                 -mov eax, dword ptr [ebp - 0x38]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 004cb152  8945bc                 -mov dword ptr [ebp - 0x44], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = cpu.eax;
    // 004cb155  8b45cc                 -mov eax, dword ptr [ebp - 0x34]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 004cb158  8945c0                 -mov dword ptr [ebp - 0x40], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.eax;
    // 004cb15b  8b45d0                 -mov eax, dword ptr [ebp - 0x30]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 004cb15e  ddd8                   -fstp st(0)
    fp0 = x86::Float(fp0);
    // 004cb160  8945c4                 -mov dword ptr [ebp - 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.eax;
    // 004cb163  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004cb165  7405                   -je 0x4cb16c
    if (cpu.flags.zf)
    {
        goto L_0x004cb16c;
    }
    // 004cb167  83fb03                 +cmp ebx, 3
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
    // 004cb16a  7535                   -jne 0x4cb1a1
    if (!cpu.flags.zf)
    {
        goto L_0x004cb1a1;
    }
L_0x004cb16c:
    // 004cb16c  d981c0000000           +fld dword ptr [ecx + 0xc0]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(192) /* 0xc0 */));
    // 004cb172  d981c4000000           +fld dword ptr [ecx + 0xc4]
    fp1 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(196) /* 0xc4 */));
    // 004cb178  d981c8000000           +fld dword ptr [ecx + 0xc8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(200) /* 0xc8 */));
    // 004cb17e  b8000080bf             -mov eax, 0xbf800000
    cpu.eax = 3212836864 /*0xbf800000*/;
    // 004cb183  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004cb185  d9e0                   +fchs 
    fp2 = -fp2;
    // 004cb187  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004cb189  d9e0                   +fchs 
    fp2 = -fp2;
    // 004cb18b  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004cb18d  d9e0                   +fchs 
    fp2 = -fp2;
    // 004cb18f  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004cb191  d95db4                 +fstp dword ptr [ebp - 0x4c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = float(fp2);
    // 004cb194  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004cb196  d95db8                 +fstp dword ptr [ebp - 0x48]
    app->getMemory<float>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = float(fp1);
    // 004cb199  d95db0                 +fstp dword ptr [ebp - 0x50]
    app->getMemory<float>(cpu.ebp + x86::reg32(-80) /* -0x50 */) = float(fp0);
    // 004cb19c  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004cb19f  eb23                   -jmp 0x4cb1c4
    goto L_0x004cb1c4;
L_0x004cb1a1:
    // 004cb1a1  8b81c0000000           -mov eax, dword ptr [ecx + 0xc0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(192) /* 0xc0 */);
    // 004cb1a7  8945b0                 -mov dword ptr [ebp - 0x50], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */) = cpu.eax;
    // 004cb1aa  8b81c4000000           -mov eax, dword ptr [ecx + 0xc4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(196) /* 0xc4 */);
    // 004cb1b0  8945b4                 -mov dword ptr [ebp - 0x4c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = cpu.eax;
    // 004cb1b3  8b81c8000000           -mov eax, dword ptr [ecx + 0xc8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(200) /* 0xc8 */);
    // 004cb1b9  b90000803f             -mov ecx, 0x3f800000
    cpu.ecx = 1065353216 /*0x3f800000*/;
    // 004cb1be  8945b8                 -mov dword ptr [ebp - 0x48], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = cpu.eax;
    // 004cb1c1  894df4                 -mov dword ptr [ebp - 0xc], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ecx;
L_0x004cb1c4:
    // 004cb1c4  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x004cb1c6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004cb1c6  3b4dec                 +cmp ecx, dword ptr [ebp - 0x14]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cb1c9  7d4a                   -jge 0x4cb215
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004cb215;
    }
    // 004cb1cb  8b75ac                 -mov esi, dword ptr [ebp - 0x54]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 004cb1ce  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cb1cf  8b7da8                 -mov edi, dword ptr [ebp - 0x58]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */);
    // 004cb1d2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cb1d3  8b45a4                 -mov eax, dword ptr [ebp - 0x5c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 004cb1d6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cb1d7  8b5db8                 -mov ebx, dword ptr [ebp - 0x48]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 004cb1da  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cb1db  8b75b4                 -mov esi, dword ptr [ebp - 0x4c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 004cb1de  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cb1df  8b7db0                 -mov edi, dword ptr [ebp - 0x50]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 004cb1e2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cb1e3  8b45c4                 -mov eax, dword ptr [ebp - 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 004cb1e6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cb1e7  8b5dc0                 -mov ebx, dword ptr [ebp - 0x40]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004cb1ea  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cb1eb  8b75bc                 -mov esi, dword ptr [ebp - 0x44]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 004cb1ee  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cb1ef  8b7da0                 -mov edi, dword ptr [ebp - 0x60]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-96) /* -0x60 */);
    // 004cb1f2  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 004cb1f5  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cb1f6  8b459c                 -mov eax, dword ptr [ebp - 0x64]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-100) /* -0x64 */);
    // 004cb1f9  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cb1fa  8b5d98                 -mov ebx, dword ptr [ebp - 0x68]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 004cb1fd  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cb1fe  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004cb200  e84bf6ffff             -call 0x4ca850
    cpu.esp -= 4;
    sub_4ca850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cb205  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004cb207  7409                   -je 0x4cb212
    if (cpu.flags.zf)
    {
        goto L_0x004cb212;
    }
    // 004cb209  d94034                 +fld dword ptr [eax + 0x34]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(52) /* 0x34 */));
    // 004cb20c  d84df4                 +fmul dword ptr [ebp - 0xc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 004cb20f  d95834                 +fstp dword ptr [eax + 0x34]
    app->getMemory<float>(cpu.eax + x86::reg32(52) /* 0x34 */) = float(fp0);
L_0x004cb212:
    // 004cb212  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004cb213  ebb1                   -jmp 0x4cb1c6
    goto L_0x004cb1c6;
L_0x004cb215:
    // 004cb215  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cb217  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb218  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb219  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb21a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb21b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb21c  c21000                 -ret 0x10
    cpu.esp += 4+16 /*0x10*/;
    return;
}

/* align: skip 0x90 */
void Application::sub_4cb220(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cb220  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cb221  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cb222  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cb223  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cb224  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cb225  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cb226  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cb228  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004cb22b  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004cb22d  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004cb22f  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004cb231  891485d06f7f00         -mov dword ptr [eax*4 + 0x7f6fd0], edx
    app->getMemory<x86::reg32>(x86::reg32(8351696) /* 0x7f6fd0 */ + cpu.eax * 4) = cpu.edx;
    // 004cb238  eb08                   -jmp 0x4cb242
    goto L_0x004cb242;
L_0x004cb23a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004cb23a  81fbc8000000           +cmp ebx, 0xc8
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(200 /*0xc8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cb240  7d6d                   -jge 0x4cb2af
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004cb2af;
    }
L_0x004cb242:
    // 004cb242  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004cb249  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004cb24b  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004cb24e  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004cb250  b9d00b7f00             -mov ecx, 0x7f0bd0
    cpu.ecx = 8326096 /*0x7f0bd0*/;
    // 004cb255  c1e009                 -shl eax, 9
    cpu.eax <<= 9 /*0x9*/ % 32;
    // 004cb258  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004cb25a  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004cb25c  c1e006                 -shl eax, 6
    cpu.eax <<= 6 /*0x6*/ % 32;
    // 004cb25f  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004cb261  8b39                   -mov edi, dword ptr [ecx]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx);
    // 004cb263  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004cb265  7445                   -je 0x4cb2ac
    if (cpu.flags.zf)
    {
        goto L_0x004cb2ac;
    }
    // 004cb267  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 004cb26a  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004cb26d  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004cb26f  8bb8cc0b5600           -mov edi, dword ptr [eax + 0x560bcc]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5639116) /* 0x560bcc */);
    // 004cb275  8a90df0b5600           -mov dl, byte ptr [eax + 0x560bdf]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5639135) /* 0x560bdf */);
    // 004cb27b  8d4128                 -lea eax, [ecx + 0x28]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(40) /* 0x28 */);
    // 004cb27e  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004cb281  ff04b5d06f7f00         -inc dword ptr [esi*4 + 0x7f6fd0]
    (app->getMemory<x86::reg32>(x86::reg32(8351696) /* 0x7f6fd0 */ + cpu.esi * 4))++;
    // 004cb288  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 004cb28a  0fafd0                 -imul edx, eax
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 004cb28d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004cb28f  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004cb292  f7ff                   -idiv edi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.edi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004cb294  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004cb297  884203                 -mov byte ptr [edx + 3], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(3) /* 0x3 */) = cpu.al;
    // 004cb29a  8b11                   -mov edx, dword ptr [ecx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx);
    // 004cb29c  4a                     -dec edx
    (cpu.edx)--;
    // 004cb29d  8a612b                 -mov ah, byte ptr [ecx + 0x2b]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(43) /* 0x2b */);
    // 004cb2a0  8911                   -mov dword ptr [ecx], edx
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.edx;
    // 004cb2a2  84e4                   +test ah, ah
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & cpu.ah));
    // 004cb2a4  7506                   -jne 0x4cb2ac
    if (!cpu.flags.zf)
    {
        goto L_0x004cb2ac;
    }
    // 004cb2a6  c70100000000           -mov dword ptr [ecx], 0
    app->getMemory<x86::reg32>(cpu.ecx) = 0 /*0x0*/;
L_0x004cb2ac:
    // 004cb2ac  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004cb2ad  eb8b                   -jmp 0x4cb23a
    goto L_0x004cb23a;
L_0x004cb2af:
    // 004cb2af  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cb2b1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb2b2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb2b3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb2b4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb2b5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb2b6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb2b7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4cb2c0: native, nfs3hp::vertices::moveParticles */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4cb590(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cb590  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cb591  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cb592  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cb593  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cb594  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cb595  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cb596  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cb598  81ec00010000           -sub esp, 0x100
    (cpu.esp) -= x86::reg32(x86::sreg32(256 /*0x100*/));
    // 004cb59e  81ed7e010000           -sub ebp, 0x17e
    (cpu.ebp) -= x86::reg32(x86::sreg32(382 /*0x17e*/));
    // 004cb5a4  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004cb5a6  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 004cb5a8  e98a000000             -jmp 0x4cb637
    goto L_0x004cb637;
L_0x004cb5ad:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004cb5ad  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cb5ae  6830045400             -push 0x540430
    app->getMemory<x86::reg32>(cpu.esp-4) = 5506096 /*0x540430*/;
    cpu.esp -= 4;
L_0x004cb5b3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004cb5b3  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004cb5b6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cb5b7  e8d4400100             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cb5bc  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004cb5bf  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004cb5c6  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004cb5c8  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004cb5ca  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004cb5cd  b968dd7d00             -mov ecx, 0x7ddd68
    cpu.ecx = 8248680 /*0x7ddd68*/;
    // 004cb5d2  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004cb5d4  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 004cb5d9  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004cb5dc  8d557e                 -lea edx, [ebp + 0x7e]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004cb5df  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004cb5e1  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004cb5e3  e8988a0000             -call 0x4d4080
    cpu.esp -= 4;
    sub_4d4080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004cb5e8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004cb5e8  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004cb5ea  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004cb5ed  83b8c80b560000         +cmp dword ptr [eax + 0x560bc8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5639112) /* 0x560bc8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cb5f4  743b                   -je 0x4cb631
    if (cpu.flags.zf)
    {
        goto L_0x004cb631;
    }
    // 004cb5f6  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cb5f7  6838045400             -push 0x540438
    app->getMemory<x86::reg32>(cpu.esp-4) = 5506104 /*0x540438*/;
    cpu.esp -= 4;
    // 004cb5fc  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004cb5ff  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cb600  e88b400100             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cb605  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004cb60c  b900db7d00             -mov ecx, 0x7ddb00
    cpu.ecx = 8248064 /*0x7ddb00*/;
    // 004cb611  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004cb613  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 004cb618  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004cb61b  8d557e                 -lea edx, [ebp + 0x7e]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004cb61e  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004cb620  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004cb623  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004cb626  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004cb628  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004cb62a  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004cb62c  e84f8a0000             -call 0x4d4080
    cpu.esp -= 4;
    sub_4d4080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004cb631:
    // 004cb631  46                     -inc esi
    (cpu.esi)++;
    // 004cb632  83fe0e                 +cmp esi, 0xe
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(14 /*0xe*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cb635  7d2b                   -jge 0x4cb662
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004cb662;
    }
L_0x004cb637:
    // 004cb637  e8947dfdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cb63c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004cb63e  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004cb641  83b8640b560000         +cmp dword ptr [eax + 0x560b64], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5639012) /* 0x560b64 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cb648  749e                   -je 0x4cb5e8
    if (cpu.flags.zf)
    {
        goto L_0x004cb5e8;
    }
    // 004cb64a  833dc8d46f0000         +cmp dword ptr [0x6fd4c8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cb651  0f8456ffffff           -je 0x4cb5ad
    if (cpu.flags.zf)
    {
        goto L_0x004cb5ad;
    }
    // 004cb657  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cb658  6828045400             -push 0x540428
    app->getMemory<x86::reg32>(cpu.esp-4) = 5506088 /*0x540428*/;
    cpu.esp -= 4;
    // 004cb65d  e951ffffff             -jmp 0x4cb5b3
    goto L_0x004cb5b3;
L_0x004cb662:
    // 004cb662  8da57e010000           -lea esp, [ebp + 0x17e]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(382) /* 0x17e */);
    // 004cb668  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb669  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb66a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb66b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb66c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb66d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb66e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4cb670: native, nfs3hp::render::clearBuffers */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4cb6f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cb6f0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cb6f1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cb6f2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cb6f4  ba002c0100             -mov edx, 0x12c00
    cpu.edx = 76800 /*0x12c00*/;
    // 004cb6f9  b8d0df7d00             -mov eax, 0x7ddfd0
    cpu.eax = 8249296 /*0x7ddfd0*/;
    // 004cb6fe  e809500100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cb703  ba00640000             -mov edx, 0x6400
    cpu.edx = 25600 /*0x6400*/;
    // 004cb708  b8d00b7f00             -mov eax, 0x7f0bd0
    cpu.eax = 8326096 /*0x7f0bd0*/;
    // 004cb70d  e8fa4f0100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cb712  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004cb714  8915d06f7f00           -mov dword ptr [0x7f6fd0], edx
    app->getMemory<x86::reg32>(x86::reg32(8351696) /* 0x7f6fd0 */) = cpu.edx;
    // 004cb71a  8915d46f7f00           -mov dword ptr [0x7f6fd4], edx
    app->getMemory<x86::reg32>(x86::reg32(8351700) /* 0x7f6fd4 */) = cpu.edx;
    // 004cb720  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb721  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb722  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4cb730(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cb730  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cb731  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cb732  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cb734  ba002c0100             -mov edx, 0x12c00
    cpu.edx = 76800 /*0x12c00*/;
    // 004cb739  b8d0df7d00             -mov eax, 0x7ddfd0
    cpu.eax = 8249296 /*0x7ddfd0*/;
    // 004cb73e  e8c94f0100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cb743  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb744  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cb745  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4cb750: native, nfs3hp::vertices::drops */

/* sub_4cbc90: native, nfs3hp::vertices::particles */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4cc540(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cc540  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cc541  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cc542  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cc543  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cc544  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cc545  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cc547  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004cc549  e8b275f5ff             -call 0x423b00
    cpu.esp -= 4;
    nfs3hp::carstate::cameraMode(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc54e  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004cc550  83f804                 +cmp eax, 4
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
    // 004cc553  7f15                   -jg 0x4cc56a
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004cc56a;
    }
    // 004cc555  83f802                 +cmp eax, 2
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
    // 004cc558  7c10                   -jl 0x4cc56a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004cc56a;
    }
    // 004cc55a  833c95a0c4790000       +cmp dword ptr [edx*4 + 0x79c4a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7980192) /* 0x79c4a0 */ + cpu.edx * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cc562  7406                   -je 0x4cc56a
    if (cpu.flags.zf)
    {
        goto L_0x004cc56a;
    }
    // 004cc564  81e1000000ff           -and ecx, 0xff000000
    cpu.ecx &= x86::reg32(x86::sreg32(4278190080 /*0xff000000*/));
L_0x004cc56a:
    // 004cc56a  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004cc571  3b88d86f7f00           +cmp ecx, dword ptr [eax + 0x7f6fd8]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8351704) /* 0x7f6fd8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cc577  7509                   -jne 0x4cc582
    if (!cpu.flags.zf)
    {
        goto L_0x004cc582;
    }
    // 004cc579  83b8f42f550000         +cmp dword ptr [eax + 0x552ff4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5582836) /* 0x552ff4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cc580  7423                   -je 0x4cc5a5
    if (cpu.flags.zf)
    {
        goto L_0x004cc5a5;
    }
L_0x004cc582:
    // 004cc582  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004cc584  eb07                   -jmp 0x4cc58d
    goto L_0x004cc58d;
L_0x004cc586:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004cc586  3d58020000             +cmp eax, 0x258
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(600 /*0x258*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cc58b  7d18                   -jge 0x4cc5a5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004cc5a5;
    }
L_0x004cc58d:
    // 004cc58d  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004cc58f  c1e607                 -shl esi, 7
    cpu.esi <<= 7 /*0x7*/ % 32;
    // 004cc592  30db                   +xor bl, bl
    cpu.clear_co();
    cpu.set_szp((cpu.bl ^= x86::reg8(x86::sreg8(cpu.bl))));
    // 004cc594  889c5648e07d00         -mov byte ptr [esi + edx*2 + 0x7de048], bl
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(8249416) /* 0x7de048 */ + cpu.edx * 2) = cpu.bl;
    // 004cc59b  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004cc59c  889c5649e07d00         -mov byte ptr [esi + edx*2 + 0x7de049], bl
    app->getMemory<x86::reg8>(cpu.esi + x86::reg32(8249417) /* 0x7de049 */ + cpu.edx * 2) = cpu.bl;
    // 004cc5a3  ebe1                   -jmp 0x4cc586
    goto L_0x004cc586;
L_0x004cc5a5:
    // 004cc5a5  890c95d86f7f00         -mov dword ptr [edx*4 + 0x7f6fd8], ecx
    app->getMemory<x86::reg32>(x86::reg32(8351704) /* 0x7f6fd8 */ + cpu.edx * 4) = cpu.ecx;
    // 004cc5ac  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc5ad  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc5ae  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc5af  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc5b0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc5b1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4cc5c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cc5c0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cc5c1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cc5c2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cc5c3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cc5c4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cc5c6  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004cc5c9  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004cc5cb  833d30bc6f0002         +cmp dword ptr [0x6fbc30], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7322672) /* 0x6fbc30 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cc5d2  7423                   -je 0x4cc5f7
    if (cpu.flags.zf)
    {
        goto L_0x004cc5f7;
    }
    // 004cc5d4  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cc5d7  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004cc5da  8d5144                 -lea edx, [ecx + 0x44]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(68) /* 0x44 */);
    // 004cc5dd  e85effffff             -call 0x4cc540
    cpu.esp -= 4;
    sub_4cc540(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc5e2  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004cc5e4  e8a7f6ffff             -call 0x4cbc90
    cpu.esp -= 4;
    nfs3hp::vertices::particles(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc5e9  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cc5ec  ff3500d07d00           -push dword ptr [0x7dd000]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(x86::reg32(8245248) /* 0x7dd000 */);
    cpu.esp -= 4;
    // 004cc5f2  e819b0ffff             -call 0x4c7610
    cpu.esp -= 4;
    nfs3hp::vertices::sortedBuckets(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004cc5f7:
    // 004cc5f7  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cc5f9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc5fa  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc5fb  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc5fc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc5fd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4cc600(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cc600  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cc601  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cc602  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cc603  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cc605  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004cc608  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004cc60a  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004cc60c  833d30bc6f0001         +cmp dword ptr [0x6fbc30], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7322672) /* 0x6fbc30 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004cc613  7c0a                   -jl 0x4cc61f
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004cc61f;
    }
    // 004cc615  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cc618  e853f8feff             -call 0x4bbe70
    cpu.esp -= 4;
    sub_4bbe70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc61d  eb1e                   -jmp 0x4cc63d
    goto L_0x004cc63d;
L_0x004cc61f:
    // 004cc61f  e8fcf8feff             -call 0x4bbf20
    cpu.esp -= 4;
    sub_4bbf20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc624  833e00                 +cmp dword ptr [esi], 0
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
    // 004cc627  7405                   -je 0x4cc62e
    if (cpu.flags.zf)
    {
        goto L_0x004cc62e;
    }
    // 004cc629  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cc62c  eb0a                   -jmp 0x4cc638
    goto L_0x004cc638;
L_0x004cc62e:
    // 004cc62e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004cc630  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cc633  e818f1ffff             -call 0x4cb750
    cpu.esp -= 4;
    nfs3hp::vertices::drops(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004cc638:
    // 004cc638  e883f9feff             -call 0x4bbfc0
    cpu.esp -= 4;
    sub_4bbfc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004cc63d:
    // 004cc63d  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cc640  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004cc642  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cc643  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004cc644  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004cc646  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cc648  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc649  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc64a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc64b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 */
void Application::sub_4cc650(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cc650  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cc651  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cc652  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cc654  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 004cc656  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004cc658  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004cc65a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc65b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc65c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4cc660(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cc660  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cc661  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cc662  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cc663  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cc665  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004cc668  8b1590125600           -mov edx, dword ptr [0x561290]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5640848) /* 0x561290 */);
    // 004cc66e  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004cc670  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004cc672  750a                   -jne 0x4cc67e
    if (!cpu.flags.zf)
    {
        goto L_0x004cc67e;
    }
    // 004cc674  e867ec0100             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc679  a390125600             -mov dword ptr [0x561290], eax
    app->getMemory<x86::reg32>(x86::reg32(5640848) /* 0x561290 */) = cpu.eax;
L_0x004cc67e:
    // 004cc67e  8b0d90125600           -mov ecx, dword ptr [0x561290]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5640848) /* 0x561290 */);
    // 004cc684  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cc685  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc68b  8b4510                 -mov eax, dword ptr [ebp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004cc68e  6850c64c00             -push 0x4cc650
    app->getMemory<x86::reg32>(cpu.esp-4) = 5031504 /*0x4cc650*/;
    cpu.esp -= 4;
    // 004cc693  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 004cc698  ba60707f00             -mov edx, 0x7f7060
    cpu.edx = 8351840 /*0x7f7060*/;
    // 004cc69d  8b1d68125600           -mov ebx, dword ptr [0x561268]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5640808) /* 0x561268 */);
    // 004cc6a3  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004cc6a6  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cc6a9  e852ec0200             -call 0x4fb300
    cpu.esp -= 4;
    nfs3hp::watcom::bsearch(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc6ae  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004cc6b0  7403                   -je 0x4cc6b5
    if (cpu.flags.zf)
    {
        goto L_0x004cc6b5;
    }
    // 004cc6b2  8b7004                 -mov esi, dword ptr [eax + 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
L_0x004cc6b5:
    // 004cc6b5  8b1d90125600           -mov ebx, dword ptr [0x561290]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5640848) /* 0x561290 */);
    // 004cc6bb  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cc6bc  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc6c2  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004cc6c4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cc6c6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc6c7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc6c8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc6c9  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4cc6d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cc6d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cc6d1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cc6d2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cc6d3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cc6d4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cc6d6  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004cc6d9  8b7d18                 -mov edi, dword ptr [ebp + 0x18]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 004cc6dc  8b1590125600           -mov edx, dword ptr [0x561290]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5640848) /* 0x561290 */);
    // 004cc6e2  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004cc6e4  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004cc6e6  750a                   -jne 0x4cc6f2
    if (!cpu.flags.zf)
    {
        goto L_0x004cc6f2;
    }
    // 004cc6e8  e8f3eb0100             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc6ed  a390125600             -mov dword ptr [0x561290], eax
    app->getMemory<x86::reg32>(x86::reg32(5640848) /* 0x561290 */) = cpu.eax;
L_0x004cc6f2:
    // 004cc6f2  8b0d90125600           -mov ecx, dword ptr [0x561290]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5640848) /* 0x561290 */);
    // 004cc6f8  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cc6f9  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc6ff  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004cc702  6850c64c00             -push 0x4cc650
    app->getMemory<x86::reg32>(cpu.esp-4) = 5031504 /*0x4cc650*/;
    cpu.esp -= 4;
    // 004cc707  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 004cc70c  ba60707f00             -mov edx, 0x7f7060
    cpu.edx = 8351840 /*0x7f7060*/;
    // 004cc711  8b1d68125600           -mov ebx, dword ptr [0x561268]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5640808) /* 0x561268 */);
    // 004cc717  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004cc71a  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cc71d  e8deeb0200             -call 0x4fb300
    cpu.esp -= 4;
    nfs3hp::watcom::bsearch(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc722  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004cc724  7436                   -je 0x4cc75c
    if (cpu.flags.zf)
    {
        goto L_0x004cc75c;
    }
    // 004cc726  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004cc728  7528                   -jne 0x4cc752
    if (!cpu.flags.zf)
    {
        goto L_0x004cc752;
    }
    // 004cc72a  b950c64c00             -mov ecx, 0x4cc650
    cpu.ecx = 5031504 /*0x4cc650*/;
    // 004cc72f  bb08000000             -mov ebx, 8
    cpu.ebx = 8 /*0x8*/;
    // 004cc734  8b1568125600           -mov edx, dword ptr [0x561268]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5640808) /* 0x561268 */);
    // 004cc73a  c700ffffff7f           -mov dword ptr [eax], 0x7fffffff
    app->getMemory<x86::reg32>(cpu.eax) = 2147483647 /*0x7fffffff*/;
    // 004cc740  b860707f00             -mov eax, 0x7f7060
    cpu.eax = 8351840 /*0x7f7060*/;
    // 004cc745  e8ae1d0200             -call 0x4ee4f8
    cpu.esp -= 4;
    nfs3hp::watcom::qsort(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc74a  ff0d68125600           +dec dword ptr [0x561268]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(5640808) /* 0x561268 */);
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004cc750  eb03                   -jmp 0x4cc755
    goto L_0x004cc755;
L_0x004cc752:
    // 004cc752  897804                 -mov dword ptr [eax + 4], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.edi;
L_0x004cc755:
    // 004cc755  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004cc75a  eb43                   -jmp 0x4cc79f
    goto L_0x004cc79f;
L_0x004cc75c:
    // 004cc75c  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004cc75e  743f                   -je 0x4cc79f
    if (cpu.flags.zf)
    {
        goto L_0x004cc79f;
    }
    // 004cc760  a168125600             -mov eax, dword ptr [0x561268]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5640808) /* 0x561268 */);
    // 004cc765  3d80000000             +cmp eax, 0x80
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
    // 004cc76a  7333                   -jae 0x4cc79f
    if (!cpu.flags.cf)
    {
        goto L_0x004cc79f;
    }
    // 004cc76c  8b5514                 -mov edx, dword ptr [ebp + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004cc76f  b950c64c00             -mov ecx, 0x4cc650
    cpu.ecx = 5031504 /*0x4cc650*/;
    // 004cc774  bb08000000             -mov ebx, 8
    cpu.ebx = 8 /*0x8*/;
    // 004cc779  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004cc77e  8914c560707f00         -mov dword ptr [eax*8 + 0x7f7060], edx
    app->getMemory<x86::reg32>(x86::reg32(8351840) /* 0x7f7060 */ + cpu.eax * 8) = cpu.edx;
    // 004cc785  8d5001                 -lea edx, [eax + 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 004cc788  893cc564707f00         -mov dword ptr [eax*8 + 0x7f7064], edi
    app->getMemory<x86::reg32>(x86::reg32(8351844) /* 0x7f7064 */ + cpu.eax * 8) = cpu.edi;
    // 004cc78f  b860707f00             -mov eax, 0x7f7060
    cpu.eax = 8351840 /*0x7f7060*/;
    // 004cc794  891568125600           -mov dword ptr [0x561268], edx
    app->getMemory<x86::reg32>(x86::reg32(5640808) /* 0x561268 */) = cpu.edx;
    // 004cc79a  e8591d0200             -call 0x4ee4f8
    cpu.esp -= 4;
    nfs3hp::watcom::qsort(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004cc79f:
    // 004cc79f  8b0d90125600           -mov ecx, dword ptr [0x561290]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5640848) /* 0x561290 */);
    // 004cc7a5  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cc7a6  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc7ac  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004cc7ae  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cc7b0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc7b1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc7b2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc7b3  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc7b4  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4cc7c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cc7c0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cc7c1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cc7c2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cc7c3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cc7c5  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 004cc7c7  8b3d243d9f00           -mov edi, dword ptr [0x9f3d24]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10435876) /* 0x9f3d24 */);
    // 004cc7cd  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004cc7cf  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004cc7d1  743c                   -je 0x4cc80f
    if (cpu.flags.zf)
    {
        goto L_0x004cc80f;
    }
    // 004cc7d3  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004cc7d5  740d                   -je 0x4cc7e4
    if (cpu.flags.zf)
    {
        goto L_0x004cc7e4;
    }
    // 004cc7d7  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cc7d8  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cc7d9  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cc7da  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cc7db  2eff1578475300         -call dword ptr cs:[0x534778]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457784) /* 0x534778 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc7e2  eb16                   -jmp 0x4cc7fa
    goto L_0x004cc7fa;
L_0x004cc7e4:
    // 004cc7e4  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cc7e5  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cc7e6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004cc7e7  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cc7e8  2eff1568475300         -call dword ptr cs:[0x534768]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457768) /* 0x534768 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc7ef  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004cc7f1  7509                   -jne 0x4cc7fc
    if (!cpu.flags.zf)
    {
        goto L_0x004cc7fc;
    }
    // 004cc7f3  2eff1534455300         -call dword ptr cs:[0x534534]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457204) /* 0x534534 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004cc7fa:
    // 004cc7fa  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
L_0x004cc7fc:
    // 004cc7fc  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004cc7fe  7509                   -jne 0x4cc809
    if (!cpu.flags.zf)
    {
        goto L_0x004cc809;
    }
    // 004cc800  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004cc805  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc806  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc807  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc808  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004cc809:
    // 004cc809  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004cc80b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc80c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc80d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc80e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004cc80f:
    // 004cc80f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004cc811  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc812  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc813  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc814  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4cc820: native, nfs3hp::dead::stub_0x4cc820 */

/* sub_4cc840: native, nfs3hp::dead::stub_0x4cc840 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4cc860(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cc860  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cc861  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cc863  a384747f00             -mov dword ptr [0x7f7484], eax
    app->getMemory<x86::reg32>(x86::reg32(8352900) /* 0x7f7484 */) = cpu.eax;
    // 004cc868  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc869  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4cc870(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cc870  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cc871  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cc873  e818e50100             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc878  b870c84c00             -mov eax, 0x4cc870
    cpu.eax = 5032048 /*0x4cc870*/;
    // 004cc87d  e85ead0100             -call 0x4e75e0
    cpu.esp -= 4;
    nfs3hp::port::removeSystemTask(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc882  e8f6610200             -call 0x4f2a7d
    cpu.esp -= 4;
    sub_4f2a7d(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc887  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004cc889  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc88a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4cc890(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cc890  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cc891  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cc892  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cc893  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cc895  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004cc898  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004cc89a  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 004cc89d  8d55f8                 -lea edx, [ebp - 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cc8a0  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 004cc8a3  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cc8a4  8b9054040000           -mov edx, dword ptr [eax + 0x454]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1108) /* 0x454 */);
    // 004cc8aa  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cc8ab  2eff1500475300         -call dword ptr cs:[0x534700]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457664) /* 0x534700 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc8b2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004cc8b4  740a                   -je 0x4cc8c0
    if (cpu.flags.zf)
    {
        goto L_0x004cc8c0;
    }
    // 004cc8b6  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004cc8b9  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cc8ba  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004cc8bd  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cc8be  eb02                   -jmp 0x4cc8c2
    goto L_0x004cc8c2;
L_0x004cc8c0:
    // 004cc8c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cc8c1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
L_0x004cc8c2:
    // 004cc8c2  2eff1584475300         -call dword ptr cs:[0x534784]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457796) /* 0x534784 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc8c9  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004cc8cb  2eff1580475300         -call dword ptr cs:[0x534780]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457792) /* 0x534780 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc8d2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cc8d4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc8d5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc8d6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc8d7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4cc8e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004cc8e0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004cc8e1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004cc8e2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004cc8e3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004cc8e4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004cc8e6  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004cc8e9  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004cc8eb  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004cc8ee  8b1578747f00           -mov edx, dword ptr [0x7f7478]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8352888) /* 0x7f7478 */);
    // 004cc8f4  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004cc8f5  8b3d7c747f00           -mov edi, dword ptr [0x7f747c]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(8352892) /* 0x7f747c */);
    // 004cc8fb  8a7dfc                 -mov bh, byte ptr [ebp - 4]
    cpu.bh = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004cc8fe  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc904  883cbde16f7f00         -mov byte ptr [edi*4 + 0x7f6fe1], bh
    app->getMemory<x86::reg8>(x86::reg32(8351713) /* 0x7f6fe1 */ + cpu.edi * 4) = cpu.bh;
    // 004cc90b  8b157c747f00           -mov edx, dword ptr [0x7f747c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8352892) /* 0x7f747c */);
    // 004cc911  881cbde06f7f00         -mov byte ptr [edi*4 + 0x7f6fe0], bl
    app->getMemory<x86::reg8>(x86::reg32(8351712) /* 0x7f6fe0 */ + cpu.edi * 4) = cpu.bl;
    // 004cc918  42                     -inc edx
    (cpu.edx)++;
    // 004cc919  8b1d80747f00           -mov ebx, dword ptr [0x7f7480]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8352896) /* 0x7f7480 */);
    // 004cc91f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004cc921  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004cc924  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004cc926  89157c747f00           -mov dword ptr [0x7f747c], edx
    app->getMemory<x86::reg32>(x86::reg32(8352892) /* 0x7f747c */) = cpu.edx;
    // 004cc92c  8b1574747f00           -mov edx, dword ptr [0x7f7474]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8352884) /* 0x7f7474 */);
    // 004cc932  8b0d7c747f00           -mov ecx, dword ptr [0x7f747c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8352892) /* 0x7f747c */);
    // 004cc938  668934bde26f7f00       -mov word ptr [edi*4 + 0x7f6fe2], si
    app->getMemory<x86::reg16>(x86::reg32(8351714) /* 0x7f6fe2 */ + cpu.edi * 4) = cpu.si;
    // 004cc940  39ca                   +cmp edx, ecx
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
    // 004cc942  7514                   -jne 0x4cc958
    if (!cpu.flags.zf)
    {
        goto L_0x004cc958;
    }
    // 004cc944  8b1574747f00           -mov edx, dword ptr [0x7f7474]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8352884) /* 0x7f7474 */);
    // 004cc94a  42                     -inc edx
    (cpu.edx)++;
    // 004cc94b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004cc94d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004cc950  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004cc952  891574747f00           -mov dword ptr [0x7f7474], edx
    app->getMemory<x86::reg32>(x86::reg32(8352884) /* 0x7f7474 */) = cpu.edx;
L_0x004cc958:
    // 004cc958  8b1d78747f00           -mov ebx, dword ptr [0x7f7478]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8352888) /* 0x7f7478 */);
    // 004cc95e  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004cc95f  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004cc965  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004cc967  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc968  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc969  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc96a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004cc96b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4cc970: native, nfs3hp::frontend::nextKey */

/* sub_4cca00: native, nfs3hp::dead::stub_0x4cca00 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4ccaa0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ccaa0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004ccaa1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004ccaa3  3d04010000             +cmp eax, 0x104
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(260 /*0x104*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccaa8  740b                   -je 0x4ccab5
    if (cpu.flags.zf)
    {
        goto L_0x004ccab5;
    }
    // 004ccaaa  3d00010000             +cmp eax, 0x100
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(256 /*0x100*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccaaf  0f85ae010000           -jne 0x4ccc63
    if (!cpu.flags.zf)
    {
    return sub_4ccc63(app, cpu.handOver());
    }
L_0x004ccab5:
    // 004ccab5  83fa65                 +cmp edx, 0x65
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(101 /*0x65*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccab8  0f8293000000           -jb 0x4ccb51
    if (cpu.flags.cf)
    {
        goto L_0x004ccb51;
    }
    // 004ccabe  0f8644010000           -jbe 0x4ccc08
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccc08(app, cpu.handOver());
    }
    // 004ccac4  83fa73                 +cmp edx, 0x73
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(115 /*0x73*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccac7  724a                   -jb 0x4ccb13
    if (cpu.flags.cf)
    {
        goto L_0x004ccb13;
    }
    // 004ccac9  0f8655010000           -jbe 0x4ccc24
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccc24(app, cpu.handOver());
    }
    // 004ccacf  83fa77                 +cmp edx, 0x77
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(119 /*0x77*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccad2  722b                   -jb 0x4ccaff
    if (cpu.flags.cf)
    {
        goto L_0x004ccaff;
    }
    // 004ccad4  0f8666010000           -jbe 0x4ccc40
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccc40(app, cpu.handOver());
    }
    // 004ccada  83fa79                 +cmp edx, 0x79
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(121 /*0x79*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccadd  0f8264010000           -jb 0x4ccc47
    if (cpu.flags.cf)
    {
    return sub_4ccc47(app, cpu.handOver());
    }
    // 004ccae3  0f8665010000           -jbe 0x4ccc4e
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccc4e(app, cpu.handOver());
    }
    // 004ccae9  83fa7a                 +cmp edx, 0x7a
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(122 /*0x7a*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccaec  0f8663010000           -jbe 0x4ccc55
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccc55(app, cpu.handOver());
    }
    // 004ccaf2  83fa7b                 +cmp edx, 0x7b
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(123 /*0x7b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccaf5  0f8461010000           -je 0x4ccc5c
    if (cpu.flags.zf)
    {
    return sub_4ccc5c(app, cpu.handOver());
    }
    // 004ccafb  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ccafd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccafe  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccaff:
    // 004ccaff  83fa75                 +cmp edx, 0x75
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(117 /*0x75*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccb02  0f8223010000           -jb 0x4ccc2b
    if (cpu.flags.cf)
    {
    return sub_4ccc2b(app, cpu.handOver());
    }
    // 004ccb08  0f8624010000           -jbe 0x4ccc32
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccc32(app, cpu.handOver());
    }
    // 004ccb0e  e926010000             -jmp 0x4ccc39
    return sub_4ccc39(app, cpu.handOver());
L_0x004ccb13:
    // 004ccb13  83fa69                 +cmp edx, 0x69
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(105 /*0x69*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccb16  7223                   -jb 0x4ccb3b
    if (cpu.flags.cf)
    {
        goto L_0x004ccb3b;
    }
    // 004ccb18  0f86ab000000           -jbe 0x4ccbc9
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccbc9(app, cpu.handOver());
    }
    // 004ccb1e  83fa71                 +cmp edx, 0x71
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(113 /*0x71*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccb21  720b                   -jb 0x4ccb2e
    if (cpu.flags.cf)
    {
        goto L_0x004ccb2e;
    }
    // 004ccb23  0f86ed000000           -jbe 0x4ccc16
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccc16(app, cpu.handOver());
    }
    // 004ccb29  e9ef000000             -jmp 0x4ccc1d
    return sub_4ccc1d(app, cpu.handOver());
L_0x004ccb2e:
    // 004ccb2e  83fa70                 +cmp edx, 0x70
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(112 /*0x70*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccb31  0f84d8000000           -je 0x4ccc0f
    if (cpu.flags.zf)
    {
    return sub_4ccc0f(app, cpu.handOver());
    }
    // 004ccb37  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ccb39  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccb3a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccb3b:
    // 004ccb3b  83fa67                 +cmp edx, 0x67
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(103 /*0x67*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccb3e  0f82a8000000           -jb 0x4ccbec
    if (cpu.flags.cf)
    {
    return sub_4ccbec(app, cpu.handOver());
    }
    // 004ccb44  0f8694000000           -jbe 0x4ccbde
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccbde(app, cpu.handOver());
    }
L_0x004ccb4a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ccb4a  b800480000             -mov eax, 0x4800
    cpu.eax = 18432 /*0x4800*/;
    // 004ccb4f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccb50  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccb51:
    // 004ccb51  83fa28                 +cmp edx, 0x28
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(40 /*0x28*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccb54  724a                   -jb 0x4ccba0
    if (cpu.flags.cf)
    {
        goto L_0x004ccba0;
    }
    // 004ccb56  0f8697000000           -jbe 0x4ccbf3
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccbf3(app, cpu.handOver());
    }
    // 004ccb5c  83fa61                 +cmp edx, 0x61
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(97 /*0x61*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccb5f  721a                   -jb 0x4ccb7b
    if (cpu.flags.cf)
    {
        goto L_0x004ccb7b;
    }
    // 004ccb61  0f8670000000           -jbe 0x4ccbd7
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccbd7(app, cpu.handOver());
    }
    // 004ccb67  83fa63                 +cmp edx, 0x63
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(99 /*0x63*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccb6a  0f8283000000           -jb 0x4ccbf3
    if (cpu.flags.cf)
    {
    return sub_4ccbf3(app, cpu.handOver());
    }
    // 004ccb70  0f865a000000           -jbe 0x4ccbd0
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccbd0(app, cpu.handOver());
    }
    // 004ccb76  e96a000000             -jmp 0x4ccbe5
    return sub_4ccbe5(app, cpu.handOver());
L_0x004ccb7b:
    // 004ccb7b  83fa2e                 +cmp edx, 0x2e
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(46 /*0x2e*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccb7e  7213                   -jb 0x4ccb93
    if (cpu.flags.cf)
    {
        goto L_0x004ccb93;
    }
    // 004ccb80  0f867b000000           -jbe 0x4ccc01
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccc01(app, cpu.handOver());
    }
    // 004ccb86  83fa60                 +cmp edx, 0x60
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(96 /*0x60*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccb89  0f846b000000           -je 0x4ccbfa
    if (cpu.flags.zf)
    {
    return sub_4ccbfa(app, cpu.handOver());
    }
    // 004ccb8f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ccb91  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccb92  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccb93:
    // 004ccb93  83fa2d                 +cmp edx, 0x2d
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(45 /*0x2d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccb96  0f845e000000           -je 0x4ccbfa
    if (cpu.flags.zf)
    {
    return sub_4ccbfa(app, cpu.handOver());
    }
    // 004ccb9c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ccb9e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccb9f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccba0:
    // 004ccba0  83fa24                 +cmp edx, 0x24
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(36 /*0x24*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccba3  7212                   -jb 0x4ccbb7
    if (cpu.flags.cf)
    {
    return sub_4ccbb7(app, cpu.handOver());
    }
    // 004ccba5  7637                   -jbe 0x4ccbde
    if (cpu.flags.cf || cpu.flags.zf)
    {
    return sub_4ccbde(app, cpu.handOver());
    }
    // 004ccba7  83fa26                 +cmp edx, 0x26
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(38 /*0x26*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccbaa  7239                   -jb 0x4ccbe5
    if (cpu.flags.cf)
    {
    return sub_4ccbe5(app, cpu.handOver());
    }
    // 004ccbac  769c                   -jbe 0x4ccb4a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ccb4a;
    }
    // 004ccbae  eb3c                   -jmp 0x4ccbec
    return sub_4ccbec(app, cpu.handOver());
}

/* sub_4ccbb0: native, nfs3hp::dead::stub_0x4ccbb0 */

/* align: skip  */
void Application::sub_4ccbb7(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ccbb7  83fa22                 +cmp edx, 0x22
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(34 /*0x22*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccbba  7204                   -jb 0x4ccbc0
    if (cpu.flags.cf)
    {
        goto L_0x004ccbc0;
    }
    // 004ccbbc  7612                   -jbe 0x4ccbd0
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ccbd0;
    }
    // 004ccbbe  eb17                   -jmp 0x4ccbd7
    goto L_0x004ccbd7;
L_0x004ccbc0:
    // 004ccbc0  83fa21                 +cmp edx, 0x21
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(33 /*0x21*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccbc3  7404                   -je 0x4ccbc9
    if (cpu.flags.zf)
    {
        goto L_0x004ccbc9;
    }
    // 004ccbc5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ccbc7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbc8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbc9:
    // 004ccbc9  b800490000             -mov eax, 0x4900
    cpu.eax = 18688 /*0x4900*/;
    // 004ccbce  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbcf  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbd0:
    // 004ccbd0  b800510000             -mov eax, 0x5100
    cpu.eax = 20736 /*0x5100*/;
    // 004ccbd5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbd6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbd7:
    // 004ccbd7  b8004f0000             -mov eax, 0x4f00
    cpu.eax = 20224 /*0x4f00*/;
    // 004ccbdc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbdd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4ccbd7(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x004ccbd7;
    // 004ccbb7  83fa22                 +cmp edx, 0x22
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(34 /*0x22*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccbba  7204                   -jb 0x4ccbc0
    if (cpu.flags.cf)
    {
        goto L_0x004ccbc0;
    }
    // 004ccbbc  7612                   -jbe 0x4ccbd0
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ccbd0;
    }
    // 004ccbbe  eb17                   -jmp 0x4ccbd7
    goto L_0x004ccbd7;
L_0x004ccbc0:
    // 004ccbc0  83fa21                 +cmp edx, 0x21
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(33 /*0x21*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccbc3  7404                   -je 0x4ccbc9
    if (cpu.flags.zf)
    {
        goto L_0x004ccbc9;
    }
    // 004ccbc5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ccbc7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbc8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbc9:
    // 004ccbc9  b800490000             -mov eax, 0x4900
    cpu.eax = 18688 /*0x4900*/;
    // 004ccbce  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbcf  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbd0:
    // 004ccbd0  b800510000             -mov eax, 0x5100
    cpu.eax = 20736 /*0x5100*/;
    // 004ccbd5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbd6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbd7:
L_entry_0x004ccbd7:
    // 004ccbd7  b8004f0000             -mov eax, 0x4f00
    cpu.eax = 20224 /*0x4f00*/;
    // 004ccbdc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbdd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4ccbd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x004ccbd0;
    // 004ccbb7  83fa22                 +cmp edx, 0x22
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(34 /*0x22*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccbba  7204                   -jb 0x4ccbc0
    if (cpu.flags.cf)
    {
        goto L_0x004ccbc0;
    }
    // 004ccbbc  7612                   -jbe 0x4ccbd0
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ccbd0;
    }
    // 004ccbbe  eb17                   -jmp 0x4ccbd7
    goto L_0x004ccbd7;
L_0x004ccbc0:
    // 004ccbc0  83fa21                 +cmp edx, 0x21
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(33 /*0x21*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccbc3  7404                   -je 0x4ccbc9
    if (cpu.flags.zf)
    {
        goto L_0x004ccbc9;
    }
    // 004ccbc5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ccbc7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbc8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbc9:
    // 004ccbc9  b800490000             -mov eax, 0x4900
    cpu.eax = 18688 /*0x4900*/;
    // 004ccbce  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbcf  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbd0:
L_entry_0x004ccbd0:
    // 004ccbd0  b800510000             -mov eax, 0x5100
    cpu.eax = 20736 /*0x5100*/;
    // 004ccbd5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbd6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbd7:
    // 004ccbd7  b8004f0000             -mov eax, 0x4f00
    cpu.eax = 20224 /*0x4f00*/;
    // 004ccbdc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbdd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4ccbc9(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x004ccbc9;
    // 004ccbb7  83fa22                 +cmp edx, 0x22
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(34 /*0x22*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccbba  7204                   -jb 0x4ccbc0
    if (cpu.flags.cf)
    {
        goto L_0x004ccbc0;
    }
    // 004ccbbc  7612                   -jbe 0x4ccbd0
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ccbd0;
    }
    // 004ccbbe  eb17                   -jmp 0x4ccbd7
    goto L_0x004ccbd7;
L_0x004ccbc0:
    // 004ccbc0  83fa21                 +cmp edx, 0x21
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(33 /*0x21*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ccbc3  7404                   -je 0x4ccbc9
    if (cpu.flags.zf)
    {
        goto L_0x004ccbc9;
    }
    // 004ccbc5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ccbc7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbc8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbc9:
L_entry_0x004ccbc9:
    // 004ccbc9  b800490000             -mov eax, 0x4900
    cpu.eax = 18688 /*0x4900*/;
    // 004ccbce  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbcf  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbd0:
    // 004ccbd0  b800510000             -mov eax, 0x5100
    cpu.eax = 20736 /*0x5100*/;
    // 004ccbd5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbd6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004ccbd7:
    // 004ccbd7  b8004f0000             -mov eax, 0x4f00
    cpu.eax = 20224 /*0x4f00*/;
    // 004ccbdc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbdd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4ccbde(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ccbde  b800470000             -mov eax, 0x4700
    cpu.eax = 18176 /*0x4700*/;
    // 004ccbe3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbe4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4ccbe5(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ccbe5  b8004b0000             -mov eax, 0x4b00
    cpu.eax = 19200 /*0x4b00*/;
    // 004ccbea  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbeb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4ccbec(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ccbec  b8004d0000             -mov eax, 0x4d00
    cpu.eax = 19712 /*0x4d00*/;
    // 004ccbf1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbf2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4ccbf3(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ccbf3  b800500000             -mov eax, 0x5000
    cpu.eax = 20480 /*0x5000*/;
    // 004ccbf8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ccbf9  c3                     -ret 
    cpu.esp += 4;
    return;
}

}
