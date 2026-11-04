#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* align: skip  */
void Application::sub_523465(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fpa1 = cpu.fpu.st(0);
  x86::Float fp0;
L_0x00523465:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523465  66d1e8                 +shr ax, 1
    {
        x86::reg16 tmp = 1 /*0x1*/ % 32;
        x86::reg16& op = cpu.ax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (16 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 00523468  7604                   -jbe 0x52346e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0052346e;
    }
    // 0052346a  d8c8                   +fmul st(0)
    fpa1 = fp::mul(fpa1, x86::Float(fpa1));
    // 0052346c  ebf7                   -jmp 0x523465
    goto L_0x00523465;
L_0x0052346e:
    // 0052346e  7313                   -jae 0x523483
    if (!cpu.flags.cf)
    {
        goto L_0x00523483;
    }
    // 00523470  d9c0                   +fld st(0)
    fp0 = x86::Float(fpa1);
L_0x00523472:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523472  740b                   -je 0x52347f
    if (cpu.flags.zf)
    {
        goto L_0x0052347f;
    }
    // 00523474  d8c8                   -fmul st(0)
    fp0 = fp::mul(fp0, x86::Float(fp0));
    // 00523476  66d1e8                 +shr ax, 1
    {
        x86::reg16 tmp = 1 /*0x1*/ % 32;
        x86::reg16& op = cpu.ax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (16 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 00523479  7302                   -jae 0x52347d
    if (!cpu.flags.cf)
    {
        goto L_0x0052347d;
    }
    // 0052347b  dcc9                   +fmul st(1), st(0)
    fpa1 = fp::mul(fpa1, fp0);
L_0x0052347d:
    // 0052347d  ebf3                   -jmp 0x523472
    goto L_0x00523472;
L_0x0052347f:
    // 0052347f  ddd8                   +fstp st(0)
    fp0 = x86::Float(fp0);
    // 00523481  eb04                   -jmp 0x523487
    goto L_0x00523487;
L_0x00523483:
    // 00523483  ddd8                   -fstp st(0)
    fpa1 = x86::Float(fpa1);
    // 00523485  d9e8                   -fld1 
    fpa1 = 1.0;
L_0x00523487:
    // 00523487  c3                     -ret 
    cpu.fpu.st(0) = fpa1;
    cpu.esp += 4;
    return;
}

/* sub_523488: native, nfs3hp::dead::stub_0x523488 */

/* sub_5234a0: native, nfs3hp::dead::stub_0x5234a0 */

/* sub_5234d0: native, nfs3hp::watcom::GetNTAccessAttr */

/* sub_523504: native, nfs3hp::watcom::GetNTShareAttr */

/* sub_523560: native, nfs3hp::watcom::isatty */

/* sub_5235d0: native, nfs3hp::dead::stub_0x5235d0 */

/* sub_5235dc: native, nfs3hp::dead::stub_0x5235dc */

/* sub_5235f0: native, nfs3hp::dead::stub_0x5235f0 */

/* sub_523610: native, nfs3hp::watcom::fullpath */

/* align: skip 0x00 0x00 0x00 0x00 */
void Application::sub_5236c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005236c0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 005236c1  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 005236c3  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 005236c5  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 005236ca  83f87b                 +cmp eax, 0x7b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(123 /*0x7b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005236cd  743f                   -je 0x52370e
    if (cpu.flags.zf)
    {
        goto L_0x0052370e;
    }
    // 005236cf  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005236d0  a880                   +test al, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 128 /*0x80*/));
    // 005236d2  7542                   -jne 0x523716
    if (!cpu.flags.zf)
    {
        goto L_0x00523716;
    }
L_0x005236d4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005236d4  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 005236d6  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 005236d8  83e07f                 -and eax, 0x7f
    cpu.eax &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 005236db  8a80546d5600           -mov al, byte ptr [eax + 0x566d54]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5664084) /* 0x566d54 */);
    // 005236e1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 005236e3  83f80f                 +cmp eax, 0xf
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(15 /*0xf*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005236e6  7505                   -jne 0x5236ed
    if (!cpu.flags.zf)
    {
        goto L_0x005236ed;
    }
    // 005236e8  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
L_0x005236ed:
    // 005236ed  8b4202                 -mov eax, dword ptr [edx + 2]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(2) /* 0x2 */);
    // 005236f0  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 005236f3  0fafc1                 -imul eax, ecx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ecx)));
    // 005236f6  8b12                   -mov edx, dword ptr [edx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx);
    // 005236f8  81e2f0000000           -and edx, 0xf0
    cpu.edx &= x86::reg32(x86::sreg32(240 /*0xf0*/));
    // 005236fe  83fa40                 +cmp edx, 0x40
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00523701  7548                   -jne 0x52374b
    if (!cpu.flags.zf)
    {
        goto L_0x0052374b;
    }
    // 00523703  83c00f                 -add eax, 0xf
    (cpu.eax) += x86::reg32(x86::sreg32(15 /*0xf*/));
    // 00523706  24f0                   -and al, 0xf0
    cpu.al &= x86::reg8(x86::sreg8(240 /*0xf0*/));
    // 00523708  c1f803                 -sar eax, 3
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (3 /*0x3*/ % 32));
    // 0052370b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052370c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052370d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052370e:
    // 0052370e  8b4202                 -mov eax, dword ptr [edx + 2]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(2) /* 0x2 */);
    // 00523711  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00523714  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523715  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00523716:
    // 00523716  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00523717  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00523718  b90c1b5500             -mov ecx, 0x551b0c
    cpu.ecx = 5577484 /*0x551b0c*/;
    // 0052371d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052371e  bb1c1b5500             -mov ebx, 0x551b1c
    cpu.ebx = 5577500 /*0x551b1c*/;
    // 00523723  be35000000             -mov esi, 0x35
    cpu.esi = 53 /*0x35*/;
    // 00523728  682c1b5500             -push 0x551b2c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5577516 /*0x551b2c*/;
    cpu.esp -= 4;
    // 0052372d  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 00523733  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 00523739  893598215500           -mov dword ptr [0x552198], esi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.esi;
    // 0052373f  e8ccd8edff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00523744  83c408                 +add esp, 8
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
    // 00523747  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523748  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523749  eb89                   -jmp 0x5236d4
    goto L_0x005236d4;
L_0x0052374b:
    // 0052374b  83c007                 -add eax, 7
    (cpu.eax) += x86::reg32(x86::sreg32(7 /*0x7*/));
    // 0052374e  24f8                   -and al, 0xf8
    cpu.al &= x86::reg8(x86::sreg8(248 /*0xf8*/));
    // 00523750  c1f803                 -sar eax, 3
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (3 /*0x3*/ % 32));
    // 00523753  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523754  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523755  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_523760(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00523760  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00523761  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00523762  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00523763  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00523764  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 00523767  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00523769  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0052376b  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 0052376d  3b0500505600           +cmp eax, dword ptr [0x565000]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656576) /* 0x565000 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00523773  7c49                   -jl 0x5237be
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005237be;
    }
    // 00523775  3b0508505600           +cmp eax, dword ptr [0x565008]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656584) /* 0x565008 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052377b  7d41                   -jge 0x5237be
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x005237be;
    }
    // 0052377d  3b3504505600           +cmp esi, dword ptr [0x565004]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656580) /* 0x565004 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00523783  7c39                   -jl 0x5237be
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x005237be;
    }
    // 00523785  3b350c505600           +cmp esi, dword ptr [0x56500c]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656588) /* 0x56500c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052378b  7d31                   -jge 0x5237be
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x005237be;
    }
    // 0052378d  803d1050560010         +cmp byte ptr [0x565010], 0x10
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5656592) /* 0x565010 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(16 /*0x10*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00523794  7509                   -jne 0x52379f
    if (!cpu.flags.zf)
    {
        goto L_0x0052379f;
    }
    // 00523796  803d1250560000         +cmp byte ptr [0x565012], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5656594) /* 0x565012 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052379d  7427                   -je 0x5237c6
    if (cpu.flags.zf)
    {
        goto L_0x005237c6;
    }
L_0x0052379f:
    // 0052379f  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 005237a1  c1eb18                 -shr ebx, 0x18
    cpu.ebx >>= 24 /*0x18*/ % 32;
    // 005237a4  81e3ff000000           -and ebx, 0xff
    cpu.ebx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 005237aa  81fbff000000           +cmp ebx, 0xff
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(255 /*0xff*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005237b0  0f84ae000000           -je 0x523864
    if (cpu.flags.zf)
    {
        goto L_0x00523864;
    }
    // 005237b6  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 005237b8  0f85c0000000           -jne 0x52387e
    if (!cpu.flags.zf)
    {
        goto L_0x0052387e;
    }
L_0x005237be:
    // 005237be  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 005237c1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005237c2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005237c3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005237c4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005237c5  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005237c6:
    // 005237c6  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 005237c8  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 005237ca  8b1524505600           -mov edx, dword ptr [0x565024]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656612) /* 0x565024 */);
    // 005237d0  8b0482                 -mov eax, dword ptr [edx + eax*4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 4);
    // 005237d3  8b1520505600           -mov edx, dword ptr [0x565020]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656608) /* 0x565020 */);
    // 005237d9  03049a                 -add eax, dword ptr [edx + ebx*4]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + cpu.ebx * 4)));
    // 005237dc  030514505600           -add eax, dword ptr [0x565014]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656596) /* 0x565014 */)));
    // 005237e2  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 005237e4  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 005237e6  81fb00000010           +cmp ebx, 0x10000000
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(268435456 /*0x10000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005237ec  726b                   -jb 0x523859
    if (cpu.flags.cf)
    {
        goto L_0x00523859;
    }
    // 005237ee  81fb000000fc           +cmp ebx, 0xfc000000
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4227858432 /*0xfc000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005237f4  7340                   -jae 0x523836
    if (!cpu.flags.cf)
    {
        goto L_0x00523836;
    }
    // 005237f6  668b06                 -mov ax, word ptr [esi]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi);
    // 005237f9  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 005237fb  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 005237fd  83f1ff                 -xor ecx, 0xffffffff
    cpu.ecx ^= x86::reg32(x86::sreg32(4294967295 /*0xffffffff*/));
    // 00523800  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 00523803  88d0                   -mov al, dl
    cpu.al = cpu.dl;
    // 00523805  81e2e0070000           -and edx, 0x7e0
    cpu.edx &= x86::reg32(x86::sreg32(2016 /*0x7e0*/));
    // 0052380b  c1e208                 -shl edx, 8
    cpu.edx <<= 8 /*0x8*/ % 32;
    // 0052380e  251f0000f8             -and eax, 0xf800001f
    cpu.eax &= x86::reg32(x86::sreg32(4160749599 /*0xf800001f*/));
    // 00523813  c1e918                 -shr ecx, 0x18
    cpu.ecx >>= 24 /*0x18*/ % 32;
    // 00523816  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00523818  f7e1                   -mul ecx
    cpu.edx_eax = x86::reg64(cpu.eax) * x86::reg64(cpu.ecx);
    // 0052381a  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 0052381d  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0052381f  c1e905                 -shr ecx, 5
    cpu.ecx >>= 5 /*0x5*/ % 32;
    // 00523822  01d3                   -add ebx, edx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edx));
    // 00523824  c1e80b                 -shr eax, 0xb
    cpu.eax >>= 11 /*0xb*/ % 32;
    // 00523827  81e1ff000000           -and ecx, 0xff
    cpu.ecx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0052382d  2500ff0000             -and eax, 0xff00
    cpu.eax &= x86::reg32(x86::sreg32(65280 /*0xff00*/));
    // 00523832  01cb                   -add ebx, ecx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00523834  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
L_0x00523836:
    // 00523836  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00523838  81e30000f800           -and ebx, 0xf80000
    cpu.ebx &= x86::reg32(x86::sreg32(16252928 /*0xf80000*/));
    // 0052383e  c1eb08                 -shr ebx, 8
    cpu.ebx >>= 8 /*0x8*/ % 32;
    // 00523841  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00523843  c1e803                 -shr eax, 3
    cpu.eax >>= 3 /*0x3*/ % 32;
    // 00523846  81e200fc0000           -and edx, 0xfc00
    cpu.edx &= x86::reg32(x86::sreg32(64512 /*0xfc00*/));
    // 0052384c  c1ea05                 -shr edx, 5
    cpu.edx >>= 5 /*0x5*/ % 32;
    // 0052384f  83e01f                 -and eax, 0x1f
    cpu.eax &= x86::reg32(x86::sreg32(31 /*0x1f*/));
    // 00523852  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00523854  01d3                   -add ebx, edx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edx));
    // 00523856  66891e                 -mov word ptr [esi], bx
    app->getMemory<x86::reg16>(cpu.esi) = cpu.bx;
L_0x00523859:
    // 00523859  8d7602                 -lea esi, [esi + 2]
    cpu.esi = x86::reg32(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 0052385c  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0052385f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523860  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523861  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523862  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523863  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00523864:
    // 00523864  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00523866  e8f5befcff             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052386b  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0052386d  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0052386f  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00523871  e88a020000             -call 0x523b00
    cpu.esp -= 4;
    sub_523b00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00523876  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 00523879  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052387a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052387b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052387c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052387d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052387e:
    // 0052387e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00523880  c1e810                 -shr eax, 0x10
    cpu.eax >>= 16 /*0x10*/ % 32;
    // 00523883  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00523888  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0052388b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052388d  c1e808                 -shr eax, 8
    cpu.eax >>= 8 /*0x8*/ % 32;
    // 00523890  81e1ff000000           -and ecx, 0xff
    cpu.ecx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00523896  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0052389b  894c2404               -mov dword ptr [esp + 4], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.ecx;
    // 0052389f  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 005238a3  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 005238a5  b800000100             -mov eax, 0x10000
    cpu.eax = 65536 /*0x10000*/;
    // 005238aa  c1e108                 -shl ecx, 8
    cpu.ecx <<= 8 /*0x8*/ % 32;
    // 005238ad  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 005238af  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 005238b1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 005238b3  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 005238b5  e80603feff             -call 0x503bc0
    cpu.esp -= 4;
    sub_503bc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005238ba  e8e18afeff             -call 0x50c3a0
    cpu.esp -= 4;
    sub_50c3a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005238bf  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 005238c1  c1fa18                 -sar edx, 0x18
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (24 /*0x18*/ % 32));
    // 005238c4  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 005238ca  89542414               -mov dword ptr [esp + 0x14], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */) = cpu.edx;
    // 005238ce  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 005238d0  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 005238d2  c1fa08                 -sar edx, 8
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (8 /*0x8*/ % 32));
    // 005238d5  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 005238da  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 005238e0  8944240c               -mov dword ptr [esp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 005238e4  89542410               -mov dword ptr [esp + 0x10], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.edx;
    // 005238e8  baff000000             -mov edx, 0xff
    cpu.edx = 255 /*0xff*/;
    // 005238ed  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
    // 005238f2  29da                   -sub edx, ebx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 005238f4  2b442414               -sub eax, dword ptr [esp + 0x14]
    (cpu.eax) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */)));
    // 005238f8  0fafd0                 -imul edx, eax
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 005238fb  bbff000000             -mov ebx, 0xff
    cpu.ebx = 255 /*0xff*/;
    // 00523900  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00523902  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00523905  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00523907  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00523909  c1fd10                 -sar ebp, 0x10
    cpu.ebp = x86::reg32(x86::sreg32(cpu.ebp) >> (16 /*0x10*/ % 32));
    // 0052390c  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0052390e  81e5ff000000           -and ebp, 0xff
    cpu.ebp &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00523914  89542414               -mov dword ptr [esp + 0x14], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */) = cpu.edx;
    // 00523918  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0052391a  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0052391c  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 0052391e  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 00523921  c1e810                 +shr eax, 0x10
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
    // 00523924  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 00523926  030424                 -add eax, dword ptr [esp]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp)));
    // 00523929  39d8                   +cmp eax, ebx
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
    // 0052392b  0f8e77000000           -jle 0x5239a8
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x005239a8;
    }
    // 00523931  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
L_0x00523933:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523933  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00523935  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00523937  8b442410               -mov eax, dword ptr [esp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 0052393b  8b6c2408               -mov ebp, dword ptr [esp + 8]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0052393f  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 00523941  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 00523944  c1e810                 +shr eax, 0x10
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
    // 00523947  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 00523949  01e8                   -add eax, ebp
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebp));
    // 0052394b  3dff000000             +cmp eax, 0xff
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
    // 00523950  7e6e                   -jle 0x5239c0
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x005239c0;
    }
    // 00523952  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
L_0x00523957:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523957  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 00523959  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0052395b  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0052395f  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 00523961  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 00523964  c1e810                 +shr eax, 0x10
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
    // 00523967  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 00523969  03442404               -add eax, dword ptr [esp + 4]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */)));
    // 0052396d  3dff000000             +cmp eax, 0xff
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
    // 00523972  7e60                   -jle 0x5239d4
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x005239d4;
    }
    // 00523974  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
