#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_46e270: native, nfs3hp::frontend::resultsTableKey */

/* sub_46e640: native, nfs3hp::frontend::resultsTableHidden */

/* sub_46e660: native, nfs3hp::frontend::standingsSortMark */

/* sub_46e690: native, nfs3hp::frontend::standingsTableDraw */

/* sub_46ef90: native, nfs3hp::frontend::standingsTableShown */

/* sub_46f0a0: native, nfs3hp::frontend::standingsTablePass */

/* sub_46f0c0: native, nfs3hp::frontend::standingsTableLayout */

/* sub_46f200: native, nfs3hp::frontend::standingsTableKey */

/* sub_46f4c0: native, nfs3hp::frontend::standingsTableHidden */

/* sub_46f4e0: native, nfs3hp::frontend::roundsSortMark */

/* sub_46f510: native, nfs3hp::frontend::roundsTableDraw */

/* sub_46fe30: native, nfs3hp::frontend::roundsTableShown */

/* sub_46fee0: native, nfs3hp::frontend::roundsTablePass */

/* sub_46ff00: native, nfs3hp::frontend::roundsTableLayout */

/* sub_470040: native, nfs3hp::frontend::roundsTableKey */

/* sub_470340: native, nfs3hp::frontend::roundsTableHidden */

/* sub_470360: native, nfs3hp::frontend::headToHeadShown */

/* sub_470390: native, nfs3hp::frontend::headToHeadPass */

/* sub_4703a0: native, nfs3hp::frontend::headToHeadDraw */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_470a60(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470a60  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470a61  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470a63  a1408b6700             -mov eax, dword ptr [0x678b40]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6785856) /* 0x678b40 */);
    // 00470a68  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470a69  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_470a70: native, nfs3hp::dead::stub_0x470a70 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_470aa0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470aa0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00470aa1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470aa2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470aa4  833d643a7a0000         +cmp dword ptr [0x7a3a64], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010340) /* 0x7a3a64 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470aab  7411                   -je 0x470abe
    if (cpu.flags.zf)
    {
        goto L_0x00470abe;
    }
    // 00470aad  a3448b6700             -mov dword ptr [0x678b44], eax
    app->getMemory<x86::reg32>(x86::reg32(6785860) /* 0x678b44 */) = cpu.eax;
    // 00470ab2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00470ab4  b80f000000             -mov eax, 0xf
    cpu.eax = 15 /*0xf*/;
    // 00470ab9  e8420efcff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00470abe:
    // 00470abe  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470abf  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470ac0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_470ad0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470ad0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00470ad1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470ad2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470ad4  833d643a7a0000         +cmp dword ptr [0x7a3a64], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010340) /* 0x7a3a64 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470adb  743e                   -je 0x470b1b
    if (cpu.flags.zf)
    {
        goto L_0x00470b1b;
    }
    // 00470add  a3408b6700             -mov dword ptr [0x678b40], eax
    app->getMemory<x86::reg32>(x86::reg32(6785856) /* 0x678b40 */) = cpu.eax;
    // 00470ae2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00470ae4  7424                   -je 0x470b0a
    if (cpu.flags.zf)
    {
        goto L_0x00470b0a;
    }
    // 00470ae6  ba64000000             -mov edx, 0x64
    cpu.edx = 100 /*0x64*/;
    // 00470aeb  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00470aed  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470aef  83fa64                 +cmp edx, 0x64
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
    // 00470af2  7f0c                   -jg 0x470b00
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00470b00;
    }
    // 00470af4  83fa01                 +cmp edx, 1
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
    // 00470af7  7d07                   -jge 0x470b00
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00470b00;
    }
    // 00470af9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00470afe  eb0a                   -jmp 0x470b0a
    goto L_0x00470b0a;
L_0x00470b00:
    // 00470b00  83f864                 +cmp eax, 0x64
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(100 /*0x64*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470b03  7e05                   -jle 0x470b0a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00470b0a;
    }
    // 00470b05  b864000000             -mov eax, 0x64
    cpu.eax = 100 /*0x64*/;
L_0x00470b0a:
    // 00470b0a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00470b0c  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00470b0f  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470b11  b80e000000             -mov eax, 0xe
    cpu.eax = 14 /*0xe*/;
    // 00470b16  e8e50dfcff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00470b1b:
    // 00470b1b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b1c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b1d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_470b20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470b20  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00470b21  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470b22  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470b24  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00470b26  833d643a7a0000         +cmp dword ptr [0x7a3a64], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010340) /* 0x7a3a64 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470b2d  7438                   -je 0x470b67
    if (cpu.flags.zf)
    {
        goto L_0x00470b67;
    }
    // 00470b2f  ba64000000             -mov edx, 0x64
    cpu.edx = 100 /*0x64*/;
    // 00470b34  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00470b36  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470b38  83fa64                 +cmp edx, 0x64
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
    // 00470b3b  7f0c                   -jg 0x470b49
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00470b49;
    }
    // 00470b3d  83fa01                 +cmp edx, 1
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
    // 00470b40  7d07                   -jge 0x470b49
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00470b49;
    }
    // 00470b42  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00470b47  eb0a                   -jmp 0x470b53
    goto L_0x00470b53;
L_0x00470b49:
    // 00470b49  83f864                 +cmp eax, 0x64
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(100 /*0x64*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470b4c  7e05                   -jle 0x470b53
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00470b53;
    }
    // 00470b4e  b864000000             -mov eax, 0x64
    cpu.eax = 100 /*0x64*/;
L_0x00470b53:
    // 00470b53  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00470b55  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00470b58  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470b5a  0fafd1                 -imul edx, ecx
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.ecx)));
    // 00470b5d  b80e000000             -mov eax, 0xe
    cpu.eax = 14 /*0xe*/;
    // 00470b62  e8990dfcff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00470b67:
    // 00470b67  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b68  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b69  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_470b70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470b70  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00470b71  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470b72  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470b74  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00470b76  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470b78  e853ffffff             -call 0x470ad0
    cpu.esp -= 4;
    sub_470ad0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470b7d  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00470b7f  e81cffffff             -call 0x470aa0
    cpu.esp -= 4;
    sub_470aa0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470b84  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b85  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b86  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_470b90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470b90  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00470b91  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00470b92  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470b93  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470b95  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
    // 00470b9a  b80e000000             -mov eax, 0xe
    cpu.eax = 14 /*0xe*/;
    // 00470b9f  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00470ba1  8915408b6700           -mov dword ptr [0x678b40], edx
    app->getMemory<x86::reg32>(x86::reg32(6785856) /* 0x678b40 */) = cpu.edx;
    // 00470ba7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00470ba9  890d448b6700           -mov dword ptr [0x678b44], ecx
    app->getMemory<x86::reg32>(x86::reg32(6785860) /* 0x678b44 */) = cpu.ecx;
    // 00470baf  e84c0dfcff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470bb4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470bb5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470bb6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470bb7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_470bc0: native, nfs3hp::controls::steadyEngineRumble */

/* sub_470c60: native, nfs3hp::controls::startEngineRumble */

/* sub_470cb0: native, nfs3hp::controls::silenceEngineRumble */

/* sub_470d10: native, nfs3hp::controls::setForceGain */

/* sub_470d30: native, nfs3hp::controls::findForceController */

/* sub_470dc0: native, nfs3hp::controls::createForceEffects */

/* sub_470f40: native, nfs3hp::controls::stopForces */

/* sub_470f80: native, nfs3hp::controls::stopForcesAgain */

/* sub_470fc0: native, nfs3hp::controls::stopForcesOnceMore */

/* sub_471000: native, nfs3hp::controls::stopForcesOrActuatorsOn */

/* sub_471070: native, nfs3hp::controls::stopForcesOrActuatorsOff */

/* sub_4710f0: native, nfs3hp::controls::forcePlayerCar */

/* sub_4711e0: native, nfs3hp::controls::startRaceForces */

/* sub_471240: native, nfs3hp::controls::stopRaceEffects */

/* sub_471270: native, nfs3hp::controls::crashJolt */

/* sub_471440: native, nfs3hp::controls::bumpJolt */

/* sub_4714f0: native, nfs3hp::controls::gearChangeKick */

/* sub_471650: native, nfs3hp::controls::steeringAmount */

/* sub_4716e0: native, nfs3hp::controls::playSteeringSpring */

/* sub_471860: native, nfs3hp::controls::playSlideSpring */

/* sub_4719f0: native, nfs3hp::controls::playRoadRumble */

/* sub_471af0: native, nfs3hp::controls::playEngineRumble */

/* sub_471dc0: native, nfs3hp::controls::isForceCar */

/* sub_471e10: native, nfs3hp::controls::raceForcesFrame */

/* sub_471fb0: native, nfs3hp::controls::allActuatorsOff */

/* sub_471fd0: native, nfs3hp::controls::playJolt */

/* sub_4720a0: native, nfs3hp::controls::stopEngineRumble */

/* sub_4720e0: native, nfs3hp::controls::stopSteeringSpring */

/* sub_472110: native, nfs3hp::controls::carSpeedMph */

/* sub_4721f0: native, nfs3hp::frontend::nameValid */

/* sub_472230: native, nfs3hp::load::languageStub */

/* sub_472250: native, nfs3hp::load::setLanguage */

/* sub_472300: native, nfs3hp::load::setCameraViews */

/* sub_472360: native, nfs3hp::port::fields::refreshCustomColours */

/* sub_4723f0: native, nfs3hp::load::defaultConfig */

/* sub_472820: native, nfs3hp::load::saveConfig */

/* sub_4728e0: native, nfs3hp::load::fileWriteTime */

/* sub_472940: native, nfs3hp::load::fileOlder */

/* sub_472980: native, nfs3hp::load::loadConfig */

/* sub_472b60: native, nfs3hp::controls::loadControllers */

/* sub_472d10: native, nfs3hp::render::detailSettings */

/* sub_472f20: native, nfs3hp::frontend::titleDraw */

/* sub_472fc0: native, nfs3hp::frontend::titleWait */

/* sub_473010: native, nfs3hp::frontend::skipIntroCompare */

/* sub_473020: native, nfs3hp::frontend::menusEnter */

/* sub_473080: native, nfs3hp::frontend::menusLeave */

/* sub_4730d0: native, nfs3hp::frontend::frontEndFirstStart */

/* sub_4732b0: native, nfs3hp::frontend::postgameRun */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_473340(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473340  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473341  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473342  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473344  ba00040000             -mov edx, 0x400
    cpu.edx = 1024 /*0x400*/;
    // 00473349  b8008c6700             -mov eax, 0x678c00
    cpu.eax = 6786048 /*0x678c00*/;
    // 0047334e  e8b9d30600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473353  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473354  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473355  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_473360(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473360  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473361  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473362  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473363  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473364  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473366  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00473369  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0047336b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047336d  e8ee180000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473372  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473374  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473377  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473379  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0047337b  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
L_0x0047337e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0047337e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00473380  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 00473383  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00473385  0fb6b8008c6700         -movzx edi, byte ptr [eax + 0x678c00]
    cpu.edi = x86::reg32(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(6786048) /* 0x678c00 */));
    // 0047338c  39f9                   +cmp ecx, edi
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
    // 0047338e  7d10                   -jge 0x4733a0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004733a0;
    }
    // 00473390  42                     -inc edx
    (cpu.edx)++;
    // 00473391  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00473393  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473396  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473398  3b55fc                 +cmp edx, dword ptr [ebp - 4]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047339b  75e1                   -jne 0x47337e
    if (!cpu.flags.zf)
    {
        goto L_0x0047337e;
    }
    // 0047339d  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047339e  ebde                   -jmp 0x47337e
    goto L_0x0047337e;
L_0x004733a0:
    // 004733a0  fe80008c6700           -inc byte ptr [eax + 0x678c00]
    (app->getMemory<x86::reg8>(cpu.eax + x86::reg32(6786048) /* 0x678c00 */))++;
    // 004733a6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004733a8  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004733aa  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004733ab  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004733ac  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004733ad  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004733ae  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4733b0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004733b0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004733b1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004733b3  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 004733b6  fe8402008c6700         -inc byte ptr [edx + eax + 0x678c00]
    (app->getMemory<x86::reg8>(cpu.edx + x86::reg32(6786048) /* 0x678c00 */ + cpu.eax * 1))++;
    // 004733bd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004733be  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4733c0: native, nfs3hp::port::view::colourNearest */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4734d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004734d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004734d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004734d2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004734d3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004734d4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004734d5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004734d7  8b15d4d46f00           -mov edx, dword ptr [0x6fd4d4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 004734dd  83fa04                 +cmp edx, 4
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
    // 004734e0  7d54                   -jge 0x473536
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473536;
    }
    // 004734e2  8b1c95f8555500         -mov ebx, dword ptr [edx*4 + 0x5555f8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5592568) /* 0x5555f8 */ + cpu.edx * 4);
    // 004734e9  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004734eb  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004734f0  e86bfeffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004734f5  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004734f7  8b1dd4d46f00           -mov ebx, dword ptr [0x6fd4d4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 004734fd  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473500  8b349df8555500         -mov esi, dword ptr [ebx*4 + 0x5555f8]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5592568) /* 0x5555f8 */ + cpu.ebx * 4);
    // 00473507  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00473509  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0047350b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047350d  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0047350f  e89c71fcff             -call 0x43a6b0
    cpu.esp -= 4;
    sub_43a6b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473514  8b35d4d46f00           -mov esi, dword ptr [0x6fd4d4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 0047351a  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0047351c  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0047351e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473520  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473523  f73cb5f8555500         +idiv dword ptr [esi*4 + 0x5555f8]
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5592568) /* 0x5555f8 */ + cpu.esi * 4));
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0047352a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0047352f  e87cfeffff             -call 0x4733b0
    cpu.esp -= 4;
    sub_4733b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473534  eb03                   -jmp 0x473539
    goto L_0x00473539;
L_0x00473536:
    // 00473536  8d5afc                 -lea ebx, [edx - 4]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(-4) /* -0x4 */);
