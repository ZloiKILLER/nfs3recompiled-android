#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_5200ec: native, nfs3hp::port::resample16MonoDown */

/* sub_5201d8: native, nfs3hp::port::resample16MonoSame */

/* sub_5201e4: native, nfs3hp::port::resample16MonoUp */

/* sub_5202fc: native, nfs3hp::port::resampleMonoStep */

/* sub_5203a0: native, nfs3hp::port::resampleMonoSetup */

/* sub_5203f0: native, nfs3hp::port::resample16StereoNeededDown */

/* sub_520438: native, nfs3hp::port::resample16StereoNeededSame */

/* sub_520440: native, nfs3hp::port::resample16StereoNeededUp */

/* sub_52046c: native, nfs3hp::port::resample16StereoDown */

/* sub_5205b0: native, nfs3hp::port::resample16StereoSame */

/* sub_5205bc: native, nfs3hp::port::resample16StereoUp */

/* sub_520740: native, nfs3hp::port::resampleStereoStep */

/* sub_5207e4: native, nfs3hp::port::resampleStereoSetup */

/* sub_520830: native, nfs3hp::mix::outputMono8 */

/* sub_520870: native, nfs3hp::mix::outputStereo8 */

/* sub_520920: native, nfs3hp::mix::outputStereo16 */

/* sub_5209a0: native, nfs3hp::mix::clear */

/* sub_520a00: native, nfs3hp::mix::finished */

/* sub_520a10: native, nfs3hp::mix::outputMono16 */

/* sub_520a40: native, nfs3hp::snd::dead_0x520a40 */

/* sub_520a5e: native, nfs3hp::snd::dead_0x520a5e */

/* sub_520aee: native, nfs3hp::snd::dead_0x520aee */

/* sub_520b7e: native, nfs3hp::snd::dead_0x520b7e */

/* sub_520bed: native, nfs3hp::snd::dead_0x520bed */

/* sub_520c69: native, nfs3hp::snd::dead_0x520c69 */

/* sub_520ce6: native, nfs3hp::snd::dead_0x520ce6 */

/* sub_520d6b: native, nfs3hp::snd::dead_0x520d6b */

/* sub_520df1: native, nfs3hp::snd::dead_0x520df1 */

/* sub_520e00: native, nfs3hp::snd::dead_0x520e00 */

/* sub_520e1e: native, nfs3hp::snd::dead_0x520e1e */

/* sub_520eae: native, nfs3hp::snd::dead_0x520eae */

/* sub_520f40: native, nfs3hp::snd::dead_0x520f40 */

/* sub_520fb1: native, nfs3hp::snd::dead_0x520fb1 */

/* sub_52102d: native, nfs3hp::snd::dead_0x52102d */

/* sub_5210ac: native, nfs3hp::snd::dead_0x5210ac */

/* sub_521133: native, nfs3hp::snd::dead_0x521133 */

/* sub_5211bb: native, nfs3hp::snd::dead_0x5211bb */

/* sub_5211c0: native, nfs3hp::snd::dead_0x5211c0 */

/* sub_521200: native, nfs3hp::snd::dead_0x521200 */

/* sub_521215: native, nfs3hp::snd::dead_0x521215 */

/* sub_52125b: native, nfs3hp::snd::dead_0x52125b */

/* sub_521310: native, nfs3hp::snd::dead_0x521310 */

/* sub_521350: native, nfs3hp::snd::dead_0x521350 */

/* sub_521365: native, nfs3hp::snd::dead_0x521365 */

/* sub_5213ab: native, nfs3hp::snd::dead_0x5213ab */

/* sub_521490: native, nfs3hp::snd::dead_0x521490 */

/* sub_5214d8: native, nfs3hp::snd::dead_0x5214d8 */

/* sub_5214e0: native, nfs3hp::snd::dead_0x5214e0 */

/* sub_52150c: native, nfs3hp::snd::dead_0x52150c */

/* sub_521614: native, nfs3hp::snd::dead_0x521614 */

/* sub_521620: native, nfs3hp::snd::dead_0x521620 */

/* sub_52175c: native, nfs3hp::snd::dead_0x52175c */