L_0x00523979:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523979  8b542414               -mov edx, dword ptr [esp + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */);
    // 0052397d  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 0052397f  c1e218                 -shl edx, 0x18
    cpu.edx <<= 24 /*0x18*/ % 32;
    // 00523982  c1e110                 -shl ecx, 0x10
    cpu.ecx <<= 16 /*0x10*/ % 32;
    // 00523985  09ca                   -or edx, ecx
    cpu.edx |= x86::reg32(x86::sreg32(cpu.ecx));
    // 00523987  89e9                   -mov ecx, ebp
    cpu.ecx = cpu.ebp;
    // 00523989  c1e108                 -shl ecx, 8
    cpu.ecx <<= 8 /*0x8*/ % 32;
    // 0052398c  09ca                   -or edx, ecx
    cpu.edx |= x86::reg32(x86::sreg32(cpu.ecx));
    // 0052398e  09d0                   -or eax, edx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.edx));
    // 00523990  e8cbbdfcff             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00523995  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00523997  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00523999  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0052399b  e860010000             -call 0x523b00
    cpu.esp -= 4;
    sub_523b00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005239a0  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 005239a3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005239a4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005239a5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005239a6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005239a7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005239a8:
    // 005239a8  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 005239aa  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 005239ac  8b1c24                 -mov ebx, dword ptr [esp]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    // 005239af  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 005239b1  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 005239b4  c1e810                 +shr eax, 0x10
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
    // 005239b7  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 005239b9  01d8                   +add eax, ebx
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
    // 005239bb  e973ffffff             -jmp 0x523933
    goto L_0x00523933;
L_0x005239c0:
    // 005239c0  8b442410               -mov eax, dword ptr [esp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 005239c4  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 005239c6  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 005239c8  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 005239cb  c1e810                 +shr eax, 0x10
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
    // 005239ce  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 005239d0  01e8                   +add eax, ebp
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
    // 005239d2  eb83                   -jmp 0x523957
    goto L_0x00523957;
L_0x005239d4:
    // 005239d4  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 005239d8  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 005239da  8b4c2404               -mov ecx, dword ptr [esp + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 005239de  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 005239e0  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 005239e3  c1e810                 +shr eax, 0x10
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
    // 005239e6  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 005239e8  01c8                   +add eax, ecx
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
    // 005239ea  eb8d                   -jmp 0x523979
    goto L_0x00523979;
}

/* align: skip  */
void Application::sub_5239ec(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005239ec  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 005239ed  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005239ee  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005239ef  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005239f0  837c242400             +cmp dword ptr [esp + 0x24], 0
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
    // 005239f5  0f8efb000000           -jle 0x523af6
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00523af6;
    }
    // 005239fb  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 005239fd  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 005239ff  8b6c2414               -mov ebp, dword ptr [esp + 0x14]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */);
    // 00523a03  8b742418               -mov esi, dword ptr [esp + 0x18]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(24) /* 0x18 */);
    // 00523a07  8b7c241c               -mov edi, dword ptr [esp + 0x1c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(28) /* 0x1c */);
    // 00523a0b  8b4c2420               -mov ecx, dword ptr [esp + 0x20]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(32) /* 0x20 */);
    // 00523a0f  83f901                 +cmp ecx, 1
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
    // 00523a12  0f8cde000000           -jl 0x523af6
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00523af6;
    }
    // 00523a18  d1e9                   +shr ecx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ecx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 00523a1a  894c2420               -mov dword ptr [esp + 0x20], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(32) /* 0x20 */) = cpu.ecx;
    // 00523a1e  7259                   -jb 0x523a79
    if (cpu.flags.cf)
    {
        goto L_0x00523a79;
    }
L_0x00523a20:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523a20  8b4c2420               -mov ecx, dword ptr [esp + 0x20]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(32) /* 0x20 */);
L_0x00523a24:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523a24  8a7e01                 -mov bh, byte ptr [esi + 1]
    cpu.bh = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00523a27  8a7601                 -mov dh, byte ptr [esi + 1]
    cpu.dh = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00523a2a  8a5f02                 -mov bl, byte ptr [edi + 2]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(2) /* 0x2 */);
    // 00523a2d  8a5703                 -mov dl, byte ptr [edi + 3]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(3) /* 0x3 */);
    // 00523a30  c0ef04                 -shr bh, 4
    cpu.bh >>= 4 /*0x4*/ % 32;
    // 00523a33  80e60f                 -and dh, 0xf
    cpu.dh &= x86::reg8(x86::sreg8(15 /*0xf*/));
    // 00523a36  8a441d00               -mov al, byte ptr [ebp + ebx]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + cpu.ebx * 1);
    // 00523a3a  8a641500               -mov ah, byte ptr [ebp + edx]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + cpu.edx * 1);
    // 00523a3e  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 00523a41  8a3e                   -mov bh, byte ptr [esi]
    cpu.bh = app->getMemory<x86::reg8>(cpu.esi);
    // 00523a43  8a36                   -mov dh, byte ptr [esi]
    cpu.dh = app->getMemory<x86::reg8>(cpu.esi);
    // 00523a45  8a1f                   -mov bl, byte ptr [edi]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi);
    // 00523a47  8a5701                 -mov dl, byte ptr [edi + 1]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 00523a4a  c0ef04                 -shr bh, 4
    cpu.bh >>= 4 /*0x4*/ % 32;
    // 00523a4d  80e60f                 -and dh, 0xf
    cpu.dh &= x86::reg8(x86::sreg8(15 /*0xf*/));
    // 00523a50  8a441d00               -mov al, byte ptr [ebp + ebx]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + cpu.ebx * 1);
    // 00523a54  8a641500               -mov ah, byte ptr [ebp + edx]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + cpu.edx * 1);
    // 00523a58  83e901                 +sub ecx, 1
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00523a5b  8907                   -mov dword ptr [edi], eax
    app->getMemory<x86::reg32>(cpu.edi) = cpu.eax;
    // 00523a5d  8d7602                 -lea esi, [esi + 2]
    cpu.esi = x86::reg32(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 00523a60  8d7f04                 -lea edi, [edi + 4]
    cpu.edi = x86::reg32(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 00523a63  7fbf                   -jg 0x523a24
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00523a24;
    }
    // 00523a65  03742428               -add esi, dword ptr [esp + 0x28]
    (cpu.esi) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(40) /* 0x28 */)));
    // 00523a69  037c242c               -add edi, dword ptr [esp + 0x2c]
    (cpu.edi) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(44) /* 0x2c */)));
    // 00523a6d  836c242401             +sub dword ptr [esp + 0x24], 1
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00523a72  75ac                   -jne 0x523a20
    if (!cpu.flags.zf)
    {
        goto L_0x00523a20;
    }
    // 00523a74  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523a75  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523a76  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523a77  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523a78  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00523a79:
    // 00523a79  8344242801             -add dword ptr [esp + 0x28], 1
    (app->getMemory<x86::reg32>(cpu.esp + x86::reg32(40) /* 0x28 */)) += x86::reg32(x86::sreg32(1 /*0x1*/));
    // 00523a7e  8344242c02             -add dword ptr [esp + 0x2c], 2
    (app->getMemory<x86::reg32>(cpu.esp + x86::reg32(44) /* 0x2c */)) += x86::reg32(x86::sreg32(2 /*0x2*/));
L_0x00523a83:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523a83  8b4c2420               -mov ecx, dword ptr [esp + 0x20]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(32) /* 0x20 */);
    // 00523a87  83f900                 +cmp ecx, 0
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
    // 00523a8a  7441                   -je 0x523acd
    if (cpu.flags.zf)
    {
        goto L_0x00523acd;
    }
L_0x00523a8c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523a8c  8a7e01                 -mov bh, byte ptr [esi + 1]
    cpu.bh = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00523a8f  8a7601                 -mov dh, byte ptr [esi + 1]
    cpu.dh = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00523a92  8a5f02                 -mov bl, byte ptr [edi + 2]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(2) /* 0x2 */);
    // 00523a95  8a5703                 -mov dl, byte ptr [edi + 3]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(3) /* 0x3 */);
    // 00523a98  c0ef04                 -shr bh, 4
    cpu.bh >>= 4 /*0x4*/ % 32;
    // 00523a9b  80e60f                 -and dh, 0xf
    cpu.dh &= x86::reg8(x86::sreg8(15 /*0xf*/));
    // 00523a9e  8a441d00               -mov al, byte ptr [ebp + ebx]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + cpu.ebx * 1);
    // 00523aa2  8a641500               -mov ah, byte ptr [ebp + edx]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + cpu.edx * 1);
    // 00523aa6  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 00523aa9  8a3e                   -mov bh, byte ptr [esi]
    cpu.bh = app->getMemory<x86::reg8>(cpu.esi);
    // 00523aab  8a36                   -mov dh, byte ptr [esi]
    cpu.dh = app->getMemory<x86::reg8>(cpu.esi);
    // 00523aad  8a1f                   -mov bl, byte ptr [edi]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi);
    // 00523aaf  8a5701                 -mov dl, byte ptr [edi + 1]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 00523ab2  c0ef04                 -shr bh, 4
    cpu.bh >>= 4 /*0x4*/ % 32;
    // 00523ab5  80e60f                 -and dh, 0xf
    cpu.dh &= x86::reg8(x86::sreg8(15 /*0xf*/));
    // 00523ab8  8a441d00               -mov al, byte ptr [ebp + ebx]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + cpu.ebx * 1);
    // 00523abc  8a641500               -mov ah, byte ptr [ebp + edx]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + cpu.edx * 1);
    // 00523ac0  83e901                 +sub ecx, 1
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00523ac3  8907                   -mov dword ptr [edi], eax
    app->getMemory<x86::reg32>(cpu.edi) = cpu.eax;
    // 00523ac5  8d7602                 -lea esi, [esi + 2]
    cpu.esi = x86::reg32(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 00523ac8  8d7f04                 -lea edi, [edi + 4]
    cpu.edi = x86::reg32(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 00523acb  7fbf                   -jg 0x523a8c
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00523a8c;
    }
L_0x00523acd:
    // 00523acd  8a3e                   -mov bh, byte ptr [esi]
    cpu.bh = app->getMemory<x86::reg8>(cpu.esi);
    // 00523acf  8a1f                   -mov bl, byte ptr [edi]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edi);
    // 00523ad1  8a5701                 -mov dl, byte ptr [edi + 1]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 00523ad4  88fe                   -mov dh, bh
    cpu.dh = cpu.bh;
    // 00523ad6  c0ef04                 -shr bh, 4
    cpu.bh >>= 4 /*0x4*/ % 32;
    // 00523ad9  80e60f                 -and dh, 0xf
    cpu.dh &= x86::reg8(x86::sreg8(15 /*0xf*/));
    // 00523adc  8a441d00               -mov al, byte ptr [ebp + ebx]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + cpu.ebx * 1);
    // 00523ae0  8a641500               -mov ah, byte ptr [ebp + edx]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + cpu.edx * 1);
    // 00523ae4  668907                 -mov word ptr [edi], ax
    app->getMemory<x86::reg16>(cpu.edi) = cpu.ax;
    // 00523ae7  03742428               -add esi, dword ptr [esp + 0x28]
    (cpu.esi) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(40) /* 0x28 */)));
    // 00523aeb  037c242c               -add edi, dword ptr [esp + 0x2c]
    (cpu.edi) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(44) /* 0x2c */)));
    // 00523aef  836c242401             +sub dword ptr [esp + 0x24], 1
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00523af4  758d                   -jne 0x523a83
    if (!cpu.flags.zf)
    {
        goto L_0x00523a83;
    }
L_0x00523af6:
    // 00523af6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523af7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523af8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523af9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523afa  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_523b00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00523b00  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00523b01  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00523b02  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00523b03  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00523b05  3b0500505600           +cmp eax, dword ptr [0x565000]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656576) /* 0x565000 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00523b0b  0f8c9c000000           -jl 0x523bad
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00523bad;
    }
    // 00523b11  3b0508505600           +cmp eax, dword ptr [0x565008]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656584) /* 0x565008 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00523b17  0f8d90000000           -jge 0x523bad
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00523bad;
    }
    // 00523b1d  3b1504505600           +cmp edx, dword ptr [0x565004]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656580) /* 0x565004 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00523b23  0f8c84000000           -jl 0x523bad
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00523bad;
    }
    // 00523b29  3b150c505600           +cmp edx, dword ptr [0x56500c]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656588) /* 0x56500c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00523b2f  0f8d78000000           -jge 0x523bad
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00523bad;
    }
    // 00523b35  a120505600             -mov eax, dword ptr [0x565020]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5656608) /* 0x565020 */);
    // 00523b3a  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00523b3d  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00523b3f  a124505600             -mov eax, dword ptr [0x565024]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5656612) /* 0x565024 */);
    // 00523b44  8b12                   -mov edx, dword ptr [edx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx);
    // 00523b46  8b3488                 -mov esi, dword ptr [eax + ecx*4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + cpu.ecx * 4);
    // 00523b49  a114505600             -mov eax, dword ptr [0x565014]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5656596) /* 0x565014 */);
    // 00523b4e  01f2                   -add edx, esi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.esi));
    // 00523b50  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00523b52  a010505600             -mov al, byte ptr [0x565010]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5656592) /* 0x565010 */);
    // 00523b57  3c0f                   +cmp al, 0xf
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(15 /*0xf*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00523b59  7356                   -jae 0x523bb1
    if (!cpu.flags.cf)
    {
        goto L_0x00523bb1;
    }
    // 00523b5b  3c04                   +cmp al, 4
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(4 /*0x4*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00523b5d  0f836b000000           -jae 0x523bce
    if (!cpu.flags.cf)
    {
        goto L_0x00523bce;
    }
L_0x00523b63:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523b63  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00523b64  c7059821550053000000   -mov dword ptr [0x552198], 0x53
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = 83 /*0x53*/;
    // 00523b6e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00523b70  a010505600             -mov al, byte ptr [0x565010]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5656592) /* 0x565010 */);
    // 00523b75  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00523b76  bf681b5500             -mov edi, 0x551b68
    cpu.edi = 5577576 /*0x551b68*/;
    // 00523b7b  bd781b5500             -mov ebp, 0x551b78
    cpu.ebp = 5577592 /*0x551b78*/;
    // 00523b80  68841b5500             -push 0x551b84
    app->getMemory<x86::reg32>(cpu.esp-4) = 5577604 /*0x551b84*/;
    cpu.esp -= 4;
    // 00523b85  893d90215500           -mov dword ptr [0x552190], edi
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edi;
    // 00523b8b  892d94215500           -mov dword ptr [0x552194], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebp;
    // 00523b91  e87ad4edff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00523b96  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00523b99  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x00523b9a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523b9a  f6c101                 +test cl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 1 /*0x1*/));
    // 00523b9d  743b                   -je 0x523bda
    if (cpu.flags.zf)
    {
        goto L_0x00523bda;
    }
    // 00523b9f  8a3a                   -mov bh, byte ptr [edx]
    cpu.bh = app->getMemory<x86::reg8>(cpu.edx);
    // 00523ba1  80e70f                 -and bh, 0xf
    cpu.bh &= x86::reg8(x86::sreg8(15 /*0xf*/));
    // 00523ba4  88d8                   -mov al, bl
    cpu.al = cpu.bl;
    // 00523ba6  c0e004                 -shl al, 4
    cpu.al <<= 4 /*0x4*/ % 32;
    // 00523ba9  08c7                   +or bh, al
    cpu.clear_co();
    cpu.set_szp((cpu.bh |= x86::reg8(x86::sreg8(cpu.al))));
    // 00523bab  883a                   -mov byte ptr [edx], bh
    app->getMemory<x86::reg8>(cpu.edx) = cpu.bh;
L_0x00523bad:
    // 00523bad  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bae  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523baf  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bb0  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00523bb1:
    // 00523bb1  7608                   -jbe 0x523bbb
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00523bbb;
    }
    // 00523bb3  3c18                   +cmp al, 0x18
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(24 /*0x18*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00523bb5  730b                   -jae 0x523bc2
    if (!cpu.flags.cf)
    {
        goto L_0x00523bc2;
    }
    // 00523bb7  3c10                   +cmp al, 0x10
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(16 /*0x10*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00523bb9  75a8                   -jne 0x523b63
    if (!cpu.flags.zf)
    {
        goto L_0x00523b63;
    }
L_0x00523bbb:
    // 00523bbb  66891a                 -mov word ptr [edx], bx
    app->getMemory<x86::reg16>(cpu.edx) = cpu.bx;
    // 00523bbe  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bbf  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bc0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bc1  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00523bc2:
    // 00523bc2  7622                   -jbe 0x523be6
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00523be6;
    }
    // 00523bc4  3c20                   +cmp al, 0x20
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
    // 00523bc6  759b                   -jne 0x523b63
    if (!cpu.flags.zf)
    {
        goto L_0x00523b63;
    }
    // 00523bc8  891a                   -mov dword ptr [edx], ebx
    app->getMemory<x86::reg32>(cpu.edx) = cpu.ebx;
    // 00523bca  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bcb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bcc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bcd  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00523bce:
    // 00523bce  76ca                   -jbe 0x523b9a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00523b9a;
    }
    // 00523bd0  3c08                   +cmp al, 8
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(8 /*0x8*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00523bd2  758f                   -jne 0x523b63
    if (!cpu.flags.zf)
    {
        goto L_0x00523b63;
    }
    // 00523bd4  881a                   -mov byte ptr [edx], bl
    app->getMemory<x86::reg8>(cpu.edx) = cpu.bl;
    // 00523bd6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bd7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bd8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bd9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00523bda:
    // 00523bda  8a02                   -mov al, byte ptr [edx]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx);
    // 00523bdc  24f0                   -and al, 0xf0
    cpu.al &= x86::reg8(x86::sreg8(240 /*0xf0*/));
    // 00523bde  08d8                   -or al, bl
    cpu.al |= x86::reg8(x86::sreg8(cpu.bl));
    // 00523be0  8802                   -mov byte ptr [edx], al
    app->getMemory<x86::reg8>(cpu.edx) = cpu.al;
    // 00523be2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523be3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523be4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523be5  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00523be6:
    // 00523be6  66891a                 -mov word ptr [edx], bx
    app->getMemory<x86::reg16>(cpu.edx) = cpu.bx;
    // 00523be9  c1eb10                 -shr ebx, 0x10
    cpu.ebx >>= 16 /*0x10*/ % 32;
    // 00523bec  885a02                 -mov byte ptr [edx + 2], bl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(2) /* 0x2 */) = cpu.bl;
    // 00523bef  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bf0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bf1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523bf2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_523c00: native, nfs3hp::watcom::mbtowc */