L_0x00473539:
    // 00473539  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0047353b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047353c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047353d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047353e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047353f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473540  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_473550(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473550  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473551  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473552  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473553  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473555  bb07000000             -mov ebx, 7
    cpu.ebx = 7 /*0x7*/;
    // 0047355a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047355c  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00473561  e8fafdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473566  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473567  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473568  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473569  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_473570(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473570  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473571  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473572  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473573  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473575  bb08000000             -mov ebx, 8
    cpu.ebx = 8 /*0x8*/;
    // 0047357a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047357c  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 00473581  e8dafdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473586  83c007                 -add eax, 7
    (cpu.eax) += x86::reg32(x86::sreg32(7 /*0x7*/));
    // 00473589  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047358a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047358b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047358c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4735c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 004735c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004735c1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004735c2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004735c3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004735c5  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004735c7  8b1dacd46f00           -mov ebx, dword ptr [0x6fd4ac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 004735cd  b832000000             -mov eax, 0x32
    cpu.eax = 50 /*0x32*/;
    // 004735d2  83fb0a                 +cmp ebx, 0xa
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(10 /*0xa*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004735d5  0f87d5000000           -ja 0x4736b0
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004736b0;
    }
    // 004735db  ff249d90354700         -jmp dword ptr [ebx*4 + 0x473590]
    cpu.ip = app->getMemory<x86::reg32>(4666768 + cpu.ebx * 4); goto dynamic_jump;
  case 0x004735e2:
    // 004735e2  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004735e4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004735e6  8a1d40b05500           -mov bl, byte ptr [0x55b040]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615680) /* 0x55b040 */);
    // 004735ec  e86ffdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004735f1  8a8041b05500           -mov al, byte ptr [eax + 0x55b041]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615681) /* 0x55b041 */);
    // 004735f7  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004735fc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004735fd  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004735fe  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004735ff  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00473600:
    // 00473600  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473602  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00473604  8a1d4cb05500           -mov bl, byte ptr [0x55b04c]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615692) /* 0x55b04c */);
    // 0047360a  e851fdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047360f  8a804db05500           -mov al, byte ptr [eax + 0x55b04d]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615693) /* 0x55b04d */);
    // 00473615  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0047361a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047361b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047361c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047361d  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0047361e:
    // 0047361e  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473620  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00473622  8a1d58b05500           -mov bl, byte ptr [0x55b058]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615704) /* 0x55b058 */);
    // 00473628  e833fdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047362d  8a8059b05500           -mov al, byte ptr [eax + 0x55b059]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615705) /* 0x55b059 */);
    // 00473633  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00473638  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473639  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047363a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047363b  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0047363c:
    // 0047363c  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047363e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00473640  8a1d64b05500           -mov bl, byte ptr [0x55b064]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615716) /* 0x55b064 */);
    // 00473646  e815fdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047364b  8a8065b05500           -mov al, byte ptr [eax + 0x55b065]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615717) /* 0x55b065 */);
    // 00473651  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00473656  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473657  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473658  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473659  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0047365a:
    // 0047365a  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047365c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0047365e  8a1d6fb05500           -mov bl, byte ptr [0x55b06f]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615727) /* 0x55b06f */);
    // 00473664  e8f7fcffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473669  8a8070b05500           -mov al, byte ptr [eax + 0x55b070]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615728) /* 0x55b070 */);
    // 0047366f  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00473674  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473675  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473676  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473677  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00473678:
    // 00473678  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047367a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0047367c  8a1d78b05500           -mov bl, byte ptr [0x55b078]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615736) /* 0x55b078 */);
    // 00473682  e8d9fcffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473687  8a8079b05500           -mov al, byte ptr [eax + 0x55b079]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615737) /* 0x55b079 */);
    // 0047368d  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00473692  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473693  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473694  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473695  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00473696:
    // 00473696  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473698  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0047369a  8a1d80b05500           -mov bl, byte ptr [0x55b080]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615744) /* 0x55b080 */);
    // 004736a0  e8bbfcffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004736a5  8a8081b05500           -mov al, byte ptr [eax + 0x55b081]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615745) /* 0x55b081 */);
    // 004736ab  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
L_0x004736b0:
    // 004736b0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004736b1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004736b2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004736b3  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4736c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004736c0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004736c1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004736c2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004736c3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004736c4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004736c5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004736c7  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004736c9  a130dc6f00             -mov eax, dword ptr [0x6fdc30]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7330864) /* 0x6fdc30 */);
    // 004736ce  894214                 -mov dword ptr [edx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 004736d1  8b4214                 -mov eax, dword ptr [edx + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */);
    // 004736d4  c7421c01000000         -mov dword ptr [edx + 0x1c], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */) = 1 /*0x1*/;
    // 004736db  e8305efcff             -call 0x439510
    cpu.esp -= 4;
    sub_439510(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004736e0  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004736e2  8d7a5c                 -lea edi, [edx + 0x5c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(92) /* 0x5c */);
    // 004736e5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004736e6  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004736e7  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004736e8  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 004736ed  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004736f2  8d7a40                 -lea edi, [edx + 0x40]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 004736f5  c7426800000000         -mov dword ptr [edx + 0x68], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(104) /* 0x68 */) = 0 /*0x0*/;
    // 004736fc  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004736fe  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004736ff  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473700  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473701  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473702  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473703  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_473710(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473710  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473711  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473712  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473713  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473714  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473716  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 00473719  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0047371c  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0047371e  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00473720  7505                   -jne 0x473727
    if (!cpu.flags.zf)
    {
        goto L_0x00473727;
    }
    // 00473722  bebcd26f00             -mov esi, 0x6fd2bc
    cpu.esi = 7328444 /*0x6fd2bc*/;
L_0x00473727:
    // 00473727  83fb01                 +cmp ebx, 1
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
    // 0047372a  7505                   -jne 0x473731
    if (!cpu.flags.zf)
    {
        goto L_0x00473731;
    }
    // 0047372c  be28d36f00             -mov esi, 0x6fd328
    cpu.esi = 7328552 /*0x6fd328*/;
L_0x00473731:
    // 00473731  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473734  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00473736  c7426800000000         -mov dword ptr [edx + 0x68], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(104) /* 0x68 */) = 0 /*0x0*/;
    // 0047373d  894214                 -mov dword ptr [edx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 00473740  8b4618                 -mov eax, dword ptr [esi + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 00473743  89421c                 -mov dword ptr [edx + 0x1c], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */) = cpu.eax;
    // 00473746  f6052eeb550020         +test byte ptr [0x55eb2e], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */) & 32 /*0x20*/));
    // 0047374d  0f846f000000           -je 0x4737c2
    if (cpu.flags.zf)
    {
        goto L_0x004737c2;
    }
    // 00473753  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 00473758  8d45dc                 -lea eax, [ebp - 0x24]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0047375b  e87053fcff             -call 0x438ad0
    cpu.esp -= 4;
    nfs3hp::load::makeCarId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473760  8d45dc                 -lea eax, [ebp - 0x24]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00473763  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473766  e82562fcff             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047376b  894214                 -mov dword ptr [edx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */) = cpu.eax;
L_0x0047376e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0047376e  e8897a0700             -call 0x4eb1fc
    cpu.esp -= 4;
    nfs3hp::watcom::rand(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473773  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473775  83e207                 +and edx, 7
    cpu.clear_co();
    cpu.set_szp((cpu.edx &= x86::reg32(x86::sreg32(7 /*0x7*/))));
    // 00473778  74f4                   -je 0x47376e
    if (cpu.flags.zf)
    {
        goto L_0x0047376e;
    }
    // 0047377a  83fa02                 +cmp edx, 2
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
    // 0047377d  7502                   -jne 0x473781
    if (!cpu.flags.zf)
    {
        goto L_0x00473781;
    }
    // 0047377f  ebed                   -jmp 0x47376e
    goto L_0x0047376e;
L_0x00473781:
    // 00473781  83fa03                 +cmp edx, 3
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
    // 00473784  74e8                   -je 0x47376e
    if (cpu.flags.zf)
    {
        goto L_0x0047376e;
    }
    // 00473786  8d45f0                 -lea eax, [ebp - 0x10]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00473789  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047378a  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047378d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047378e  8d45f4                 -lea eax, [ebp - 0xc]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00473791  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00473792  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473795  8d4dfc                 -lea ecx, [ebp - 4]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473798  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0047379a  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 0047379d  891500906700           -mov dword ptr [0x679000], edx
    app->getMemory<x86::reg32>(x86::reg32(6787072) /* 0x679000 */) = cpu.edx;
    // 004737a3  e86860fcff             -call 0x439810
    cpu.esp -= 4;
    nfs3hp::port::fields::carColourRgb(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004737a8  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004737ab  8a45fc                 -mov al, byte ptr [ebp - 4]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004737ae  884220                 -mov byte ptr [edx + 0x20], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(32) /* 0x20 */) = cpu.al;
    // 004737b1  8a45f4                 -mov al, byte ptr [ebp - 0xc]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004737b4  884221                 -mov byte ptr [edx + 0x21], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(33) /* 0x21 */) = cpu.al;
    // 004737b7  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004737ba  884222                 -mov byte ptr [edx + 0x22], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(34) /* 0x22 */) = cpu.al;
    // 004737bd  8a45f0                 -mov al, byte ptr [ebp - 0x10]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004737c0  eb15                   -jmp 0x4737d7
    goto L_0x004737d7;
L_0x004737c2:
    // 004737c2  8a464b                 -mov al, byte ptr [esi + 0x4b]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(75) /* 0x4b */);
    // 004737c5  884220                 -mov byte ptr [edx + 0x20], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(32) /* 0x20 */) = cpu.al;
    // 004737c8  8a464c                 -mov al, byte ptr [esi + 0x4c]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(76) /* 0x4c */);
    // 004737cb  884221                 -mov byte ptr [edx + 0x21], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(33) /* 0x21 */) = cpu.al;
    // 004737ce  8a464d                 -mov al, byte ptr [esi + 0x4d]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(77) /* 0x4d */);
    // 004737d1  884222                 -mov byte ptr [edx + 0x22], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(34) /* 0x22 */) = cpu.al;
    // 004737d4  8a464e                 -mov al, byte ptr [esi + 0x4e]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(78) /* 0x4e */);
L_0x004737d7:
    // 004737d7  884223                 -mov byte ptr [edx + 0x23], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(35) /* 0x23 */) = cpu.al;
    // 004737da  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004737dd  c7401801000000         -mov dword ptr [eax + 0x18], 1
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = 1 /*0x1*/;
    // 004737e4  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004737e7  8b4620                 -mov eax, dword ptr [esi + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    // 004737ea  894224                 -mov dword ptr [edx + 0x24], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(36) /* 0x24 */) = cpu.eax;
    // 004737ed  8b4624                 -mov eax, dword ptr [esi + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
    // 004737f0  894228                 -mov dword ptr [edx + 0x28], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(40) /* 0x28 */) = cpu.eax;
    // 004737f3  8b4628                 -mov eax, dword ptr [esi + 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(40) /* 0x28 */);
    // 004737f6  bb0b000000             -mov ebx, 0xb
    cpu.ebx = 11 /*0xb*/;
    // 004737fb  89422c                 -mov dword ptr [edx + 0x2c], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(44) /* 0x2c */) = cpu.eax;
    // 004737fe  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473801  8d5640                 -lea edx, [esi + 0x40]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 00473804  83c034                 -add eax, 0x34
    (cpu.eax) += x86::reg32(x86::sreg32(52 /*0x34*/));
    // 00473807  e824d60600             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047380c  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0047380f  897810                 -mov dword ptr [eax + 0x10], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */) = cpu.edi;
    // 00473812  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 00473819  7f1d                   -jg 0x473838
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00473838;
    }
    // 0047381b  8b0db8d36f00           -mov ecx, dword ptr [0x6fd3b8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 00473821  83f901                 +cmp ecx, 1
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
    // 00473824  7412                   -je 0x473838
    if (cpu.flags.zf)
    {
        goto L_0x00473838;
    }
    // 00473826  83f902                 +cmp ecx, 2
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
    // 00473829  740d                   -je 0x473838
    if (cpu.flags.zf)
    {
        goto L_0x00473838;
    }
    // 0047382b  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00473830  8d7840                 -lea edi, [eax + 0x40]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 00473833  8d7650                 -lea esi, [esi + 0x50]
    cpu.esi = x86::reg32(cpu.esi + x86::reg32(80) /* 0x50 */);
    // 00473836  eb10                   -jmp 0x473848
    goto L_0x00473848;
L_0x00473838:
    // 00473838  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0047383b  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00473840  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00473845  8d7840                 -lea edi, [eax + 0x40]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
L_0x00473848:
    // 00473848  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0047384a  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0047384d  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 00473850  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473853  e8b85cfcff             -call 0x439510
    cpu.esp -= 4;
    sub_439510(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473858  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0047385a  8d7a5c                 -lea edi, [edx + 0x5c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(92) /* 0x5c */);
    // 0047385d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0047385e  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0047385f  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00473860  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473862  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473863  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473864  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473865  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473866  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_473890(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00473890  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473891  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473892  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473893  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473894  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473896  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00473899  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0047389c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047389e  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004738a0  741e                   -je 0x4738c0
    if (cpu.flags.zf)
    {
        goto L_0x004738c0;
    }
    // 004738a2  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004738a5  8a152eeb5500           -mov dl, byte ptr [0x55eb2e]
    cpu.dl = app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */);
    // 004738ab  c74314f8010000         -mov dword ptr [ebx + 0x14], 0x1f8
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 504 /*0x1f8*/;
    // 004738b2  f6c220                 +test dl, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 32 /*0x20*/));
    // 004738b5  7449                   -je 0x473900
    if (cpu.flags.zf)
    {
        goto L_0x00473900;
    }
    // 004738b7  c74314f9010000         -mov dword ptr [ebx + 0x14], 0x1f9
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 505 /*0x1f9*/;
    // 004738be  eb40                   -jmp 0x473900
    goto L_0x00473900;
L_0x004738c0:
    // 004738c0  8b1dacd46f00           -mov ebx, dword ptr [0x6fd4ac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 004738c6  83fb08                 +cmp ebx, 8
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
    // 004738c9  772b                   -ja 0x4738f6
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004738f6;
    }
    // 004738cb  ff249d68384700         -jmp dword ptr [ebx*4 + 0x473868]
    cpu.ip = app->getMemory<x86::reg32>(4667496 + cpu.ebx * 4); goto dynamic_jump;
  case 0x004738d2:
    // 004738d2  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004738d5  c74314f5010000         -mov dword ptr [ebx + 0x14], 0x1f5
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 501 /*0x1f5*/;
    // 004738dc  eb22                   -jmp 0x473900
    goto L_0x00473900;
  case 0x004738de:
    // 004738de  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004738e1  c74314f6010000         -mov dword ptr [ebx + 0x14], 0x1f6
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 502 /*0x1f6*/;
    // 004738e8  eb16                   -jmp 0x473900
    goto L_0x00473900;
  case 0x004738ea:
    // 004738ea  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004738ed  c74314f7010000         -mov dword ptr [ebx + 0x14], 0x1f7
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 503 /*0x1f7*/;
    // 004738f4  eb0a                   -jmp 0x473900
    goto L_0x00473900;
  case 0x004738f6:
L_0x004738f6:
    // 004738f6  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004738f9  c74314f4010000         -mov dword ptr [ebx + 0x14], 0x1f4
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 500 /*0x1f4*/;
L_0x00473900:
    // 00473900  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473903  c7431c01000000         -mov dword ptr [ebx + 0x1c], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(28) /* 0x1c */) = 1 /*0x1*/;
    // 0047390a  c7431808000000         -mov dword ptr [ebx + 0x18], 8
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */) = 8 /*0x8*/;
    // 00473911  e85afcffff             -call 0x473570
    cpu.esp -= 4;
    sub_473570(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473916  894330                 -mov dword ptr [ebx + 0x30], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(48) /* 0x30 */) = cpu.eax;
    // 00473919  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0047391b  e8d00ef9ff             -call 0x4047f0
    cpu.esp -= 4;
    nfs3hp::airace::classValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473920  8d4b34                 -lea ecx, [ebx + 0x34]
    cpu.ecx = x86::reg32(cpu.ebx + x86::reg32(52) /* 0x34 */);
    // 00473923  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473925  bb0b000000             -mov ebx, 0xb
    cpu.ebx = 11 /*0xb*/;
    // 0047392a  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0047392c  e8ffd40600             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473931  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473934  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00473939  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 0047393e  8d7840                 -lea edi, [eax + 0x40]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 00473941  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00473943  be8c445600             -mov esi, 0x56448c
    cpu.esi = 5653644 /*0x56448c*/;
    // 00473948  8d785c                 -lea edi, [eax + 0x5c]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(92) /* 0x5c */);
    // 0047394b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0047394c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0047394d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0047394e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473950  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473951  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473952  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473953  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473954  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_473960(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473960  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473961  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473962  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473963  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473964  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473965  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473967  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 0047396a  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0047396d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047396f  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473972  e859fbffff             -call 0x4734d0
    cpu.esp -= 4;
    sub_4734d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473977  c7411c01000000         -mov dword ptr [ecx + 0x1c], 1
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */) = 1 /*0x1*/;
    // 0047397e  c7416800000000         -mov dword ptr [ecx + 0x68], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(104) /* 0x68 */) = 0 /*0x0*/;
    // 00473985  bb0b000000             -mov ebx, 0xb
    cpu.ebx = 11 /*0xb*/;
    // 0047398a  894114                 -mov dword ptr [ecx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 0047398d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047398f  c7411802000000         -mov dword ptr [ecx + 0x18], 2
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */) = 2 /*0x2*/;
    // 00473996  e8b5fbffff             -call 0x473550
    cpu.esp -= 4;
    sub_473550(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047399b  894130                 -mov dword ptr [ecx + 0x30], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(48) /* 0x30 */) = cpu.eax;
    // 0047399e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004739a0  895110                 -mov dword ptr [ecx + 0x10], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */) = cpu.edx;
    // 004739a3  e8480ef9ff             -call 0x4047f0
    cpu.esp -= 4;
    nfs3hp::airace::classValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004739a8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004739aa  8d4134                 -lea eax, [ecx + 0x34]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(52) /* 0x34 */);
    // 004739ad  e87ed40600             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004739b2  f6052eeb550020         +test byte ptr [0x55eb2e], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */) & 32 /*0x20*/));
    // 004739b9  0f8473000000           -je 0x473a32
    if (cpu.flags.zf)
    {
        goto L_0x00473a32;
    }
    // 004739bf  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 004739c4  8d45dc                 -lea eax, [ebp - 0x24]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004739c7  e80451fcff             -call 0x438ad0
    cpu.esp -= 4;
    nfs3hp::load::makeCarId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004739cc  8d45dc                 -lea eax, [ebp - 0x24]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004739cf  e8bc5ffcff             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004739d4  894114                 -mov dword ptr [ecx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */) = cpu.eax;
L_0x004739d7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004739d7  e820780700             -call 0x4eb1fc
    cpu.esp -= 4;
    nfs3hp::watcom::rand(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004739dc  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004739de  83e207                 +and edx, 7
    cpu.clear_co();
    cpu.set_szp((cpu.edx &= x86::reg32(x86::sreg32(7 /*0x7*/))));
    // 004739e1  74f4                   -je 0x4739d7
    if (cpu.flags.zf)
    {
        goto L_0x004739d7;
    }
    // 004739e3  83fa02                 +cmp edx, 2
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
    // 004739e6  7502                   -jne 0x4739ea
    if (!cpu.flags.zf)
    {
        goto L_0x004739ea;
    }
    // 004739e8  ebed                   -jmp 0x4739d7
    goto L_0x004739d7;
L_0x004739ea:
    // 004739ea  83fa03                 +cmp edx, 3
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
    // 004739ed  7502                   -jne 0x4739f1
    if (!cpu.flags.zf)
    {
        goto L_0x004739f1;
    }
    // 004739ef  ebe6                   -jmp 0x4739d7
    goto L_0x004739d7;
L_0x004739f1:
    // 004739f1  3b1500906700           +cmp edx, dword ptr [0x679000]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6787072) /* 0x679000 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004739f7  74de                   -je 0x4739d7
    if (cpu.flags.zf)
    {
        goto L_0x004739d7;
    }
    // 004739f9  8d45fc                 -lea eax, [ebp - 4]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004739fc  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004739fd  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00473a00  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00473a01  8d45f4                 -lea eax, [ebp - 0xc]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00473a04  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00473a05  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473a08  8d4df0                 -lea ecx, [ebp - 0x10]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00473a0b  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00473a0d  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 00473a10  e8fb5dfcff             -call 0x439810
    cpu.esp -= 4;
    nfs3hp::port::fields::carColourRgb(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473a15  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473a18  8a45f0                 -mov al, byte ptr [ebp - 0x10]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00473a1b  884220                 -mov byte ptr [edx + 0x20], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(32) /* 0x20 */) = cpu.al;
    // 00473a1e  8a45f4                 -mov al, byte ptr [ebp - 0xc]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00473a21  884221                 -mov byte ptr [edx + 0x21], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(33) /* 0x21 */) = cpu.al;
    // 00473a24  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00473a27  884222                 -mov byte ptr [edx + 0x22], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(34) /* 0x22 */) = cpu.al;
    // 00473a2a  8a45fc                 -mov al, byte ptr [ebp - 4]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473a2d  884223                 -mov byte ptr [edx + 0x23], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(35) /* 0x23 */) = cpu.al;
    // 00473a30  eb10                   -jmp 0x473a42
    goto L_0x00473a42;
L_0x00473a32:
    // 00473a32  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473a34  e8d70df9ff             -call 0x404810
    cpu.esp -= 4;
    nfs3hp::airace::classValueToo(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473a39  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473a3b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473a3d  e87ef9ffff             -call 0x4733c0
    cpu.esp -= 4;
    nfs3hp::port::view::colourNearest(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00473a42:
    // 00473a42  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473a45  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00473a4a  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00473a4f  8d7840                 -lea edi, [eax + 0x40]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 00473a52  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00473a54  be8c445600             -mov esi, 0x56448c
    cpu.esi = 5653644 /*0x56448c*/;
    // 00473a59  8d785c                 -lea edi, [eax + 0x5c]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(92) /* 0x5c */);
    // 00473a5c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00473a5d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00473a5e  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00473a5f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473a61  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473a62  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473a63  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473a64  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473a65  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473a66  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_473a70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473a70  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473a71  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473a72  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473a73  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473a74  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473a75  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473a77  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00473a79  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00473a7b  e840fbffff             -call 0x4735c0
    cpu.esp -= 4;
    sub_4735c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473a80  c7431804000000         -mov dword ptr [ebx + 0x18], 4
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */) = 4 /*0x4*/;
    // 00473a87  c7431c01000000         -mov dword ptr [ebx + 0x1c], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(28) /* 0x1c */) = 1 /*0x1*/;
    // 00473a8e  c6433400               -mov byte ptr [ebx + 0x34], 0
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(52) /* 0x34 */) = 0 /*0x0*/;
    // 00473a92  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00473a97  c7436800000000         -mov dword ptr [ebx + 0x68], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(104) /* 0x68 */) = 0 /*0x0*/;
    // 00473a9e  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00473aa3  8d7b40                 -lea edi, [ebx + 0x40]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(64) /* 0x40 */);
    // 00473aa6  894314                 -mov dword ptr [ebx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 00473aa9  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00473aab  be8c445600             -mov esi, 0x56448c
    cpu.esi = 5653644 /*0x56448c*/;
    // 00473ab0  8d7b5c                 -lea edi, [ebx + 0x5c]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(92) /* 0x5c */);
    // 00473ab3  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00473ab4  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00473ab5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00473ab6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ab7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ab8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ab9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473aba  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473abb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_473ac0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473ac0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473ac1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473ac2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473ac3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473ac4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473ac5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473ac7  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 00473aca  b938010000             -mov ecx, 0x138
    cpu.ecx = 312 /*0x138*/;
    // 00473acf  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00473ad1  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00473ad6  895485dc               -mov dword ptr [ebp + eax*4 - 0x24], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */ + cpu.eax * 4) = cpu.edx;
    // 00473ada  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
L_0x00473adc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473adc  83f809                 +cmp eax, 9
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(9 /*0x9*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473adf  7d0d                   -jge 0x473aee
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473aee;
    }
    // 00473ae1  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00473ae6  895485dc               -mov dword ptr [ebp + eax*4 - 0x24], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */ + cpu.eax * 4) = cpu.edx;
    // 00473aea  01d0                   +add eax, edx
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
    // 00473aec  ebee                   -jmp 0x473adc
    goto L_0x00473adc;
L_0x00473aee:
    // 00473aee  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 00473af3  eb2d                   -jmp 0x473b22
    goto L_0x00473b22;
L_0x00473af5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473af5  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473af7  e864110000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473afc  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473afe  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 00473b03  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473b06  41                     -inc ecx
    (cpu.ecx)++;
    // 00473b07  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
L_0x00473b09:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473b09  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473b10  837c28dc00             +cmp dword ptr [eax + ebp - 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-36) /* -0x24 */ + cpu.ebp * 1);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b15  7407                   -je 0x473b1e
    if (cpu.flags.zf)
    {
        goto L_0x00473b1e;
    }
    // 00473b17  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473b19  4b                     -dec ebx
    (cpu.ebx)--;
    // 00473b1a  895428dc               -mov dword ptr [eax + ebp - 0x24], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-36) /* -0x24 */ + cpu.ebp * 1) = cpu.edx;
L_0x00473b1e:
    // 00473b1e  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00473b20  7421                   -je 0x473b43
    if (cpu.flags.zf)
    {
        goto L_0x00473b43;
    }
L_0x00473b22:
    // 00473b22  833d70d46f0000         +cmp dword ptr [0x6fd470], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328880) /* 0x6fd470 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b29  75ca                   -jne 0x473af5
    if (!cpu.flags.zf)
    {
        goto L_0x00473af5;
    }
    // 00473b2b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473b2d  e82e110000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473b32  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473b34  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 00473b39  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473b3c  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473b3e  41                     -inc ecx
    (cpu.ecx)++;
    // 00473b3f  01f2                   +add edx, esi
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473b41  ebc6                   -jmp 0x473b09
    goto L_0x00473b09;
L_0x00473b43:
    // 00473b43  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00473b45  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00473b47  eb0f                   -jmp 0x473b58
    goto L_0x00473b58;
L_0x00473b49:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473b49  66890455fed36f00       -mov word ptr [edx*2 + 0x6fd3fe], ax
    app->getMemory<x86::reg16>(x86::reg32(7328766) /* 0x6fd3fe */ + cpu.edx * 2) = cpu.ax;
    // 00473b51  42                     -inc edx
    (cpu.edx)++;
    // 00473b52  40                     -inc eax
    (cpu.eax)++;
    // 00473b53  83fa07                 +cmp edx, 7
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b56  7d0a                   -jge 0x473b62
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473b62;
    }
L_0x00473b58:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473b58  837c85dc00             +cmp dword ptr [ebp + eax*4 - 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */ + cpu.eax * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b5d  75ea                   -jne 0x473b49
    if (!cpu.flags.zf)
    {
        goto L_0x00473b49;
    }
    // 00473b5f  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473b60  ebf6                   -jmp 0x473b58
    goto L_0x00473b58;
L_0x00473b62:
    // 00473b62  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473b64  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473b65  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473b66  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473b67  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473b68  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473b69  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_473b70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473b70  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473b71  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473b72  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473b73  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473b74  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473b75  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473b77  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00473b7a  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00473b7d  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00473b7f  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00473b81  7511                   -jne 0x473b94
    if (!cpu.flags.zf)
    {
        goto L_0x00473b94;
    }
    // 00473b83  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00473b85:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473b85  3b45fc                 +cmp eax, dword ptr [ebp - 4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b88  7d45                   -jge 0x473bcf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473bcf;
    }
    // 00473b8a  890485ecdb6f00         -mov dword ptr [eax*4 + 0x6fdbec], eax
    app->getMemory<x86::reg32>(x86::reg32(7330796) /* 0x6fdbec */ + cpu.eax * 4) = cpu.eax;
    // 00473b91  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473b92  ebf1                   -jmp 0x473b85
    goto L_0x00473b85;
L_0x00473b94:
    // 00473b94  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473b96  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 00473b98  eb06                   -jmp 0x473ba0
    goto L_0x00473ba0;
L_0x00473b9a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473b9a  43                     -inc ebx
    (cpu.ebx)++;
    // 00473b9b  83fb10                 +cmp ebx, 0x10
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b9e  7d2f                   -jge 0x473bcf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473bcf;
    }
L_0x00473ba0:
    // 00473ba0  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00473ba2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473ba2  3b55fc                 +cmp edx, dword ptr [ebp - 4]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473ba5  7df3                   -jge 0x473b9a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473b9a;
    }
    // 00473ba7  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473bae  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00473bb0  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00473bb3  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00473bb5  8b84c13cd56f00         -mov eax, dword ptr [ecx + eax*8 + 0x6fd53c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7329084) /* 0x6fd53c */ + cpu.eax * 8);
    // 00473bbc  0fbf0447               -movsx eax, word ptr [edi + eax*2]
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(app->getMemory<x86::reg16>(cpu.edi + cpu.eax * 2)));
    // 00473bc0  39d8                   +cmp eax, ebx
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
    // 00473bc2  7508                   -jne 0x473bcc
    if (!cpu.flags.zf)
    {
        goto L_0x00473bcc;
    }
    // 00473bc4  893495ecdb6f00         -mov dword ptr [edx*4 + 0x6fdbec], esi
    app->getMemory<x86::reg32>(x86::reg32(7330796) /* 0x6fdbec */ + cpu.edx * 4) = cpu.esi;
    // 00473bcb  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
L_0x00473bcc:
    // 00473bcc  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473bcd  ebd3                   -jmp 0x473ba2
    goto L_0x00473ba2;