/* sub_521800: native, nfs3hp::snd::dead_0x521800 */

/* sub_521840: native, nfs3hp::snd::dead_0x521840 */

/* sub_5218c0: native, nfs3hp::snd::dead_0x5218c0 */

/* sub_521908: native, nfs3hp::snd::dead_0x521908 */

/* sub_521910: native, nfs3hp::snd::dead_0x521910 */

/* sub_52193c: native, nfs3hp::snd::dead_0x52193c */

/* sub_521a10: native, nfs3hp::snd::dead_0x521a10 */

/* sub_521a1c: native, nfs3hp::snd::dead_0x521a1c */

/* sub_521b20: native, nfs3hp::snd::dead_0x521b20 */

/* sub_521bc4: native, nfs3hp::snd::dead_0x521bc4 */

/* sub_521c10: native, nfs3hp::snd::dead_0x521c10 */

/* sub_521c90: native, nfs3hp::snd::dead_0x521c90 */

/* sub_521d00: native, nfs3hp::snd::dead_0x521d00 */

/* sub_521d80: native, nfs3hp::port::pcmTablesFirst */

/* sub_521e60: native, nfs3hp::port::pcmTablesSecond */

/* sub_521f30: native, nfs3hp::port::xaFloatTable */

/* sub_521f7c: native, nfs3hp::port::xaIntTable */

/* sub_521fc0: native, nfs3hp::port::xaTables */

/* sub_5220a0: native, nfs3hp::port::lastTables */

/* sub_522100: native, nfs3hp::snd::dead_0x522100 */

/* sub_522130: native, nfs3hp::snd::dead_0x522130 */

/* sub_5221f0: native, nfs3hp::ea::physicalMemory */

/* sub_522220: native, nfs3hp::snd::dead_0x522220 */

/* sub_522250: native, nfs3hp::snd::dead_0x522250 */

/* sub_5222f0: native, nfs3hp::port::panSplit */

/* sub_522320: native, nfs3hp::port::panSurround */

/* sub_5224b0: native, nfs3hp::port::panLawPositive */

/* sub_5224e0: native, nfs3hp::port::randomOffset */

/* sub_522510: native, nfs3hp::port::iSNDplatformplay */

/* sub_5229d0: native, nfs3hp::port::nextTag */

/* sub_522a80: native, nfs3hp::port::iSNDplatformresolve */

/* sub_522d08: native, nfs3hp::port::platformUnresolve */

/* sub_522d80: native, nfs3hp::watcom::fsync */

/* sub_522de0: native, nfs3hp::watcom::GetIOMode */

/* sub_522e38: native, nfs3hp::watcom::SetIOMode */

/* sub_522e60: native, nfs3hp::watcom::getche */

/* sub_522e90: native, nfs3hp::port::pitchFactor */

/* sub_522f10: native, nfs3hp::port::mixStep */

/* sub_522f50: native, nfs3hp::port::setMixStep */

/* sub_522f90: native, nfs3hp::snd::dead_0x522f90 */

/* sub_522fd4: native, nfs3hp::snd::dead_0x522fd4 */

/* sub_5230b8: native, nfs3hp::port::reverbStart */

/* sub_5230dc: native, nfs3hp::snd::dead_0x5230dc */

/* sub_523118: native, nfs3hp::snd::dead_0x523118 */

/* sub_523200: native, nfs3hp::port::reverbLength */

/* sub_52325c: native, nfs3hp::port::reverbInit */

/* sub_523310: native, nfs3hp::port::setVoiceSend */