/* sub_523ca0: native, nfs3hp::watcom::FDFS */

/* sub_523cec: native, nfs3hp::watcom::U8M */

/* sub_523d10: native, nfs3hp::watcom::mbsnicmp */

/* sub_523d90: native, nfs3hp::port::platformPacketSize */

/* sub_523d94: native, nfs3hp::port::iSNDplatformpacketplay */

/* sub_523f90: native, nfs3hp::port::iSNDcheckplayopts */

/* sub_524190: native, nfs3hp::watcom::EnterWVIDEO */

/* sub_5241c0: native, nfs3hp::watcom::nheapshrink */

/* sub_5241fc: native, nfs3hp::watcom::ReturnMemToSystem */

/* sub_52425c: native, nfs3hp::watcom::ReleaseNHeap */

/* sub_524290: native, nfs3hp::watcom::callMain */

/* sub_5242f0: native, nfs3hp::watcom::wcstombs */

/* sub_5243a7: native, nfs3hp::watcom::reporterror */

/* sub_5243b1: native, nfs3hp::watcom::reportError */

/* align: skip  */
void Application::sub_52448c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052448c  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052448d  668b5008               -mov dx, word ptr [eax + 8]
    cpu.dx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00524491  80e67f                 -and dh, 0x7f
    cpu.dh &= x86::reg8(x86::sreg8(127 /*0x7f*/));
    // 00524494  6681faff7f             +cmp dx, 0x7fff
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(32767 /*0x7fff*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00524499  751c                   -jne 0x5244b7
    if (!cpu.flags.zf)
    {
        goto L_0x005244b7;
    }
    // 0052449b  81780400000080         +cmp dword ptr [eax + 4], 0x80000000
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005244a2  750c                   -jne 0x5244b0
    if (!cpu.flags.zf)
    {
        goto L_0x005244b0;
    }
    // 005244a4  833800                 +cmp dword ptr [eax], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005244a7  7507                   -jne 0x5244b0
    if (!cpu.flags.zf)
    {
        goto L_0x005244b0;
    }
    // 005244a9  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 005244ae  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005244af  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005244b0:
    // 005244b0  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 005244b5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005244b6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005244b7:
    // 005244b7  66f74008ff7f           +test word ptr [eax + 8], 0x7fff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */) & 32767 /*0x7fff*/));
    // 005244bd  7516                   -jne 0x5244d5
    if (!cpu.flags.zf)
    {
        goto L_0x005244d5;
    }
    // 005244bf  83780400               +cmp dword ptr [eax + 4], 0
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
    // 005244c3  7509                   -jne 0x5244ce
    if (!cpu.flags.zf)
    {
        goto L_0x005244ce;
    }
    // 005244c5  833800                 +cmp dword ptr [eax], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005244c8  7504                   -jne 0x5244ce
    if (!cpu.flags.zf)
    {
        goto L_0x005244ce;
    }
    // 005244ca  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005244cc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005244cd  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005244ce:
    // 005244ce  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 005244d3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005244d4  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005244d5:
    // 005244d5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 005244da  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005244db  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_5244e0: native, nfs3hp::graphics::driverIdle */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_5244f0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005244f0  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 005244f2  e93931fcff             -jmp 0x4e7630
    return nfs3hp::port::runSystemTasks(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_524500(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524500  8b542404               -mov edx, dword ptr [esp + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00524504  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00524506  741f                   -je 0x524527
    if (cpu.flags.zf)
    {
        goto L_0x00524527;
    }
    // 00524508  68d0389f00             -push 0x9f38d0
    app->getMemory<x86::reg32>(cpu.esp-4) = 10434768 /*0x9f38d0*/;
    cpu.esp -= 4;
    // 0052450d  6828b95600             -push 0x56b928
    app->getMemory<x86::reg32>(cpu.esp-4) = 5683496 /*0x56b928*/;
    cpu.esp -= 4;
    // 00524512  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 00524514  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00524515  ff10                   -call dword ptr [eax]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524517  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00524519  7403                   -je 0x52451e
    if (cpu.flags.zf)
    {
        goto L_0x0052451e;
    }
    // 0052451b  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x0052451e:
    // 0052451e  ff053c3d9f00           -inc dword ptr [0x9f3d3c]
    (app->getMemory<x86::reg32>(x86::reg32(10435900) /* 0x9f3d3c */))++;
    // 00524524  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00524527:
    // 00524527  a1d0389f00             -mov eax, dword ptr [0x9f38d0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10434768) /* 0x9f38d0 */);
    // 0052452c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052452d  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 0052452f  ff5208                 -call dword ptr [edx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524532  8b0d3c3d9f00           -mov ecx, dword ptr [0x9f3d3c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10435900) /* 0x9f3d3c */);
    // 00524538  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0052453a  41                     -inc ecx
    (cpu.ecx)++;
    // 0052453b  8915d0389f00           -mov dword ptr [0x9f38d0], edx
    app->getMemory<x86::reg32>(x86::reg32(10434768) /* 0x9f38d0 */) = cpu.edx;
    // 00524541  890d3c3d9f00           -mov dword ptr [0x9f3d3c], ecx
    app->getMemory<x86::reg32>(x86::reg32(10435900) /* 0x9f3d3c */) = cpu.ecx;
    // 00524547  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_524550(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524550  837c240400             +cmp dword ptr [esp + 4], 0
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
    // 00524555  741b                   -je 0x524572
    if (cpu.flags.zf)
    {
        goto L_0x00524572;
    }
    // 00524557  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00524558  8b1d80445600           -mov ebx, dword ptr [0x564480]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653632) /* 0x564480 */);
    // 0052455e  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052455f  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524565  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524566  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0052456a  a380715600             -mov dword ptr [0x567180], eax
    app->getMemory<x86::reg32>(x86::reg32(5665152) /* 0x567180 */) = cpu.eax;
    // 0052456f  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00524572:
    // 00524572  8b0d80445600           -mov ecx, dword ptr [0x564480]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5653632) /* 0x564480 */);
    // 00524578  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00524579  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052457f  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00524583  a380715600             -mov dword ptr [0x567180], eax
    app->getMemory<x86::reg32>(x86::reg32(5665152) /* 0x567180 */) = cpu.eax;
    // 00524588  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_5245a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 005245a0  8b542404               -mov edx, dword ptr [esp + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 005245a4  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 005245a8  83fa03                 +cmp edx, 3
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
    // 005245ab  7710                   -ja 0x5245bd
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x005245bd;
    }
    // 005245ad  ff24958c455200         -jmp dword ptr [edx*4 + 0x52458c]
    cpu.ip = app->getMemory<x86::reg32>(5391756 + cpu.edx * 4); goto dynamic_jump;
  case 0x005245b4:
    // 005245b4  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 005245b5  e856caedff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005245ba  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x005245bd:
    // 005245bd  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
  case 0x005245c0:
    // 005245c0  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 005245c1  e82ac3fbff             -call 0x4e08f0
    cpu.esp -= 4;
    nfs3hp::ea::monoPrintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005245c6  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 005245c9  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
  case 0x005245cc:
    // 005245cc  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 005245cd  e8eeb0fbff             -call 0x4df6c0
    cpu.esp -= 4;
    nfs3hp::Message(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005245d2  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 005245d5  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
  case 0x005245d8:
    // 005245d8  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 005245d9  2eff159c455300         -call dword ptr cs:[0x53459c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457308) /* 0x53459c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005245e0  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_5245f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005245f0  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 005245f4  83f801                 +cmp eax, 1
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
    // 005245f7  7210                   -jb 0x524609
    if (cpu.flags.cf)
    {
        goto L_0x00524609;
    }
    // 005245f9  760e                   -jbe 0x524609
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00524609;
    }
    // 005245fb  83f802                 +cmp eax, 2
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
    // 005245fe  7509                   -jne 0x524609
    if (!cpu.flags.zf)
    {
        goto L_0x00524609;
    }
    // 00524600  833d8471560000         +cmp dword ptr [0x567184], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5665156) /* 0x567184 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00524607  7503                   -jne 0x52460c
    if (!cpu.flags.zf)
    {
        goto L_0x0052460c;
    }
L_0x00524609:
    // 00524609  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x0052460c:
    // 0052460c  ff1584715600           -call dword ptr [0x567184]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5665156) /* 0x567184 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524612  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_524620(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524620  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00524621  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00524626  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00524628  e823000000             -call 0x524650
    cpu.esp -= 4;
    nfs3hp::graphics::closeDriver(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052462d  891518445600           -mov dword ptr [0x564418], edx
    app->getMemory<x86::reg32>(x86::reg32(5653528) /* 0x564418 */) = cpu.edx;
    // 00524633  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524634  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_524640: native, nfs3hp::graphics::closeDriverAtExit */

/* align: skip  */
void Application::sub_52468f(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x0052468f;
    // 00524640  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00524645  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 0052464b  8d5200                 -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
    // 0052464e  8bdb                   -mov ebx, ebx
    cpu.ebx = cpu.ebx;
    // 00524650  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00524651  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00524653  8a25f0af5600           -mov ah, byte ptr [0x56aff0]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(5681136) /* 0x56aff0 */);
    // 00524659  84e4                   +test ah, ah
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & cpu.ah));
    // 0052465b  0f844a010000           -je 0x5247ab
    if (cpu.flags.zf)
    {
        return sub_5247ab(app, cpu.handOver());
    }
    // 00524661  833d3cf99e0000         +cmp dword ptr [0x9ef93c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10418492) /* 0x9ef93c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00524668  740a                   -je 0x524674
    if (cpu.flags.zf)
    {
        goto L_0x00524674;
    }
    // 0052466a  b8e0445200             -mov eax, 0x5244e0
    cpu.eax = 5391584 /*0x5244e0*/;
    // 0052466f  e86c2ffcff             -call 0x4e75e0
    cpu.esp -= 4;
    nfs3hp::port::removeSystemTask(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00524674:
    // 00524674  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00524675  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00524676  ff157cf99e00           -call dword ptr [0x9ef97c]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(10418556) /* 0x9ef97c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052467c  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0052467e  7405                   -je 0x524685
    if (cpu.flags.zf)
    {
        goto L_0x00524685;
    }
    // 00524680  e81b94faff             -call 0x4cdaa0
    cpu.esp -= 4;
    sub_4cdaa0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00524685:
    // 00524685  30f6                   -xor dh, dh
    cpu.dh ^= x86::reg8(x86::sreg8(cpu.dh));
    // 00524687  8835f0af5600           -mov byte ptr [0x56aff0], dh
    app->getMemory<x86::reg8>(x86::reg32(5681136) /* 0x56aff0 */) = cpu.dh;
    // 0052468d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052468e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_entry_0x0052468f:
    // 0052468f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00524691  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524692  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_524650: native, nfs3hp::graphics::closeDriver */

/* sub_5246a0: native, nfs3hp::graphics::openDriver */

/* align: skip  */
void Application::sub_5247ab(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005247ab  0f84defeffff           -je 0x52468f
    if (cpu.flags.zf)
    {
    return sub_52468f(app, cpu.handOver());
    }
    // 005247b1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 005247b6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005247b7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_5247c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005247c0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005247c2  7505                   -jne 0x5247c9
    if (!cpu.flags.zf)
    {
        goto L_0x005247c9;
    }
    // 005247c4  b880020000             -mov eax, 0x280
    cpu.eax = 640 /*0x280*/;
L_0x005247c9:
    // 005247c9  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 005247cb  7505                   -jne 0x5247d2
    if (!cpu.flags.zf)
    {
        goto L_0x005247d2;
    }
    // 005247cd  bae0010000             -mov edx, 0x1e0
    cpu.edx = 480 /*0x1e0*/;
L_0x005247d2:
    // 005247d2  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 005247d4  7505                   -jne 0x5247db
    if (!cpu.flags.zf)
    {
        goto L_0x005247db;
    }
    // 005247d6  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x005247db:
    // 005247db  891d8c435600           -mov dword ptr [0x56438c], ebx
    app->getMemory<x86::reg32>(x86::reg32(5653388) /* 0x56438c */) = cpu.ebx;
    // 005247e1  8b1d9c435600           -mov ebx, dword ptr [0x56439c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653404) /* 0x56439c */);
    // 005247e7  a384435600             -mov dword ptr [0x564384], eax
    app->getMemory<x86::reg32>(x86::reg32(5653380) /* 0x564384 */) = cpu.eax;
    // 005247ec  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 005247ee  a3a4435600             -mov dword ptr [0x5643a4], eax
    app->getMemory<x86::reg32>(x86::reg32(5653412) /* 0x5643a4 */) = cpu.eax;
    // 005247f3  a1a0435600             -mov eax, dword ptr [0x5643a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653408) /* 0x5643a0 */);
    // 005247f8  891588435600           -mov dword ptr [0x564388], edx
    app->getMemory<x86::reg32>(x86::reg32(5653384) /* 0x564388 */) = cpu.edx;
    // 005247fe  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00524800  8915a8435600           -mov dword ptr [0x5643a8], edx
    app->getMemory<x86::reg32>(x86::reg32(5653416) /* 0x5643a8 */) = cpu.edx;
    // 00524806  833d6443560000         +cmp dword ptr [0x564364], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653348) /* 0x564364 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052480d  0f842d51feff           -je 0x509940
    if (cpu.flags.zf)
    {
    return sub_509940(app, cpu.handOver());
    }
    // 00524813  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_524820: native, nfs3hp::graphics::dead_0x524820 */

/* sub_5248f0: native, nfs3hp::graphics::unlockFrameBuffer */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_524930(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524930  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00524931  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00524933  e8e863fcff             -call 0x4ead20
    cpu.esp -= 4;
    sub_4ead20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524938  8b4120                 -mov eax, dword ptr [ecx + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */);
    // 0052493b  e870e3fcff             -call 0x4f2cb0
    cpu.esp -= 4;
    sub_4f2cb0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524940  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524941  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_524950(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524950  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00524951  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00524953  e8c863fcff             -call 0x4ead20
    cpu.esp -= 4;
    sub_4ead20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524958  8b4120                 -mov eax, dword ptr [ecx + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */);
    // 0052495b  e860c0fcff             -call 0x4f09c0
    cpu.esp -= 4;
    sub_4f09c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524960  e82b64fcff             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524965  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524966  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_524970(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524970  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00524971  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00524972  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00524973  81ec44010000           -sub esp, 0x144
    (cpu.esp) -= x86::reg32(x86::sreg32(324 /*0x144*/));
    // 00524979  8bbc2458010000         -mov edi, dword ptr [esp + 0x158]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(344) /* 0x158 */);
    // 00524980  89842430010000         -mov dword ptr [esp + 0x130], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(304) /* 0x130 */) = cpu.eax;
    // 00524987  89942440010000         -mov dword ptr [esp + 0x140], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(320) /* 0x140 */) = cpu.edx;
    // 0052498e  89dd                   -mov ebp, ebx
    cpu.ebp = cpu.ebx;
    // 00524990  898c243c010000         -mov dword ptr [esp + 0x13c], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(316) /* 0x13c */) = cpu.ecx;
    // 00524997  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052499c  e85ba6fdff             -call 0x4feffc
    cpu.esp -= 4;
    sub_4feffc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005249a1  89842434010000         -mov dword ptr [esp + 0x134], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(308) /* 0x134 */) = cpu.eax;
    // 005249a8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005249aa  8b942454010000         -mov edx, dword ptr [esp + 0x154]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(340) /* 0x154 */);
    // 005249b1  e846a6fdff             -call 0x4feffc
    cpu.esp -= 4;
    sub_4feffc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005249b6  8b84243c010000         -mov eax, dword ptr [esp + 0x13c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(316) /* 0x13c */);
    // 005249bd  8b0df4435600           -mov ecx, dword ptr [0x5643f4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 005249c3  8b1d8c435600           -mov ebx, dword ptr [0x56438c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653388) /* 0x56438c */);
    // 005249c9  e8c2a7fcff             -call 0x4ef190
    cpu.esp -= 4;
    sub_4ef190(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005249ce  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 005249d0  89842438010000         -mov dword ptr [esp + 0x138], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(312) /* 0x138 */) = cpu.eax;
    // 005249d7  8d8424b8000000         -lea eax, [esp + 0xb8]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 005249de  e8edfcfcff             -call 0x4f46d0
    cpu.esp -= 4;
    sub_4f46d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005249e3  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 005249e5  e856fdfcff             -call 0x4f4740
    cpu.esp -= 4;
    sub_4f4740(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005249ea  b8e4625600             -mov eax, 0x5662e4
    cpu.eax = 5661412 /*0x5662e4*/;
    // 005249ef  e82cabfcff             -call 0x4ef520
    cpu.esp -= 4;
    sub_4ef520(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005249f4  8b842454010000         -mov eax, dword ptr [esp + 0x154]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(340) /* 0x154 */);
    // 005249fb  01e8                   -add eax, ebp
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebp));
    // 005249fd  8b8c243c010000         -mov ecx, dword ptr [esp + 0x13c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(316) /* 0x13c */);
    // 00524a04  89842428010000         -mov dword ptr [esp + 0x128], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(296) /* 0x128 */) = cpu.eax;
    // 00524a0b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00524a0c  8b842444010000         -mov eax, dword ptr [esp + 0x144]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(324) /* 0x144 */);
    // 00524a13  8b942444010000         -mov edx, dword ptr [esp + 0x144]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(324) /* 0x144 */);
    // 00524a1a  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00524a1c  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 00524a1e  89842430010000         -mov dword ptr [esp + 0x130], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(304) /* 0x130 */) = cpu.eax;
    // 00524a25  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00524a27  b82c505600             -mov eax, 0x56502c
    cpu.eax = 5656620 /*0x56502c*/;
    // 00524a2c  e88f6c0000             -call 0x52b6c0
    cpu.esp -= 4;
    sub_52b6c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524a31  ff1560445600           -call dword ptr [0x564460]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5653600) /* 0x564460 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524a37  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00524a39  8b942440010000         -mov edx, dword ptr [esp + 0x140]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(320) /* 0x140 */);
    // 00524a40  e85b63fcff             -call 0x4eada0
    cpu.esp -= 4;
    sub_4eada0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524a45  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 00524a47  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00524a49  8b8c2454010000         -mov ecx, dword ptr [esp + 0x154]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(340) /* 0x154 */);
    // 00524a50  e8dbfeffff             -call 0x524930
    cpu.esp -= 4;
    sub_524930(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524a55  e8c662fcff             -call 0x4ead20
    cpu.esp -= 4;
    sub_4ead20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524a5a  b8000000ff             -mov eax, 0xff000000
    cpu.eax = 4278190080 /*0xff000000*/;
    // 00524a5f  8b9c243c010000         -mov ebx, dword ptr [esp + 0x13c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(316) /* 0x13c */);
    // 00524a66  e8f5acfcff             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524a6b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00524a6c  89ea                   -mov edx, ebp
    cpu.edx = cpu.ebp;
    // 00524a6e  8b842444010000         -mov eax, dword ptr [esp + 0x144]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(324) /* 0x144 */);
    // 00524a75  e8c68afdff             -call 0x4fd540
    cpu.esp -= 4;
    sub_4fd540(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524a7a  b80000aaff             -mov eax, 0xffaa0000
    cpu.eax = 4289331200 /*0xffaa0000*/;
    // 00524a7f  8b8c2428010000         -mov ecx, dword ptr [esp + 0x128]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(296) /* 0x128 */);
    // 00524a86  8b9c242c010000         -mov ebx, dword ptr [esp + 0x12c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(300) /* 0x12c */);
    // 00524a8d  e8ceacfcff             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524a92  8d5504                 -lea edx, [ebp + 4]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(4) /* 0x4 */);
    // 00524a95  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00524a96  83e904                 -sub ecx, 4
    (cpu.ecx) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00524a99  8b842444010000         -mov eax, dword ptr [esp + 0x144]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(324) /* 0x144 */);
    // 00524aa0  83eb05                 -sub ebx, 5
    (cpu.ebx) -= x86::reg32(x86::sreg32(5 /*0x5*/));
    // 00524aa3  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00524aa6  e8756c0000             -call 0x52b720
    cpu.esp -= 4;
    sub_52b720(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524aab  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 00524ab0  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00524ab2  e8a9acfcff             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524ab7  e874adfcff             -call 0x4ef830
    cpu.esp -= 4;
    sub_4ef830(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524abc  8b842430010000         -mov eax, dword ptr [esp + 0x130]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(304) /* 0x130 */);
    // 00524ac3  8d5509                 -lea edx, [ebp + 9]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(9) /* 0x9 */);
    // 00524ac6  e8956d0000             -call 0x52b860
    cpu.esp -= 4;
    sub_52b860(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524acb  ff1564445600           -call dword ptr [0x564464]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5653604) /* 0x564464 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524ad1  b82c505600             -mov eax, 0x56502c
    cpu.eax = 5656620 /*0x56502c*/;
    // 00524ad6  e8d56d0000             -call 0x52b8b0
    cpu.esp -= 4;
    sub_52b8b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524adb  e8b062fcff             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00524ae0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00524ae0  ff1568445600           -call dword ptr [0x564468]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5653608) /* 0x564468 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524ae6  85f8                   +test eax, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.edi));
    // 00524ae8  75f6                   -jne 0x524ae0
    if (!cpu.flags.zf)
    {
        goto L_0x00524ae0;
    }
L_0x00524aea:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00524aea  e8e1b5fcff             -call 0x4f00d0
    cpu.esp -= 4;
    sub_4f00d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524aef  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00524af1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00524af3  ff1568445600           -call dword ptr [0x564468]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5653608) /* 0x564468 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524af9  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00524afb  21fe                   -and esi, edi
    cpu.esi &= x86::reg32(x86::sreg32(cpu.edi));
    // 00524afd  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00524aff  0f8497000000           -je 0x524b9c
    if (cpu.flags.zf)
    {
        goto L_0x00524b9c;
    }
L_0x00524b05:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00524b05  80c920                 -or cl, 0x20
    cpu.cl |= x86::reg8(x86::sreg8(32 /*0x20*/));
    // 00524b08  83e601                 -and esi, 1
    cpu.esi &= x86::reg32(x86::sreg32(1 /*0x1*/));
    // 00524b0b  83f979                 +cmp ecx, 0x79
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(121 /*0x79*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00524b0e  7505                   -jne 0x524b15
    if (!cpu.flags.zf)
    {
        goto L_0x00524b15;
    }
    // 00524b10  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
L_0x00524b15:
    // 00524b15  8b842454010000         -mov eax, dword ptr [esp + 0x154]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(340) /* 0x154 */);
    // 00524b1c  8b8c2440010000         -mov ecx, dword ptr [esp + 0x140]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(320) /* 0x140 */);
    // 00524b23  8b9c243c010000         -mov ebx, dword ptr [esp + 0x13c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(316) /* 0x13c */);
    // 00524b2a  8b942440010000         -mov edx, dword ptr [esp + 0x140]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(320) /* 0x140 */);
    // 00524b31  01e8                   -add eax, ebp
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebp));
    // 00524b33  01d9                   -add ecx, ebx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00524b35  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00524b36  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 00524b38  b82c505600             -mov eax, 0x56502c
    cpu.eax = 5656620 /*0x56502c*/;
    // 00524b3d  e87e6b0000             -call 0x52b6c0
    cpu.esp -= 4;
    sub_52b6c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524b42  ff155c445600           -call dword ptr [0x56445c]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5653596) /* 0x56445c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524b48  8b942440010000         -mov edx, dword ptr [esp + 0x140]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(320) /* 0x140 */);
    // 00524b4f  8b842438010000         -mov eax, dword ptr [esp + 0x138]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(312) /* 0x138 */);
    // 00524b56  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 00524b58  e8f3fdffff             -call 0x524950
    cpu.esp -= 4;
    sub_524950(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524b5d  ff1564445600           -call dword ptr [0x564464]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5653604) /* 0x564464 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524b63  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 00524b65  e8f6fbfcff             -call 0x4f4760
    cpu.esp -= 4;
    sub_4f4760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524b6a  8d8424b8000000         -lea eax, [esp + 0xb8]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00524b71  e88afbfcff             -call 0x4f4700
    cpu.esp -= 4;
    sub_4f4700(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524b76  8b842438010000         -mov eax, dword ptr [esp + 0x138]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(312) /* 0x138 */);
    // 00524b7d  e87eb3fcff             -call 0x4eff00
    cpu.esp -= 4;
    sub_4eff00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524b82  8b842434010000         -mov eax, dword ptr [esp + 0x134]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(308) /* 0x134 */);
    // 00524b89  e86ea4fdff             -call 0x4feffc
    cpu.esp -= 4;
    sub_4feffc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524b8e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00524b90  81c444010000           -add esp, 0x144
    (cpu.esp) += x86::reg32(x86::sreg32(324 /*0x144*/));
    // 00524b96  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524b97  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524b98  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524b99  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x00524b9c:
    // 00524b9c  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00524b9e  0f8561ffffff           -jne 0x524b05
    if (!cpu.flags.zf)
    {
        goto L_0x00524b05;
    }
    // 00524ba4  e941ffffff             -jmp 0x524aea
    goto L_0x00524aea;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_524bb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524bb0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00524bb1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00524bb2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00524bb3  833d6443560000         +cmp dword ptr [0x564364], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653348) /* 0x564364 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00524bba  7410                   -je 0x524bcc
    if (cpu.flags.zf)
    {
        goto L_0x00524bcc;
    }
    // 00524bbc  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00524bbe  e86dadfbff             -call 0x4df930
    cpu.esp -= 4;
    nfs3hp::ea::isThread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524bc3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00524bc5  7442                   -je 0x524c09
    if (cpu.flags.zf)
    {
        goto L_0x00524c09;
    }
    // 00524bc7  e8c461fcff             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00524bcc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00524bcc  a188435600             -mov eax, dword ptr [0x564388]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653384) /* 0x564388 */);
    // 00524bd1  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00524bd3  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00524bd6  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00524bd8  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00524bda  8d58f8                 -lea ebx, [eax - 8]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(-8) /* -0x8 */);
    // 00524bdd  a184435600             -mov eax, dword ptr [0x564384]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653380) /* 0x564384 */);
    // 00524be2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00524be4  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00524be7  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00524be9  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00524beb  6a07                   -push 7
    app->getMemory<x86::reg32>(cpu.esp-4) = 7 /*0x7*/;
    cpu.esp -= 4;
    // 00524bed  6a18                   -push 0x18
    app->getMemory<x86::reg32>(cpu.esp-4) = 24 /*0x18*/;
    cpu.esp -= 4;
    // 00524bef  8d50b0                 -lea edx, [eax - 0x50]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(-80) /* -0x50 */);
    // 00524bf2  b9a0000000             -mov ecx, 0xa0
    cpu.ecx = 160 /*0xa0*/;
    // 00524bf7  b8741e5500             -mov eax, 0x551e74
    cpu.eax = 5578356 /*0x551e74*/;
    // 00524bfc  e86ffdffff             -call 0x524970
    cpu.esp -= 4;
    sub_524970(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524c01  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00524c03  7520                   -jne 0x524c25
    if (!cpu.flags.zf)
    {
        goto L_0x00524c25;
    }
    // 00524c05  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524c06  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524c07  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524c08  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00524c09:
    // 00524c09  8b0d80445600           -mov ecx, dword ptr [0x564480]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5653632) /* 0x564480 */);
    // 00524c0f  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00524c10  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524c16  8b1d80445600           -mov ebx, dword ptr [0x564480]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653632) /* 0x564480 */);
    // 00524c1c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00524c1d  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524c23  eba7                   -jmp 0x524bcc
    goto L_0x00524bcc;
L_0x00524c25:
    // 00524c25  e853defcff             -call 0x4f2a7d
    cpu.esp -= 4;
    sub_4f2a7d(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524c2a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524c2b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524c2c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524c2d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_524c30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524c30  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00524c31  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00524c32  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00524c33  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00524c34  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00524c35  81ec2c010000           -sub esp, 0x12c
    (cpu.esp) -= x86::reg32(x86::sreg32(300 /*0x12c*/));
    // 00524c3b  89842428010000         -mov dword ptr [esp + 0x128], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(296) /* 0x128 */) = cpu.eax;
    // 00524c42  89d5                   -mov ebp, edx
    cpu.ebp = cpu.edx;
    // 00524c44  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 00524c49  e8aea3fdff             -call 0x4feffc
    cpu.esp -= 4;
    sub_4feffc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524c4e  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00524c50  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00524c52  ba0a000000             -mov edx, 0xa
    cpu.edx = 10 /*0xa*/;
    // 00524c57  e8a0a3fdff             -call 0x4feffc
    cpu.esp -= 4;
    sub_4feffc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524c5c  8b0df4435600           -mov ecx, dword ptr [0x5643f4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 00524c62  8b1d8c435600           -mov ebx, dword ptr [0x56438c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653388) /* 0x56438c */);
    // 00524c68  a184435600             -mov eax, dword ptr [0x564384]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653380) /* 0x564384 */);
    // 00524c6d  e81ea5fcff             -call 0x4ef190
    cpu.esp -= 4;
    sub_4ef190(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524c72  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00524c74  8d8424b8000000         -lea eax, [esp + 0xb8]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00524c7b  e850fafcff             -call 0x4f46d0
    cpu.esp -= 4;
    sub_4f46d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524c80  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 00524c82  e8b9fafcff             -call 0x4f4740
    cpu.esp -= 4;
    sub_4f4740(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524c87  b8e4625600             -mov eax, 0x5662e4
    cpu.eax = 5661412 /*0x5662e4*/;
    // 00524c8c  e88fa8fcff             -call 0x4ef520
    cpu.esp -= 4;
    sub_4ef520(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524c91  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00524c93  e80861fcff             -call 0x4eada0
    cpu.esp -= 4;
    sub_4eada0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524c98  8b1588435600           -mov edx, dword ptr [0x564388]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653384) /* 0x564388 */);
    // 00524c9e  b82c505600             -mov eax, 0x56502c
    cpu.eax = 5656620 /*0x56502c*/;
    // 00524ca3  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00524ca4  8d5af6                 -lea ebx, [edx - 0xa]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(-10) /* -0xa */);
    // 00524ca7  8b0d84435600           -mov ecx, dword ptr [0x564384]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5653380) /* 0x564384 */);
    // 00524cad  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00524caf  e80c6a0000             -call 0x52b6c0
    cpu.esp -= 4;
    sub_52b6c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524cb4  ff1560445600           -call dword ptr [0x564460]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5653600) /* 0x564460 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524cba  8b1d88435600           -mov ebx, dword ptr [0x564388]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653384) /* 0x564388 */);
    // 00524cc0  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00524cc2  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00524cc4  83eb0a                 -sub ebx, 0xa
    (cpu.ebx) -= x86::reg32(x86::sreg32(10 /*0xa*/));
    // 00524cc7  e864fcffff             -call 0x524930
    cpu.esp -= 4;
    sub_524930(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524ccc  e84f60fcff             -call 0x4ead20
    cpu.esp -= 4;
    sub_4ead20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524cd1  b8000000ff             -mov eax, 0xff000000
    cpu.eax = 4278190080 /*0xff000000*/;
    // 00524cd6  b90a000000             -mov ecx, 0xa
    cpu.ecx = 10 /*0xa*/;
    // 00524cdb  e880aafcff             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524ce0  8b1d84435600           -mov ebx, dword ptr [0x564384]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653380) /* 0x564384 */);
    // 00524ce6  8b1588435600           -mov edx, dword ptr [0x564388]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653384) /* 0x564388 */);
    // 00524cec  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00524ced  83ea0a                 -sub edx, 0xa
    (cpu.edx) -= x86::reg32(x86::sreg32(10 /*0xa*/));
    // 00524cf0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00524cf2  e84988fdff             -call 0x4fd540
    cpu.esp -= 4;
    sub_4fd540(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524cf7  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 00524cfc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00524cfe  e85daafcff             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d03  e828abfcff             -call 0x4ef830
    cpu.esp -= 4;
    sub_4ef830(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d08  8b1588435600           -mov edx, dword ptr [0x564388]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653384) /* 0x564388 */);
    // 00524d0e  8b842428010000         -mov eax, dword ptr [esp + 0x128]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(296) /* 0x128 */);
    // 00524d15  83ea09                 -sub edx, 9
    (cpu.edx) -= x86::reg32(x86::sreg32(9 /*0x9*/));
    // 00524d18  e8436b0000             -call 0x52b860
    cpu.esp -= 4;
    sub_52b860(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d1d  ff1564445600           -call dword ptr [0x564464]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5653604) /* 0x564464 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d23  e86860fcff             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d28  e86360fcff             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d2d  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00524d2f  e8acb9fcff             -call 0x4f06e0
    cpu.esp -= 4;
    sub_4f06e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d34  e8e75ffcff             -call 0x4ead20
    cpu.esp -= 4;
    sub_4ead20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d39  ff155c445600           -call dword ptr [0x56445c]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5653596) /* 0x56445c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d3f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00524d41  8b1d88435600           -mov ebx, dword ptr [0x564388]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653384) /* 0x564388 */);
    // 00524d47  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00524d49  83eb0a                 -sub ebx, 0xa
    (cpu.ebx) -= x86::reg32(x86::sreg32(10 /*0xa*/));
    // 00524d4c  e8fffbffff             -call 0x524950
    cpu.esp -= 4;
    sub_524950(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d51  ff1564445600           -call dword ptr [0x564464]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5653604) /* 0x564464 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d57  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 00524d59  e802fafcff             -call 0x4f4760
    cpu.esp -= 4;
    sub_4f4760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d5e  8d8424b8000000         -lea eax, [esp + 0xb8]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 00524d65  e896f9fcff             -call 0x4f4700
    cpu.esp -= 4;
    sub_4f4700(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d6a  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00524d6c  e88fb1fcff             -call 0x4eff00
    cpu.esp -= 4;
    sub_4eff00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d71  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00524d73  e884a2fdff             -call 0x4feffc
    cpu.esp -= 4;
    sub_4feffc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524d78  81c42c010000           -add esp, 0x12c
    (cpu.esp) += x86::reg32(x86::sreg32(300 /*0x12c*/));
    // 00524d7e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524d7f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524d80  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524d81  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524d82  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524d83  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_524d90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524d90  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00524d91  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00524d92  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00524d93  a188435600             -mov eax, dword ptr [0x564388]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653384) /* 0x564388 */);
    // 00524d98  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00524d9a  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00524d9d  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00524d9f  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00524da1  8d58f8                 -lea ebx, [eax - 8]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(-8) /* -0x8 */);
    // 00524da4  a184435600             -mov eax, dword ptr [0x564384]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653380) /* 0x564384 */);
    // 00524da9  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00524dab  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00524dae  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00524db0  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00524db2  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 00524db4  6a18                   -push 0x18
    app->getMemory<x86::reg32>(cpu.esp-4) = 24 /*0x18*/;
    cpu.esp -= 4;
    // 00524db6  8d9070ffffff           -lea edx, [eax - 0x90]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(-144) /* -0x90 */);
    // 00524dbc  b920010000             -mov ecx, 0x120
    cpu.ecx = 288 /*0x120*/;
    // 00524dc1  b8801e5500             -mov eax, 0x551e80
    cpu.eax = 5578368 /*0x551e80*/;
    // 00524dc6  e8a5fbffff             -call 0x524970
    cpu.esp -= 4;
    sub_524970(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524dcb  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524dcc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524dcd  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524dce  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 */
void Application::sub_524dd0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524dd0  e9fb6a0000             -jmp 0x52b8d0
    return sub_52b8d0(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_524de0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524de0  e93b6d0000             -jmp 0x52bb20
    return sub_52bb20(app, cpu.handOver());
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_524df0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524df0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00524df1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00524df2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00524df3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00524df4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00524df5  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00524df8  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00524dfa  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 00524dfc  83e810                 -sub eax, 0x10
    (cpu.eax) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00524dff  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00524e01  668b5802               -mov bx, word ptr [eax + 2]
    cpu.bx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(2) /* 0x2 */);
    // 00524e05  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 00524e09  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00524e0b  83e00f                 -and eax, 0xf
    cpu.eax &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 00524e0e  8b2c85c0f59e00         -mov ebp, dword ptr [eax*4 + 0x9ef5c0]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(10417600) /* 0x9ef5c0 */ + cpu.eax * 4);
    // 00524e15  8b5538                 -mov edx, dword ptr [ebp + 0x38]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(56) /* 0x38 */);
    // 00524e18  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00524e1a  0f85f2000000           -jne 0x524f12
    if (!cpu.flags.zf)
    {
        goto L_0x00524f12;
    }