L_0x00473bcf:
    // 00473bcf  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473bd1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473bd2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473bd3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473bd4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473bd5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473bd6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_473bf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00473bf0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473bf1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473bf2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473bf3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473bf4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473bf5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473bf6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473bf8  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00473bfb  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473bfd  b894d36f00             -mov eax, 0x6fd394
    cpu.eax = 7328660 /*0x6fd394*/;
    // 00473c02  891510d56f00           -mov dword ptr [0x6fd510], edx
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.edx;
    // 00473c08  891514d56f00           -mov dword ptr [0x6fd514], edx
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.edx;
    // 00473c0e  891518d56f00           -mov dword ptr [0x6fd518], edx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.edx;
    // 00473c14  89151cd56f00           -mov dword ptr [0x6fd51c], edx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.edx;
    // 00473c1a  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 00473c20  891524d56f00           -mov dword ptr [0x6fd524], edx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.edx;
    // 00473c26  891528d56f00           -mov dword ptr [0x6fd528], edx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edx;
    // 00473c2c  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00473c31  e8d6ca0600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473c36  a174d46f00             -mov eax, dword ptr [0x6fd474]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328884) /* 0x6fd474 */);
    // 00473c3b  a390d46f00             -mov dword ptr [0x6fd490], eax
    app->getMemory<x86::reg32>(x86::reg32(7328912) /* 0x6fd490 */) = cpu.eax;
    // 00473c40  a178d46f00             -mov eax, dword ptr [0x6fd478]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328888) /* 0x6fd478 */);
    // 00473c45  a394d46f00             -mov dword ptr [0x6fd494], eax
    app->getMemory<x86::reg32>(x86::reg32(7328916) /* 0x6fd494 */) = cpu.eax;
    // 00473c4a  a17cd46f00             -mov eax, dword ptr [0x6fd47c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328892) /* 0x6fd47c */);
    // 00473c4f  a398d46f00             -mov dword ptr [0x6fd498], eax
    app->getMemory<x86::reg32>(x86::reg32(7328920) /* 0x6fd498 */) = cpu.eax;
    // 00473c54  a180d46f00             -mov eax, dword ptr [0x6fd480]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328896) /* 0x6fd480 */);
    // 00473c59  a39cd46f00             -mov dword ptr [0x6fd49c], eax
    app->getMemory<x86::reg32>(x86::reg32(7328924) /* 0x6fd49c */) = cpu.eax;
    // 00473c5e  a184d46f00             -mov eax, dword ptr [0x6fd484]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328900) /* 0x6fd484 */);
    // 00473c63  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00473c65  a3a0d46f00             -mov dword ptr [0x6fd4a0], eax
    app->getMemory<x86::reg32>(x86::reg32(7328928) /* 0x6fd4a0 */) = cpu.eax;
    // 00473c6a  a188d46f00             -mov eax, dword ptr [0x6fd488]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328904) /* 0x6fd488 */);
    // 00473c6f  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 00473c72  a3a4d46f00             -mov dword ptr [0x6fd4a4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328932) /* 0x6fd4a4 */) = cpu.eax;
    // 00473c77  a1b0d36f00             -mov eax, dword ptr [0x6fd3b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 00473c7c  890da8d46f00           -mov dword ptr [0x6fd4a8], ecx
    app->getMemory<x86::reg32>(x86::reg32(7328936) /* 0x6fd4a8 */) = cpu.ecx;
    // 00473c82  83f804                 +cmp eax, 4
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
    // 00473c85  0f874e000000           -ja 0x473cd9
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00473cd9;
    }
    // 00473c8b  ff2485d83b4700         -jmp dword ptr [eax*4 + 0x473bd8]
    cpu.ip = app->getMemory<x86::reg32>(4668376 + cpu.eax * 4); goto dynamic_jump;
  case 0x00473c92:
    // 00473c92  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473c95  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473c9c  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00473c9e  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00473ca0  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00473ca3  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473ca5  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473ca7  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00473caa  8935f4d46f00           -mov dword ptr [0x6fd4f4], esi
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.esi;
    // 00473cb0  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00473cb2  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473cb5  052cd56f00             -add eax, 0x6fd52c
    (cpu.eax) += x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
    // 00473cba  8935f8d46f00           -mov dword ptr [0x6fd4f8], esi
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.esi;
    // 00473cc0  e84bfaffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473cc5  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473cc8  8b1518d56f00           -mov edx, dword ptr [0x6fd518]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473cce  40                     -inc eax
    (cpu.eax)++;
    // 00473ccf  42                     -inc edx
    (cpu.edx)++;
    // 00473cd0  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00473cd3  891518d56f00           -mov dword ptr [0x6fd518], edx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.edx;
L_0x00473cd9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473cd9  a1d0d46f00             -mov eax, dword ptr [0x6fd4d0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328976) /* 0x6fd4d0 */);
    // 00473cde  83f802                 +cmp eax, 2
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
    // 00473ce1  0f8ce6000000           -jl 0x473dcd
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00473dcd;
    }
    // 00473ce7  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00473cec  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473cf1  29c2                   +sub edx, eax
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473cf3  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 00473cf9  e9dd000000             -jmp 0x473ddb
    goto L_0x00473ddb;
  case 0x00473cfe:
    // 00473cfe  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 00473d03  890df4d46f00           -mov dword ptr [0x6fd4f4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.ecx;
    // 00473d09  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473d0b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473d0d  891df8d46f00           -mov dword ptr [0x6fd4f8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.ebx;
    // 00473d13  e8f8f9ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473d18  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 00473d1d  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473d23  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00473d28  46                     -inc esi
    (cpu.esi)++;
    // 00473d29  83c06c                 +add eax, 0x6c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(108 /*0x6c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473d2c  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00473d2e  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 00473d34  e8d7f9ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473d39  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473d3e  bf02000000             -mov edi, 2
    cpu.edi = 2 /*0x2*/;
    // 00473d43  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473d44  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 00473d47  a318d56f00             -mov dword ptr [0x6fd518], eax
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.eax;
    // 00473d4c  eb8b                   -jmp 0x473cd9
    goto L_0x00473cd9;
  case 0x00473d4e:
    // 00473d4e  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 00473d53  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00473d56  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473d58  a3f4d46f00             -mov dword ptr [0x6fd4f4], eax
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.eax;
    // 00473d5d  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
L_0x00473d62:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473d62  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 00473d67  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00473d6a  39c3                   +cmp ebx, eax
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
    // 00473d6c  0f8d67ffffff           -jge 0x473cd9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473cd9;
    }
    // 00473d72  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473d75  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473d7c  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00473d7e  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00473d81  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473d83  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00473d86  01c2                   +add edx, eax
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
    // 00473d88  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00473d8d  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00473d92  8dba6cd56f00           -lea edi, [edx + 0x6fd56c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(7329132) /* 0x6fd56c */);
    // 00473d98  898a44d56f00           -mov dword ptr [edx + 0x6fd544], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329092) /* 0x6fd544 */) = cpu.ecx;
    // 00473d9e  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00473da3  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00473da5  8b8240d56f00           -mov eax, dword ptr [edx + 0x6fd540]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329088) /* 0x6fd540 */);
    // 00473dab  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473dac  e8ef57fcff             -call 0x4395a0
    cpu.esp -= 4;
    sub_4395a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473db1  898294d56f00           -mov dword ptr [edx + 0x6fd594], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329172) /* 0x6fd594 */) = cpu.eax;
    // 00473db7  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473dba  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473dc0  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473dc1  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473dc2  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 00473dc5  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 00473dcb  eb95                   -jmp 0x473d62
    goto L_0x00473d62;
L_0x00473dcd:
    // 00473dcd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00473dcf  7405                   -je 0x473dd6
    if (cpu.flags.zf)
    {
        goto L_0x00473dd6;
    }
    // 00473dd1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x00473dd6:
    // 00473dd6  a320d56f00             -mov dword ptr [0x6fd520], eax
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.eax;
L_0x00473ddb:
    // 00473ddb  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00473ddd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473ddd  8b1d20d56f00           -mov ebx, dword ptr [0x6fd520]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00473de3  39d9                   +cmp ecx, ebx
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
    // 00473de5  7d2d                   -jge 0x473e14
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473e14;
    }
    // 00473de7  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473dea  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473df1  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00473df3  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00473df6  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473df9  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473dfb  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00473dfe  41                     -inc ecx
    (cpu.ecx)++;
    // 00473dff  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00473e01  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473e04  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473e09  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473e0a  e851fbffff             -call 0x473960
    cpu.esp -= 4;
    sub_473960(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473e0f  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 00473e12  ebc9                   -jmp 0x473ddd
    goto L_0x00473ddd;
L_0x00473e14:
    // 00473e14  8b3de8d46f00           -mov edi, dword ptr [0x6fd4e8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */);
    // 00473e1a  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00473e1c  7557                   -jne 0x473e75
    if (!cpu.flags.zf)
    {
        goto L_0x00473e75;
    }
    // 00473e1e  8b1518d56f00           -mov edx, dword ptr [0x6fd518]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473e24  8d0413                 -lea eax, [ebx + edx]
    cpu.eax = x86::reg32(cpu.ebx + cpu.edx * 1);
    // 00473e27  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00473e2c  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00473e2e  891528d56f00           -mov dword ptr [0x6fd528], edx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edx;
    // 00473e34  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00473e36  7d06                   -jge 0x473e3e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473e3e;
    }
    // 00473e38  893d28d56f00           -mov dword ptr [0x6fd528], edi
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edi;
L_0x00473e3e:
    // 00473e3e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00473e40:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473e40  3b0d28d56f00           +cmp ecx, dword ptr [0x6fd528]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473e46  7d2d                   -jge 0x473e75
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473e75;
    }
    // 00473e48  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473e4b  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473e52  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00473e54  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00473e57  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473e5a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473e5c  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00473e5f  41                     -inc ecx
    (cpu.ecx)++;
    // 00473e60  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00473e62  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473e65  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473e6a  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473e6b  e800fcffff             -call 0x473a70
    cpu.esp -= 4;
    sub_473a70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473e70  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 00473e73  ebcb                   -jmp 0x473e40
    goto L_0x00473e40;
L_0x00473e75:
    // 00473e75  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473e7a  8b1520d56f00           -mov edx, dword ptr [0x6fd520]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00473e80  a314d56f00             -mov dword ptr [0x6fd514], eax
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.eax;
    // 00473e85  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00473e87  a128d56f00             -mov eax, dword ptr [0x6fd528]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */);
    // 00473e8c  8b0d20d56f00           -mov ecx, dword ptr [0x6fd520]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00473e92  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00473e94  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473e99  891510d56f00           -mov dword ptr [0x6fd510], edx
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.edx;
    // 00473e9f  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00473ea1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473ea3  e8c8fcffff             -call 0x473b70
    cpu.esp -= 4;
    sub_473b70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473ea8  833db0d36f0000         +cmp dword ptr [0x6fd3b0], 0
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
    // 00473eaf  740e                   -je 0x473ebf
    if (cpu.flags.zf)
    {
        goto L_0x00473ebf;
    }
    // 00473eb1  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00473eb3  893500d56f00           -mov dword ptr [0x6fd500], esi
    app->getMemory<x86::reg32>(x86::reg32(7329024) /* 0x6fd500 */) = cpu.esi;
    // 00473eb9  8935fcd46f00           -mov dword ptr [0x6fd4fc], esi
    app->getMemory<x86::reg32>(x86::reg32(7329020) /* 0x6fd4fc */) = cpu.esi;
L_0x00473ebf:
    // 00473ebf  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473ec1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec7  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_473ee0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00473ee0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473ee1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473ee2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473ee3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473ee4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473ee5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473ee6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473ee8  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00473eeb  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00473ef0  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473ef2  b890d46f00             -mov eax, 0x6fd490
    cpu.eax = 7328912 /*0x6fd490*/;
    // 00473ef7  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 00473efa  ba1c000000             -mov edx, 0x1c
    cpu.edx = 28 /*0x1c*/;
    // 00473eff  890df0d46f00           -mov dword ptr [0x6fd4f0], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329008) /* 0x6fd4f0 */) = cpu.ecx;
    // 00473f05  e802c80600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473f0a  66833dbcd36f0000       +cmp word ptr [0x6fd3bc], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328700) /* 0x6fd3bc */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00473f12  753e                   -jne 0x473f52
    if (!cpu.flags.zf)
    {
        goto L_0x00473f52;
    }
    // 00473f14  e8a7fbffff             -call 0x473ac0
    cpu.esp -= 4;
    sub_473ac0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473f19  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00473f1b  eb05                   -jmp 0x473f22
    goto L_0x00473f22;
L_0x00473f1d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473f1d  83fb10                 +cmp ebx, 0x10
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473f20  7f0b                   -jg 0x473f2d
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00473f2d;
    }
L_0x00473f22:
    // 00473f22  66891c5dbed36f00       -mov word ptr [ebx*2 + 0x6fd3be], bx
    app->getMemory<x86::reg16>(x86::reg32(7328702) /* 0x6fd3be */ + cpu.ebx * 2) = cpu.bx;
    // 00473f2a  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473f2b  ebf0                   -jmp 0x473f1d
    goto L_0x00473f1d;
L_0x00473f2d:
    // 00473f2d  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00473f2f  eb05                   -jmp 0x473f36
    goto L_0x00473f36;
L_0x00473f31:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473f31  83fb10                 +cmp ebx, 0x10
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473f34  7d0d                   -jge 0x473f43
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473f43;
    }
L_0x00473f36:
    // 00473f36  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473f37  66c7045ddcd36f000100   -mov word ptr [ebx*2 + 0x6fd3dc], 1
    app->getMemory<x86::reg16>(x86::reg32(7328732) /* 0x6fd3dc */ + cpu.ebx * 2) = 1 /*0x1*/;
    // 00473f41  ebee                   -jmp 0x473f31
    goto L_0x00473f31;
L_0x00473f43:
    // 00473f43  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00473f48  b894d36f00             -mov eax, 0x6fd394
    cpu.eax = 7328660 /*0x6fd394*/;
    // 00473f4d  e8bac70600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00473f52:
    // 00473f52  8b1d70d46f00           -mov ebx, dword ptr [0x6fd470]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328880) /* 0x6fd470 */);
    // 00473f58  83fb01                 +cmp ebx, 1
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
    // 00473f5b  756a                   -jne 0x473fc7
    if (!cpu.flags.zf)
    {
        goto L_0x00473fc7;
    }
    // 00473f5d  a1bad36f00             -mov eax, dword ptr [0x6fd3ba]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328698) /* 0x6fd3ba */);
    // 00473f62  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00473f65  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 00473f6c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473f6e  e8ed0c0000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473f73  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473f75  01d9                   -add ecx, ebx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00473f77  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 00473f7c  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473f7f  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473f81  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473f83  8915c0d46f00           -mov dword ptr [0x6fd4c0], edx
    app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */) = cpu.edx;
    // 00473f89  e8d20c0000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473f8e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473f90  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473f93  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473f95  41                     -inc ecx
    (cpu.ecx)++;
    // 00473f96  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473f98  8915c4d46f00           -mov dword ptr [0x6fd4c4], edx
    app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */) = cpu.edx;
    // 00473f9e  e8bd0c0000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473fa3  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473fa5  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473fa8  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473faa  41                     -inc ecx
    (cpu.ecx)++;
    // 00473fab  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473fad  8915c8d46f00           -mov dword ptr [0x6fd4c8], edx
    app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */) = cpu.edx;
    // 00473fb3  e8a80c0000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473fb8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473fba  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473fbd  f7fb                   +idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473fbf  8915ccd46f00           -mov dword ptr [0x6fd4cc], edx
    app->getMemory<x86::reg32>(x86::reg32(7328972) /* 0x6fd4cc */) = cpu.edx;
    // 00473fc5  eb1a                   -jmp 0x473fe1
    goto L_0x00473fe1;