/* align: skip 0x00 */
void Application::sub_523332(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fpa1 = cpu.fpu.st(0);
  x86::Float fpa2 = cpu.fpu.st(1);
  x86::Float fp0, fp1;
  x86::reg16 fsw = 0;
    // 00523332  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00523333  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00523335  d9e4                   -ftst 
    fsw = fp::compare(fpa1, 0.0);
    // 00523337  83ec18                 +sub esp, 0x18
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(24 /*0x18*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052333a  9b                     -wait 
    /*nothing*/;
    // 0052333b  dd7df8                 -fnstsw word ptr [ebp - 8]
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = fsw;
    // 0052333e  dd55e8                 +fst qword ptr [ebp - 0x18]
    app->getMemory<double>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = double(fpa1);
    // 00523341  8a65f9                 -mov ah, byte ptr [ebp - 7]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-7) /* -0x7 */);
    // 00523344  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00523345  751a                   -jne 0x523361
    if (!cpu.flags.zf)
    {
        goto L_0x00523361;
    }
    // 00523347  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
L_0x00523349:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523349  ddd8                   -fstp st(0)
    fpa1 = x86::Float(fpa1);
    // 0052334b  dd5df0                 -fstp qword ptr [ebp - 0x10]
    app->getMemory<double>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = double(fpa2);
    // 0052334e  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00523351  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00523354  e8ca810000             -call 0x52b523
    cpu.esp -= 4;
    sub_52b523(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fpa2 = cpu.fpu.take();
    // 00523359  83ec08                 +sub esp, 8
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052335c  e9f9000000             -jmp 0x52345a
    goto L_0x0052345a;
L_0x00523361:
    // 00523361  d9c1                   +fld st(1)
    fp0 = x86::Float(fpa2);
    // 00523363  d9fc                   +frndint 
    fp0 = fp::rndint(fp0);
    // 00523365  d8da                   +fcomp st(2)
    fsw = fp::compare(fp0, x86::Float(fpa2));
    // 00523367  9b                     -wait 
    /*nothing*/;
    // 00523368  dd7dfa                 -fnstsw word ptr [ebp - 6]
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = fsw;
    // 0052336b  9b                     -wait 
    /*nothing*/;
    // 0052336c  8a65fb                 -mov ah, byte ptr [ebp - 5]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-5) /* -0x5 */);
    // 0052336f  733b                   -jae 0x5233ac
    if (!cpu.flags.cf)
    {
        goto L_0x005233ac;
    }
    // 00523371  b001                   -mov al, 1
    cpu.al = 1 /*0x1*/;
    // 00523373  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00523374  75d3                   -jne 0x523349
    if (!cpu.flags.zf)
    {
        goto L_0x00523349;
    }
    // 00523376  66b80200               -mov ax, 2
    cpu.ax = 2 /*0x2*/;
    // 0052337a  668945fc               -mov word ptr [ebp - 4], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ax;
    // 0052337e  df45fc                 +fild word ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg16(app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00523381  d9c2                   +fld st(2)
    fp1 = x86::Float(fpa2);
    // 00523383  d9f8                   +fprem 
    fp1 = fp::rem(fp1, fp0);
    fsw &= ~0x400;
    // 00523385  9b                     -wait 
    /*nothing*/;
    // 00523386  dd7dfc                 -fnstsw word ptr [ebp - 4]
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = fsw;
    // 00523389  9b                     -wait 
    /*nothing*/;
    // 0052338a  8a65fd                 -mov ah, byte ptr [ebp - 3]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-3) /* -0x3 */);
    // 0052338d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0052338e  b400                   -mov ah, 0
    cpu.ah = 0 /*0x0*/;
    // 00523390  7a11                   -jp 0x5233a3
    if (cpu.flags.pf)
    {
        goto L_0x005233a3;
    }
    // 00523392  d9e4                   +ftst 
    fsw = fp::compare(fp1, 0.0);
    // 00523394  9b                     -wait 
    /*nothing*/;
    // 00523395  dd7dfc                 -fnstsw word ptr [ebp - 4]
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = fsw;
    // 00523398  9b                     -wait 
    /*nothing*/;
    // 00523399  8a65fd                 -mov ah, byte ptr [ebp - 3]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-3) /* -0x3 */);
    // 0052339c  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0052339d  b400                   -mov ah, 0
    cpu.ah = 0 /*0x0*/;
    // 0052339f  7402                   -je 0x5233a3
    if (cpu.flags.zf)
    {
        goto L_0x005233a3;
    }
    // 005233a1  b401                   -mov ah, 1
    cpu.ah = 1 /*0x1*/;
L_0x005233a3:
    // 005233a3  8865f9                 -mov byte ptr [ebp - 7], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-7) /* -0x7 */) = cpu.ah;
    // 005233a6  ddd8                   +fstp st(0)
    fp1 = x86::Float(fp1);
    // 005233a8  ddd8                   +fstp st(0)
    fp0 = x86::Float(fp0);
    // 005233aa  eb08                   -jmp 0x5233b4
    goto L_0x005233b4;
L_0x005233ac:
    // 005233ac  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 005233ad  7405                   -je 0x5233b4
    if (cpu.flags.zf)
    {
        goto L_0x005233b4;
    }
    // 005233af  e987000000             -jmp 0x52343b
    goto L_0x0052343b;
L_0x005233b4:
    // 005233b4  d9c1                   -fld st(1)
    fp0 = x86::Float(fpa2);
    // 005233b6  dd5df0                 -fstp qword ptr [ebp - 0x10]
    app->getMemory<double>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = double(fp0);
    // 005233b9  9b                     -wait 
    /*nothing*/;
    // 005233ba  668b45f6               -mov ax, word ptr [ebp - 0xa]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 005233be  6625f07f               -and ax, 0x7ff0
    cpu.ax &= x86::reg16(x86::sreg16(32752 /*0x7ff0*/));
    // 005233c2  662df03f               -sub ax, 0x3ff0
    (cpu.ax) -= x86::reg16(x86::sreg16(16368 /*0x3ff0*/));
    // 005233c6  663d0001               +cmp ax, 0x100
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(256 /*0x100*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 005233ca  736f                   -jae 0x52343b
    if (!cpu.flags.cf)
    {
        goto L_0x0052343b;
    }
    // 005233cc  d9c1                   -fld st(1)
    fp0 = x86::Float(fpa2);
    // 005233ce  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 005233d1  9b                     -wait 
    /*nothing*/;
    // 005233d2  668b45fe               -mov ax, word ptr [ebp - 2]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-2) /* -0x2 */);
    // 005233d6  6609c0                 +or ax, ax
    cpu.clear_co();
    cpu.set_szp((cpu.ax |= x86::reg16(x86::sreg16(cpu.ax))));
    // 005233d9  750b                   -jne 0x5233e6
    if (!cpu.flags.zf)
    {
        goto L_0x005233e6;
    }
    // 005233db  668b45fc               -mov ax, word ptr [ebp - 4]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 005233df  e881000000             -call 0x523465
    cpu.fpu.push(fpa1);
    cpu.esp -= 4;
    sub_523465(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fpa1 = cpu.fpu.take();
    // 005233e4  eb2b                   -jmp 0x523411
    goto L_0x00523411;
L_0x005233e6:
    // 005233e6  6640                   +inc ax
    {
        x86::reg16& tmp = cpu.ax;
        cpu.flags.of = ~(1 & (tmp >> 15));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 15);
        cpu.set_szp(tmp);
    }
    // 005233e8  7551                   -jne 0x52343b
    if (!cpu.flags.zf)
    {
        goto L_0x0052343b;
    }
    // 005233ea  660b45fc               +or ax, word ptr [ebp - 4]
    cpu.clear_co();
    cpu.set_szp((cpu.ax |= x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */)))));
    // 005233ee  744b                   -je 0x52343b
    if (cpu.flags.zf)
    {
        goto L_0x0052343b;
    }
    // 005233f0  66f7d8                 -neg ax
    cpu.ax = ~cpu.ax + 1;
    // 005233f3  e86d000000             -call 0x523465
    cpu.fpu.push(fpa1);
    cpu.esp -= 4;
    sub_523465(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fpa1 = cpu.fpu.take();
    // 005233f8  d9e8                   -fld1 
    fp0 = 1.0;
    // 005233fa  f6055878560001         +test byte ptr [0x567858], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5666904) /* 0x567858 */) & 1 /*0x1*/));
    // 00523401  7504                   -jne 0x523407
    if (!cpu.flags.zf)
    {
        goto L_0x00523407;
    }
    // 00523403  def1                   +fdivrp st(1)
    fpa1 = fp::div(fp0, x86::Float(fpa1));
    // 00523405  eb0a                   -jmp 0x523411
    goto L_0x00523411;
L_0x00523407:
    // 00523407  b80f000000             -mov eax, 0xf
    cpu.eax = 15 /*0xf*/;
    // 0052340c  e8da360000             -call 0x526aeb
    cpu.fpu.push(fpa1);
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::dead::stub_0x526aeb(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fpa1 = cpu.fpu.take();
L_0x00523411:
    // 00523411  dd55f8                 -fst qword ptr [ebp - 8]
    app->getMemory<double>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = double(fpa1);
    // 00523414  9b                     -wait 
    /*nothing*/;
    // 00523415  668b45f8               -mov ax, word ptr [ebp - 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00523419  660b45fa               -or ax, word ptr [ebp - 6]
    cpu.ax |= x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-6) /* -0x6 */)));
    // 0052341d  660b45fc               +or ax, word ptr [ebp - 4]
    cpu.clear_co();
    cpu.set_szp((cpu.ax |= x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */)))));
    // 00523421  7514                   -jne 0x523437
    if (!cpu.flags.zf)
    {
        goto L_0x00523437;
    }
    // 00523423  668b45fe               -mov ax, word ptr [ebp - 2]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-2) /* -0x2 */);
    // 00523427  66d1e0                 -shl ax, 1
    cpu.ax <<= 1 /*0x1*/ % 32;
    // 0052342a  663de0ff               +cmp ax, 0xffe0
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(65504 /*0xffe0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0052342e  7507                   -jne 0x523437
    if (!cpu.flags.zf)
    {
        goto L_0x00523437;
    }
L_0x00523430:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00523430  b002                   -mov al, 2
    cpu.al = 2 /*0x2*/;
    // 00523432  e912ffffff             -jmp 0x523349
    goto L_0x00523349;
L_0x00523437:
    // 00523437  ddd9                   +fstp st(1)
    fpa2 = x86::Float(fpa1);
    // 00523439  eb1f                   -jmp 0x52345a
    goto L_0x0052345a;
L_0x0052343b:
    // 0052343b  d9ed                   -fldln2 
    fp0 = 0.6931471805599453;
    // 0052343d  d8ca                   -fmul st(2)
    fp0 = fp::mul(fp0, x86::Float(fpa2));
    // 0052343f  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp0;
        fp0 = fpa1;
        fpa1 = tmp;
    }
    // 00523441  d9e1                   -fabs 
    fp0 = fp::abs(fp0);
    // 00523443  d9f1                   -fyl2x 
    fpa1 = fp::log2(fp0) * fpa1;
    // 00523445  b007                   -mov al, 7
    cpu.al = 7 /*0x7*/;
    // 00523447  e86a4cfdff             -call 0x4f80b6
    cpu.fpu.push(fpa1);
    cpu.esp -= 4;
    sub_4f80b6(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fpa1 = cpu.fpu.take();
    // 0052344c  3c00                   +cmp al, 0
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
    // 0052344e  75e0                   -jne 0x523430
    if (!cpu.flags.zf)
    {
        goto L_0x00523430;
    }
    // 00523450  8a65f9                 -mov ah, byte ptr [ebp - 7]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-7) /* -0x7 */);
    // 00523453  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00523454  7302                   -jae 0x523458
    if (!cpu.flags.cf)
    {
        goto L_0x00523458;
    }
    // 00523456  d9e0                   -fchs 
    fpa1 = -fpa1;
L_0x00523458:
    // 00523458  ddd9                   -fstp st(1)
    fpa2 = x86::Float(fpa1);
L_0x0052345a:
    // 0052345a  dd5df8                 -fstp qword ptr [ebp - 8]
    app->getMemory<double>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = double(fpa2);
    // 0052345d  dd45f8                 -fld qword ptr [ebp - 8]
    fpa2 = x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 00523460  9b                     -wait 
    /*nothing*/;
    // 00523461  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00523463  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00523464  c3                     -ret 
    cpu.fpu.count -= 1;
    cpu.fpu.st(0) = fpa2;
    cpu.esp += 4;
    return;
}

}