L_0x00524e20:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00524e20  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 00524e24  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00524e27  8a5003                 -mov dl, byte ptr [eax + 3]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(3) /* 0x3 */);
    // 00524e2a  8944240c               -mov dword ptr [esp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 00524e2e  f6c240                 +test dl, 0x40
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 64 /*0x40*/));
    // 00524e31  741f                   -je 0x524e52
    if (cpu.flags.zf)
    {
        goto L_0x00524e52;
    }
    // 00524e33  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00524e35  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00524e37  e8a4c2fbff             -call 0x4e10e0
    cpu.esp -= 4;
    nfs3hp::ea::unlinkFreeBlock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524e3c  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 00524e40  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00524e43  8944240c               -mov dword ptr [esp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 00524e47  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 00524e4b  8b54240c               -mov edx, dword ptr [esp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 00524e4f  895008                 -mov dword ptr [eax + 8], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.edx;
L_0x00524e52:
    // 00524e52  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 00524e54  83fe08                 +cmp esi, 8
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
    // 00524e57  7d0e                   -jge 0x524e67
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00524e67;
    }
    // 00524e59  83feff                 +cmp esi, -1
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
    // 00524e5c  0f85bc000000           -jne 0x524f1e
    if (!cpu.flags.zf)
    {
        goto L_0x00524f1e;
    }
    // 00524e62  b900000040             -mov ecx, 0x40000000
    cpu.ecx = 1073741824 /*0x40000000*/;
L_0x00524e67:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00524e67  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00524e69  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00524e6b  e870c7fbff             -call 0x4e15e0
    cpu.esp -= 4;
    nfs3hp::ea::memBlockName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524e70  e87bc3fbff             -call 0x4e11f0
    cpu.esp -= 4;
    nfs3hp::ea::memBlockOverhead(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524e75  8b5528                 -mov edx, dword ptr [ebp + 0x28]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(40) /* 0x28 */);
    // 00524e78  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00524e7a  4a                     -dec edx
    (cpu.edx)--;
    // 00524e7b  83c110                 -add ecx, 0x10
    (cpu.ecx) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00524e7e  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 00524e80  f7d2                   -not edx
    cpu.edx = ~cpu.edx;
    // 00524e82  21d1                   -and ecx, edx
    cpu.ecx &= x86::reg32(x86::sreg32(cpu.edx));
    // 00524e84  8b54240c               -mov edx, dword ptr [esp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 00524e88  2b542408               -sub edx, dword ptr [esp + 8]
    (cpu.edx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */)));
    // 00524e8c  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00524e8e  83ea10                 -sub edx, 0x10
    (cpu.edx) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00524e91  83e910                 -sub ecx, 0x10
    (cpu.ecx) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00524e94  89542404               -mov dword ptr [esp + 4], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 00524e98  39d1                   +cmp ecx, edx
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
    // 00524e9a  7e06                   -jle 0x524ea2
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00524ea2;
    }
    // 00524e9c  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 00524e9e  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00524ea0  29c6                   -sub esi, eax
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.eax));
L_0x00524ea2:
    // 00524ea2  8d0437                 -lea eax, [edi + esi]
    cpu.eax = x86::reg32(cpu.edi + cpu.esi * 1);
    // 00524ea5  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 00524ea8  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 00524eac  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00524eaf  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 00524eb2  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 00524eb4  e83756fcff             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524eb9  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 00524ebd  897004                 -mov dword ptr [eax + 4], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.esi;
    // 00524ec0  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00524ec4  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00524ec6  83f840                 +cmp eax, 0x40
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
    // 00524ec9  7e35                   -jle 0x524f00
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00524f00;
    }
    // 00524ecb  8d7710                 -lea esi, [edi + 0x10]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(16) /* 0x10 */);
    // 00524ece  01ce                   -add esi, ecx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00524ed0  8b4c240c               -mov ecx, dword ptr [esp + 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 00524ed4  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00524ed5  8b5c240c               -mov ebx, dword ptr [esp + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 00524ed9  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00524eda  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00524edc  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00524ede  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00524ee0  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00524ee2  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00524ee4  e837c2fbff             -call 0x4e1120
    cpu.esp -= 4;
    nfs3hp::ea::markMemBlock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524ee9  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00524eeb  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00524eed  e88ec1fbff             -call 0x4e1080
    cpu.esp -= 4;
    nfs3hp::ea::linkFreeBlock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524ef2  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 00524ef6  89700c                 -mov dword ptr [eax + 0xc], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = cpu.esi;
    // 00524ef9  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 00524efd  897008                 -mov dword ptr [eax + 8], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.esi;