L_0x00473fc7:
    // 00473fc7  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00473fc9  8935c4d46f00           -mov dword ptr [0x6fd4c4], esi
    app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */) = cpu.esi;
    // 00473fcf  8935c8d46f00           -mov dword ptr [0x6fd4c8], esi
    app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */) = cpu.esi;
    // 00473fd5  8935ccd46f00           -mov dword ptr [0x6fd4cc], esi
    app->getMemory<x86::reg32>(x86::reg32(7328972) /* 0x6fd4cc */) = cpu.esi;
    // 00473fdb  8935c0d46f00           -mov dword ptr [0x6fd4c0], esi
    app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */) = cpu.esi;
L_0x00473fe1:
    // 00473fe1  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 00473fe6  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00473fe8  a1b0d36f00             -mov eax, dword ptr [0x6fd3b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 00473fed  890d14d56f00           -mov dword ptr [0x6fd514], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.ecx;
    // 00473ff3  890d18d56f00           -mov dword ptr [0x6fd518], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.ecx;
    // 00473ff9  890d1cd56f00           -mov dword ptr [0x6fd51c], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.ecx;
    // 00473fff  890d20d56f00           -mov dword ptr [0x6fd520], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.ecx;
    // 00474005  890d24d56f00           -mov dword ptr [0x6fd524], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.ecx;
    // 0047400b  890d28d56f00           -mov dword ptr [0x6fd528], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ecx;
    // 00474011  891db4d46f00           -mov dword ptr [0x6fd4b4], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */) = cpu.ebx;
    // 00474017  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 0047401a  890d10d56f00           -mov dword ptr [0x6fd510], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.ecx;
    // 00474020  83f804                 +cmp eax, 4
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
    // 00474023  0f87f0010000           -ja 0x474219
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00474219;
    }
    // 00474029  ff2485c83e4700         -jmp dword ptr [eax*4 + 0x473ec8]
    cpu.ip = app->getMemory<x86::reg32>(4669128 + cpu.eax * 4); goto dynamic_jump;
  case 0x00474030:
    // 00474030  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474033  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0047403a  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0047403c  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0047403e  893df4d46f00           -mov dword ptr [0x6fd4f4], edi
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.edi;
    // 00474044  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474047  893df8d46f00           -mov dword ptr [0x6fd4f8], edi
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.edi;
    // 0047404d  8d3cc500000000         -lea edi, [eax*8]
    cpu.edi = x86::reg32(cpu.eax * 8);
    // 00474054  be2cd56f00             -mov esi, 0x6fd52c
    cpu.esi = 7329068 /*0x6fd52c*/;
    // 00474059  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0047405b  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047405d  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 0047405f  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00474061  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00474063  e8a8f6ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474068  898f3cd56f00           -mov dword ptr [edi + 0x6fd53c], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(7329084) /* 0x6fd53c */) = cpu.ecx;
    // 0047406e  66833c4dded36f0000     +cmp word ptr [ecx*2 + 0x6fd3de], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328734) /* 0x6fd3de */ + cpu.ecx * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00474077  750e                   -jne 0x474087
    if (!cpu.flags.zf)
    {
        goto L_0x00474087;
    }
    // 00474079  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047407c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0047407e  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047407f  e83cf6ffff             -call 0x4736c0
    cpu.esp -= 4;
    sub_4736c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474084  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
L_0x00474087:
    // 00474087  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047408a  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474090  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474091  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474092  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474093  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 00474099  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 0047409c  e978010000             -jmp 0x474219
    goto L_0x00474219;
  case 0x004740a1:
    // 004740a1  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 004740a6  890df4d46f00           -mov dword ptr [0x6fd4f4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.ecx;
    // 004740ac  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004740ae  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004740b0  890df8d46f00           -mov dword ptr [0x6fd4f8], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.ecx;
    // 004740b6  e855f6ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004740bb  66a1ded36f00           -mov ax, word ptr [0x6fd3de]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(7328734) /* 0x6fd3de */);
    // 004740c1  890d3cd56f00           -mov dword ptr [0x6fd53c], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329084) /* 0x6fd53c */) = cpu.ecx;
    // 004740c7  6685c0                 +test ax, ax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.ax & cpu.ax));
    // 004740ca  7511                   -jne 0x4740dd
    if (!cpu.flags.zf)
    {
        goto L_0x004740dd;
    }
    // 004740cc  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004740cf  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 004740d4  43                     -inc ebx
    (cpu.ebx)++;
    // 004740d5  e8e6f5ffff             -call 0x4736c0
    cpu.esp -= 4;
    sub_4736c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004740da  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
L_0x004740dd:
    // 004740dd  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004740e0  46                     -inc esi
    (cpu.esi)++;
    // 004740e1  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 004740e4  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004740eb  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004740ed  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004740f2  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004740f5  41                     -inc ecx
    (cpu.ecx)++;
    // 004740f6  8d3cc500000000         -lea edi, [eax*8]
    cpu.edi = x86::reg32(cpu.eax * 8);
    // 004740fd  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004740ff  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 00474101  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474106  be2cd56f00             -mov esi, 0x6fd52c
    cpu.esi = 7329068 /*0x6fd52c*/;
    // 0047410b  40                     -inc eax
    (cpu.eax)++;
    // 0047410c  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 0047410e  a318d56f00             -mov dword ptr [0x6fd518], eax
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.eax;
    // 00474113  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00474115  e8f6f5ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047411a  898f3cd56f00           -mov dword ptr [edi + 0x6fd53c], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(7329084) /* 0x6fd53c */) = cpu.ecx;
    // 00474120  66833c4dded36f0000     +cmp word ptr [ecx*2 + 0x6fd3de], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328734) /* 0x6fd3de */ + cpu.ecx * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00474129  750e                   -jne 0x474139
    if (!cpu.flags.zf)
    {
        goto L_0x00474139;
    }
    // 0047412b  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047412e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00474130  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474131  e88af5ffff             -call 0x4736c0
    cpu.esp -= 4;
    sub_4736c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474136  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
L_0x00474139:
    // 00474139  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047413c  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474142  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474143  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474144  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474145  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 0047414b  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 0047414e  e9c6000000             -jmp 0x474219
    goto L_0x00474219;
  case 0x00474153:
    // 00474153  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 00474158  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047415b  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047415d  a3f4d46f00             -mov dword ptr [0x6fd4f4], eax
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.eax;
    // 00474162  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
L_0x00474167:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474167  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 0047416c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047416f  39c3                   +cmp ebx, eax
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
    // 00474171  0f8d9d000000           -jge 0x474214
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474214;
    }
    // 00474177  0fbfc3                 -movsx eax, bx
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.bx));
    // 0047417a  e811250300             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047417f  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 00474180  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00474183  83f8ff                 +cmp eax, -1
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
    // 00474186  0f8482000000           -je 0x47420e
    if (cpu.flags.zf)
    {
        goto L_0x0047420e;
    }
    // 0047418c  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047418f  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474196  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474198  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0047419b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047419d  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004741a0  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 004741a5  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004741a7  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004741ac  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004741b1  898244d56f00           -mov dword ptr [edx + 0x6fd544], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329092) /* 0x6fd544 */) = cpu.eax;
    // 004741b7  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004741ba  8dba6cd56f00           -lea edi, [edx + 0x6fd56c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(7329132) /* 0x6fd56c */);
    // 004741c0  89823cd56f00           -mov dword ptr [edx + 0x6fd53c], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329084) /* 0x6fd53c */) = cpu.eax;
    // 004741c6  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004741c8  8b8240d56f00           -mov eax, dword ptr [edx + 0x6fd540]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329088) /* 0x6fd540 */);
    // 004741ce  e8cd53fcff             -call 0x4395a0
    cpu.esp -= 4;
    sub_4395a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004741d3  898294d56f00           -mov dword ptr [edx + 0x6fd594], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329172) /* 0x6fd594 */) = cpu.eax;
    // 004741d9  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004741dc  66833c45ded36f0000     +cmp word ptr [eax*2 + 0x6fd3de], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328734) /* 0x6fd3de */ + cpu.eax * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004741e5  7513                   -jne 0x4741fa
    if (!cpu.flags.zf)
    {
        goto L_0x004741fa;
    }
    // 004741e7  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 004741ec  01d0                   +add eax, edx
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
    // 004741ee  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004741f1  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004741f2  e8c9f4ffff             -call 0x4736c0
    cpu.esp -= 4;
    sub_4736c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004741f7  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
L_0x004741fa:
    // 004741fa  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474200  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474203  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474204  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474205  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 0047420b  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
L_0x0047420e:
    // 0047420e  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047420f  e953ffffff             -jmp 0x474167
    goto L_0x00474167;
L_0x00474214:
    // 00474214  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
L_0x00474219:
    // 00474219  bf08000000             -mov edi, 8
    cpu.edi = 8 /*0x8*/;
    // 0047421e  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474223  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00474225  29c7                   -sub edi, eax
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.eax));
L_0x00474227:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474227  39fb                   +cmp ebx, edi
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
    // 00474229  7d56                   -jge 0x474281
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474281;
    }
    // 0047422b  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047422e  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474235  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474237  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0047423a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047423c  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0047423f  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474241  668b344dded36f00       -mov si, word ptr [ecx*2 + 0x6fd3de]
    cpu.si = app->getMemory<x86::reg16>(x86::reg32(7328734) /* 0x6fd3de */ + cpu.ecx * 2);
    // 00474249  052cd56f00             -add eax, 0x6fd52c
    (cpu.eax) += x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
    // 0047424e  6685f6                 +test si, si
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.si & cpu.si));
    // 00474251  7511                   -jne 0x474264
    if (!cpu.flags.zf)
    {
        goto L_0x00474264;
    }
    // 00474253  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00474255  e806f7ffff             -call 0x473960
    cpu.esp -= 4;
    sub_473960(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047425a  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047425d  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047425e  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047425f  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 00474262  eb1a                   -jmp 0x47427e
    goto L_0x0047427e;
L_0x00474264:
    // 00474264  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00474266  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474269  e8f2f6ffff             -call 0x473960
    cpu.esp -= 4;
    sub_473960(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047426e  a120d56f00             -mov eax, dword ptr [0x6fd520]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00474273  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474274  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474275  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474276  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 00474279  a320d56f00             -mov dword ptr [0x6fd520], eax
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.eax;
L_0x0047427e:
    // 0047427e  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047427f  eba6                   -jmp 0x474227
    goto L_0x00474227;
L_0x00474281:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474281  a1bad36f00             -mov eax, dword ptr [0x6fd3ba]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328698) /* 0x6fd3ba */);
    // 00474286  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00474289  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047428c  39d0                   +cmp eax, edx
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
    // 0047428e  7e1d                   -jle 0x4742ad
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004742ad;
    }
    // 00474290  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474293  8b3d20d56f00           -mov edi, dword ptr [0x6fd520]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00474299  8d5a01                 -lea ebx, [edx + 1]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 0047429c  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0047429d  4e                     +dec esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0047429e  4f                     +dec edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0047429f  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 004742a2  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 004742a5  893d20d56f00           -mov dword ptr [0x6fd520], edi
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edi;
    // 004742ab  ebd4                   -jmp 0x474281
    goto L_0x00474281;
L_0x004742ad:
    // 004742ad  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004742b2  8b1520d56f00           -mov edx, dword ptr [0x6fd520]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 004742b8  a314d56f00             -mov dword ptr [0x6fd514], eax
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.eax;
    // 004742bd  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004742bf  babed36f00             -mov edx, 0x6fd3be
    cpu.edx = 7328702 /*0x6fd3be*/;
    // 004742c4  a310d56f00             -mov dword ptr [0x6fd510], eax
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.eax;
    // 004742c9  e8a2f8ffff             -call 0x473b70
    cpu.esp -= 4;
    sub_473b70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004742ce  a1bad36f00             -mov eax, dword ptr [0x6fd3ba]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328698) /* 0x6fd3ba */);
    // 004742d3  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004742d6  8b0445fcd36f00         -mov eax, dword ptr [eax*2 + 0x6fd3fc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328764) /* 0x6fd3fc */ + cpu.eax * 2);
    // 004742dd  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004742df  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004742e2  890dfcd46f00           -mov dword ptr [0x6fd4fc], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329020) /* 0x6fd4fc */) = cpu.ecx;
    // 004742e8  890d00d56f00           -mov dword ptr [0x6fd500], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329024) /* 0x6fd500 */) = cpu.ecx;
    // 004742ee  a3acd46f00             -mov dword ptr [0x6fd4ac], eax
    app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */) = cpu.eax;
    // 004742f3  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004742f5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742f6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742f7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742f8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742f9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742fa  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742fb  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip  */
void Application::sub_474310(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00474310  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00474311  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00474312  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00474313  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00474314  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00474315  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00474316  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00474318  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0047431b  a10ad46f00             -mov eax, dword ptr [0x6fd40a]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328778) /* 0x6fd40a */);
    // 00474320  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00474325  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00474328  8915f0d46f00           -mov dword ptr [0x6fd4f0], edx
    app->getMemory<x86::reg32>(x86::reg32(7329008) /* 0x6fd4f0 */) = cpu.edx;
    // 0047432e  8b048504d05500         -mov eax, dword ptr [eax*4 + 0x55d004]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5623812) /* 0x55d004 */ + cpu.eax * 4);
    // 00474335  ba1c000000             -mov edx, 0x1c
    cpu.edx = 28 /*0x1c*/;
    // 0047433a  a3acd46f00             -mov dword ptr [0x6fd4ac], eax
    app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */) = cpu.eax;
    // 0047433f  b890d46f00             -mov eax, 0x6fd490
    cpu.eax = 7328912 /*0x6fd490*/;
    // 00474344  e8c3c30600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474349  66833d0cd46f0000       +cmp word ptr [0x6fd40c], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328780) /* 0x6fd40c */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00474351  7523                   -jne 0x474376
    if (!cpu.flags.zf)
    {
        goto L_0x00474376;
    }
    // 00474353  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00474355  eb05                   -jmp 0x47435c
    goto L_0x0047435c;
L_0x00474357:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474357  83fb10                 +cmp ebx, 0x10
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047435a  7f0b                   -jg 0x474367
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00474367;
    }
L_0x0047435c:
    // 0047435c  66891c5d0ed46f00       -mov word ptr [ebx*2 + 0x6fd40e], bx
    app->getMemory<x86::reg16>(x86::reg32(7328782) /* 0x6fd40e */ + cpu.ebx * 2) = cpu.bx;
    // 00474364  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474365  ebf0                   -jmp 0x474357
    goto L_0x00474357;
L_0x00474367:
    // 00474367  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 0047436c  b894d36f00             -mov eax, 0x6fd394
    cpu.eax = 7328660 /*0x6fd394*/;
    // 00474371  e896c30600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00474376:
    // 00474376  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00474378  8b1db0d36f00           -mov ebx, dword ptr [0x6fd3b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0047437e  890d14d56f00           -mov dword ptr [0x6fd514], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.ecx;
    // 00474384  890d18d56f00           -mov dword ptr [0x6fd518], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.ecx;
    // 0047438a  890d1cd56f00           -mov dword ptr [0x6fd51c], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.ecx;
    // 00474390  890d20d56f00           -mov dword ptr [0x6fd520], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.ecx;
    // 00474396  890d24d56f00           -mov dword ptr [0x6fd524], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.ecx;
    // 0047439c  890d28d56f00           -mov dword ptr [0x6fd528], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ecx;
    // 004743a2  890d10d56f00           -mov dword ptr [0x6fd510], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.ecx;
    // 004743a8  83fb01                 +cmp ebx, 1
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
    // 004743ab  7e0c                   -jle 0x4743b9
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004743b9;
    }
    // 004743ad  c705b4d46f0002000000   -mov dword ptr [0x6fd4b4], 2
    app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */) = 2 /*0x2*/;
    // 004743b7  eb0a                   -jmp 0x4743c3
    goto L_0x004743c3;
L_0x004743b9:
    // 004743b9  c705b4d46f0004000000   -mov dword ptr [0x6fd4b4], 4
    app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */) = 4 /*0x4*/;
L_0x004743c3:
    // 004743c3  833d70d46f0001         +cmp dword ptr [0x6fd470], 1
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
    // 004743ca  7569                   -jne 0x474435
    if (!cpu.flags.zf)
    {
        goto L_0x00474435;
    }
    // 004743cc  a10ad46f00             -mov eax, dword ptr [0x6fd40a]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328778) /* 0x6fd40a */);
    // 004743d1  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004743d4  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 004743db  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004743dd  e87e080000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004743e2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004743e4  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 004743e9  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004743ec  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004743ee  41                     -inc ecx
    (cpu.ecx)++;
    // 004743ef  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004743f1  8915c0d46f00           -mov dword ptr [0x6fd4c0], edx
    app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */) = cpu.edx;
    // 004743f7  e864080000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004743fc  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004743fe  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00474401  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00474403  41                     -inc ecx
    (cpu.ecx)++;
    // 00474404  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00474406  8915c4d46f00           -mov dword ptr [0x6fd4c4], edx
    app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */) = cpu.edx;
    // 0047440c  e84f080000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474411  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474413  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00474416  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00474418  41                     -inc ecx
    (cpu.ecx)++;
    // 00474419  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0047441b  8915c8d46f00           -mov dword ptr [0x6fd4c8], edx
    app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */) = cpu.edx;
    // 00474421  e83a080000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474426  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474428  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0047442b  f7fb                   +idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0047442d  8915ccd46f00           -mov dword ptr [0x6fd4cc], edx
    app->getMemory<x86::reg32>(x86::reg32(7328972) /* 0x6fd4cc */) = cpu.edx;
    // 00474433  eb1a                   -jmp 0x47444f
    goto L_0x0047444f;
L_0x00474435:
    // 00474435  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00474437  8915c4d46f00           -mov dword ptr [0x6fd4c4], edx
    app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */) = cpu.edx;
    // 0047443d  8915c8d46f00           -mov dword ptr [0x6fd4c8], edx
    app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */) = cpu.edx;
    // 00474443  8915ccd46f00           -mov dword ptr [0x6fd4cc], edx
    app->getMemory<x86::reg32>(x86::reg32(7328972) /* 0x6fd4cc */) = cpu.edx;
    // 00474449  8915c0d46f00           -mov dword ptr [0x6fd4c0], edx
    app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */) = cpu.edx;
L_0x0047444f:
    // 0047444f  a1b0d36f00             -mov eax, dword ptr [0x6fd3b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 00474454  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00474456  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00474458  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 0047445b  83f804                 +cmp eax, 4
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
    // 0047445e  0f873a010000           -ja 0x47459e
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0047459e;
    }
    // 00474464  ff2485fc424700         -jmp dword ptr [eax*4 + 0x4742fc]
    cpu.ip = app->getMemory<x86::reg32>(4670204 + cpu.eax * 4); goto dynamic_jump;
  case 0x0047446b:
    // 0047446b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0047446d  a3f4d46f00             -mov dword ptr [0x6fd4f4], eax
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.eax;
    // 00474472  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
    // 00474477  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047447a  8d1c8500000000         -lea ebx, [eax*4]
    cpu.ebx = x86::reg32(cpu.eax * 4);
    // 00474481  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00474483  c1e302                 -shl ebx, 2
    cpu.ebx <<= 2 /*0x2*/ % 32;
    // 00474486  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00474488  c1e303                 -shl ebx, 3
    cpu.ebx <<= 3 /*0x3*/ % 32;
    // 0047448b  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0047448d  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0047448f  052cd56f00             -add eax, 0x6fd52c
    (cpu.eax) += x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
    // 00474494  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00474496  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474497  e874f2ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047449c  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047449f  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004744a5  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004744a6  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004744a7  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 004744aa  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 004744b0  e9e9000000             -jmp 0x47459e
    goto L_0x0047459e;
  case 0x004744b5:
    // 004744b5  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 004744ba  893df4d46f00           -mov dword ptr [0x6fd4f4], edi
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.edi;
    // 004744c0  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004744c2  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004744c4  893df8d46f00           -mov dword ptr [0x6fd4f8], edi
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.edi;
    // 004744ca  e841f2ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004744cf  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 004744d4  8b1518d56f00           -mov edx, dword ptr [0x6fd518]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004744da  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004744df  42                     -inc edx
    (cpu.edx)++;
    // 004744e0  83c06c                 +add eax, 0x6c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(108 /*0x6c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004744e3  891518d56f00           -mov dword ptr [0x6fd518], edx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.edx;
    // 004744e9  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004744eb  e820f2ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004744f0  8b1d18d56f00           -mov ebx, dword ptr [0x6fd518]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004744f6  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 004744fb  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004744fc  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 004744ff  891d18d56f00           -mov dword ptr [0x6fd518], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.ebx;
    // 00474505  e994000000             -jmp 0x47459e
    goto L_0x0047459e;
  case 0x0047450a:
    // 0047450a  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 0047450f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00474512  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00474514  a3f4d46f00             -mov dword ptr [0x6fd4f4], eax
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.eax;
    // 00474519  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
L_0x0047451e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0047451e  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 00474523  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00474526  39c3                   +cmp ebx, eax
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
    // 00474528  7d6f                   -jge 0x474599
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474599;
    }
    // 0047452a  0fbfc3                 -movsx eax, bx
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.bx));
    // 0047452d  e85e210300             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474532  0fbfc8                 -movsx ecx, ax
    cpu.ecx = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 00474535  83f9ff                 +cmp ecx, -1
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
    // 00474538  745c                   -je 0x474596
    if (cpu.flags.zf)
    {
        goto L_0x00474596;
    }
    // 0047453a  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047453d  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474544  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474546  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474549  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047454b  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0047454e  01c2                   +add edx, eax
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
    // 00474550  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 00474555  8dba6cd56f00           -lea edi, [edx + 0x6fd56c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(7329132) /* 0x6fd56c */);
    // 0047455b  898a3cd56f00           -mov dword ptr [edx + 0x6fd53c], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329084) /* 0x6fd53c */) = cpu.ecx;
    // 00474561  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00474566  89b244d56f00           -mov dword ptr [edx + 0x6fd544], esi
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329092) /* 0x6fd544 */) = cpu.esi;
    // 0047456c  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00474571  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00474573  8b8240d56f00           -mov eax, dword ptr [edx + 0x6fd540]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329088) /* 0x6fd540 */);
    // 00474579  e82250fcff             -call 0x4395a0
    cpu.esp -= 4;
    sub_4395a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047457e  898294d56f00           -mov dword ptr [edx + 0x6fd594], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329172) /* 0x6fd594 */) = cpu.eax;
    // 00474584  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474587  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 0047458c  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047458d  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047458e  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 00474591  a318d56f00             -mov dword ptr [0x6fd518], eax
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.eax;
L_0x00474596:
    // 00474596  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474597  eb85                   -jmp 0x47451e
    goto L_0x0047451e;
L_0x00474599:
    // 00474599  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
L_0x0047459e:
    // 0047459e  be08000000             -mov esi, 8
    cpu.esi = 8 /*0x8*/;
    // 004745a3  8b1518d56f00           -mov edx, dword ptr [0x6fd518]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004745a9  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004745ab  29d6                   -sub esi, edx
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.edx));
L_0x004745ad:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004745ad  39f3                   +cmp ebx, esi
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
    // 004745af  7d38                   -jge 0x4745e9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004745e9;
    }
    // 004745b1  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004745b4  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004745bb  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004745bd  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004745c0  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004745c2  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004745c5  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004745c8  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004745ca  43                     -inc ebx
    (cpu.ebx)++;
    // 004745cb  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004745d0  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004745d2  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004745d3  e888f3ffff             -call 0x473960
    cpu.esp -= 4;
    sub_473960(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004745d8  a120d56f00             -mov eax, dword ptr [0x6fd520]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 004745dd  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 004745e0  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004745e1  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004745e2  a320d56f00             -mov dword ptr [0x6fd520], eax
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.eax;
    // 004745e7  ebc4                   -jmp 0x4745ad
    goto L_0x004745ad;
L_0x004745e9:
    // 004745e9  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004745ee  8b1520d56f00           -mov edx, dword ptr [0x6fd520]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 004745f4  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004745f6  a314d56f00             -mov dword ptr [0x6fd514], eax
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.eax;
    // 004745fb  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004745fd  ba0ed46f00             -mov edx, 0x6fd40e
    cpu.edx = 7328782 /*0x6fd40e*/;
    // 00474602  a310d56f00             -mov dword ptr [0x6fd510], eax
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.eax;
    // 00474607  e864f5ffff             -call 0x473b70
    cpu.esp -= 4;
    sub_473b70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047460c  890dfcd46f00           -mov dword ptr [0x6fd4fc], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329020) /* 0x6fd4fc */) = cpu.ecx;
    // 00474612  890d00d56f00           -mov dword ptr [0x6fd500], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329024) /* 0x6fd500 */) = cpu.ecx;
    // 00474618  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0047461a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047461b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047461c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047461d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047461e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047461f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474620  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_474640(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00474640  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00474641  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00474642  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00474643  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00474644  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00474645  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00474646  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00474648  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0047464b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0047464d  8a252eeb5500           -mov ah, byte ptr [0x55eb2e]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */);
    // 00474653  891514d56f00           -mov dword ptr [0x6fd514], edx
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.edx;
    // 00474659  891518d56f00           -mov dword ptr [0x6fd518], edx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.edx;
    // 0047465f  89151cd56f00           -mov dword ptr [0x6fd51c], edx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.edx;
    // 00474665  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 0047466b  891524d56f00           -mov dword ptr [0x6fd524], edx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.edx;
    // 00474671  891528d56f00           -mov dword ptr [0x6fd528], edx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edx;
    // 00474677  891510d56f00           -mov dword ptr [0x6fd510], edx
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.edx;
    // 0047467d  f6c420                 +test ah, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 32 /*0x20*/));
    // 00474680  7417                   -je 0x474699
    if (cpu.flags.zf)
    {
        goto L_0x00474699;
    }
    // 00474682  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00474684  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00474689  e822dc0700             -call 0x4f22b0
    cpu.esp -= 4;
    nfs3hp::watcom::time(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047468e  e88d6b0700             -call 0x4eb220
    cpu.esp -= 4;
    nfs3hp::watcom::srand(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474693  890df0d46f00           -mov dword ptr [0x6fd4f0], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329008) /* 0x6fd4f0 */) = cpu.ecx;
L_0x00474699:
    // 00474699  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 0047469e  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004746a0  a18cd46f00             -mov eax, dword ptr [0x6fd48c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328908) /* 0x6fd48c */);
    // 004746a5  891d90d46f00           -mov dword ptr [0x6fd490], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328912) /* 0x6fd490 */) = cpu.ebx;
    // 004746ab  891d94d46f00           -mov dword ptr [0x6fd494], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328916) /* 0x6fd494 */) = cpu.ebx;
    // 004746b1  891d98d46f00           -mov dword ptr [0x6fd498], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328920) /* 0x6fd498 */) = cpu.ebx;
    // 004746b7  891d9cd46f00           -mov dword ptr [0x6fd49c], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328924) /* 0x6fd49c */) = cpu.ebx;
    // 004746bd  891da0d46f00           -mov dword ptr [0x6fd4a0], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328928) /* 0x6fd4a0 */) = cpu.ebx;
    // 004746c3  a3a8d46f00             -mov dword ptr [0x6fd4a8], eax
    app->getMemory<x86::reg32>(x86::reg32(7328936) /* 0x6fd4a8 */) = cpu.eax;
    // 004746c8  b894d36f00             -mov eax, 0x6fd394
    cpu.eax = 7328660 /*0x6fd394*/;
    // 004746cd  891da4d46f00           -mov dword ptr [0x6fd4a4], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328932) /* 0x6fd4a4 */) = cpu.ebx;
    // 004746d3  e834c00600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004746d8  a1b0d36f00             -mov eax, dword ptr [0x6fd3b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 004746dd  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 004746e0  83f804                 +cmp eax, 4
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
    // 004746e3  0f872c040000           -ja 0x474b15
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00474b15;
    }
    // 004746e9  ff248524464700         -jmp dword ptr [eax*4 + 0x474624]
    cpu.ip = app->getMemory<x86::reg32>(4671012 + cpu.eax * 4); goto dynamic_jump;
  case 0x004746f0:
    // 004746f0  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004746f3  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004746fa  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004746fc  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004746ff  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00474701  8d0cc500000000         -lea ecx, [eax*8]
    cpu.ecx = x86::reg32(cpu.eax * 8);
    // 00474708  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047470a  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0047470c  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 00474711  8935f4d46f00           -mov dword ptr [0x6fd4f4], esi
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.esi;
    // 00474717  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00474719  8935f8d46f00           -mov dword ptr [0x6fd4f8], esi
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.esi;
    // 0047471f  e8ecefffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474724  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 00474729  e8824ffcff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047472e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00474730  7470                   -je 0x4747a2
    if (cpu.flags.zf)
    {
        goto L_0x004747a2;
    }
    // 00474732  808944d56f0008         -or byte ptr [ecx + 0x6fd544], 8
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(7329092) /* 0x6fd544 */) |= x86::reg8(x86::sreg8(8 /*0x8*/));
    // 00474739  ba06000000             -mov edx, 6
    cpu.edx = 6 /*0x6*/;
    // 0047473e  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00474743  8b1d1cd56f00           -mov ebx, dword ptr [0x6fd51c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
    // 00474749  893524d56f00           -mov dword ptr [0x6fd524], esi
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.esi;
    // 0047474f  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 00474755  890d28d56f00           -mov dword ptr [0x6fd528], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ecx;
    // 0047475b  01cb                   -add ebx, ecx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0047475d  8a352eeb5500           -mov dh, byte ptr [0x55eb2e]
    cpu.dh = app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */);
    // 00474763  891d1cd56f00           -mov dword ptr [0x6fd51c], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.ebx;
    // 00474769  f6c620                 +test dh, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dh & 32 /*0x20*/));
    // 0047476c  741d                   -je 0x47478b
    if (cpu.flags.zf)
    {
        goto L_0x0047478b;
    }
    // 0047476e  bf03000000             -mov edi, 3
    cpu.edi = 3 /*0x3*/;
    // 00474773  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 00474778  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0047477a  893d28d56f00           -mov dword ptr [0x6fd528], edi
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edi;
    // 00474780  a3e8d46f00             -mov dword ptr [0x6fd4e8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */) = cpu.eax;
    // 00474785  893524d56f00           -mov dword ptr [0x6fd524], esi
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.esi;
L_0x0047478b:
    // 0047478b  a120d56f00             -mov eax, dword ptr [0x6fd520]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00474790  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00474795  a3b8d46f00             -mov dword ptr [0x6fd4b8], eax
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.eax;
    // 0047479a  8915bcd46f00           -mov dword ptr [0x6fd4bc], edx
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.edx;
    // 004747a0  eb6d                   -jmp 0x47480f
    goto L_0x0047480f;