L_0x00524f00:
    // 00524f00  8b7538                 -mov esi, dword ptr [ebp + 0x38]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(56) /* 0x38 */);
    // 00524f03  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00524f05  7565                   -jne 0x524f6c
    if (!cpu.flags.zf)
    {
        goto L_0x00524f6c;
    }
    // 00524f07  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00524f09  83c410                 +add esp, 0x10
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
    // 00524f0c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524f0d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524f0e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524f0f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524f10  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524f11  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00524f12:
    // 00524f12  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00524f13  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524f19  e902ffffff             -jmp 0x524e20
    goto L_0x00524e20;
L_0x00524f1e:
    // 00524f1e  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00524f20  7c0a                   -jl 0x524f2c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00524f2c;
    }
    // 00524f22  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 00524f27  e93bffffff             -jmp 0x524e67
    goto L_0x00524e67;
L_0x00524f2c:
    // 00524f2c  833d0c44560000         +cmp dword ptr [0x56440c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653516) /* 0x56440c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00524f33  0f842effffff           -je 0x524e67
    if (cpu.flags.zf)
    {
        goto L_0x00524e67;
    }
    // 00524f39  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00524f3a  baa01e5500             -mov edx, 0x551ea0
    cpu.edx = 5578400 /*0x551ea0*/;
    // 00524f3f  b8b01e5500             -mov eax, 0x551eb0
    cpu.eax = 5578416 /*0x551eb0*/;
    // 00524f44  68c01e5500             -push 0x551ec0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5578432 /*0x551ec0*/;
    cpu.esp -= 4;
    // 00524f49  891590215500           -mov dword ptr [0x552190], edx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.edx;
    // 00524f4f  ba63000000             -mov edx, 0x63
    cpu.edx = 99 /*0x63*/;
    // 00524f54  a394215500             -mov dword ptr [0x552194], eax
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.eax;
    // 00524f59  891598215500           -mov dword ptr [0x552198], edx
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.edx;
    // 00524f5f  e8acc0edff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524f64  83c408                 +add esp, 8
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
    // 00524f67  e9fbfeffff             -jmp 0x524e67
    goto L_0x00524e67;
L_0x00524f6c:
    // 00524f6c  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00524f6d  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524f73  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00524f75  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00524f78  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524f79  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524f7a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524f7b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524f7c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00524f7d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_524f80: native, nfs3hp::dead::stub_0x524f80 */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_524f88(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00524f88  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00524f89  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00524f8a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00524f8b  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00524f8c  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00524f8d  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00524f8e  8b3d54b1a000           -mov edi, dword ptr [0xa0b154]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 00524f94  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00524f96  0f85c1000000           -jne 0x52505d
    if (!cpu.flags.zf)
    {
        goto L_0x0052505d;
    }
    // 00524f9c  8b2d39785600           -mov ebp, dword ptr [0x567839]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(5666873) /* 0x567839 */);
    // 00524fa2  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00524fa4  8a5500                 -mov dl, byte ptr [ebp]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ebp);
    // 00524fa7  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00524fa9  84d2                   +test dl, dl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & cpu.dl));
    // 00524fab  7416                   -je 0x524fc3
    if (cpu.flags.zf)
    {
        goto L_0x00524fc3;
    }
L_0x00524fad:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00524fad  8a30                   -mov dh, byte ptr [eax]
    cpu.dh = app->getMemory<x86::reg8>(cpu.eax);
    // 00524faf  8d5801                 -lea ebx, [eax + 1]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 00524fb2  84f6                   +test dh, dh
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dh & cpu.dh));
    // 00524fb4  7404                   -je 0x524fba
    if (cpu.flags.zf)
    {
        goto L_0x00524fba;
    }
    // 00524fb6  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00524fb8  ebf3                   -jmp 0x524fad
    goto L_0x00524fad;
L_0x00524fba:
    // 00524fba  41                     -inc ecx
    (cpu.ecx)++;
    // 00524fbb  8a33                   -mov dh, byte ptr [ebx]
    cpu.dh = app->getMemory<x86::reg8>(cpu.ebx);
    // 00524fbd  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00524fbf  84f6                   +test dh, dh
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dh & cpu.dh));
    // 00524fc1  75ea                   -jne 0x524fad
    if (!cpu.flags.zf)
    {
        goto L_0x00524fad;
    }
L_0x00524fc3:
    // 00524fc3  893d54b1a000           -mov dword ptr [0xa0b154], edi
    app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */) = cpu.edi;
    // 00524fc9  29e8                   +sub eax, ebp
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00524fcb  7505                   -jne 0x524fd2
    if (!cpu.flags.zf)
    {
        goto L_0x00524fd2;
    }
    // 00524fcd  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x00524fd2:
    // 00524fd2  e82929fdff             -call 0x4f7900
    cpu.esp -= 4;
    nfs3hp::watcom::nmalloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524fd7  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00524fd9  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00524fdb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00524fdd  0f8475000000           -je 0x525058
    if (cpu.flags.zf)
    {
        goto L_0x00525058;
    }
    // 00524fe3  a33082a100             -mov dword ptr [0xa18230], eax
    app->getMemory<x86::reg32>(x86::reg32(10584624) /* 0xa18230 */) = cpu.eax;
    // 00524fe8  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 00524fef  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00524ff2  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00524ff4  e80729fdff             -call 0x4f7900
    cpu.esp -= 4;
    nfs3hp::watcom::nmalloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00524ff9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00524ffb  7454                   -je 0x525051
    if (cpu.flags.zf)
    {
        goto L_0x00525051;
    }
    // 00524ffd  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00524fff  8a5500                 -mov dl, byte ptr [ebp]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ebp);
    // 00525002  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00525004  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00525006  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00525008  84d2                   +test dl, dl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & cpu.dl));
    // 0052500a  741a                   -je 0x525026
    if (cpu.flags.zf)
    {
        goto L_0x00525026;
    }
L_0x0052500c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052500c  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0052500e  891c11                 -mov dword ptr [ecx + edx], ebx
    app->getMemory<x86::reg32>(cpu.ecx + cpu.edx * 1) = cpu.ebx;
L_0x00525011:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00525011  43                     -inc ebx
    (cpu.ebx)++;
    // 00525012  8a10                   -mov dl, byte ptr [eax]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax);
    // 00525014  40                     -inc eax
    (cpu.eax)++;
    // 00525015  8853ff                 -mov byte ptr [ebx - 1], dl
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(-1) /* -0x1 */) = cpu.dl;
    // 00525018  84d2                   +test dl, dl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & cpu.dl));
    // 0052501a  75f5                   -jne 0x525011
    if (!cpu.flags.zf)
    {
        goto L_0x00525011;
    }
    // 0052501c  83c104                 -add ecx, 4
    (cpu.ecx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0052501f  8a10                   -mov dl, byte ptr [eax]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax);
    // 00525021  46                     -inc esi
    (cpu.esi)++;
    // 00525022  84d2                   +test dl, dl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & cpu.dl));
    // 00525024  75e6                   -jne 0x52500c
    if (!cpu.flags.zf)
    {
        goto L_0x0052500c;
    }
L_0x00525026:
    // 00525026  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00525028  c7040100000000         -mov dword ptr [ecx + eax], 0
    app->getMemory<x86::reg32>(cpu.ecx + cpu.eax * 1) = 0 /*0x0*/;
    // 0052502f  83c104                 -add ecx, 4
    (cpu.ecx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00525032  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00525034  8d040f                 -lea eax, [edi + ecx]
    cpu.eax = x86::reg32(cpu.edi + cpu.ecx * 1);
    // 00525037  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00525039  a350b1a000             -mov dword ptr [0xa0b150], eax
    app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */) = cpu.eax;
    // 0052503e  893d54b1a000           -mov dword ptr [0xa0b154], edi
    app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */) = cpu.edi;
    // 00525044  e8f7b5fbff             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525049  8b3d54b1a000           -mov edi, dword ptr [0xa0b154]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 0052504f  eb07                   -jmp 0x525058
    goto L_0x00525058;
L_0x00525051:
    // 00525051  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00525053  e89829fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00525058:
    // 00525058  e8536d0000             -call 0x52bdb0
    cpu.esp -= 4;
    sub_52bdb0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052505d:
    // 0052505d  8b3d54b1a000           -mov edi, dword ptr [0xa0b154]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 00525063  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525064  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525065  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525066  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525067  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525068  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525069  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_52506c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052506c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052506d  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052506e  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052506f  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00525070  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00525071  e85a6e0000             -call 0x52bed0
    cpu.esp -= 4;
    sub_52bed0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525076  8b1554b1a000           -mov edx, dword ptr [0xa0b154]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 0052507c  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0052507e  740f                   -je 0x52508f
    if (cpu.flags.zf)
    {
        goto L_0x0052508f;
    }
    // 00525080  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00525082  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00525084  e86729fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525089  891d54b1a000           -mov dword ptr [0xa0b154], ebx
    app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */) = cpu.ebx;
L_0x0052508f:
    // 0052508f  8b0d3082a100           -mov ecx, dword ptr [0xa18230]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10584624) /* 0xa18230 */);
    // 00525095  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00525097  740f                   -je 0x5250a8
    if (cpu.flags.zf)
    {
        goto L_0x005250a8;
    }
    // 00525099  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052509b  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0052509d  e84e29fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005250a2  89353082a100           -mov dword ptr [0xa18230], esi
    app->getMemory<x86::reg32>(x86::reg32(10584624) /* 0xa18230 */) = cpu.esi;
L_0x005250a8:
    // 005250a8  8b3d39785600           -mov edi, dword ptr [0x567839]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5666873) /* 0x567839 */);
    // 005250ae  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 005250b0  7408                   -je 0x5250ba
    if (cpu.flags.zf)
    {
        goto L_0x005250ba;
    }
    // 005250b2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005250b3  2eff15d8445300         -call dword ptr cs:[0x5344d8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457112) /* 0x5344d8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x005250ba:
    // 005250ba  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005250bb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005250bc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005250bd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005250be  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005250bf  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_5250c0: native, nfs3hp::watcom::mbterm */

/* sub_525100: native, nfs3hp::watcom::mbsinc */

/* sub_525130: native, nfs3hp::dead::stub_0x525130 */

/* align: skip 0x00 0x00 0x00 0x00 */
void Application::sub_5251a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005251a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 005251a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005251a2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005251a3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005251a4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005251a5  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 005251a8  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 005251ab  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 005251ad  66813d417856000080     +cmp word ptr [0x567841], 0x8000
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(5666881) /* 0x567841 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(32768 /*0x8000*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 005251b6  730e                   -jae 0x5251c6
    if (!cpu.flags.cf)
    {
        goto L_0x005251c6;
    }
    // 005251b8  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005251b9  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 005251ba  2eff15e0455300         -call dword ptr cs:[0x5345e0]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457376) /* 0x5345e0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005251c1  e9c3000000             -jmp 0x525289
    goto L_0x00525289;
L_0x005251c6:
    // 005251c6  e835010000             -call 0x525300
    cpu.esp -= 4;
    nfs3hp::watcom::wcslen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005251cb  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 005251cd  8d4801                 -lea ecx, [eax + 1]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 005251d0  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 005251d2  e82927fdff             -call 0x4f7900
    cpu.esp -= 4;
    nfs3hp::watcom::nmalloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005251d7  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 005251d9  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 005251db  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 005251dd  0f84a6000000           -je 0x525289
    if (cpu.flags.zf)
    {
        goto L_0x00525289;
    }
    // 005251e3  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 005251e5  7504                   -jne 0x5251eb
    if (!cpu.flags.zf)
    {
        goto L_0x005251eb;
    }
    // 005251e7  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 005251e9  eb2b                   -jmp 0x525216
    goto L_0x00525216;
L_0x005251eb:
    // 005251eb  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 005251ed  e80e010000             -call 0x525300
    cpu.esp -= 4;
    nfs3hp::watcom::wcslen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005251f2  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 005251f4  8d4801                 -lea ecx, [eax + 1]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 005251f7  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 005251f9  e80227fdff             -call 0x4f7900
    cpu.esp -= 4;
    nfs3hp::watcom::nmalloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005251fe  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00525200  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00525202  7512                   -jne 0x525216
    if (!cpu.flags.zf)
    {
        goto L_0x00525216;
    }
    // 00525204  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00525206  e8e527fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052520b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052520d  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00525210  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525211  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525212  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525213  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525214  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525215  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00525216:
    // 00525216  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 00525219  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0052521b  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0052521d  e8cef0ffff             -call 0x5242f0
    cpu.esp -= 4;
    nfs3hp::watcom::wcstombs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525222  83f8ff                 +cmp eax, -1
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
    // 00525225  751d                   -jne 0x525244
    if (!cpu.flags.zf)
    {
        goto L_0x00525244;
    }
    // 00525227  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00525229  e8c227fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052522e  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00525230  7407                   -je 0x525239
    if (cpu.flags.zf)
    {
        goto L_0x00525239;
    }
    // 00525232  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00525234  e8b727fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00525239:
    // 00525239  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052523b  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0052523e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052523f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525240  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525241  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525242  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525243  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00525244:
    // 00525244  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00525246  7422                   -je 0x52526a
    if (cpu.flags.zf)
    {
        goto L_0x0052526a;
    }
    // 00525248  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0052524a  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0052524c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052524e  e89df0ffff             -call 0x5242f0
    cpu.esp -= 4;
    nfs3hp::watcom::wcstombs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525253  83f8ff                 +cmp eax, -1
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
    // 00525256  7512                   -jne 0x52526a
    if (!cpu.flags.zf)
    {
        goto L_0x0052526a;
    }
    // 00525258  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052525a  e89127fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052525f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00525261  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00525264  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525265  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525266  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525267  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525268  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525269  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052526a:
    // 0052526a  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052526b  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052526c  2eff15dc455300         -call dword ptr cs:[0x5345dc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457372) /* 0x5345dc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525273  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00525275  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00525277  e87427fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052527c  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0052527e  7407                   -je 0x525287
    if (cpu.flags.zf)
    {
        goto L_0x00525287;
    }
    // 00525280  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00525282  e86927fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00525287:
    // 00525287  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x00525289:
    // 00525289  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0052528c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052528d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052528e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052528f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525290  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525291  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_5252a0: native, nfs3hp::watcom::create_wide_environment */

/* sub_525300: native, nfs3hp::watcom::wcslen */

/* sub_525320: native, nfs3hp::watcom::memcpy */

/* sub_525350: native, nfs3hp::watcom::wcscpy */

/* sub_525370: native, nfs3hp::watcom::towupper */

/* sub_525380: native, nfs3hp::watcom::msize */

/* sub_525390: native, nfs3hp::watcom::nexpand */

/* sub_5253cc: native, nfs3hp::watcom::HeapManager_expand */

/* sub_525578: native, nfs3hp::watcom::nexpandFailed */

/* sub_525590: native, nfs3hp::dead::stub_0x525590 */

/* align: skip 0x90 */
void Application::sub_525600(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00525600  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00525601  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00525603  83785400               +cmp dword ptr [eax + 0x54], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(84) /* 0x54 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525607  7502                   -jne 0x52560b
    if (!cpu.flags.zf)
    {
        goto L_0x0052560b;
    }
    // 00525609  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052560a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052560b:
    // 0052560b  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052560c  8b0d8c715600           -mov ecx, dword ptr [0x56718c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5665164) /* 0x56718c */);
    // 00525612  8b4054                 -mov eax, dword ptr [eax + 0x54]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(84) /* 0x54 */);
    // 00525615  ff5108                 -call dword ptr [ecx + 8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525618  c7425400000000         -mov dword ptr [edx + 0x54], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(84) /* 0x54 */) = 0 /*0x0*/;
    // 0052561f  c682bc00000001         -mov byte ptr [edx + 0xbc], 1
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(188) /* 0xbc */) = 1 /*0x1*/;
    // 00525626  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525627  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525628  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_525630(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00525630  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00525631  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00525632  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 00525634  42                     -inc edx
    (cpu.edx)++;
    // 00525635  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00525637  8a4aff                 -mov cl, byte ptr [edx - 1]
    cpu.cl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 0052563a  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 0052563c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052563e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052563f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525640  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_525650(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00525650  83f820                 +cmp eax, 0x20
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
    // 00525653  7c12                   -jl 0x525667
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00525667;
    }
    // 00525655  83f87f                 +cmp eax, 0x7f
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(127 /*0x7f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525658  7f0d                   -jg 0x525667
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00525667;
    }
    // 0052565a  668b0445b8af5600       -mov ax, word ptr [eax*2 + 0x56afb8]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(5681080) /* 0x56afb8 */ + cpu.eax * 2);
    // 00525662  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
L_0x00525667:
    // 00525667  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_525670(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00525670  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00525671  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00525672  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00525674  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 00525676  30e4                   -xor ah, ah
    cpu.ah ^= x86::reg8(x86::sreg8(cpu.ah));
    // 00525678  8a02                   -mov al, byte ptr [edx]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx);
    // 0052567a  42                     -inc edx
    (cpu.edx)++;
    // 0052567b  a880                   +test al, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 128 /*0x80*/));
    // 0052567d  7416                   -je 0x525695
    if (cpu.flags.zf)
    {
        goto L_0x00525695;
    }
    // 0052567f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00525680  30ff                   -xor bh, bh
    cpu.bh ^= x86::reg8(x86::sreg8(cpu.bh));
    // 00525682  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 00525685  8a1a                   -mov bl, byte ptr [edx]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx);
    // 00525687  42                     -inc edx
    (cpu.edx)++;
    // 00525688  09d8                   -or eax, ebx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ebx));
    // 0052568a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052568b  8911                   -mov dword ptr [ecx], edx
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.edx;
    // 0052568d  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 00525692  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525693  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525694  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00525695:
    // 00525695  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 0052569a  e8b1ffffff             -call 0x525650
    cpu.esp -= 4;
    sub_525650(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052569f  8911                   -mov dword ptr [ecx], edx
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.edx;
    // 005256a1  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 005256a6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005256a7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005256a8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_5256b0: native, nfs3hp::dead::stub_0x5256b0 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_5257c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 005257c0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005257c1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 005257c2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 005257c3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 005257c4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 005257c6  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 005257c9  83e4f8                 -and esp, 0xfffffff8
    cpu.esp &= x86::reg32(x86::sreg32(4294967288 /*0xfffffff8*/));
    // 005257cc  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 005257ce  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 005257d0  83fa10                 +cmp edx, 0x10
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
    // 005257d3  774a                   -ja 0x52581f
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052581f;
    }
L_0x005257d5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 005257d5  ff249570575200         -jmp dword ptr [edx*4 + 0x525770]
    cpu.ip = app->getMemory<x86::reg32>(5396336 + cpu.edx * 4); goto dynamic_jump;
  case 0x005257dc:
    // 005257dc  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 005257e1  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 005257e3  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 005257e5  e8d6ffffff             -call 0x5257c0
    cpu.esp -= 4;
    sub_5257c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005257ea  bb00000100             -mov ebx, 0x10000
    cpu.ebx = 65536 /*0x10000*/;
    // 005257ef  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 005257f4  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 005257f6  e8c5ffffff             -call 0x5257c0
    cpu.esp -= 4;
    sub_5257c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005257fb  b800000100             -mov eax, 0x10000
    cpu.eax = 65536 /*0x10000*/;
    // 00525800  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 00525805  ebce                   -jmp 0x5257d5
    goto L_0x005257d5;
  case 0x00525807:
    // 00525807  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00525809  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0052580c  897c2404               -mov dword ptr [esp + 4], edi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edi;
    // 00525810  df2c24                 -fild qword ptr [esp]
    fp0 = x86::Float(x86::sreg64(app->getMemory<x86::reg64>(cpu.esp)));
    // 00525813  dc0de01e5500           -fmul qword ptr [0x551ee0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5578464) /* 0x551ee0 */)));
    // 00525819  d999c0000000           -fstp dword ptr [ecx + 0xc0]
    app->getMemory<float>(cpu.ecx + x86::reg32(192) /* 0xc0 */) = float(fp0);
L_0x0052581f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052581f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00525821  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525822  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525823  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525824  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525825  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525826:
    // 00525826  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00525828  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0052582b  89742404               -mov dword ptr [esp + 4], esi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.esi;
    // 0052582f  df2c24                 -fild qword ptr [esp]
    fp0 = x86::Float(x86::sreg64(app->getMemory<x86::reg64>(cpu.esp)));
    // 00525832  dc0de01e5500           -fmul qword ptr [0x551ee0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5578464) /* 0x551ee0 */)));
    // 00525838  d999c4000000           -fstp dword ptr [ecx + 0xc4]
    app->getMemory<float>(cpu.ecx + x86::reg32(196) /* 0xc4 */) = float(fp0);
    // 0052583e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00525840  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525841  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525842  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525843  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525844  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525845:
    // 00525845  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00525847  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0052584a  895c2404               -mov dword ptr [esp + 4], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.ebx;
    // 0052584e  df2c24                 -fild qword ptr [esp]
    fp0 = x86::Float(x86::sreg64(app->getMemory<x86::reg64>(cpu.esp)));
    // 00525851  dc0de01e5500           -fmul qword ptr [0x551ee0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5578464) /* 0x551ee0 */)));
    // 00525857  d999c8000000           -fstp dword ptr [ecx + 0xc8]
    app->getMemory<float>(cpu.ecx + x86::reg32(200) /* 0xc8 */) = float(fp0);
    // 0052585d  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0052585f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525860  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525861  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525862  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525863  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525864:
    // 00525864  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00525866  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 00525869  89542404               -mov dword ptr [esp + 4], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 0052586d  df2c24                 -fild qword ptr [esp]
    fp0 = x86::Float(x86::sreg64(app->getMemory<x86::reg64>(cpu.esp)));
    // 00525870  dc0de01e5500           -fmul qword ptr [0x551ee0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5578464) /* 0x551ee0 */)));
    // 00525876  d8b1c8000000           -fdiv dword ptr [ecx + 0xc8]
    fp0 = fp::div(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(200) /* 0xc8 */)));
    // 0052587c  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0052587f  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00525882  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525883  894118                 -mov dword ptr [ecx + 0x18], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */) = cpu.eax;
    // 00525886  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00525888  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525889  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052588a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052588b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052588c  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052588d:
    // 0052588d  83f801                 +cmp eax, 1
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
    // 00525890  750e                   -jne 0x5258a0
    if (!cpu.flags.zf)
    {
        goto L_0x005258a0;
    }
    // 00525892  8089cc00000001         -or byte ptr [ecx + 0xcc], 1
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(204) /* 0xcc */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 00525899  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0052589b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052589c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052589d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052589e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052589f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005258a0:
    // 005258a0  80a1cc000000fe         -and byte ptr [ecx + 0xcc], 0xfe
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(204) /* 0xcc */) &= x86::reg8(x86::sreg8(254 /*0xfe*/));
    // 005258a7  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 005258a9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258aa  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258ab  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258ac  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258ad  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x005258ae:
    // 005258ae  83f801                 +cmp eax, 1
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
    // 005258b1  750e                   -jne 0x5258c1
    if (!cpu.flags.zf)
    {
        goto L_0x005258c1;
    }
    // 005258b3  8089cc00000002         -or byte ptr [ecx + 0xcc], 2
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(204) /* 0xcc */) |= x86::reg8(x86::sreg8(2 /*0x2*/));
    // 005258ba  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 005258bc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258bd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258be  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258bf  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258c0  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005258c1:
    // 005258c1  80a1cc000000fd         -and byte ptr [ecx + 0xcc], 0xfd
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(204) /* 0xcc */) &= x86::reg8(x86::sreg8(253 /*0xfd*/));
    // 005258c8  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 005258ca  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258cb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258cc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258cd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258ce  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x005258cf:
    // 005258cf  83f801                 +cmp eax, 1
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
    // 005258d2  750e                   -jne 0x5258e2
    if (!cpu.flags.zf)
    {
        goto L_0x005258e2;
    }
    // 005258d4  8089cc00000004         -or byte ptr [ecx + 0xcc], 4
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(204) /* 0xcc */) |= x86::reg8(x86::sreg8(4 /*0x4*/));
    // 005258db  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 005258dd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258de  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258df  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258e0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258e1  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005258e2:
    // 005258e2  80a1cc000000fb         -and byte ptr [ecx + 0xcc], 0xfb
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(204) /* 0xcc */) &= x86::reg8(x86::sreg8(251 /*0xfb*/));
    // 005258e9  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 005258eb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258ec  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258ed  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258ee  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005258ef  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x005258f0:
    // 005258f0  3b416c                 +cmp eax, dword ptr [ecx + 0x6c]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(108) /* 0x6c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005258f3  0f8426ffffff           -je 0x52581f
    if (cpu.flags.zf)
    {
        goto L_0x0052581f;
    }
    // 005258f9  c681bc00000001         -mov byte ptr [ecx + 0xbc], 1
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(188) /* 0xbc */) = 1 /*0x1*/;
    // 00525900  89416c                 -mov dword ptr [ecx + 0x6c], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(108) /* 0x6c */) = cpu.eax;
    // 00525903  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00525905  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525906  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525907  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525908  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525909  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052590a:
    // 0052590a  3b4170                 +cmp eax, dword ptr [ecx + 0x70]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(112) /* 0x70 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052590d  0f840cffffff           -je 0x52581f
    if (cpu.flags.zf)
    {
        goto L_0x0052581f;
    }
    // 00525913  c681bc00000001         -mov byte ptr [ecx + 0xbc], 1
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(188) /* 0xbc */) = 1 /*0x1*/;
    // 0052591a  894170                 -mov dword ptr [ecx + 0x70], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(112) /* 0x70 */) = cpu.eax;
    // 0052591d  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0052591f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525920  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525921  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525922  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525923  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525924:
    // 00525924  3b4174                 +cmp eax, dword ptr [ecx + 0x74]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(116) /* 0x74 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525927  0f84f2feffff           -je 0x52581f
    if (cpu.flags.zf)
    {
        goto L_0x0052581f;
    }
    // 0052592d  c681bc00000001         -mov byte ptr [ecx + 0xbc], 1
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(188) /* 0xbc */) = 1 /*0x1*/;
    // 00525934  894174                 -mov dword ptr [ecx + 0x74], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(116) /* 0x74 */) = cpu.eax;
    // 00525937  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00525939  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052593a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052593b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052593c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052593d  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052593e:
    // 0052593e  3b4178                 +cmp eax, dword ptr [ecx + 0x78]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(120) /* 0x78 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525941  0f84d8feffff           -je 0x52581f
    if (cpu.flags.zf)
    {
        goto L_0x0052581f;
    }
    // 00525947  c681bc00000001         -mov byte ptr [ecx + 0xbc], 1
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(188) /* 0xbc */) = 1 /*0x1*/;
    // 0052594e  894178                 -mov dword ptr [ecx + 0x78], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(120) /* 0x78 */) = cpu.eax;
    // 00525951  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00525953  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525954  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525955  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525956  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525957  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525958:
    // 00525958  894160                 -mov dword ptr [ecx + 0x60], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(96) /* 0x60 */) = cpu.eax;
    // 0052595b  894164                 -mov dword ptr [ecx + 0x64], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(100) /* 0x64 */) = cpu.eax;
    // 0052595e  894168                 -mov dword ptr [ecx + 0x68], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(104) /* 0x68 */) = cpu.eax;
  [[fallthrough]];
  case 0x00525961:
    // 00525961  89415c                 -mov dword ptr [ecx + 0x5c], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(92) /* 0x5c */) = cpu.eax;
    // 00525964  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00525966  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525967  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525968  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525969  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052596a  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052596b:
    // 0052596b  894160                 -mov dword ptr [ecx + 0x60], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(96) /* 0x60 */) = cpu.eax;
    // 0052596e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00525970  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525971  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525972  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525973  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525974  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525975:
    // 00525975  894164                 -mov dword ptr [ecx + 0x64], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(100) /* 0x64 */) = cpu.eax;
    // 00525978  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0052597a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052597b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052597c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052597d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052597e  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052597f:
    // 0052597f  894168                 -mov dword ptr [ecx + 0x68], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(104) /* 0x68 */) = cpu.eax;
    // 00525982  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00525984  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525985  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525986  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525987  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525988  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_5259d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 005259d0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 005259d1  4a                     -dec edx
    (cpu.edx)--;
    // 005259d2  83fa0f                 +cmp edx, 0xf
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(15 /*0xf*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 005259d5  771c                   -ja 0x5259f3
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x005259f3;
    }
    // 005259d7  ff24958c595200         -jmp dword ptr [edx*4 + 0x52598c]
    cpu.ip = app->getMemory<x86::reg32>(5396876 + cpu.edx * 4); goto dynamic_jump;
  case 0x005259de:
    // 005259de  d980c0000000           -fld dword ptr [eax + 0xc0]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(192) /* 0xc0 */));
    // 005259e4  dc0de81e5500           -fmul qword ptr [0x551ee8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5578472) /* 0x551ee8 */)));
    // 005259ea  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 005259ed  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 005259f0  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005259f1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x005259f3:
    // 005259f3  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 005259f5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005259f6  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x005259f7:
    // 005259f7  d980c4000000           -fld dword ptr [eax + 0xc4]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(196) /* 0xc4 */));
    // 005259fd  dc0de81e5500           -fmul qword ptr [0x551ee8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5578472) /* 0x551ee8 */)));
    // 00525a03  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00525a06  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00525a09  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a0a  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00525a0c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525a0e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a0f  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525a10:
    // 00525a10  d980c8000000           -fld dword ptr [eax + 0xc8]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(200) /* 0xc8 */));
    // 00525a16  dc0de81e5500           -fmul qword ptr [0x551ee8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5578472) /* 0x551ee8 */)));
    // 00525a1c  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00525a1f  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00525a22  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a23  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00525a25  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525a27  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a28  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525a29:
    // 00525a29  db4018                 -fild dword ptr [eax + 0x18]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */)));
    // 00525a2c  d888c8000000           -fmul dword ptr [eax + 0xc8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(200) /* 0xc8 */)));
    // 00525a32  dc0de81e5500           -fmul qword ptr [0x551ee8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5578472) /* 0x551ee8 */)));
    // 00525a38  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00525a3b  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00525a3e  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a3f  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00525a41  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525a43  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a44  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525a45:
    // 00525a45  f680cc00000001         +test byte ptr [eax + 0xcc], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(204) /* 0xcc */) & 1 /*0x1*/));
    // 00525a4c  7409                   -je 0x525a57
    if (cpu.flags.zf)
    {
        goto L_0x00525a57;
    }
    // 00525a4e  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00525a53  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525a55  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a56  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00525a57:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00525a57  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00525a59  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525a5b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a5c  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525a5d:
    // 00525a5d  f680cc00000002         +test byte ptr [eax + 0xcc], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(204) /* 0xcc */) & 2 /*0x2*/));
    // 00525a64  74f1                   -je 0x525a57
    if (cpu.flags.zf)
    {
        goto L_0x00525a57;
    }
    // 00525a66  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00525a6b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525a6d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a6e  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525a6f:
    // 00525a6f  f680cc00000004         +test byte ptr [eax + 0xcc], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(204) /* 0xcc */) & 4 /*0x4*/));
    // 00525a76  74df                   -je 0x525a57
    if (cpu.flags.zf)
    {
        goto L_0x00525a57;
    }
    // 00525a78  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00525a7d  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525a7f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a80  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525a81:
    // 00525a81  8b486c                 -mov ecx, dword ptr [eax + 0x6c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(108) /* 0x6c */);
    // 00525a84  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525a86  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a87  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525a88:
    // 00525a88  8b4870                 -mov ecx, dword ptr [eax + 0x70]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(112) /* 0x70 */);
    // 00525a8b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525a8d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a8e  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525a8f:
    // 00525a8f  8b4874                 -mov ecx, dword ptr [eax + 0x74]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(116) /* 0x74 */);
    // 00525a92  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525a94  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a95  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525a96:
    // 00525a96  8b4878                 -mov ecx, dword ptr [eax + 0x78]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(120) /* 0x78 */);
    // 00525a99  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525a9b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525a9c  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525a9d:
    // 00525a9d  8b485c                 -mov ecx, dword ptr [eax + 0x5c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(92) /* 0x5c */);
    // 00525aa0  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525aa2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525aa3  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525aa4:
    // 00525aa4  8b4860                 -mov ecx, dword ptr [eax + 0x60]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(96) /* 0x60 */);
    // 00525aa7  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525aa9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525aaa  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525aab:
    // 00525aab  8b4864                 -mov ecx, dword ptr [eax + 0x64]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */);
    // 00525aae  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525ab0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525ab1  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00525ab2:
    // 00525ab2  8b4868                 -mov ecx, dword ptr [eax + 0x68]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(104) /* 0x68 */);
    // 00525ab5  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525ab7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525ab8  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* sub_525ac0: native, nfs3hp::dead::stub_0x525ac0 */

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_525b80(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00525b80  895850                 -mov dword ptr [eax + 0x50], ebx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(80) /* 0x50 */) = cpu.ebx;
    // 00525b83  89504c                 -mov dword ptr [eax + 0x4c], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(76) /* 0x4c */) = cpu.edx;
    // 00525b86  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_525b90: native, nfs3hp::dead::stub_0x525b90 */