L_0x004747a2:
    // 004747a2  bb06000000             -mov ebx, 6
    cpu.ebx = 6 /*0x6*/;
    // 004747a7  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 004747ac  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004747b1  8b3d18d56f00           -mov edi, dword ptr [0x6fd518]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004747b7  891d24d56f00           -mov dword ptr [0x6fd524], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.ebx;
    // 004747bd  893528d56f00           -mov dword ptr [0x6fd528], esi
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.esi;
    // 004747c3  890d20d56f00           -mov dword ptr [0x6fd520], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.ecx;
    // 004747c9  01cf                   -add edi, ecx
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004747cb  8a1d2eeb5500           -mov bl, byte ptr [0x55eb2e]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */);
    // 004747d1  893d18d56f00           -mov dword ptr [0x6fd518], edi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.edi;
    // 004747d7  f6c320                 +test bl, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bl & 32 /*0x20*/));
    // 004747da  741d                   -je 0x4747f9
    if (cpu.flags.zf)
    {
        goto L_0x004747f9;
    }
    // 004747dc  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 004747e1  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 004747e6  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004747e8  891528d56f00           -mov dword ptr [0x6fd528], edx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edx;
    // 004747ee  890de8d46f00           -mov dword ptr [0x6fd4e8], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */) = cpu.ecx;
    // 004747f4  a324d56f00             -mov dword ptr [0x6fd524], eax
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.eax;
L_0x004747f9:
    // 004747f9  a1b4d46f00             -mov eax, dword ptr [0x6fd4b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */);
    // 004747fe  bb64000000             -mov ebx, 0x64
    cpu.ebx = 100 /*0x64*/;
    // 00474803  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474804  891db8d46f00           -mov dword ptr [0x6fd4b8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.ebx;
    // 0047480a  a3bcd46f00             -mov dword ptr [0x6fd4bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.eax;
L_0x0047480f:
    // 0047480f  ff45fc                 +inc dword ptr [ebp - 4]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474812  e9fe020000             -jmp 0x474b15
    goto L_0x00474b15;
  case 0x00474817:
    // 00474817  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 0047481c  891df4d46f00           -mov dword ptr [0x6fd4f4], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.ebx;
    // 00474822  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00474824  891df8d46f00           -mov dword ptr [0x6fd4f8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.ebx;
    // 0047482a  e8e1eeffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047482f  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 00474834  e8774efcff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474839  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0047483b  741e                   -je 0x47485b
    if (cpu.flags.zf)
    {
        goto L_0x0047485b;
    }
    // 0047483d  8b151cd56f00           -mov edx, dword ptr [0x6fd51c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
    // 00474843  8a3d44d56f00           -mov bh, byte ptr [0x6fd544]
    cpu.bh = app->getMemory<x86::reg8>(x86::reg32(7329092) /* 0x6fd544 */);
    // 00474849  42                     -inc edx
    (cpu.edx)++;
    // 0047484a  80cf08                 +or bh, 8
    cpu.clear_co();
    cpu.set_szp((cpu.bh |= x86::reg8(x86::sreg8(8 /*0x8*/))));
    // 0047484d  89151cd56f00           -mov dword ptr [0x6fd51c], edx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.edx;
    // 00474853  883d44d56f00           -mov byte ptr [0x6fd544], bh
    app->getMemory<x86::reg8>(x86::reg32(7329092) /* 0x6fd544 */) = cpu.bh;
    // 00474859  eb06                   -jmp 0x474861
    goto L_0x00474861;
L_0x0047485b:
    // 0047485b  ff0518d56f00           -inc dword ptr [0x6fd518]
    (app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */))++;
L_0x00474861:
    // 00474861  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474864  43                     -inc ebx
    (cpu.ebx)++;
    // 00474865  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 00474868  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 0047486f  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00474871  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474874  8d0cc500000000         -lea ecx, [eax*8]
    cpu.ecx = x86::reg32(cpu.eax * 8);
    // 0047487b  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047487e  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00474880  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 00474885  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0047488a  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0047488c  e87feeffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474891  a128d36f00             -mov eax, dword ptr [0x6fd328]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */);
    // 00474896  e8154efcff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047489b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0047489d  740f                   -je 0x4748ae
    if (cpu.flags.zf)
    {
        goto L_0x004748ae;
    }
    // 0047489f  808944d56f0008         +or byte ptr [ecx + 0x6fd544], 8
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(7329092) /* 0x6fd544 */) |= x86::reg8(x86::sreg8(8 /*0x8*/))));
    // 004748a6  ff051cd56f00           +inc dword ptr [0x6fd51c]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004748ac  eb06                   -jmp 0x4748b4
    goto L_0x004748b4;
L_0x004748ae:
    // 004748ae  ff0518d56f00           -inc dword ptr [0x6fd518]
    (app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */))++;
L_0x004748b4:
    // 004748b4  ff45fc                 -inc dword ptr [ebp - 4]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */))++;
    // 004748b7  a1b4d46f00             -mov eax, dword ptr [0x6fd4b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */);
    // 004748bc  8b1518d56f00           -mov edx, dword ptr [0x6fd518]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004748c2  40                     -inc eax
    (cpu.eax)++;
    // 004748c3  83fa02                 +cmp edx, 2
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
    // 004748c6  7533                   -jne 0x4748fb
    if (!cpu.flags.zf)
    {
        goto L_0x004748fb;
    }
    // 004748c8  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 004748cd  bf64000000             -mov edi, 0x64
    cpu.edi = 100 /*0x64*/;
    // 004748d2  b906000000             -mov ecx, 6
    cpu.ecx = 6 /*0x6*/;
    // 004748d7  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004748d9  a3bcd46f00             -mov dword ptr [0x6fd4bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.eax;
    // 004748de  891d20d56f00           -mov dword ptr [0x6fd520], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.ebx;
    // 004748e4  893528d56f00           -mov dword ptr [0x6fd528], esi
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.esi;
    // 004748ea  893db8d46f00           -mov dword ptr [0x6fd4b8], edi
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.edi;
    // 004748f0  890d24d56f00           -mov dword ptr [0x6fd524], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.ecx;
    // 004748f6  e91a020000             -jmp 0x474b15
    goto L_0x00474b15;
L_0x004748fb:
    // 004748fb  833d1cd56f0002         +cmp dword ptr [0x6fd51c], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474902  7533                   -jne 0x474937
    if (!cpu.flags.zf)
    {
        goto L_0x00474937;
    }
    // 00474904  bb06000000             -mov ebx, 6
    cpu.ebx = 6 /*0x6*/;
    // 00474909  bf03000000             -mov edi, 3
    cpu.edi = 3 /*0x3*/;
    // 0047490e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00474913  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00474915  891d20d56f00           -mov dword ptr [0x6fd520], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.ebx;
    // 0047491b  890d28d56f00           -mov dword ptr [0x6fd528], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ecx;
    // 00474921  893db8d46f00           -mov dword ptr [0x6fd4b8], edi
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.edi;
    // 00474927  a3bcd46f00             -mov dword ptr [0x6fd4bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.eax;
    // 0047492c  890d24d56f00           -mov dword ptr [0x6fd524], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.ecx;
    // 00474932  e9de010000             -jmp 0x474b15
    goto L_0x00474b15;
L_0x00474937:
    // 00474937  bb06000000             -mov ebx, 6
    cpu.ebx = 6 /*0x6*/;
    // 0047493c  a3b8d46f00             -mov dword ptr [0x6fd4b8], eax
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.eax;
    // 00474941  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00474943  a3bcd46f00             -mov dword ptr [0x6fd4bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.eax;
    // 00474948  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 0047494e  891d28d56f00           -mov dword ptr [0x6fd528], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ebx;
    // 00474954  891524d56f00           -mov dword ptr [0x6fd524], edx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.edx;
    // 0047495a  e9b6010000             -jmp 0x474b15
    goto L_0x00474b15;
  case 0x0047495f:
    // 0047495f  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 00474964  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00474967  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00474969  a3f4d46f00             -mov dword ptr [0x6fd4f4], eax
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.eax;
    // 0047496e  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
L_0x00474973:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474973  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 00474978  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047497b  39c3                   +cmp ebx, eax
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
    // 0047497d  7d72                   -jge 0x4749f1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004749f1;
    }
    // 0047497f  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474982  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474989  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0047498b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0047498e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474990  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474993  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00474995  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 0047499a  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 0047499f  8dba6cd56f00           -lea edi, [edx + 0x6fd56c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(7329132) /* 0x6fd56c */);
    // 004749a5  89b244d56f00           -mov dword ptr [edx + 0x6fd544], esi
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329092) /* 0x6fd544 */) = cpu.esi;
    // 004749ab  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004749b0  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004749b2  8b8240d56f00           -mov eax, dword ptr [edx + 0x6fd540]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329088) /* 0x6fd540 */);
    // 004749b8  e8e34bfcff             -call 0x4395a0
    cpu.esp -= 4;
    sub_4395a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004749bd  898294d56f00           -mov dword ptr [edx + 0x6fd594], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329172) /* 0x6fd594 */) = cpu.eax;
    // 004749c3  8b8240d56f00           -mov eax, dword ptr [edx + 0x6fd540]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329088) /* 0x6fd540 */);
    // 004749c9  e8e24cfcff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004749ce  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004749d0  740f                   -je 0x4749e1
    if (cpu.flags.zf)
    {
        goto L_0x004749e1;
    }
    // 004749d2  808a44d56f0008         +or byte ptr [edx + 0x6fd544], 8
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.edx + x86::reg32(7329092) /* 0x6fd544 */) |= x86::reg8(x86::sreg8(8 /*0x8*/))));
    // 004749d9  ff051cd56f00           +inc dword ptr [0x6fd51c]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004749df  eb06                   -jmp 0x4749e7
    goto L_0x004749e7;
L_0x004749e1:
    // 004749e1  ff0518d56f00           +inc dword ptr [0x6fd518]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
L_0x004749e7:
    // 004749e7  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004749ea  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004749eb  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004749ec  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004749ef  eb82                   -jmp 0x474973
    goto L_0x00474973;
L_0x004749f1:
    // 004749f1  8b0d1cd56f00           -mov ecx, dword ptr [0x6fd51c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
    // 004749f7  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004749f9  0f8562000000           -jne 0x474a61
    if (!cpu.flags.zf)
    {
        goto L_0x00474a61;
    }
    // 004749ff  833de8d46f0000         +cmp dword ptr [0x6fd4e8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474a06  7515                   -jne 0x474a1d
    if (!cpu.flags.zf)
    {
        goto L_0x00474a1d;
    }
    // 00474a08  833d18d56f0004         +cmp dword ptr [0x6fd518], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474a0f  7d0c                   -jge 0x474a1d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474a1d;
    }
    // 00474a11  c70528d56f0001000000   -mov dword ptr [0x6fd528], 1
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = 1 /*0x1*/;
    // 00474a1b  eb07                   -jmp 0x474a24
    goto L_0x00474a24;
L_0x00474a1d:
    // 00474a1d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00474a1f  a328d56f00             -mov dword ptr [0x6fd528], eax
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.eax;
L_0x00474a24:
    // 00474a24  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00474a29  8b0d28d56f00           -mov ecx, dword ptr [0x6fd528]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */);
    // 00474a2f  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474a34  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00474a36  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00474a38  891524d56f00           -mov dword ptr [0x6fd524], edx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.edx;
    // 00474a3e  83fa06                 +cmp edx, 6
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6 /*0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474a41  7e07                   -jle 0x474a4a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00474a4a;
    }
    // 00474a43  b806000000             -mov eax, 6
    cpu.eax = 6 /*0x6*/;
    // 00474a48  eb02                   -jmp 0x474a4c
    goto L_0x00474a4c;
L_0x00474a4a:
    // 00474a4a  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x00474a4c:
    // 00474a4c  a324d56f00             -mov dword ptr [0x6fd524], eax
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.eax;
    // 00474a51  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00474a53  7f02                   -jg 0x474a57
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00474a57;
    }
    // 00474a55  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
L_0x00474a57:
    // 00474a57  a324d56f00             -mov dword ptr [0x6fd524], eax
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.eax;
    // 00474a5c  e9a4000000             -jmp 0x474b05
    goto L_0x00474b05;
L_0x00474a61:
    // 00474a61  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474a67  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00474a69  0f8560000000           -jne 0x474acf
    if (!cpu.flags.zf)
    {
        goto L_0x00474acf;
    }
    // 00474a6f  833de8d46f0000         +cmp dword ptr [0x6fd4e8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474a76  7511                   -jne 0x474a89
    if (!cpu.flags.zf)
    {
        goto L_0x00474a89;
    }
    // 00474a78  83f906                 +cmp ecx, 6
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6 /*0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474a7b  7d0c                   -jge 0x474a89
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474a89;
    }
    // 00474a7d  c70528d56f0001000000   -mov dword ptr [0x6fd528], 1
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = 1 /*0x1*/;
    // 00474a87  eb08                   -jmp 0x474a91
    goto L_0x00474a91;