/* align: skip 0x00 0x00 0x00 */
void Application::sub_525ba0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00525ba0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00525ba1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00525ba2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00525ba3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00525ba4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00525ba5  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 00525ba8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00525baa  83f8ff                 +cmp eax, -1
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
    // 00525bad  750e                   -jne 0x525bbd
    if (!cpu.flags.zf)
    {
        goto L_0x00525bbd;
    }
    // 00525baf  2eff15e0445300         -call dword ptr cs:[0x5344e0]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457120) /* 0x5344e0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525bb6  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00525bb8  e998000000             -jmp 0x525c55
    goto L_0x00525c55;
L_0x00525bbd:
    // 00525bbd  83f8fe                 +cmp eax, -2
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-2 /*-0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525bc0  750e                   -jne 0x525bd0
    if (!cpu.flags.zf)
    {
        goto L_0x00525bd0;
    }
    // 00525bc2  2eff1550455300         -call dword ptr cs:[0x534550]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457232) /* 0x534550 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525bc9  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00525bcb  e985000000             -jmp 0x525c55
    goto L_0x00525c55;
L_0x00525bd0:
    // 00525bd0  83f8fd                 +cmp eax, -3
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-3 /*-0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525bd3  7526                   -jne 0x525bfb
    if (!cpu.flags.zf)
    {
        goto L_0x00525bfb;
    }
    // 00525bd5  bb01010000             -mov ebx, 0x101
    cpu.ebx = 257 /*0x101*/;
    // 00525bda  b810b2a000             -mov eax, 0xa0b210
    cpu.eax = 10531344 /*0xa0b210*/;
    // 00525bdf  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00525be1  e85aaafbff             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525be6  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00525be8  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00525bea  891500b2a000           -mov dword ptr [0xa0b200], edx
    app->getMemory<x86::reg32>(x86::reg32(10531328) /* 0xa0b200 */) = cpu.edx;
    // 00525bf0  8915b8b05600           -mov dword ptr [0x56b0b8], edx
    app->getMemory<x86::reg32>(x86::reg32(5681336) /* 0x56b0b8 */) = cpu.edx;
    // 00525bf6  e9ff000000             -jmp 0x525cfa
    goto L_0x00525cfa;
L_0x00525bfb:
    // 00525bfb  83f8fc                 +cmp eax, -4
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-4 /*-0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525bfe  7555                   -jne 0x525c55
    if (!cpu.flags.zf)
    {
        goto L_0x00525c55;
    }
    // 00525c00  bb01010000             -mov ebx, 0x101
    cpu.ebx = 257 /*0x101*/;
    // 00525c05  b810b2a000             -mov eax, 0xa0b210
    cpu.eax = 10531344 /*0xa0b210*/;
    // 00525c0a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00525c0c  e82faafbff             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525c11  b881000000             -mov eax, 0x81
    cpu.eax = 129 /*0x81*/;
    // 00525c16  b201                   -mov dl, 1
    cpu.dl = 1 /*0x1*/;
L_0x00525c18:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00525c18  40                     -inc eax
    (cpu.eax)++;
    // 00525c19  889010b2a000           -mov byte ptr [eax + 0xa0b210], dl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10531344) /* 0xa0b210 */) = cpu.dl;
    // 00525c1f  3d9f000000             +cmp eax, 0x9f
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(159 /*0x9f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525c24  7ef2                   -jle 0x525c18
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00525c18;
    }
    // 00525c26  b8e0000000             -mov eax, 0xe0
    cpu.eax = 224 /*0xe0*/;
    // 00525c2b  b601                   -mov dh, 1
    cpu.dh = 1 /*0x1*/;
L_0x00525c2d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00525c2d  40                     -inc eax
    (cpu.eax)++;
    // 00525c2e  88b010b2a000           -mov byte ptr [eax + 0xa0b210], dh
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10531344) /* 0xa0b210 */) = cpu.dh;
    // 00525c34  3dfc000000             +cmp eax, 0xfc
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(252 /*0xfc*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525c39  7ef2                   -jle 0x525c2d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00525c2d;
    }
    // 00525c3b  bd01000000             -mov ebp, 1
    cpu.ebp = 1 /*0x1*/;
    // 00525c40  b8a4030000             -mov eax, 0x3a4
    cpu.eax = 932 /*0x3a4*/;
    // 00525c45  892d00b2a000           -mov dword ptr [0xa0b200], ebp
    app->getMemory<x86::reg32>(x86::reg32(10531328) /* 0xa0b200 */) = cpu.ebp;
    // 00525c4b  a3b8b05600             -mov dword ptr [0x56b0b8], eax
    app->getMemory<x86::reg32>(x86::reg32(5681336) /* 0x56b0b8 */) = cpu.eax;
    // 00525c50  e9a3000000             -jmp 0x525cf8
    goto L_0x00525cf8;
L_0x00525c55:
    // 00525c55  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00525c57  7505                   -jne 0x525c5e
    if (!cpu.flags.zf)
    {
        goto L_0x00525c5e;
    }
    // 00525c59  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
L_0x00525c5e:
    // 00525c5e  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 00525c60  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00525c61  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00525c62  2eff15e4445300         -call dword ptr cs:[0x5344e4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457124) /* 0x5344e4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525c69  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00525c6b  750e                   -jne 0x525c7b
    if (!cpu.flags.zf)
    {
        goto L_0x00525c7b;
    }
    // 00525c6d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00525c72  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 00525c75  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525c76  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525c77  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525c78  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525c79  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525c7a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00525c7b:
    // 00525c7b  bb01010000             -mov ebx, 0x101
    cpu.ebx = 257 /*0x101*/;
    // 00525c80  b810b2a000             -mov eax, 0xa0b210
    cpu.eax = 10531344 /*0xa0b210*/;
    // 00525c85  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00525c87  e8b4a9fbff             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525c8c  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00525c8e  8a642406               -mov ah, byte ptr [esp + 6]
    cpu.ah = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(6) /* 0x6 */);
    // 00525c92  890d00b2a000           -mov dword ptr [0xa0b200], ecx
    app->getMemory<x86::reg32>(x86::reg32(10531328) /* 0xa0b200 */) = cpu.ecx;
    // 00525c98  84e4                   +test ah, ah
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & cpu.ah));
    // 00525c9a  740a                   -je 0x525ca6
    if (cpu.flags.zf)
    {
        goto L_0x00525ca6;
    }
    // 00525c9c  c70500b2a00001000000   -mov dword ptr [0xa0b200], 1
    app->getMemory<x86::reg32>(x86::reg32(10531328) /* 0xa0b200 */) = 1 /*0x1*/;
L_0x00525ca6:
    // 00525ca6  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00525ca8  b101                   -mov cl, 1
    cpu.cl = 1 /*0x1*/;
    // 00525caa  eb1c                   -jmp 0x525cc8
    goto L_0x00525cc8;
L_0x00525cac:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00525cac  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00525cae  8a441c06               -mov al, byte ptr [esp + ebx + 6]
    cpu.al = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(6) /* 0x6 */ + cpu.ebx * 1);
    // 00525cb2  eb07                   -jmp 0x525cbb
    goto L_0x00525cbb;
L_0x00525cb4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00525cb4  40                     -inc eax
    (cpu.eax)++;
    // 00525cb5  888810b2a000           -mov byte ptr [eax + 0xa0b210], cl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(10531344) /* 0xa0b210 */) = cpu.cl;
L_0x00525cbb:
    // 00525cbb  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00525cbd  8a541c07               -mov dl, byte ptr [esp + ebx + 7]
    cpu.dl = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(7) /* 0x7 */ + cpu.ebx * 1);
    // 00525cc1  39d0                   +cmp eax, edx
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
    // 00525cc3  7eef                   -jle 0x525cb4
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00525cb4;
    }
    // 00525cc5  83c302                 -add ebx, 2
    (cpu.ebx) += x86::reg32(x86::sreg32(2 /*0x2*/));
L_0x00525cc8:
    // 00525cc8  807c1c0600             +cmp byte ptr [esp + ebx + 6], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(6) /* 0x6 */ + cpu.ebx * 1);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00525ccd  75dd                   -jne 0x525cac
    if (!cpu.flags.zf)
    {
        goto L_0x00525cac;
    }
    // 00525ccf  807c1c0700             +cmp byte ptr [esp + ebx + 7], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(7) /* 0x7 */ + cpu.ebx * 1);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00525cd4  75d6                   -jne 0x525cac
    if (!cpu.flags.zf)
    {
        goto L_0x00525cac;
    }
    // 00525cd6  83fe01                 +cmp esi, 1
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
    // 00525cd9  7517                   -jne 0x525cf2
    if (!cpu.flags.zf)
    {
        goto L_0x00525cf2;
    }
    // 00525cdb  2eff1550455300         -call dword ptr cs:[0x534550]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457232) /* 0x534550 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525ce2  a3b8b05600             -mov dword ptr [0x56b0b8], eax
    app->getMemory<x86::reg32>(x86::reg32(5681336) /* 0x56b0b8 */) = cpu.eax;
    // 00525ce7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00525ce9  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 00525cec  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525ced  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525cee  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525cef  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525cf0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525cf1  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00525cf2:
    // 00525cf2  8935b8b05600           -mov dword ptr [0x56b0b8], esi
    app->getMemory<x86::reg32>(x86::reg32(5681336) /* 0x56b0b8 */) = cpu.esi;
L_0x00525cf8:
    // 00525cf8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00525cfa:
    // 00525cfa  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 00525cfd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525cfe  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525cff  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525d00  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525d01  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525d02  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_525d04: native, nfs3hp::dead::stub_0x525d04 */

/* sub_525d59: native, nfs3hp::watcom::U8D */

/* sub_525df0: native, nfs3hp::watcom::calloc */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_525e10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00525e10  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00525e11  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00525e12  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00525e13  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00525e15  e86e8affff             -call 0x51e888
    cpu.esp -= 4;
    sub_51e888(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525e1a  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00525e1c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00525e1e  7410                   -je 0x525e30
    if (cpu.flags.zf)
    {
        goto L_0x00525e30;
    }
    // 00525e20  8b1560775600           -mov edx, dword ptr [0x567760]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */);
    // 00525e26  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00525e27  2eff1514465300         -call dword ptr cs:[0x534614]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457428) /* 0x534614 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525e2e  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
L_0x00525e30:
    // 00525e30  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00525e32  750f                   -jne 0x525e43
    if (!cpu.flags.zf)
    {
        goto L_0x00525e43;
    }
    // 00525e34  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00525e39  b8f01e5500             -mov eax, 0x551ef0
    cpu.eax = 5578480 /*0x551ef0*/;
    // 00525e3e  e8c9befeff             -call 0x511d0c
    cpu.esp -= 4;
    nfs3hp::watcom::fatal_runtime_error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00525e43:
    // 00525e43  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00525e45  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525e46  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525e47  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525e48  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_525e4c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00525e4c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00525e4d  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00525e4e  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00525e4f  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00525e50  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00525e51  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00525e52  ff1590775600           -call dword ptr [0x567790]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666704) /* 0x567790 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525e58  2eff150c455300         -call dword ptr cs:[0x53450c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457164) /* 0x53450c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525e5f  8b1d5082a100           -mov ebx, dword ptr [0xa18250]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10584656) /* 0xa18250 */);
    // 00525e65  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00525e67  740b                   -je 0x525e74
    if (cpu.flags.zf)
    {
        goto L_0x00525e74;
    }
L_0x00525e69:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00525e69  3b4304                 +cmp eax, dword ptr [ebx + 4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525e6c  7406                   -je 0x525e74
    if (cpu.flags.zf)
    {
        goto L_0x00525e74;
    }
    // 00525e6e  8b1b                   -mov ebx, dword ptr [ebx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx);
    // 00525e70  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00525e72  75f5                   -jne 0x525e69
    if (!cpu.flags.zf)
    {
        goto L_0x00525e69;
    }
L_0x00525e74:
    // 00525e74  837b0c00               +cmp dword ptr [ebx + 0xc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525e78  7425                   -je 0x525e9f
    if (cpu.flags.zf)
    {
        goto L_0x00525e9f;
    }
    // 00525e7a  8b1570af5600           -mov edx, dword ptr [0x56af70]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5681008) /* 0x56af70 */);
    // 00525e80  8b4308                 -mov eax, dword ptr [ebx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 00525e83  e83827ffff             -call 0x5185c0
    cpu.esp -= 4;
    nfs3hp::watcom::nrealloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525e88  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 00525e8a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00525e8c  755e                   -jne 0x525eec
    if (!cpu.flags.zf)
    {
        goto L_0x00525eec;
    }
    // 00525e8e  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00525e93  b8181f5500             -mov eax, 0x551f18
    cpu.eax = 5578520 /*0x551f18*/;
    // 00525e98  e86fbefeff             -call 0x511d0c
    cpu.esp -= 4;
    nfs3hp::watcom::fatal_runtime_error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525e9d  eb4d                   -jmp 0x525eec
    goto L_0x00525eec;
L_0x00525e9f:
    // 00525e9f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00525ea4  8b1570af5600           -mov edx, dword ptr [0x56af70]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5681008) /* 0x56af70 */);
    // 00525eaa  e841ffffff             -call 0x525df0
    cpu.esp -= 4;
    nfs3hp::watcom::calloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525eaf  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 00525eb1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00525eb3  750f                   -jne 0x525ec4
    if (!cpu.flags.zf)
    {
        goto L_0x00525ec4;
    }
    // 00525eb5  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00525eba  b8401f5500             -mov eax, 0x551f40
    cpu.eax = 5578560 /*0x551f40*/;
    // 00525ebf  e848befeff             -call 0x511d0c
    cpu.esp -= 4;
    nfs3hp::watcom::fatal_runtime_error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00525ec4:
    // 00525ec4  8b7308                 -mov esi, dword ptr [ebx + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 00525ec7  89ef                   -mov edi, ebp
    cpu.edi = cpu.ebp;
    // 00525ec9  8b8ef0000000           -mov ecx, dword ptr [esi + 0xf0]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(240) /* 0xf0 */);
    // 00525ecf  06                     -push es
    app->getMemory<x86::reg16>(cpu.esp-4) = cpu.es;
    cpu.esp -= 4;
    // 00525ed0  8cd8                   -mov eax, ds
    cpu.eax = cpu.ds;
    // 00525ed2  8ec0                   -mov es, eax
    cpu.es = cpu.eax;
    // 00525ed4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00525ed5  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525ed7  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 00525eda  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00525edc  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 00525ede  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 00525ee1  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 00525ee3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525ee4  07                     -pop es
    cpu.es = app->getMemory<x86::reg16>(cpu.esp);
    cpu.esp += 4;
    // 00525ee5  c7430c01000000         -mov dword ptr [ebx + 0xc], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */) = 1 /*0x1*/;
L_0x00525eec:
    // 00525eec  896b08                 -mov dword ptr [ebx + 8], ebp
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */) = cpu.ebp;
    // 00525eef  a170af5600             -mov eax, dword ptr [0x56af70]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5681008) /* 0x56af70 */);
    // 00525ef4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00525ef5  c6455201               -mov byte ptr [ebp + 0x52], 1
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(82) /* 0x52 */) = 1 /*0x1*/;
    // 00525ef9  8b3560775600           -mov esi, dword ptr [0x567760]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */);
    // 00525eff  c6455300               -mov byte ptr [ebp + 0x53], 0
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(83) /* 0x53 */) = 0 /*0x0*/;
    // 00525f03  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00525f04  8985f0000000           -mov dword ptr [ebp + 0xf0], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(240) /* 0xf0 */) = cpu.eax;
    // 00525f0a  2eff1518465300         -call dword ptr cs:[0x534618]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457432) /* 0x534618 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525f11  ff1594775600           -call dword ptr [0x567794]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666708) /* 0x567794 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525f17  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00525f19  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525f1a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525f1b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525f1c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525f1d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525f1e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525f1f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_525f20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00525f20  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00525f21  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00525f22  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00525f23  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00525f25  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 00525f27  ff1590775600           -call dword ptr [0x567790]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666704) /* 0x567790 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525f2d  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00525f32  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 00525f37  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525f39  e8b2feffff             -call 0x525df0
    cpu.esp -= 4;
    nfs3hp::watcom::calloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525f3e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00525f40  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00525f42  742f                   -je 0x525f73
    if (cpu.flags.zf)
    {
        goto L_0x00525f73;
    }
    // 00525f44  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00525f46  e8d9680000             -call 0x52c824
    cpu.esp -= 4;
    sub_52c824(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525f4b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00525f4d  7409                   -je 0x525f58
    if (cpu.flags.zf)
    {
        goto L_0x00525f58;
    }
    // 00525f4f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00525f51  e89a1afdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525f56  eb1b                   -jmp 0x525f73
    goto L_0x00525f73;
L_0x00525f58:
    // 00525f58  895a08                 -mov dword ptr [edx + 8], ebx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */) = cpu.ebx;
    // 00525f5b  897204                 -mov dword ptr [edx + 4], esi
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */) = cpu.esi;
    // 00525f5e  8a4352                 -mov al, byte ptr [ebx + 0x52]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(82) /* 0x52 */);
    // 00525f61  89420c                 -mov dword ptr [edx + 0xc], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 00525f64  a15082a100             -mov eax, dword ptr [0xa18250]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10584656) /* 0xa18250 */);
    // 00525f69  89155082a100           -mov dword ptr [0xa18250], edx
    app->getMemory<x86::reg32>(x86::reg32(10584656) /* 0xa18250 */) = cpu.edx;
    // 00525f6f  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
    // 00525f71  eb02                   -jmp 0x525f75
    goto L_0x00525f75;
L_0x00525f73:
    // 00525f73  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00525f75:
    // 00525f75  ff1594775600           -call dword ptr [0x567794]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666708) /* 0x567794 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525f7b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00525f7d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525f7e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525f7f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525f80  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_525f84(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00525f84  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00525f85  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00525f86  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00525f87  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00525f89  ff1590775600           -call dword ptr [0x567790]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666704) /* 0x567790 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525f8f  8b155082a100           -mov edx, dword ptr [0xa18250]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10584656) /* 0xa18250 */);
    // 00525f95  b95082a100             -mov ecx, 0xa18250
    cpu.ecx = 10584656 /*0xa18250*/;
    // 00525f9a  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00525f9c  7428                   -je 0x525fc6
    if (cpu.flags.zf)
    {
        goto L_0x00525fc6;
    }
L_0x00525f9e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00525f9e  3b5a04                 +cmp ebx, dword ptr [edx + 4]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525fa1  751b                   -jne 0x525fbe
    if (!cpu.flags.zf)
    {
        goto L_0x00525fbe;
    }
    // 00525fa3  837a0c00               +cmp dword ptr [edx + 0xc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00525fa7  7408                   -je 0x525fb1
    if (cpu.flags.zf)
    {
        goto L_0x00525fb1;
    }
    // 00525fa9  8b4208                 -mov eax, dword ptr [edx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 00525fac  e83f1afdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00525fb1:
    // 00525fb1  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 00525fb3  8901                   -mov dword ptr [ecx], eax
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.eax;
    // 00525fb5  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00525fb7  e8341afdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525fbc  eb08                   -jmp 0x525fc6
    goto L_0x00525fc6;
L_0x00525fbe:
    // 00525fbe  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00525fc0  8b12                   -mov edx, dword ptr [edx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx);
    // 00525fc2  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00525fc4  75d8                   -jne 0x525f9e
    if (!cpu.flags.zf)
    {
        goto L_0x00525f9e;
    }
L_0x00525fc6:
    // 00525fc6  ff1594775600           -call dword ptr [0x567794]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666708) /* 0x567794 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00525fcc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525fcd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525fce  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00525fcf  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_525fd0: native, nfs3hp::dead::stub_0x525fd0 */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_525ff8(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00525ff8  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00525ff9  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00525ffa  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00525ffb  8b155082a100           -mov edx, dword ptr [0xa18250]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10584656) /* 0xa18250 */);
    // 00526001  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00526003  741e                   -je 0x526023
    if (cpu.flags.zf)
    {
        goto L_0x00526023;
    }
L_0x00526005:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00526005  8b4a0c                 -mov ecx, dword ptr [edx + 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    // 00526008  8b1a                   -mov ebx, dword ptr [edx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx);
    // 0052600a  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0052600c  7408                   -je 0x526016
    if (cpu.flags.zf)
    {
        goto L_0x00526016;
    }
    // 0052600e  8b4208                 -mov eax, dword ptr [edx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 00526011  e8da19fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00526016:
    // 00526016  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00526018  e8d319fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052601d  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0052601f  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00526021  75e2                   -jne 0x526005
    if (!cpu.flags.zf)
    {
        goto L_0x00526005;
    }
L_0x00526023:
    // 00526023  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526024  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526025  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526026  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_526030: native, nfs3hp::watcom::initThread */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_526060(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00526060  e993ffffff             -jmp 0x525ff8
    return sub_525ff8(app, cpu.handOver());
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_526070(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00526070  dbe2                   -fnclex 
    /*nothing*/;
    // 00526072  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_526080(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00526080  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00526081  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00526082  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00526084  83f807                 +cmp eax, 7
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
    // 00526087  7405                   -je 0x52608e
    if (cpu.flags.zf)
    {
        goto L_0x0052608e;
    }
    // 00526089  83f804                 +cmp eax, 4
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
    // 0052608c  7518                   -jne 0x5260a6
    if (!cpu.flags.zf)
    {
        goto L_0x005260a6;
    }
L_0x0052608e:
    // 0052608e  8d04dd00000000         -lea eax, [ebx*8]
    cpu.eax = x86::reg32(cpu.ebx * 8);
    // 00526095  8b98bcb05600           -mov ebx, dword ptr [eax + 0x56b0bc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5681340) /* 0x56b0bc */);
    // 0052609b  8990bcb05600           -mov dword ptr [eax + 0x56b0bc], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5681340) /* 0x56b0bc */) = cpu.edx;
    // 005260a1  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 005260a3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005260a4  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005260a5  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005260a6:
    // 005260a6  8d0cdd00000000         -lea ecx, [ebx*8]
    cpu.ecx = x86::reg32(cpu.ebx * 8);
    // 005260ad  ff1564775600           -call dword ptr [0x567764]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666660) /* 0x567764 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005260b3  8b5c0158               -mov ebx, dword ptr [ecx + eax + 0x58]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(88) /* 0x58 */ + cpu.eax * 1);
    // 005260b7  ff1564775600           -call dword ptr [0x567764]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666660) /* 0x567764 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005260bd  89540158               -mov dword ptr [ecx + eax + 0x58], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(88) /* 0x58 */ + cpu.eax * 1) = cpu.edx;
    // 005260c1  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 005260c3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005260c4  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005260c5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_5260c8(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005260c8  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 005260c9  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 005260cb  83f807                 +cmp eax, 7
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
    // 005260ce  7405                   -je 0x5260d5
    if (cpu.flags.zf)
    {
        goto L_0x005260d5;
    }
    // 005260d0  83f804                 +cmp eax, 4
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
    // 005260d3  7509                   -jne 0x5260de
    if (!cpu.flags.zf)
    {
        goto L_0x005260de;
    }
L_0x005260d5:
    // 005260d5  8b04d5bcb05600         -mov eax, dword ptr [edx*8 + 0x56b0bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5681340) /* 0x56b0bc */ + cpu.edx * 8);
    // 005260dc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005260dd  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005260de:
    // 005260de  ff1564775600           -call dword ptr [0x567764]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666660) /* 0x567764 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005260e4  8b44d058               -mov eax, dword ptr [eax + edx*8 + 0x58]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(88) /* 0x58 */ + cpu.edx * 8);
    // 005260e8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005260e9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_5260ec(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 005260ec  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 005260ed  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 005260ef  83f807                 +cmp eax, 7
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
    // 005260f2  7405                   -je 0x5260f9
    if (cpu.flags.zf)
    {
        goto L_0x005260f9;
    }
    // 005260f4  83f804                 +cmp eax, 4
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
    // 005260f7  7509                   -jne 0x526102
    if (!cpu.flags.zf)
    {
        goto L_0x00526102;
    }
L_0x005260f9:
    // 005260f9  8b04d5c0b05600         -mov eax, dword ptr [edx*8 + 0x56b0c0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5681344) /* 0x56b0c0 */ + cpu.edx * 8);
    // 00526100  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526101  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00526102:
    // 00526102  ff1564775600           -call dword ptr [0x567764]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666660) /* 0x567764 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526108  8b44d05c               -mov eax, dword ptr [eax + edx*8 + 0x5c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(92) /* 0x5c */ + cpu.edx * 8);
    // 0052610c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052610d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_526110(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00526110  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00526111  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00526113  e8d4ffffff             -call 0x5260ec
    cpu.esp -= 4;
    sub_5260ec(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526118  39c2                   +cmp edx, eax
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
    // 0052611a  7509                   -jne 0x526125
    if (!cpu.flags.zf)
    {
        goto L_0x00526125;
    }
    // 0052611c  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0052611e  e8a5ffffff             -call 0x5260c8
    cpu.esp -= 4;
    sub_5260c8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526123  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526124  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00526125:
    // 00526125  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00526127  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00526128  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_52612c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052612c  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 00526130  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00526132  760a                   -jbe 0x52613e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0052613e;
    }
    // 00526134  83f801                 +cmp eax, 1
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
    // 00526137  7424                   -je 0x52615d
    if (cpu.flags.zf)
    {
        goto L_0x0052615d;
    }
    // 00526139  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052613b  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x0052613e:
    // 0052613e  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 00526143  e880ffffff             -call 0x5260c8
    cpu.esp -= 4;
    sub_5260c8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526148  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052614a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052614c  7503                   -jne 0x526151
    if (!cpu.flags.zf)
    {
        goto L_0x00526151;
    }
    // 0052614e  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00526151:
    // 00526151  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 00526156  e8dd010000             -call 0x526338
    cpu.esp -= 4;
    sub_526338(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052615b  eb1d                   -jmp 0x52617a
    goto L_0x0052617a;
L_0x0052615d:
    // 0052615d  b807000000             -mov eax, 7
    cpu.eax = 7 /*0x7*/;
    // 00526162  e861ffffff             -call 0x5260c8
    cpu.esp -= 4;
    sub_5260c8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00526167  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00526169  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052616b  7503                   -jne 0x526170
    if (!cpu.flags.zf)
    {
        goto L_0x00526170;
    }
    // 0052616d  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00526170:
    // 00526170  b807000000             -mov eax, 7
    cpu.eax = 7 /*0x7*/;
    // 00526175  e8be010000             -call 0x526338
    cpu.esp -= 4;
    sub_526338(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052617a:
    // 0052617a  83fa02                 +cmp edx, 2
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
    // 0052617d  7405                   -je 0x526184
    if (cpu.flags.zf)
    {
        goto L_0x00526184;
    }
    // 0052617f  83fa03                 +cmp edx, 3
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
    // 00526182  7505                   -jne 0x526189
    if (!cpu.flags.zf)
    {
        goto L_0x00526189;
    }
L_0x00526184:
    // 00526184  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00526186  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00526189:
    // 00526189  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0052618e  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_526194(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00526194  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00526195  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 0052619a  e829ffffff             -call 0x5260c8
    cpu.esp -= 4;
    sub_5260c8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052619f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 005261a1  b807000000             -mov eax, 7
    cpu.eax = 7 /*0x7*/;
    // 005261a6  e81dffffff             -call 0x5260c8
    cpu.esp -= 4;
    sub_5260c8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 005261ab  83fa02                 +cmp edx, 2
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
    // 005261ae  7405                   -je 0x5261b5
    if (cpu.flags.zf)
    {
        goto L_0x005261b5;
    }
    // 005261b0  83fa03                 +cmp edx, 3
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
    // 005261b3  750a                   -jne 0x5261bf
    if (!cpu.flags.zf)
    {
        goto L_0x005261bf;
    }
L_0x005261b5:
    // 005261b5  83f802                 +cmp eax, 2
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
    // 005261b8  740c                   -je 0x5261c6
    if (cpu.flags.zf)
    {
        goto L_0x005261c6;
    }
    // 005261ba  83f803                 +cmp eax, 3
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
    // 005261bd  7407                   -je 0x5261c6
    if (cpu.flags.zf)
    {
        goto L_0x005261c6;
    }
L_0x005261bf:
    // 005261bf  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 005261c4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005261c5  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x005261c6:
    // 005261c6  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 005261c8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005261c9  c3                     -ret 
    cpu.esp += 4;
    return;
}

}