L_0x00474a89:
    // 00474a89  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00474a8b  890d28d56f00           -mov dword ptr [0x6fd528], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ecx;
L_0x00474a91:
    // 00474a91  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00474a96  8b1d28d56f00           -mov ebx, dword ptr [0x6fd528]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */);
    // 00474a9c  a11cd56f00             -mov eax, dword ptr [0x6fd51c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
    // 00474aa1  29da                   -sub edx, ebx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00474aa3  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00474aa5  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 00474aab  83fa06                 +cmp edx, 6
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6 /*0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474aae  7e07                   -jle 0x474ab7
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00474ab7;
    }
    // 00474ab0  b806000000             -mov eax, 6
    cpu.eax = 6 /*0x6*/;
    // 00474ab5  eb02                   -jmp 0x474ab9
    goto L_0x00474ab9;
L_0x00474ab7:
    // 00474ab7  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x00474ab9:
    // 00474ab9  a320d56f00             -mov dword ptr [0x6fd520], eax
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.eax;
    // 00474abe  83f801                 +cmp eax, 1
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
    // 00474ac1  7f05                   -jg 0x474ac8
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00474ac8;
    }
    // 00474ac3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x00474ac8:
    // 00474ac8  a320d56f00             -mov dword ptr [0x6fd520], eax
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.eax;
    // 00474acd  eb36                   -jmp 0x474b05
    goto L_0x00474b05;
L_0x00474acf:
    // 00474acf  833de8d46f0000         +cmp dword ptr [0x6fd4e8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474ad6  752d                   -jne 0x474b05
    if (!cpu.flags.zf)
    {
        goto L_0x00474b05;
    }
    // 00474ad8  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00474add  29f2                   -sub edx, esi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 00474adf  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00474ae1  891528d56f00           -mov dword ptr [0x6fd528], edx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edx;
    // 00474ae7  83fa04                 +cmp edx, 4
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
    // 00474aea  7e07                   -jle 0x474af3
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00474af3;
    }
    // 00474aec  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 00474af1  eb02                   -jmp 0x474af5
    goto L_0x00474af5;
L_0x00474af3:
    // 00474af3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x00474af5:
    // 00474af5  a328d56f00             -mov dword ptr [0x6fd528], eax
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.eax;
    // 00474afa  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00474afc  7f02                   -jg 0x474b00
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00474b00;
    }
    // 00474afe  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00474b00:
    // 00474b00  a328d56f00             -mov dword ptr [0x6fd528], eax
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.eax;
L_0x00474b05:
    // 00474b05  a1b4d46f00             -mov eax, dword ptr [0x6fd4b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */);
    // 00474b0a  40                     -inc eax
    (cpu.eax)++;
    // 00474b0b  a3b8d46f00             -mov dword ptr [0x6fd4b8], eax
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.eax;
    // 00474b10  a3bcd46f00             -mov dword ptr [0x6fd4bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.eax;
L_0x00474b15:
    // 00474b15  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00474b17:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474b17  3b0d20d56f00           +cmp ecx, dword ptr [0x6fd520]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474b1d  7d2d                   -jge 0x474b4c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474b4c;
    }
    // 00474b1f  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474b22  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474b29  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474b2b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474b2e  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474b31  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474b33  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474b36  41                     -inc ecx
    (cpu.ecx)++;
    // 00474b37  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474b39  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474b3c  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00474b41  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474b42  e819eeffff             -call 0x473960
    cpu.esp -= 4;
    sub_473960(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474b47  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 00474b4a  ebcb                   -jmp 0x474b17
    goto L_0x00474b17;
L_0x00474b4c:
    // 00474b4c  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00474b4e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474b4e  3b0d24d56f00           +cmp ecx, dword ptr [0x6fd524]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474b54  7d5f                   -jge 0x474bb5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474bb5;
    }
    // 00474b56  f6052eeb550020         +test byte ptr [0x55eb2e], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */) & 32 /*0x20*/));
    // 00474b5d  741b                   -je 0x474b7a
    if (cpu.flags.zf)
    {
        goto L_0x00474b7a;
    }
    // 00474b5f  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474b62  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474b69  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474b6b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474b6e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474b70  c1e003                 +shl eax, 3
    {
        x86::reg8 tmp = 3 /*0x3*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00474b73  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00474b78  eb22                   -jmp 0x474b9c
    goto L_0x00474b9c;
L_0x00474b7a:
    // 00474b7a  f6c103                 +test cl, 3
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 3 /*0x3*/));
    // 00474b7d  7507                   -jne 0x474b86
    if (!cpu.flags.zf)
    {
        goto L_0x00474b86;
    }
    // 00474b7f  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00474b84  eb02                   -jmp 0x474b88
    goto L_0x00474b88;
L_0x00474b86:
    // 00474b86  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x00474b88:
    // 00474b88  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474b8b  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474b92  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474b94  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474b97  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474b99  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
L_0x00474b9c:
    // 00474b9c  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474b9e  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474ba1  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00474ba6  e8e5ecffff             -call 0x473890
    cpu.esp -= 4;
    sub_473890(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474bab  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474bae  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474baf  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474bb0  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00474bb3  eb99                   -jmp 0x474b4e
    goto L_0x00474b4e;
L_0x00474bb5:
    // 00474bb5  833de8d46f0000         +cmp dword ptr [0x6fd4e8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474bbc  7537                   -jne 0x474bf5
    if (!cpu.flags.zf)
    {
        goto L_0x00474bf5;
    }
    // 00474bbe  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00474bc0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474bc0  3b0d28d56f00           +cmp ecx, dword ptr [0x6fd528]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474bc6  7d35                   -jge 0x474bfd
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474bfd;
    }
    // 00474bc8  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474bcb  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474bd2  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474bd4  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474bd7  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474bda  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474bdc  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474bdf  41                     -inc ecx
    (cpu.ecx)++;
    // 00474be0  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474be2  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474be5  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00474bea  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474beb  e880eeffff             -call 0x473a70
    cpu.esp -= 4;
    sub_473a70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474bf0  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 00474bf3  ebcb                   -jmp 0x474bc0
    goto L_0x00474bc0;
L_0x00474bf5:
    // 00474bf5  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00474bf7  893d28d56f00           -mov dword ptr [0x6fd528], edi
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edi;
L_0x00474bfd:
    // 00474bfd  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474c02  8b151cd56f00           -mov edx, dword ptr [0x6fd51c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
    // 00474c08  8b0d24d56f00           -mov ecx, dword ptr [0x6fd524]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */);
    // 00474c0e  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474c10  8b1d20d56f00           -mov ebx, dword ptr [0x6fd520]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00474c16  a314d56f00             -mov dword ptr [0x6fd514], eax
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.eax;
    // 00474c1b  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00474c1d  8b1528d56f00           -mov edx, dword ptr [0x6fd528]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */);
    // 00474c23  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00474c25  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474c27  a310d56f00             -mov dword ptr [0x6fd510], eax
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.eax;
    // 00474c2c  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474c31  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00474c33  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00474c35  e836efffff             -call 0x473b70
    cpu.esp -= 4;
    sub_473b70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474c3a  8b3dfcd46f00           -mov edi, dword ptr [0x6fd4fc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7329020) /* 0x6fd4fc */);
    // 00474c40  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00474c42  83e710                 -and edi, 0x10
    cpu.edi &= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00474c45  a300d56f00             -mov dword ptr [0x6fd500], eax
    app->getMemory<x86::reg32>(x86::reg32(7329024) /* 0x6fd500 */) = cpu.eax;
    // 00474c4a  893dfcd46f00           -mov dword ptr [0x6fd4fc], edi
    app->getMemory<x86::reg32>(x86::reg32(7329020) /* 0x6fd4fc */) = cpu.edi;
    // 00474c50  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00474c52  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c53  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c54  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c55  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c56  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c57  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c58  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_474c60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00474c60  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00474c61  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00474c62  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00474c63  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00474c65  8b15b4d36f00           -mov edx, dword ptr [0x6fd3b4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328692) /* 0x6fd3b4 */);
    // 00474c6b  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00474c6d  b900010000             -mov ecx, 0x100
    cpu.ecx = 256 /*0x100*/;
    // 00474c72  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00474c74  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00474c77  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00474c79  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00474c7b  8a8238295500           -mov al, byte ptr [edx + 0x552938]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(5581112) /* 0x552938 */);
    // 00474c81  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c82  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c83  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c84  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_474ca0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00474ca0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00474ca1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00474ca2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00474ca4  ba0d000000             -mov edx, 0xd
    cpu.edx = 13 /*0xd*/;
    // 00474ca9  b83c000000             -mov eax, 0x3c
    cpu.eax = 60 /*0x3c*/;
    // 00474cae  e85d1d0300             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cb3  e8c858fcff             -call 0x43a580
    cpu.esp -= 4;
    nfs3hp::load::buildCarClassLists(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cb8  e883e6ffff             -call 0x473340
    cpu.esp -= 4;
    sub_473340(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cbd  a1b4d36f00             -mov eax, dword ptr [0x6fd3b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328692) /* 0x6fd3b4 */);
    // 00474cc2  e827c10600             -call 0x4e0dee
    cpu.esp -= 4;
    nfs3hp::ea::seedRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cc7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00474cc9  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 00474cce  8915acd36f00           -mov dword ptr [0x6fd3ac], edx
    app->getMemory<x86::reg32>(x86::reg32(7328684) /* 0x6fd3ac */) = cpu.edx;
    // 00474cd4  83f803                 +cmp eax, 3
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
    // 00474cd7  7721                   -ja 0x474cfa
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00474cfa;
    }
    // 00474cd9  ff2485884c4700         -jmp dword ptr [eax*4 + 0x474c88]
    cpu.ip = app->getMemory<x86::reg32>(4672648 + cpu.eax * 4); goto dynamic_jump;
  case 0x00474ce0:
    // 00474ce0  e80befffff             -call 0x473bf0
    cpu.esp -= 4;
    sub_473bf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474ce5  eb13                   -jmp 0x474cfa
    goto L_0x00474cfa;
  case 0x00474ce7:
    // 00474ce7  e824f6ffff             -call 0x474310
    cpu.esp -= 4;
    sub_474310(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cec  eb0c                   -jmp 0x474cfa
    goto L_0x00474cfa;
  case 0x00474cee:
    // 00474cee  e8edf1ffff             -call 0x473ee0
    cpu.esp -= 4;
    sub_473ee0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cf3  eb05                   -jmp 0x474cfa
    goto L_0x00474cfa;
  case 0x00474cf5:
    // 00474cf5  e846f9ffff             -call 0x474640
    cpu.esp -= 4;
    sub_474640(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00474cfa:
    // 00474cfa  a1aad46f00             -mov eax, dword ptr [0x6fd4aa]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328938) /* 0x6fd4aa */);
    // 00474cff  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00474d02  e8f9a10500             -call 0x4cef00
    cpu.esp -= 4;
    nfs3hp::frontend::recordsHolder(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474d07  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 00474d08  a3b4d26f00             -mov dword ptr [0x6fd2b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328436) /* 0x6fd2b4 */) = cpu.eax;
    // 00474d0d  a350bb6f00             -mov dword ptr [0x6fbb50], eax
    app->getMemory<x86::reg32>(x86::reg32(7322448) /* 0x6fbb50 */) = cpu.eax;
    // 00474d12  b840bb6f00             -mov eax, 0x6fbb40
    cpu.eax = 7322432 /*0x6fbb40*/;
    // 00474d17  e894340000             -call 0x4781b0
    cpu.esp -= 4;
    sub_4781b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474d1c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474d1d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474d1e  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x00 */
void Application::sub_474d20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00474d20  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d22  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d24  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d26  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d28  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d2a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d2c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d2e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d30  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d32  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d34  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d36  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d38  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d3a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d3c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d3e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d40  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d42  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d44  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 00474d4a  8d9200000000           -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
    // 00474d50  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00474d51  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00474d52  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00474d53  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00474d55  8b1588b05500           -mov edx, dword ptr [0x55b088]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5615752) /* 0x55b088 */);
    // 00474d5b  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 00474d62  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474d64  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474d67  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474d69  ba8cb05500             -mov edx, 0x55b08c
    cpu.edx = 5615756 /*0x55b08c*/;
    // 00474d6e  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474d71  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00474d73  83ba3002000000         +cmp dword ptr [edx + 0x230], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(560) /* 0x230 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474d7a  742d                   -je 0x474da9
    if (cpu.flags.zf)
    {
        goto L_0x00474da9;
    }
    // 00474d7c  c7823402000000000000   -mov dword ptr [edx + 0x234], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(564) /* 0x234 */) = 0 /*0x0*/;
    // 00474d86  c7823002000000000000   -mov dword ptr [edx + 0x230], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(560) /* 0x230 */) = 0 /*0x0*/;
    // 00474d90  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00474d92  eb05                   -jmp 0x474d99
    goto L_0x00474d99;
L_0x00474d94:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474d94  83f808                 +cmp eax, 8
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
    // 00474d97  7d10                   -jge 0x474da9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474da9;
    }
L_0x00474d99:
    // 00474d99  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00474d9b  c1e106                 +shl ecx, 6
    {
        x86::reg8 tmp = 6 /*0x6*/ % 32;
        x86::reg32& op = cpu.ecx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00474d9e  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474d9f  c744113c00000000       -mov dword ptr [ecx + edx + 0x3c], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */ + cpu.edx * 1) = 0 /*0x0*/;
    // 00474da7  ebeb                   -jmp 0x474d94
    goto L_0x00474d94;
L_0x00474da9:
    // 00474da9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474daa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474dab  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474dac  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_474d50: native, nfs3hp::frontend::textLookClearLights */

/* sub_474db0: native, nfs3hp::port::view::edgeTriangles */

/* sub_475240: native, nfs3hp::port::view::glyphResource */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_475b10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00475b10  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00475b11  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00475b12  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00475b13  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00475b15  8b1510906700           -mov edx, dword ptr [0x679010]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6787088) /* 0x679010 */);
    // 00475b1b  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00475b1d  740f                   -je 0x475b2e
    if (cpu.flags.zf)
    {
        goto L_0x00475b2e;
    }
    // 00475b1f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00475b21  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00475b23  e868bd0600             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00475b28  890d10906700           -mov dword ptr [0x679010], ecx
    app->getMemory<x86::reg32>(x86::reg32(6787088) /* 0x679010 */) = cpu.ecx;
L_0x00475b2e:
    // 00475b2e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00475b2f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00475b30  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00475b31  c3                     -ret 
    cpu.esp += 4;
    return;
}

}
