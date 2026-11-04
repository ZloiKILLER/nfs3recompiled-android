#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_41e3c0: native, nfs3hp::trackdata::nearestNodeCoarse */

/* sub_41e420: native, nfs3hp::trackdata::followNode */

/* sub_41e470: native, nfs3hp::trackdata::nearestNode */

/* sub_41e6a0: native, nfs3hp::trackdata::quadVertices */

/* sub_41e720: native, nfs3hp::trackdata::quadStepForward */

/* sub_41e770: native, nfs3hp::trackdata::quadStepBack */

/* sub_41e7c0: native, nfs3hp::trackdata::quadToFixed */

/* sub_41e8d0: native, nfs3hp::trackdata::quadSet */

/* sub_41e8f0: native, nfs3hp::trackdata::nodeSection */

/* sub_41e940: native, nfs3hp::trackdata::resetPosition */

/* sub_41e990: native, nfs3hp::trackdata::findQuad */

/* sub_41f090: native, nfs3hp::trackdata::findQuadNearby */

/* sub_41f330: native, nfs3hp::trackdata::findQuadByLinks */

/* sub_41f500: native, nfs3hp::trackdata::findQuadFallback */

/* sub_41f650: native, nfs3hp::trackdata::locateOnRoad */

/* sub_41f6e0: native, nfs3hp::trackdata::updatePosition */

/* sub_41f870: native, nfs3hp::trackdata::findMatchingQuad */

/* sub_41fd60: native, nfs3hp::trackdata::quadEdgeFlags */

/* sub_41ff10: native, nfs3hp::trackdata::sectionItemCount */

/* sub_41ff20: native, nfs3hp::trackdata::sectionItem */

/* sub_41ffa0: native, nfs3hp::trackdata::setLaneOffsets */

/* sub_41ffb0: native, nfs3hp::dead::stub_0x41ffb0 */

/* sub_41ffd0: native, nfs3hp::trackdata::lanePosition */

/* sub_420230: native, nfs3hp::trackdata::positionFrame */

/* sub_4205a0: native, nfs3hp::trackdata::snapToTrack */

/* sub_420650: native, nfs3hp::trackdata::placeOnTrack */

/* sub_4206f0: native, nfs3hp::trackdata::positionColour */

/* sub_4207e0: native, nfs3hp::trackdata::positionSurface */

/* sub_420840: native, nfs3hp::trackdata::surfaceUnder */

/* sub_420bf0: native, nfs3hp::trackdata::inTunnel */

/* sub_420d60: native, nfs3hp::trackdata::carFacingWay */

/* sub_420ff0: native, nfs3hp::port::view::sectionAheadIsTwo */

/* sub_4211c0: native, nfs3hp::trackdata::carSectionKind */

/* sub_421360: native, nfs3hp::carstate::idleHook */

/* sub_421370: native, nfs3hp::dead::stub_0x421370 */

/* sub_421380: native, nfs3hp::carstate::viewFromCamera */

/* sub_4213b0: native, nfs3hp::carstate::viewFromScreenCamera */

/* sub_421450: native, nfs3hp::carstate::chooseView */

/* sub_421480: native, nfs3hp::carstate::idleHookToo */

/* sub_421490: native, nfs3hp::carstate::pickChaseNode */

/* sub_421570: native, nfs3hp::carstate::turnAround */

/* sub_4215c0: native, nfs3hp::carstate::attachCameras */

/* sub_421730: native, nfs3hp::carstate::headTurn */

/* sub_4217e0: native, nfs3hp::carstate::cameraNode */

/* sub_421880: native, nfs3hp::carstate::cameraTrackPlace */

/* sub_421900: native, nfs3hp::carstate::keepOutOfCars */

/* sub_421b90: native, nfs3hp::carstate::keepAboveRoad */

/* sub_421cc0: native, nfs3hp::carstate::keepOnTrack */

/* sub_421e80: native, nfs3hp::carstate::keepFromRightWall */

/* sub_421f90: native, nfs3hp::carstate::keepFromLeftWall */

/* sub_4220a0: native, nfs3hp::carstate::keepAboveGround */

/* sub_422290: native, nfs3hp::carstate::lookAtTarget */

/* sub_4223c0: native, nfs3hp::carstate::blendCamera */

/* sub_422520: native, nfs3hp::carstate::placeCamera */

/* sub_422610: native, nfs3hp::carstate::switchCamera */

/* sub_4227e0: native, nfs3hp::carstate::cameraZoom */

/* sub_422850: native, nfs3hp::carstate::viewMatrix */

/* sub_4228e0: native, nfs3hp::carstate::updateCamera */

/* sub_4229c0: native, nfs3hp::carstate::cameraView */

/* sub_422aa0: native, nfs3hp::carstate::cockpitShown */

/* sub_422b70: native, nfs3hp::dead::stub_0x422b70 */

/* sub_422ba0: native, nfs3hp::carstate::cameraCar */

/* sub_422bd0: native, nfs3hp::carstate::cameraTarget */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_422c00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00422c00  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00422c01  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00422c02  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00422c03  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00422c05  8b0db0d36f00           -mov ecx, dword ptr [0x6fd3b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 00422c0b  83f901                 +cmp ecx, 1
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
    // 00422c0e  7504                   -jne 0x422c14
    if (!cpu.flags.zf)
    {
        goto L_0x00422c14;
    }
    // 00422c10  21c8                   +and eax, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.eax &= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00422c12  eb02                   -jmp 0x422c16
    goto L_0x00422c16;
L_0x00422c14:
    // 00422c14  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00422c16:
    // 00422c16  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 00422c1d  8b99b0105e00           -mov ebx, dword ptr [ecx + 0x5e10b0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6164656) /* 0x5e10b0 */);
    // 00422c23  3b5304                 +cmp edx, dword ptr [ebx + 4]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00422c26  740a                   -je 0x422c32
    if (cpu.flags.zf)
    {
        goto L_0x00422c32;
    }
    // 00422c28  c781f42f550001000000   -mov dword ptr [ecx + 0x552ff4], 1
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5582836) /* 0x552ff4 */) = 1 /*0x1*/;
L_0x00422c32:
    // 00422c32  8b0485b0105e00         -mov eax, dword ptr [eax*4 + 0x5e10b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6164656) /* 0x5e10b0 */ + cpu.eax * 4);
    // 00422c39  895004                 -mov dword ptr [eax + 4], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 00422c3c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00422c3d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00422c3e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00422c3f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_422c40: native, nfs3hp::dead::stub_0x422c40 */

/* sub_422c80: native, nfs3hp::dead::stub_0x422c80 */

/* sub_422cf0: native, nfs3hp::carstate::initCameras */

/* sub_4238b0: native, nfs3hp::carstate::setCameraMode */

/* sub_423960: native, nfs3hp::carstate::setOverrideMode */

/* sub_423a20: native, nfs3hp::carstate::nextCycleMode */

/* sub_423a60: native, nfs3hp::carstate::buildCameraCycle */

/* sub_423ab0: native, nfs3hp::carstate::modeInCycle */

/* sub_423ae0: native, nfs3hp::carstate::rememberMode */

/* sub_423b00: native, nfs3hp::carstate::cameraMode */

/* sub_423b50: native, nfs3hp::carstate::setModeAnew */

/* sub_423b80: native, nfs3hp::carstate::settleCamera */

/* sub_423d90: native, nfs3hp::carstate::nearestSpot */

/* sub_423fb0: native, nfs3hp::carstate::scaledIfSet */

/* sub_423fe0: native, nfs3hp::carstate::bitsBroken */

/* sub_424030: native, nfs3hp::carstate::dampVector */

/* sub_4240c0: native, nfs3hp::carstate::chaseOffset */

/* sub_424260: native, nfs3hp::carstate::chaseOffsetLevel */

/* align: skip  */
void Application::sub_4243a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
  x86::reg16 fsw = 0;
    // 004243a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004243a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004243a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004243a3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004243a4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004243a5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004243a6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004243a8  83ec5c                 -sub esp, 0x5c
    (cpu.esp) -= x86::reg32(x86::sreg32(92 /*0x5c*/));
    // 004243ab  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004243ae  8d7da4                 -lea edi, [ebp - 0x5c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 004243b1  8db010010000           -lea esi, [eax + 0x110]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(272) /* 0x110 */);
    // 004243b7  8b1da8367d00           -mov ebx, dword ptr [0x7d36a8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */);
    // 004243bd  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004243be  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004243bf  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004243c0  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004243c2  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004243c5  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004243c7  740d                   -je 0x4243d6
    if (cpu.flags.zf)
    {
        goto L_0x004243d6;
    }
    // 004243c9  833d60d97c0000         +cmp dword ptr [0x7cd960], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8182112) /* 0x7cd960 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004243d0  0f84b1020000           -je 0x424687
    if (cpu.flags.zf)
    {
        goto L_0x00424687;
    }
L_0x004243d6:
    // 004243d6  833dec00560000         +cmp dword ptr [0x5600ec], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5636332) /* 0x5600ec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004243dd  0f85a4020000           -jne 0x424687
    if (!cpu.flags.zf)
    {
        goto L_0x00424687;
    }
    // 004243e3  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004243e6  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004243e9  81b80c010000f4fd343f   +cmp dword ptr [eax + 0x10c], 0x3f34fdf4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1060437492 /*0x3f34fdf4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004243f3  7d36                   -jge 0x42442b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0042442b;
    }
    // 004243f5  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004243f8  83b80c01000000         +cmp dword ptr [eax + 0x10c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004243ff  751b                   -jne 0x42441c
    if (!cpu.flags.zf)
    {
        goto L_0x0042441c;
    }
    // 00424401  83ba9809000000         +cmp dword ptr [edx + 0x998], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(2456) /* 0x998 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424408  7e07                   -jle 0x424411
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00424411;
    }
    // 0042440a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0042440f  eb02                   -jmp 0x424413
    goto L_0x00424413;
L_0x00424411:
    // 00424411  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
L_0x00424413:
    // 00424413  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424416  8981fc000000           -mov dword ptr [ecx + 0xfc], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(252) /* 0xfc */) = cpu.eax;
L_0x0042441c:
    // 0042441c  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0042441f  c7800c01000003000000   -mov dword ptr [eax + 0x10c], 3
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */) = 3 /*0x3*/;
    // 00424429  eb0c                   -jmp 0x424437
    goto L_0x00424437;
L_0x0042442b:
    // 0042442b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0042442e  83b80c01000000         +cmp dword ptr [eax + 0x10c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424435  7417                   -je 0x42444e
    if (cpu.flags.zf)
    {
        goto L_0x0042444e;
    }
L_0x00424437:
    // 00424437  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0042443a  8bb80c010000           -mov edi, dword ptr [eax + 0x10c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
    // 00424440  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 00424442  4f                     +dec edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 00424443  8975f8                 -mov dword ptr [ebp - 8], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.esi;
    // 00424446  89b80c010000           -mov dword ptr [eax + 0x10c], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */) = cpu.edi;
    // 0042444c  eb30                   -jmp 0x42447e
    goto L_0x0042447e;
L_0x0042444e:
    // 0042444e  ffb11c010000           -push dword ptr [ecx + 0x11c]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(284) /* 0x11c */);
    cpu.esp -= 4;
    // 00424454  e857fbffff             -call 0x423fb0
    cpu.esp -= 4;
    nfs3hp::carstate::scaledIfSet(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00424459  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0042445c  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 0042445f  8b1c85a0c47900         -mov ebx, dword ptr [eax*4 + 0x79c4a0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7980192) /* 0x79c4a0 */ + cpu.eax * 4);
    // 00424466  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 00424469  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0042446b  7511                   -jne 0x42447e
    if (!cpu.flags.zf)
    {
        goto L_0x0042447e;
    }
    // 0042446d  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 00424470  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00424471  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00424473  ffb120010000           -push dword ptr [ecx + 0x120]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(288) /* 0x120 */);
    cpu.esp -= 4;
    // 00424479  e8b2fbffff             -call 0x424030
    cpu.esp -= 4;
    nfs3hp::carstate::dampVector(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0042447e:
    // 0042447e  8b8228050000           -mov eax, dword ptr [edx + 0x528]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1320) /* 0x528 */);
    // 00424484  db80c0020000           +fild dword ptr [eax + 0x2c0]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(704) /* 0x2c0 */)));
    // 0042448a  dc0dbc6e5300           +fmul qword ptr [0x536ebc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5467836) /* 0x536ebc */)));
    // 00424490  dd55e0                 +fst qword ptr [ebp - 0x20]
    app->getMemory<double>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = double(fp0);
    // 00424493  dc1dc46e5300           +fcomp qword ptr [0x536ec4]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5467844) /* 0x536ec4 */)));
    // 00424499  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 0042449b  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0042449c  730e                   -jae 0x4244ac
    if (!cpu.flags.cf)
    {
        goto L_0x004244ac;
    }
    // 0042449e  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004244a1  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 004244a4  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004244a7  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004244aa  eb0d                   -jmp 0x4244b9
    goto L_0x004244b9;
L_0x004244ac:
    // 004244ac  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004244ae  bf0000223f             -mov edi, 0x3f220000
    cpu.edi = 1059192832 /*0x3f220000*/;
    // 004244b3  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
    // 004244b6  897df4                 -mov dword ptr [ebp - 0xc], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edi;
L_0x004244b9:
    // 004244b9  d945a8                 -fld dword ptr [ebp - 0x58]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */));
    // 004244bc  d9c0                   -fld st(0)
    fp1 = x86::Float(fp0);
    // 004244be  dc45f0                 -fadd qword ptr [ebp - 0x10]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 004244c1  ddd9                   -fstp st(1)
    fp0 = x86::Float(fp1);
    // 004244c3  d95da8                 -fstp dword ptr [ebp - 0x58]
    app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = float(fp0);
    // 004244c6  d905cc6e5300           -fld dword ptr [0x536ecc]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5467852) /* 0x536ecc */));
    // 004244cc  d98218010000           -fld dword ptr [edx + 0x118]
    fp1 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(280) /* 0x118 */));
    // 004244d2  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004244d4  d845a8                 -fadd dword ptr [ebp - 0x58]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */)));
    // 004244d7  d95da8                 -fstp dword ptr [ebp - 0x58]
    app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = float(fp1);
    // 004244da  d88a1c010000           -fmul dword ptr [edx + 0x11c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(284) /* 0x11c */)));
    // 004244e0  d86dac                 -fsubr dword ptr [ebp - 0x54]
    fp0 = fp::sub(x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-84) /* -0x54 */)), fp0);
    // 004244e3  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004244e6  d95dac                 -fstp dword ptr [ebp - 0x54]
    app->getMemory<float>(cpu.ebp + x86::reg32(-84) /* -0x54 */) = float(fp0);
    // 004244e9  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004244ec  833c85a0c4790000       +cmp dword ptr [eax*4 + 0x79c4a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7980192) /* 0x79c4a0 */ + cpu.eax * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004244f4  0f848d000000           -je 0x424587
    if (cpu.flags.zf)
    {
        goto L_0x00424587;
    }
    // 004244fa  8075af80               -xor byte ptr [ebp - 0x51], 0x80
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-81) /* -0x51 */) ^= x86::reg8(x86::sreg8(128 /*0x80*/));
    // 004244fe  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424501  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00424504  8d90c0000000           -lea edx, [eax + 0xc0]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(192) /* 0xc0 */);
    // 0042450a  8d5dc8                 -lea ebx, [ebp - 0x38]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 0042450d  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 00424510  d900                   -fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424512  d80a                   -fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00424514  d900                   -fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424516  d84a04                 -fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00424519  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 0042451b  d84a08                 -fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 0042451e  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00424520  d94004                 -fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424523  d84a0c                 -fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00424526  d94004                 -fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424529  d84a10                 -fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 0042452c  d94004                 -fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 0042452f  d84a14                 -fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424532  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00424534  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424536  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424538  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 0042453a  d94008                 -fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 0042453d  d84a18                 -fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424540  d94008                 -fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424543  d84a1c                 -fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424546  d94008                 -fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424549  d84a20                 -fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 0042454c  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 0042454e  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424550  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424552  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00424554  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424557  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 00424559  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 0042455c  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 0042455f  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424562  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424565  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00424568  83c114                 -add ecx, 0x14
    (cpu.ecx) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0042456b  8d9098000000           -lea edx, [eax + 0x98]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(152) /* 0x98 */);
    // 00424571  8d5dc8                 -lea ebx, [ebp - 0x38]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 00424574  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424579  e832ba0b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042457e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00424580  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424581  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424582  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424583  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424584  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424585  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424586  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00424587:
    // 00424587  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0042458a  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0042458d  8d90c0000000           -lea edx, [eax + 0xc0]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(192) /* 0xc0 */);
    // 00424593  8d5dc8                 -lea ebx, [ebp - 0x38]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 00424596  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 00424599  d900                   -fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 0042459b  d80a                   -fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 0042459d  d900                   -fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 0042459f  d84a04                 -fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 004245a2  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004245a4  d84a08                 -fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004245a7  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004245a9  d94004                 -fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004245ac  d84a0c                 -fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 004245af  d94004                 -fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004245b2  d84a10                 -fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 004245b5  d94004                 -fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004245b8  d84a14                 -fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 004245bb  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 004245bd  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 004245bf  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 004245c1  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 004245c3  d94008                 -fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004245c6  d84a18                 -fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 004245c9  d94008                 -fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004245cc  d84a1c                 -fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 004245cf  d94008                 -fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004245d2  d84a20                 -fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 004245d5  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 004245d7  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 004245d9  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 004245db  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 004245dd  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004245e0  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 004245e2  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 004245e5  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 004245e8  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004245eb  83b80c01000001         +cmp dword ptr [eax + 0x10c], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004245f2  7515                   -jne 0x424609
    if (!cpu.flags.zf)
    {
        goto L_0x00424609;
    }
    // 004245f4  8d7dbc                 -lea edi, [ebp - 0x44]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 004245f7  8d75c8                 -lea esi, [ebp - 0x38]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 004245fa  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004245fb  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004245fc  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004245fd  c7800c01000000000000   -mov dword ptr [eax + 0x10c], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */) = 0 /*0x0*/;
    // 00424607  eb21                   -jmp 0x42462a
    goto L_0x0042462a;
L_0x00424609:
    // 00424609  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 0042460c  833cc510105e0000       +cmp dword ptr [eax*8 + 0x5e1010], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6164496) /* 0x5e1010 */ + cpu.eax * 8);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424614  7408                   -je 0x42461e
    if (cpu.flags.zf)
    {
        goto L_0x0042461e;
    }
    // 00424616  8d7dbc                 -lea edi, [ebp - 0x44]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 00424619  8d75c8                 -lea esi, [ebp - 0x38]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 0042461c  eb09                   -jmp 0x424627
    goto L_0x00424627;
L_0x0042461e:
    // 0042461e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424621  8d7dbc                 -lea edi, [ebp - 0x44]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 00424624  8d7020                 -lea esi, [eax + 0x20]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(32) /* 0x20 */);
L_0x00424627:
    // 00424627  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424628  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424629  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x0042462a:
    // 0042462a  8d4dd4                 -lea ecx, [ebp - 0x2c]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0042462d  8d5dbc                 -lea ebx, [ebp - 0x44]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 00424630  8d55c8                 -lea edx, [ebp - 0x38]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 00424633  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424638  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0042463b  e8c0b90b00             -call 0x4e0000
    cpu.esp -= 4;
    nfs3hp::vertices::subtractVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424640  8d45b0                 -lea eax, [ebp - 0x50]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 00424643  8d55d4                 -lea edx, [ebp - 0x2c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 00424646  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00424647  8d5db0                 -lea ebx, [ebp - 0x50]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 0042464a  83c620                 -add esi, 0x20
    (cpu.esi) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 0042464d  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 00424650  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424655  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 00424657  e8f4b90b00             -call 0x4e0050
    cpu.esp -= 4;
    nfs3hp::vertices::scaleVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042465c  8d55bc                 -lea edx, [ebp - 0x44]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 0042465f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424664  e847b90b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424669  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0042466c  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0042466f  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00424672  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00424674  8d9098000000           -lea edx, [eax + 0x98]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(152) /* 0x98 */);
    // 0042467a  83c114                 -add ecx, 0x14
    (cpu.ecx) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0042467d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424682  e829b90b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00424687:
    // 00424687  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00424689  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042468a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042468b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042468c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042468d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042468e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042468f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_424690(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 00424690  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00424691  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00424692  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00424693  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00424694  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00424696  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 00424699  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0042469b  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0042469e  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 004246a5  7406                   -je 0x4246ad
    if (cpu.flags.zf)
    {
        goto L_0x004246ad;
    }
    // 004246a7  8b80a4090000           -mov eax, dword ptr [eax + 0x9a4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(2468) /* 0x9a4 */);
L_0x004246ad:
    // 004246ad  8b9098000000           -mov edx, dword ptr [eax + 0x98]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(152) /* 0x98 */);
    // 004246b3  8955e8                 -mov dword ptr [ebp - 0x18], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.edx;
    // 004246b6  8b909c000000           -mov edx, dword ptr [eax + 0x9c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(156) /* 0x9c */);
    // 004246bc  8955ec                 -mov dword ptr [ebp - 0x14], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edx;
    // 004246bf  8b90a0000000           -mov edx, dword ptr [eax + 0xa0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(160) /* 0xa0 */);
    // 004246c5  8955f0                 -mov dword ptr [ebp - 0x10], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edx;
    // 004246c8  894108                 -mov dword ptr [ecx + 8], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004246cb  8b15a8367d00           -mov edx, dword ptr [0x7d36a8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */);
    // 004246d1  894104                 -mov dword ptr [ecx + 4], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 004246d4  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004246d6  7409                   -je 0x4246e1
    if (cpu.flags.zf)
    {
        goto L_0x004246e1;
    }
    // 004246d8  833d60d97c0000         +cmp dword ptr [0x7cd960], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8182112) /* 0x7cd960 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004246df  7453                   -je 0x424734
    if (cpu.flags.zf)
    {
        goto L_0x00424734;
    }
L_0x004246e1:
    // 004246e1  833dec00560000         +cmp dword ptr [0x5600ec], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5636332) /* 0x5600ec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004246e8  754a                   -jne 0x424734
    if (!cpu.flags.zf)
    {
        goto L_0x00424734;
    }
    // 004246ea  db0584367d00           -fild dword ptr [0x7d3684]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */)));
    // 004246f0  d80dd06e5300           -fmul dword ptr [0x536ed0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467856) /* 0x536ed0 */)));
    // 004246f6  8d5dfc                 -lea ebx, [ebp - 4]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004246f9  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004246fc  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004246ff  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 00424702  e869620c00             -call 0x4ea970
    cpu.esp -= 4;
    sub_4ea970(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424707  d945fc                 -fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 0042470a  d80dd46e5300           -fmul dword ptr [0x536ed4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467860) /* 0x536ed4 */)));
    // 00424710  d845e8                 -fadd dword ptr [ebp - 0x18]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */)));
    // 00424713  d95914                 -fstp dword ptr [ecx + 0x14]
    app->getMemory<float>(cpu.ecx + x86::reg32(20) /* 0x14 */) = float(fp0);
    // 00424716  d945f4                 -fld dword ptr [ebp - 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 00424719  d80dd86e5300           -fmul dword ptr [0x536ed8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467864) /* 0x536ed8 */)));
    // 0042471f  d845ec                 -fadd dword ptr [ebp - 0x14]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */)));
    // 00424722  d95918                 -fstp dword ptr [ecx + 0x18]
    app->getMemory<float>(cpu.ecx + x86::reg32(24) /* 0x18 */) = float(fp0);
    // 00424725  d945f4                 -fld dword ptr [ebp - 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 00424728  d80ddc6e5300           -fmul dword ptr [0x536edc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467868) /* 0x536edc */)));
    // 0042472e  d845f0                 -fadd dword ptr [ebp - 0x10]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 00424731  d9591c                 -fstp dword ptr [ecx + 0x1c]
    app->getMemory<float>(cpu.ecx + x86::reg32(28) /* 0x1c */) = float(fp0);
L_0x00424734:
    // 00424734  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00424736  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424737  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424738  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424739  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042473a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_424740(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
  x86::reg16 fsw = 0;
    // 00424740  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00424741  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00424742  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00424743  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00424744  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00424745  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00424746  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00424748  83ec78                 -sub esp, 0x78
    (cpu.esp) -= x86::reg32(x86::sreg32(120 /*0x78*/));
    // 0042474b  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0042474e  8d7da0                 -lea edi, [ebp - 0x60]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-96) /* -0x60 */);
    // 00424751  8db010010000           -lea esi, [eax + 0x110]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(272) /* 0x110 */);
    // 00424757  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424758  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424759  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0042475a  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0042475d  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0042475f  8b8298000000           -mov eax, dword ptr [edx + 0x98]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(152) /* 0x98 */);
    // 00424765  8945c4                 -mov dword ptr [ebp - 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.eax;
    // 00424768  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0042476a  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0042476d  8b809c000000           -mov eax, dword ptr [eax + 0x9c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(156) /* 0x9c */);
    // 00424773  8945c8                 -mov dword ptr [ebp - 0x38], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.eax;
    // 00424776  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00424778  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0042477b  8b80a0000000           -mov eax, dword ptr [eax + 0xa0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(160) /* 0xa0 */);
    // 00424781  8b1da8367d00           -mov ebx, dword ptr [0x7d36a8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */);
    // 00424787  8945cc                 -mov dword ptr [ebp - 0x34], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.eax;
    // 0042478a  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0042478c  740d                   -je 0x42479b
    if (cpu.flags.zf)
    {
        goto L_0x0042479b;
    }
    // 0042478e  833d60d97c0000         +cmp dword ptr [0x7cd960], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8182112) /* 0x7cd960 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424795  0f84a1030000           -je 0x424b3c
    if (cpu.flags.zf)
    {
        goto L_0x00424b3c;
    }
L_0x0042479b:
    // 0042479b  8b3dec005600           -mov edi, dword ptr [0x5600ec]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5636332) /* 0x5600ec */);
    // 004247a1  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004247a3  0f8593030000           -jne 0x424b3c
    if (!cpu.flags.zf)
    {
        goto L_0x00424b3c;
    }
    // 004247a9  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004247ac  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004247af  833cc514105e0020       +cmp dword ptr [eax*8 + 0x5e1014], 0x20
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6164500) /* 0x5e1014 */ + cpu.eax * 8);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(32 /*0x20*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004247b7  7e13                   -jle 0x4247cc
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004247cc;
    }
    // 004247b9  b8000050c0             -mov eax, 0xc0500000
    cpu.eax = 3226468352 /*0xc0500000*/;
    // 004247be  897da0                 -mov dword ptr [ebp - 0x60], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-96) /* -0x60 */) = cpu.edi;
    // 004247c1  bf0000803f             -mov edi, 0x3f800000
    cpu.edi = 1065353216 /*0x3f800000*/;
    // 004247c6  8945a8                 -mov dword ptr [ebp - 0x58], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = cpu.eax;
    // 004247c9  897da4                 -mov dword ptr [ebp - 0x5c], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */) = cpu.edi;
L_0x004247cc:
    // 004247cc  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004247cf  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004247d2  81b80c010000f4fd343f   +cmp dword ptr [eax + 0x10c], 0x3f34fdf4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1060437492 /*0x3f34fdf4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004247dc  7d36                   -jge 0x424814
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00424814;
    }
    // 004247de  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004247e1  83b80c01000000         +cmp dword ptr [eax + 0x10c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004247e8  751b                   -jne 0x424805
    if (!cpu.flags.zf)
    {
        goto L_0x00424805;
    }
    // 004247ea  83ba9809000000         +cmp dword ptr [edx + 0x998], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(2456) /* 0x998 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004247f1  7e07                   -jle 0x4247fa
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004247fa;
    }
    // 004247f3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004247f8  eb02                   -jmp 0x4247fc
    goto L_0x004247fc;
L_0x004247fa:
    // 004247fa  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
L_0x004247fc:
    // 004247fc  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004247ff  8981fc000000           -mov dword ptr [ecx + 0xfc], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(252) /* 0xfc */) = cpu.eax;
L_0x00424805:
    // 00424805  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424808  c7800c01000003000000   -mov dword ptr [eax + 0x10c], 3
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */) = 3 /*0x3*/;
    // 00424812  eb0c                   -jmp 0x424820
    goto L_0x00424820;
L_0x00424814:
    // 00424814  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424817  83b80c01000000         +cmp dword ptr [eax + 0x10c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0042481e  7417                   -je 0x424837
    if (cpu.flags.zf)
    {
        goto L_0x00424837;
    }
L_0x00424820:
    // 00424820  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424823  8bb80c010000           -mov edi, dword ptr [eax + 0x10c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
    // 00424829  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 0042482b  4f                     +dec edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0042482c  8975e4                 -mov dword ptr [ebp - 0x1c], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.esi;
    // 0042482f  89b80c010000           -mov dword ptr [eax + 0x10c], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */) = cpu.edi;
    // 00424835  eb30                   -jmp 0x424867
    goto L_0x00424867;
L_0x00424837:
    // 00424837  ffb11c010000           -push dword ptr [ecx + 0x11c]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(284) /* 0x11c */);
    cpu.esp -= 4;
    // 0042483d  e86ef7ffff             -call 0x423fb0
    cpu.esp -= 4;
    nfs3hp::carstate::scaledIfSet(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00424842  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424845  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424848  8b1c85a0c47900         -mov ebx, dword ptr [eax*4 + 0x79c4a0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7980192) /* 0x79c4a0 */ + cpu.eax * 4);
    // 0042484f  d95de4                 -fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp0);
    // 00424852  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00424854  7511                   -jne 0x424867
    if (!cpu.flags.zf)
    {
        goto L_0x00424867;
    }
    // 00424856  8d45a0                 -lea eax, [ebp - 0x60]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-96) /* -0x60 */);
    // 00424859  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0042485a  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0042485c  ffb120010000           -push dword ptr [ecx + 0x120]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(288) /* 0x120 */);
    cpu.esp -= 4;
    // 00424862  e8c9f7ffff             -call 0x424030
    cpu.esp -= 4;
    nfs3hp::carstate::dampVector(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00424867:
    // 00424867  d905e06e5300           +fld dword ptr [0x536ee0]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5467872) /* 0x536ee0 */));
    // 0042486d  d98218010000           +fld dword ptr [edx + 0x118]
    fp1 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(280) /* 0x118 */));
    // 00424873  d8c9                   +fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 00424875  d845a4                 +fadd dword ptr [ebp - 0x5c]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-92) /* -0x5c */)));
    // 00424878  d95da4                 +fstp dword ptr [ebp - 0x5c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-92) /* -0x5c */) = float(fp1);
    // 0042487b  d88a1c010000           +fmul dword ptr [edx + 0x11c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(284) /* 0x11c */)));
    // 00424881  d86da8                 +fsubr dword ptr [ebp - 0x58]
    fp0 = fp::sub(x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */)), fp0);
    // 00424884  d9ee                   +fldz 
    fp1 = 0.0;
    // 00424886  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00424888  d95da8                 +fstp dword ptr [ebp - 0x58]
    app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = float(fp1);
    // 0042488b  d89aec040000           +fcomp dword ptr [edx + 0x4ec]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */)));
    // 00424891  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00424893  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00424894  760d                   -jbe 0x4248a3
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004248a3;
    }
    // 00424896  d982ec040000           +fld dword ptr [edx + 0x4ec]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */));
    // 0042489c  d9e0                   +fchs 
    fp0 = -fp0;
    // 0042489e  d95dec                 +fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fp0);
    // 004248a1  eb09                   -jmp 0x4248ac
    goto L_0x004248ac;
L_0x004248a3:
    // 004248a3  8b82ec040000           -mov eax, dword ptr [edx + 0x4ec]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1260) /* 0x4ec */);
    // 004248a9  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
L_0x004248ac:
    // 004248ac  d9ee                   +fldz 
    fp0 = 0.0;
    // 004248ae  d89af4040000           +fcomp dword ptr [edx + 0x4f4]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */)));
    // 004248b4  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004248b6  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004248b7  760d                   -jbe 0x4248c6
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004248c6;
    }
    // 004248b9  d982f4040000           +fld dword ptr [edx + 0x4f4]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */));
    // 004248bf  d9e0                   +fchs 
    fp0 = -fp0;
    // 004248c1  d95df0                 +fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp0);
    // 004248c4  eb09                   -jmp 0x4248cf
    goto L_0x004248cf;
L_0x004248c6:
    // 004248c6  8b82f4040000           -mov eax, dword ptr [edx + 0x4f4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1268) /* 0x4f4 */);
    // 004248cc  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
L_0x004248cf:
    // 004248cf  d945ec                 +fld dword ptr [ebp - 0x14]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */));
    // 004248d2  d85df0                 +fcomp dword ptr [ebp - 0x10]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 004248d5  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004248d7  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004248d8  765c                   -jbe 0x424936
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00424936;
    }
    // 004248da  d9ee                   +fldz 
    fp0 = 0.0;
    // 004248dc  d89af4040000           +fcomp dword ptr [edx + 0x4f4]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */)));
    // 004248e2  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004248e4  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004248e5  760d                   -jbe 0x4248f4
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004248f4;
    }
    // 004248e7  d982f4040000           +fld dword ptr [edx + 0x4f4]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */));
    // 004248ed  d9e0                   +fchs 
    fp0 = -fp0;
    // 004248ef  d95de0                 +fstp dword ptr [ebp - 0x20]
    app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = float(fp0);
    // 004248f2  eb09                   -jmp 0x4248fd
    goto L_0x004248fd;
L_0x004248f4:
    // 004248f4  8b82f4040000           -mov eax, dword ptr [edx + 0x4f4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1268) /* 0x4f4 */);
    // 004248fa  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
L_0x004248fd:
    // 004248fd  d945e0                 +fld dword ptr [ebp - 0x20]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */));
    // 00424900  d80de46e5300           +fmul dword ptr [0x536ee4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467876) /* 0x536ee4 */)));
    // 00424906  d9ee                   +fldz 
    fp1 = 0.0;
    // 00424908  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0042490a  d95dd0                 +fstp dword ptr [ebp - 0x30]
    app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = float(fp1);
    // 0042490d  d89aec040000           +fcomp dword ptr [edx + 0x4ec]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */)));
    // 00424913  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00424915  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00424916  760d                   -jbe 0x424925
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00424925;
    }
    // 00424918  d982ec040000           +fld dword ptr [edx + 0x4ec]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */));
    // 0042491e  d9e0                   +fchs 
    fp0 = -fp0;
    // 00424920  d95dd8                 +fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fp0);
    // 00424923  eb09                   -jmp 0x42492e
    goto L_0x0042492e;
L_0x00424925:
    // 00424925  8b82ec040000           -mov eax, dword ptr [edx + 0x4ec]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1260) /* 0x4ec */);
    // 0042492b  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
L_0x0042492e:
    // 0042492e  d945d8                 +fld dword ptr [ebp - 0x28]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */));
    // 00424931  d845d0                 +fadd dword ptr [ebp - 0x30]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */)));
    // 00424934  eb5a                   -jmp 0x424990
    goto L_0x00424990;
L_0x00424936:
    // 00424936  d9ee                   +fldz 
    fp0 = 0.0;
    // 00424938  d89aec040000           +fcomp dword ptr [edx + 0x4ec]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */)));
    // 0042493e  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00424940  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00424941  760d                   -jbe 0x424950
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00424950;
    }
    // 00424943  d982ec040000           +fld dword ptr [edx + 0x4ec]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */));
    // 00424949  d9e0                   +fchs 
    fp0 = -fp0;
    // 0042494b  d95de8                 +fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp0);
    // 0042494e  eb09                   -jmp 0x424959
    goto L_0x00424959;
L_0x00424950:
    // 00424950  8b82ec040000           -mov eax, dword ptr [edx + 0x4ec]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1260) /* 0x4ec */);
    // 00424956  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
L_0x00424959:
    // 00424959  d945e8                 +fld dword ptr [ebp - 0x18]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */));
    // 0042495c  d80de46e5300           +fmul dword ptr [0x536ee4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467876) /* 0x536ee4 */)));
    // 00424962  d9ee                   +fldz 
    fp1 = 0.0;
    // 00424964  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00424966  d95dd4                 +fstp dword ptr [ebp - 0x2c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = float(fp1);
    // 00424969  d89af4040000           +fcomp dword ptr [edx + 0x4f4]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */)));
    // 0042496f  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00424971  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00424972  760d                   -jbe 0x424981
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00424981;
    }
    // 00424974  d982f4040000           +fld dword ptr [edx + 0x4f4]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */));
    // 0042497a  d9e0                   +fchs 
    fp0 = -fp0;
    // 0042497c  d95ddc                 +fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp0);
    // 0042497f  eb09                   -jmp 0x42498a
    goto L_0x0042498a;
L_0x00424981:
    // 00424981  8b82f4040000           -mov eax, dword ptr [edx + 0x4f4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1268) /* 0x4f4 */);
    // 00424987  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
L_0x0042498a:
    // 0042498a  d945dc                 +fld dword ptr [ebp - 0x24]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */));
    // 0042498d  d845d4                 +fadd dword ptr [ebp - 0x2c]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */)));
L_0x00424990:
    // 00424990  d95df4                 +fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp0);
    // 00424993  d945f4                 +fld dword ptr [ebp - 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 00424996  d80de86e5300           +fmul dword ptr [0x536ee8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467880) /* 0x536ee8 */)));
    // 0042499c  d955f8                 +fst dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 0042499f  d81dec6e5300           +fcomp dword ptr [0x536eec]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467884) /* 0x536eec */)));
    // 004249a5  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004249a7  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004249a8  7607                   -jbe 0x4249b1
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004249b1;
    }
    // 004249aa  c745f80000c040         -mov dword ptr [ebp - 8], 0x40c00000
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = 1086324736 /*0x40c00000*/;
L_0x004249b1:
    // 004249b1  d945a8                 -fld dword ptr [ebp - 0x58]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */));
    // 004249b4  d865f8                 -fsub dword ptr [ebp - 8]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004249b7  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004249ba  d95da8                 -fstp dword ptr [ebp - 0x58]
    app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = float(fp0);
    // 004249bd  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004249c0  833c85a0c4790000       +cmp dword ptr [eax*4 + 0x79c4a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7980192) /* 0x79c4a0 */ + cpu.eax * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004249c8  747b                   -je 0x424a45
    if (cpu.flags.zf)
    {
        goto L_0x00424a45;
    }
    // 004249ca  8075ab80               -xor byte ptr [ebp - 0x55], 0x80
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-85) /* -0x55 */) ^= x86::reg8(x86::sreg8(128 /*0x80*/));
    // 004249ce  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004249d1  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004249d4  8d5db8                 -lea ebx, [ebp - 0x48]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 004249d7  8d90c0000000           -lea edx, [eax + 0xc0]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(192) /* 0xc0 */);
    // 004249dd  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004249e0  8d45a0                 -lea eax, [ebp - 0x60]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-96) /* -0x60 */);
    // 004249e3  83c114                 +add ecx, 0x14
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004249e6  d900                   +fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004249e8  d80a                   +fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 004249ea  d900                   +fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004249ec  d84a04                 +fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 004249ef  d900                   +fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004249f1  d84a08                 +fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004249f4  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004249f6  d94004                 +fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004249f9  d84a0c                 +fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 004249fc  d94004                 +fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004249ff  d84a10                 +fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00424a02  d94004                 +fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424a05  d84a14                 +fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424a08  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00424a0a  dec3                   +faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424a0c  dec3                   +faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424a0e  dec3                   +faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00424a10  d94008                 +fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a13  d84a18                 +fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424a16  d94008                 +fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a19  d84a1c                 +fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424a1c  d94008                 +fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a1f  d84a20                 +fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00424a22  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00424a24  dec3                   +faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424a26  dec3                   +faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424a28  dec3                   +faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00424a2a  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424a2d  d91b                   +fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 00424a2f  d95b04                 +fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 00424a32  d95b08                 +fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 00424a35  8d5db8                 -lea ebx, [ebp - 0x48]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 00424a38  8d55c4                 -lea edx, [ebp - 0x3c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00424a3b  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424a40  e9f2000000             -jmp 0x424b37
    goto L_0x00424b37;
L_0x00424a45:
    // 00424a45  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424a48  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00424a4b  8d90c0000000           -lea edx, [eax + 0xc0]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(192) /* 0xc0 */);
    // 00424a51  8d5db8                 -lea ebx, [ebp - 0x48]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 00424a54  8d45a0                 -lea eax, [ebp - 0x60]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-96) /* -0x60 */);
    // 00424a57  d900                   -fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424a59  d80a                   -fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00424a5b  d900                   -fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424a5d  d84a04                 -fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00424a60  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424a62  d84a08                 -fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00424a65  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00424a67  d94004                 -fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424a6a  d84a0c                 -fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00424a6d  d94004                 -fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424a70  d84a10                 -fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00424a73  d94004                 -fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424a76  d84a14                 -fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424a79  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00424a7b  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424a7d  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424a7f  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00424a81  d94008                 -fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a84  d84a18                 -fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424a87  d94008                 -fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a8a  d84a1c                 -fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424a8d  d94008                 -fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a90  d84a20                 -fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00424a93  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00424a95  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424a97  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424a99  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00424a9b  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424a9e  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 00424aa0  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 00424aa3  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 00424aa6  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424aa9  83b80c01000001         +cmp dword ptr [eax + 0x10c], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424ab0  7515                   -jne 0x424ac7
    if (!cpu.flags.zf)
    {
        goto L_0x00424ac7;
    }
    // 00424ab2  8d7dac                 -lea edi, [ebp - 0x54]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 00424ab5  8d75b8                 -lea esi, [ebp - 0x48]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 00424ab8  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424ab9  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424aba  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424abb  c7800c01000000000000   -mov dword ptr [eax + 0x10c], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */) = 0 /*0x0*/;
    // 00424ac5  eb21                   -jmp 0x424ae8
    goto L_0x00424ae8;
L_0x00424ac7:
    // 00424ac7  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424aca  833cc510105e0000       +cmp dword ptr [eax*8 + 0x5e1010], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6164496) /* 0x5e1010 */ + cpu.eax * 8);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424ad2  7408                   -je 0x424adc
    if (cpu.flags.zf)
    {
        goto L_0x00424adc;
    }
    // 00424ad4  8d7dac                 -lea edi, [ebp - 0x54]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 00424ad7  8d75b8                 -lea esi, [ebp - 0x48]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 00424ada  eb09                   -jmp 0x424ae5
    goto L_0x00424ae5;
L_0x00424adc:
    // 00424adc  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424adf  8d7dac                 -lea edi, [ebp - 0x54]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 00424ae2  8d7020                 -lea esi, [eax + 0x20]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(32) /* 0x20 */);
L_0x00424ae5:
    // 00424ae5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424ae6  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424ae7  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x00424ae8:
    // 00424ae8  8d4d94                 -lea ecx, [ebp - 0x6c]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-108) /* -0x6c */);
    // 00424aeb  8d5dac                 -lea ebx, [ebp - 0x54]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 00424aee  8d55b8                 -lea edx, [ebp - 0x48]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 00424af1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424af6  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424af9  e802b50b00             -call 0x4e0000
    cpu.esp -= 4;
    nfs3hp::vertices::subtractVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424afe  8d4588                 -lea eax, [ebp - 0x78]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-120) /* -0x78 */);
    // 00424b01  8d5594                 -lea edx, [ebp - 0x6c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-108) /* -0x6c */);
    // 00424b04  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00424b05  8d5d88                 -lea ebx, [ebp - 0x78]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-120) /* -0x78 */);
    // 00424b08  83c620                 -add esi, 0x20
    (cpu.esi) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 00424b0b  ff75e4                 -push dword ptr [ebp - 0x1c]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    cpu.esp -= 4;
    // 00424b0e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424b13  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 00424b15  e836b50b00             -call 0x4e0050
    cpu.esp -= 4;
    nfs3hp::vertices::scaleVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424b1a  8d55ac                 -lea edx, [ebp - 0x54]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 00424b1d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424b22  e889b40b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424b27  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424b2a  8d55c4                 -lea edx, [ebp - 0x3c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00424b2d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424b32  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00424b34  83c114                 -add ecx, 0x14
    (cpu.ecx) += x86::reg32(x86::sreg32(20 /*0x14*/));
L_0x00424b37:
    // 00424b37  e874b40b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00424b3c:
    // 00424b3c  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00424b3e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424b3f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424b40  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424b41  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424b42  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424b43  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424b44  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_424b50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
    // 00424b50  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00424b51  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00424b52  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00424b53  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00424b54  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00424b55  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00424b56  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00424b58  83ec2c                 -sub esp, 0x2c
    (cpu.esp) -= x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 00424b5b  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00424b5d  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00424b60  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00424b62  8b8298000000           -mov eax, dword ptr [edx + 0x98]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(152) /* 0x98 */);
    // 00424b68  8945d4                 -mov dword ptr [ebp - 0x2c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.eax;
    // 00424b6b  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00424b6e  8b809c000000           -mov eax, dword ptr [eax + 0x9c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(156) /* 0x9c */);
    // 00424b74  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
    // 00424b77  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00424b7a  8b80a0000000           -mov eax, dword ptr [eax + 0xa0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(160) /* 0xa0 */);
    // 00424b80  8b0da8367d00           -mov ecx, dword ptr [0x7d36a8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */);
    // 00424b86  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
    // 00424b89  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00424b8b  740d                   -je 0x424b9a
    if (cpu.flags.zf)
    {
        goto L_0x00424b9a;
    }
    // 00424b8d  833d60d97c0000         +cmp dword ptr [0x7cd960], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8182112) /* 0x7cd960 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424b94  0f8433010000           -je 0x424ccd
    if (cpu.flags.zf)
    {
        goto L_0x00424ccd;
    }
L_0x00424b9a:
    // 00424b9a  833dec00560000         +cmp dword ptr [0x5600ec], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5636332) /* 0x5600ec */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424ba1  0f8526010000           -jne 0x424ccd
    if (!cpu.flags.zf)
    {
        goto L_0x00424ccd;
    }
    // 00424ba7  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00424baa  81b80c010000f4fd343f   +cmp dword ptr [eax + 0x10c], 0x3f34fdf4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1060437492 /*0x3f34fdf4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424bb4  7d3e                   -jge 0x424bf4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00424bf4;
    }
    // 00424bb6  83be0c01000000         +cmp dword ptr [esi + 0x10c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424bbd  752b                   -jne 0x424bea
    if (!cpu.flags.zf)
    {
        goto L_0x00424bea;
    }
    // 00424bbf  83ba9809000000         +cmp dword ptr [edx + 0x998], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(2456) /* 0x998 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424bc6  7e07                   -jle 0x424bcf
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00424bcf;
    }
    // 00424bc8  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424bcd  eb02                   -jmp 0x424bd1
    goto L_0x00424bd1;
L_0x00424bcf:
    // 00424bcf  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00424bd1:
    // 00424bd1  8d55d4                 -lea edx, [ebp - 0x2c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 00424bd4  8d4e20                 -lea ecx, [esi + 0x20]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(32) /* 0x20 */);
    // 00424bd7  8986fc000000           -mov dword ptr [esi + 0xfc], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(252) /* 0xfc */) = cpu.eax;
    // 00424bdd  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424be2  8d5e14                 -lea ebx, [esi + 0x14]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 00424be5  e816b40b00             -call 0x4e0000
    cpu.esp -= 4;
    nfs3hp::vertices::subtractVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00424bea:
    // 00424bea  c7860c01000040000000   -mov dword ptr [esi + 0x10c], 0x40
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(268) /* 0x10c */) = 64 /*0x40*/;
L_0x00424bf4:
    // 00424bf4  8b8724010000           -mov eax, dword ptr [edi + 0x124]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(292) /* 0x124 */);
    // 00424bfa  8d5001                 -lea edx, [eax + 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 00424bfd  899724010000           -mov dword ptr [edi + 0x124], edx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(292) /* 0x124 */) = cpu.edx;
    // 00424c03  3b8720010000           +cmp eax, dword ptr [edi + 0x120]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edi + x86::reg32(288) /* 0x120 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424c09  7e1c                   -jle 0x424c27
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00424c27;
    }
    // 00424c0b  d9871c010000           -fld dword ptr [edi + 0x11c]
    fp0 = x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(284) /* 0x11c */));
    // 00424c11  d88728010000           -fadd dword ptr [edi + 0x128]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(296) /* 0x128 */)));
    // 00424c17  c7872401000000000000   -mov dword ptr [edi + 0x124], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(292) /* 0x124 */) = 0 /*0x0*/;
    // 00424c21  d99f28010000           -fstp dword ptr [edi + 0x128]
    app->getMemory<float>(cpu.edi + x86::reg32(296) /* 0x128 */) = float(fp0);
L_0x00424c27:
    // 00424c27  8d5dfc                 -lea ebx, [ebp - 4]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424c2a  8d55f8                 -lea edx, [ebp - 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00424c2d  8d8728010000           -lea eax, [edi + 0x128]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(296) /* 0x128 */);
    // 00424c33  e8385d0c00             -call 0x4ea970
    cpu.esp -= 4;
    sub_4ea970(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424c38  d98710010000           -fld dword ptr [edi + 0x110]
    fp0 = x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(272) /* 0x110 */));
    // 00424c3e  d84dfc                 -fmul dword ptr [ebp - 4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00424c41  d95dec                 -fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fp0);
    // 00424c44  d98718010000           -fld dword ptr [edi + 0x118]
    fp0 = x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(280) /* 0x118 */));
    // 00424c4a  d84df8                 -fmul dword ptr [ebp - 8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00424c4d  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp0);
    // 00424c50  d98714010000           -fld dword ptr [edi + 0x114]
    fp0 = x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(276) /* 0x114 */));
    // 00424c56  d845f8                 -fadd dword ptr [ebp - 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00424c59  d95df0                 -fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp0);
    // 00424c5c  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00424c5f  8d90c0000000           -lea edx, [eax + 0xc0]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(192) /* 0xc0 */);
    // 00424c65  8d5de0                 -lea ebx, [ebp - 0x20]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00424c68  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00424c6b  8d4e14                 -lea ecx, [esi + 0x14]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 00424c6e  d900                   -fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424c70  d80a                   -fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00424c72  d900                   -fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424c74  d84a04                 -fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00424c77  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424c79  d84a08                 -fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00424c7c  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00424c7e  d94004                 -fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424c81  d84a0c                 -fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00424c84  d94004                 -fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424c87  d84a10                 -fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00424c8a  d94004                 -fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424c8d  d84a14                 -fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424c90  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00424c92  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424c94  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424c96  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00424c98  d94008                 -fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424c9b  d84a18                 -fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424c9e  d94008                 -fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424ca1  d84a1c                 -fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424ca4  d94008                 -fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424ca7  d84a20                 -fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00424caa  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00424cac  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424cae  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424cb0  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00424cb2  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424cb5  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 00424cb7  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 00424cba  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 00424cbd  8d5de0                 -lea ebx, [ebp - 0x20]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00424cc0  8d55d4                 -lea edx, [ebp - 0x2c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 00424cc3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424cc8  e8e3b20b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00424ccd:
    // 00424ccd  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00424ccf  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424cd0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424cd1  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424cd2  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424cd3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424cd4  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424cd5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_424ce0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
    // 00424ce0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00424ce1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00424ce2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00424ce3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00424ce4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00424ce5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00424ce6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00424ce8  83ec20                 -sub esp, 0x20
    (cpu.esp) -= x86::reg32(x86::sreg32(32 /*0x20*/));
    // 00424ceb  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00424ced  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00424cef  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 00424cf4  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00424cf6  2b9624010000           -sub edx, dword ptr [esi + 0x124]
    (cpu.edx) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(292) /* 0x124 */)));
    // 00424cfc  83fa02                 +cmp edx, 2
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
    // 00424cff  0f8c16010000           -jl 0x424e1b
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00424e1b;
    }
    // 00424d05  8b3d1c305500           -mov edi, dword ptr [0x55301c]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5582876) /* 0x55301c */);
    // 00424d0b  898624010000           -mov dword ptr [esi + 0x124], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(292) /* 0x124 */) = cpu.eax;
    // 00424d11  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00424d13  7409                   -je 0x424d1e
    if (cpu.flags.zf)
    {
        goto L_0x00424d1e;
    }
    // 00424d15  833d2030550000         +cmp dword ptr [0x553020], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5582880) /* 0x553020 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424d1c  7451                   -je 0x424d6f
    if (cpu.flags.zf)
    {
        goto L_0x00424d6f;
    }
L_0x00424d1e:
    // 00424d1e  8b151c305500           -mov edx, dword ptr [0x55301c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5582876) /* 0x55301c */);
    // 00424d24  c7811c01000000000000   -mov dword ptr [ecx + 0x11c], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(284) /* 0x11c */) = 0 /*0x0*/;
    // 00424d2e  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00424d30  7525                   -jne 0x424d57
    if (!cpu.flags.zf)
    {
        goto L_0x00424d57;
    }
    // 00424d32  833d20305500ff         +cmp dword ptr [0x553020], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5582880) /* 0x553020 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424d39  750e                   -jne 0x424d49
    if (!cpu.flags.zf)
    {
        goto L_0x00424d49;
    }
    // 00424d3b  d98120010000           +fld dword ptr [ecx + 0x120]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(288) /* 0x120 */));
    // 00424d41  d805f06e5300           +fadd dword ptr [0x536ef0]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467888) /* 0x536ef0 */)));
    // 00424d47  eb20                   -jmp 0x424d69
    goto L_0x00424d69;
L_0x00424d49:
    // 00424d49  d98120010000           +fld dword ptr [ecx + 0x120]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(288) /* 0x120 */));
    // 00424d4f  d805f46e5300           +fadd dword ptr [0x536ef4]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467892) /* 0x536ef4 */)));
    // 00424d55  eb12                   -jmp 0x424d69
    goto L_0x00424d69;
L_0x00424d57:
    // 00424d57  db0520305500           -fild dword ptr [0x553020]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5582880) /* 0x553020 */)));
    // 00424d5d  d80df86e5300           -fmul dword ptr [0x536ef8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467896) /* 0x536ef8 */)));
    // 00424d63  d88120010000           -fadd dword ptr [ecx + 0x120]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(288) /* 0x120 */)));
L_0x00424d69:
    // 00424d69  d99920010000           -fstp dword ptr [ecx + 0x120]
    app->getMemory<float>(cpu.ecx + x86::reg32(288) /* 0x120 */) = float(fp0);
L_0x00424d6f:
    // 00424d6f  8d5dfc                 -lea ebx, [ebp - 4]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424d72  8d55f8                 -lea edx, [ebp - 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00424d75  8d8120010000           -lea eax, [ecx + 0x120]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(288) /* 0x120 */);
    // 00424d7b  e8f05b0c00             -call 0x4ea970
    cpu.esp -= 4;
    sub_4ea970(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424d80  d98110010000           -fld dword ptr [ecx + 0x110]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(272) /* 0x110 */));
    // 00424d86  d84dfc                 -fmul dword ptr [ebp - 4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00424d89  d95de0                 -fstp dword ptr [ebp - 0x20]
    app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = float(fp0);
    // 00424d8c  d98118010000           -fld dword ptr [ecx + 0x118]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(280) /* 0x118 */));
    // 00424d92  d84df8                 -fmul dword ptr [ebp - 8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00424d95  d95de8                 -fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp0);
    // 00424d98  d98114010000           -fld dword ptr [ecx + 0x114]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(276) /* 0x114 */));
    // 00424d9e  d845f8                 -fadd dword ptr [ebp - 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00424da1  d95de4                 -fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp0);
    // 00424da4  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00424da7  8d90c0000000           -lea edx, [eax + 0xc0]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(192) /* 0xc0 */);
    // 00424dad  8d5dec                 -lea ebx, [ebp - 0x14]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00424db0  8d45e0                 -lea eax, [ebp - 0x20]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00424db3  d900                   -fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424db5  d80a                   -fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00424db7  d900                   -fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424db9  d84a04                 -fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00424dbc  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424dbe  d84a08                 -fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00424dc1  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00424dc3  d94004                 -fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424dc6  d84a0c                 -fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00424dc9  d94004                 -fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424dcc  d84a10                 -fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00424dcf  d94004                 -fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424dd2  d84a14                 -fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424dd5  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00424dd7  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424dd9  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424ddb  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00424ddd  d94008                 -fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424de0  d84a18                 -fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424de3  d94008                 -fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424de6  d84a1c                 -fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424de9  d94008                 -fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424dec  d84a20                 -fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00424def  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00424df1  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424df3  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424df5  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00424df7  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424dfa  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 00424dfc  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 00424dff  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 00424e02  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00424e05  8d4e14                 -lea ecx, [esi + 0x14]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 00424e08  8d9098000000           -lea edx, [eax + 0x98]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(152) /* 0x98 */);
    // 00424e0e  8d5dec                 -lea ebx, [ebp - 0x14]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00424e11  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424e16  e895b10b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00424e1b:
    // 00424e1b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00424e1d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424e1e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424e1f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424e20  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424e21  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424e22  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424e23  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_424e30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
    // 00424e30  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00424e31  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00424e32  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00424e33  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00424e34  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00424e35  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00424e36  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00424e38  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 00424e3b  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00424e3d  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00424e3f  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00424e42  db8024050000           -fild dword ptr [eax + 0x524]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1316) /* 0x524 */)));
    // 00424e48  d80dfc6e5300           -fmul dword ptr [0x536efc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467900) /* 0x536efc */)));
    // 00424e4e  d805006f5300           -fadd dword ptr [0x536f00]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(x86::reg32(5467904) /* 0x536f00 */)));
    // 00424e54  d99e20010000           -fstp dword ptr [esi + 0x120]
    app->getMemory<float>(cpu.esi + x86::reg32(288) /* 0x120 */) = float(fp0);
    // 00424e5a  f6401c01               +test byte ptr [eax + 0x1c], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(28) /* 0x1c */) & 1 /*0x1*/));
    // 00424e5e  7409                   -je 0x424e69
    if (cpu.flags.zf)
    {
        goto L_0x00424e69;
    }
    // 00424e60  c745fc0000803f         -mov dword ptr [ebp - 4], 0x3f800000
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = 1065353216 /*0x3f800000*/;
    // 00424e67  eb07                   -jmp 0x424e70
    goto L_0x00424e70;
L_0x00424e69:
    // 00424e69  c745fc000080bf         -mov dword ptr [ebp - 4], 0xbf800000
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = 3212836864 /*0xbf800000*/;
L_0x00424e70:
    // 00424e70  8d5df8                 -lea ebx, [ebp - 8]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00424e73  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00424e76  8d8120010000           -lea eax, [ecx + 0x120]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(288) /* 0x120 */);
    // 00424e7c  e8ef5a0c00             -call 0x4ea970
    cpu.esp -= 4;
    sub_4ea970(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424e81  d945f8                 -fld dword ptr [ebp - 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 00424e84  d84dfc                 -fmul dword ptr [ebp - 4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00424e87  d88910010000           -fmul dword ptr [ecx + 0x110]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(272) /* 0x110 */)));
    // 00424e8d  d95ddc                 -fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp0);
    // 00424e90  d98118010000           -fld dword ptr [ecx + 0x118]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(280) /* 0x118 */));
    // 00424e96  d84df4                 -fmul dword ptr [ebp - 0xc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 00424e99  d95de4                 -fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp0);
    // 00424e9c  8b8114010000           -mov eax, dword ptr [ecx + 0x114]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(276) /* 0x114 */);
    // 00424ea2  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
    // 00424ea5  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00424ea8  8d90c0000000           -lea edx, [eax + 0xc0]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(192) /* 0xc0 */);
    // 00424eae  8d5de8                 -lea ebx, [ebp - 0x18]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00424eb1  8d45dc                 -lea eax, [ebp - 0x24]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00424eb4  d900                   -fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424eb6  d80a                   -fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00424eb8  d900                   -fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424eba  d84a04                 -fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00424ebd  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424ebf  d84a08                 -fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00424ec2  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00424ec4  d94004                 -fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424ec7  d84a0c                 -fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00424eca  d94004                 -fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424ecd  d84a10                 -fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00424ed0  d94004                 -fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424ed3  d84a14                 -fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424ed6  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00424ed8  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424eda  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424edc  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00424ede  d94008                 -fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424ee1  d84a18                 -fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424ee4  d94008                 -fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424ee7  d84a1c                 -fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424eea  d94008                 -fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424eed  d84a20                 -fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00424ef0  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00424ef2  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00424ef4  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00424ef6  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00424ef8  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424efb  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 00424efd  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 00424f00  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 00424f03  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00424f06  8d4e14                 -lea ecx, [esi + 0x14]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 00424f09  8d9098000000           -lea edx, [eax + 0x98]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(152) /* 0x98 */);
    // 00424f0f  8d5de8                 -lea ebx, [ebp - 0x18]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00424f12  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00424f17  e894b00b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424f1c  8d7e20                 -lea edi, [esi + 0x20]
    cpu.edi = x86::reg32(cpu.esi + x86::reg32(32) /* 0x20 */);
    // 00424f1f  8d75e8                 -lea esi, [ebp - 0x18]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00424f22  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424f23  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424f24  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424f25  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00424f27  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424f28  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424f29  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424f2a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424f2b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424f2c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424f2d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_424f30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00424f30  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00424f31  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00424f32  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00424f33  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00424f34  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00424f35  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00424f36  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00424f38  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 00424f3b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00424f3d  8d55dc                 -lea edx, [ebp - 0x24]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00424f40  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00424f42  83c02c                 -add eax, 0x2c
    (cpu.eax) += x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 00424f45  e876240900             -call 0x4b73c0
    cpu.esp -= 4;
    nfs3hp::frontend::simQuatFromMatrix(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424f4a  83b92401000000         +cmp dword ptr [ecx + 0x124], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(292) /* 0x124 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424f51  7457                   -je 0x424faa
    if (cpu.flags.zf)
    {
        goto L_0x00424faa;
    }
    // 00424f53  8b8128010000           -mov eax, dword ptr [ecx + 0x128]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(296) /* 0x128 */);
    // 00424f59  8b1d58da7c00           -mov ebx, dword ptr [0x7cda58]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182360) /* 0x7cda58 */);
    // 00424f5f  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00424f62  83fb02                 +cmp ebx, 2
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
    // 00424f65  7514                   -jne 0x424f7b
    if (!cpu.flags.zf)
    {
        goto L_0x00424f7b;
    }
    // 00424f67  3b1d4cd97c00           +cmp ebx, dword ptr [0x7cd94c]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182092) /* 0x7cd94c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00424f6d  7e0c                   -jle 0x424f7b
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00424f7b;
    }
    // 00424f6f  c7812401000000000000   -mov dword ptr [ecx + 0x124], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(292) /* 0x124 */) = 0 /*0x0*/;
    // 00424f79  eb46                   -jmp 0x424fc1
    goto L_0x00424fc1;
L_0x00424f7b:
    // 00424f7b  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00424f7e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00424f7f  8d55dc                 -lea edx, [ebp - 0x24]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00424f82  8d8614010000           -lea eax, [esi + 0x114]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(276) /* 0x114 */);
    // 00424f88  ff75fc                 -push dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    // 00424f8b  e850220900             -call 0x4b71e0
    cpu.esp -= 4;
    nfs3hp::frontend::simQuatSlerp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424f90  8dbe14010000           -lea edi, [esi + 0x114]
    cpu.edi = x86::reg32(cpu.esi + x86::reg32(276) /* 0x114 */);
    // 00424f96  8d75ec                 -lea esi, [ebp - 0x14]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00424f99  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00424f9c  8d512c                 -lea edx, [ecx + 0x2c]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(44) /* 0x2c */);
    // 00424f9f  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424fa0  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424fa1  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424fa2  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424fa3  e8e8250900             -call 0x4b7590
    cpu.esp -= 4;
    nfs3hp::frontend::simMatrixFromQuat(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424fa8  eb17                   -jmp 0x424fc1
    goto L_0x00424fc1;
L_0x00424faa:
    // 00424faa  8d75dc                 -lea esi, [ebp - 0x24]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00424fad  8db914010000           -lea edi, [ecx + 0x114]
    cpu.edi = x86::reg32(cpu.ecx + x86::reg32(276) /* 0x114 */);
    // 00424fb3  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424fb4  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424fb5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424fb6  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424fb7  c7812401000001000000   -mov dword ptr [ecx + 0x124], 1
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(292) /* 0x124 */) = 1 /*0x1*/;
L_0x00424fc1:
    // 00424fc1  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00424fc3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424fc4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424fc5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424fc6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424fc7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424fc8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00424fc9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_424fd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
  x86::reg16 fsw = 0;
    // 00424fd0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00424fd1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00424fd2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00424fd3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00424fd4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00424fd5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00424fd7  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00424fda  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00424fdc  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00424fde  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 00424fe0  8d7b14                 -lea edi, [ebx + 0x14]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(20) /* 0x14 */);
    // 00424fe3  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424fe4  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424fe5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00424fe6  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00424fe9  0598000000             +add eax, 0x98
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(152 /*0x98*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00424fee  e8bd150700             -call 0x4965b0
    cpu.esp -= 4;
    nfs3hp::frontend::simQuickDistance(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00424ff3  d9ee                   +fldz 
    fp1 = 0.0;
    // 00424ff5  ded9                   +fcompp 
    fsw = fp::compare(fp1, fp0);
    // 00424ff7  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00424ff9  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00424ffa  7613                   -jbe 0x42500f
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0042500f;
    }
    // 00424ffc  8b4308                 -mov eax, dword ptr [ebx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 00424fff  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00425001  0598000000             +add eax, 0x98
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(152 /*0x98*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00425006  e8a5150700             -call 0x4965b0
    cpu.esp -= 4;
    nfs3hp::frontend::simQuickDistance(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0042500b  d9e0                   +fchs 
    fp0 = -fp0;
    // 0042500d  eb0f                   -jmp 0x42501e
    goto L_0x0042501e;
L_0x0042500f:
    // 0042500f  8b4308                 -mov eax, dword ptr [ebx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 00425012  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00425014  0598000000             +add eax, 0x98
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(152 /*0x98*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00425019  e892150700             -call 0x4965b0
    cpu.esp -= 4;
    nfs3hp::frontend::simQuickDistance(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
L_0x0042501e:
    // 0042501e  d95dfc                 +fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 00425021  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00425024  d85d18                 +fcomp dword ptr [ebp + 0x18]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(24) /* 0x18 */)));
    // 00425027  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00425029  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0042502a  7614                   -jbe 0x425040
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00425040;
    }
    // 0042502c  f74518ffffff7f         +test dword ptr [ebp + 0x18], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */) & 2147483647 /*0x7fffffff*/));
    // 00425033  740b                   -je 0x425040
    if (cpu.flags.zf)
    {
        goto L_0x00425040;
    }
    // 00425035  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00425038  d87518                 +fdiv dword ptr [ebp + 0x18]
    fp0 = fp::div(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(24) /* 0x18 */)));
    // 0042503b  d95b54                 +fstp dword ptr [ebx + 0x54]
    app->getMemory<float>(cpu.ebx + x86::reg32(84) /* 0x54 */) = float(fp0);
    // 0042503e  eb07                   -jmp 0x425047
    goto L_0x00425047;
L_0x00425040:
    // 00425040  c7435400000000         -mov dword ptr [ebx + 0x54], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(84) /* 0x54 */) = 0 /*0x0*/;
L_0x00425047:
    // 00425047  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00425049  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042504a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042504b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042504c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042504d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042504e  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* sub_425060: native, nfs3hp::frontend::simCameraBesideRoad */

/* sub_4251f0: native, nfs3hp::frontend::simCameraNearestSpot */

/* sub_4253d0: native, nfs3hp::frontend::simCameraPlayPath */

/* sub_425640: native, nfs3hp::frontend::simCameraEndPath */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_425990(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
    // 00425990  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00425991  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00425992  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00425993  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00425994  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00425995  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00425996  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00425998  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0042599b  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0042599d  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004259a0  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004259a2  8b8298000000           -mov eax, dword ptr [edx + 0x98]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(152) /* 0x98 */);
    // 004259a8  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004259ab  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004259ae  8b809c000000           -mov eax, dword ptr [eax + 0x9c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(156) /* 0x9c */);
    // 004259b4  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004259b7  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004259ba  8b80a0000000           -mov eax, dword ptr [eax + 0xa0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(160) /* 0xa0 */);
    // 004259c0  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004259c3  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004259c6  81b80c010000f4fd343f   +cmp dword ptr [eax + 0x10c], 0x3f34fdf4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1060437492 /*0x3f34fdf4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004259d0  7d3e                   -jge 0x425a10
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00425a10;
    }
    // 004259d2  83be0c01000000         +cmp dword ptr [esi + 0x10c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(268) /* 0x10c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004259d9  752b                   -jne 0x425a06
    if (!cpu.flags.zf)
    {
        goto L_0x00425a06;
    }
    // 004259db  83ba9809000000         +cmp dword ptr [edx + 0x998], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(2456) /* 0x998 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004259e2  7e07                   -jle 0x4259eb
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004259eb;
    }
    // 004259e4  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004259e9  eb02                   -jmp 0x4259ed
    goto L_0x004259ed;
L_0x004259eb:
    // 004259eb  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004259ed:
    // 004259ed  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004259f0  8d4e20                 -lea ecx, [esi + 0x20]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(32) /* 0x20 */);
    // 004259f3  8986fc000000           -mov dword ptr [esi + 0xfc], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(252) /* 0xfc */) = cpu.eax;
    // 004259f9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004259fe  8d5614                 -lea edx, [esi + 0x14]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 00425a01  e8faa50b00             -call 0x4e0000
    cpu.esp -= 4;
    nfs3hp::vertices::subtractVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00425a06:
    // 00425a06  c7860c01000040000000   -mov dword ptr [esi + 0x10c], 0x40
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(268) /* 0x10c */) = 64 /*0x40*/;
L_0x00425a10:
    // 00425a10  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 00425a13  833c85a0c4790000       +cmp dword ptr [eax*4 + 0x79c4a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7980192) /* 0x79c4a0 */ + cpu.eax * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00425a1b  740f                   -je 0x425a2c
    if (cpu.flags.zf)
    {
        goto L_0x00425a2c;
    }
    // 00425a1d  8aa71b010000           -mov ah, byte ptr [edi + 0x11b]
    cpu.ah = app->getMemory<x86::reg8>(cpu.edi + x86::reg32(283) /* 0x11b */);
    // 00425a23  80f480                 -xor ah, 0x80
    cpu.ah ^= x86::reg8(x86::sreg8(128 /*0x80*/));
    // 00425a26  88a71b010000           -mov byte ptr [edi + 0x11b], ah
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(283) /* 0x11b */) = cpu.ah;
L_0x00425a2c:
    // 00425a2c  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00425a2f  8d90c0000000           -lea edx, [eax + 0xc0]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(192) /* 0xc0 */);
    // 00425a35  8d5de8                 -lea ebx, [ebp - 0x18]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00425a38  8d8710010000           -lea eax, [edi + 0x110]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(272) /* 0x110 */);
    // 00425a3e  8d4e14                 -lea ecx, [esi + 0x14]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 00425a41  d900                   -fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425a43  d80a                   -fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00425a45  d900                   -fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425a47  d84a04                 -fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00425a4a  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425a4c  d84a08                 -fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00425a4f  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00425a51  d94004                 -fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425a54  d84a0c                 -fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00425a57  d94004                 -fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425a5a  d84a10                 -fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00425a5d  d94004                 -fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425a60  d84a14                 -fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00425a63  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00425a65  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00425a67  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00425a69  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00425a6b  d94008                 -fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425a6e  d84a18                 -fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00425a71  d94008                 -fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425a74  d84a1c                 -fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00425a77  d94008                 -fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425a7a  d84a20                 -fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00425a7d  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00425a7f  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00425a81  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00425a83  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00425a85  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00425a88  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 00425a8a  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 00425a8d  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 00425a90  8d5de8                 -lea ebx, [ebp - 0x18]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00425a93  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00425a96  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00425a9b  e810a50b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425aa0  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00425aa2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425aa3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425aa4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425aa5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425aa6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425aa7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425aa8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_425ab0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
    // 00425ab0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00425ab1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00425ab2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00425ab3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00425ab4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00425ab6  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00425ab9  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00425abb  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00425abe  8d90c0000000           -lea edx, [eax + 0xc0]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(192) /* 0xc0 */);
    // 00425ac4  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00425ac7  8d8110010000           -lea eax, [ecx + 0x110]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(272) /* 0x110 */);
    // 00425acd  d900                   -fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425acf  d80a                   -fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00425ad1  d900                   -fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425ad3  d84a04                 -fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00425ad6  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425ad8  d84a08                 -fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00425adb  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00425add  d94004                 -fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425ae0  d84a0c                 -fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00425ae3  d94004                 -fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425ae6  d84a10                 -fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00425ae9  d94004                 -fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425aec  d84a14                 -fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00425aef  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00425af1  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00425af3  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00425af5  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00425af7  d94008                 -fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425afa  d84a18                 -fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00425afd  d94008                 -fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425b00  d84a1c                 -fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00425b03  d94008                 -fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425b06  d84a20                 -fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00425b09  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00425b0b  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00425b0d  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00425b0f  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00425b11  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00425b14  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 00425b16  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 00425b19  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 00425b1c  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00425b1f  8b5104                 -mov edx, dword ptr [ecx + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 00425b22  8d4114                 -lea eax, [ecx + 0x14]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(20) /* 0x14 */);
    // 00425b25  81c298000000           -add edx, 0x98
    (cpu.edx) += x86::reg32(x86::sreg32(152 /*0x98*/));
    // 00425b2b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00425b2d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00425b32  e879a40b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425b37  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00425b39  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425b3a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425b3b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425b3c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425b3d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_425b40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
  x86::reg16 fsw = 0;
    // 00425b40  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00425b41  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00425b42  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00425b43  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00425b44  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00425b45  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00425b47  81ec04010000           -sub esp, 0x104
    (cpu.esp) -= x86::reg32(x86::sreg32(260 /*0x104*/));
    // 00425b4d  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00425b4f  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00425b51  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 00425b53  8d7b14                 -lea edi, [ebx + 0x14]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(20) /* 0x14 */);
    // 00425b56  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00425b57  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00425b58  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00425b59  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00425b5c  0598000000             +add eax, 0x98
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(152 /*0x98*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00425b61  e84a0a0700             -call 0x4965b0
    cpu.esp -= 4;
    nfs3hp::frontend::simQuickDistance(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00425b66  d9ee                   +fldz 
    fp1 = 0.0;
    // 00425b68  ded9                   +fcompp 
    fsw = fp::compare(fp1, fp0);
    // 00425b6a  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00425b6c  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00425b6d  7613                   -jbe 0x425b82
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00425b82;
    }
    // 00425b6f  8b4308                 -mov eax, dword ptr [ebx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 00425b72  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00425b74  0598000000             +add eax, 0x98
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(152 /*0x98*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00425b79  e8320a0700             -call 0x4965b0
    cpu.esp -= 4;
    nfs3hp::frontend::simQuickDistance(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00425b7e  d9e0                   +fchs 
    fp0 = -fp0;
    // 00425b80  eb0f                   -jmp 0x425b91
    goto L_0x00425b91;
L_0x00425b82:
    // 00425b82  8b4308                 -mov eax, dword ptr [ebx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 00425b85  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00425b87  0598000000             -add eax, 0x98
    (cpu.eax) += x86::reg32(x86::sreg32(152 /*0x98*/));
    // 00425b8c  e81f0a0700             -call 0x4965b0
    cpu.esp -= 4;
    nfs3hp::frontend::simQuickDistance(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
L_0x00425b91:
    // 00425b91  d95dfc                 -fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 00425b94  d945fc                 -fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00425b97  8b1528305500           -mov edx, dword ptr [0x553028]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5582888) /* 0x553028 */);
    // 00425b9d  d84d1c                 -fmul dword ptr [ebp + 0x1c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(28) /* 0x1c */)));
    // 00425ba0  42                     -inc edx
    (cpu.edx)++;
    // 00425ba1  d84518                 -fadd dword ptr [ebp + 0x18]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(24) /* 0x18 */)));
    // 00425ba4  891528305500           -mov dword ptr [0x553028], edx
    app->getMemory<x86::reg32>(x86::reg32(5582888) /* 0x553028 */) = cpu.edx;
    // 00425baa  d95b54                 -fstp dword ptr [ebx + 0x54]
    app->getMemory<float>(cpu.ebx + x86::reg32(84) /* 0x54 */) = float(fp0);
    // 00425bad  83fa05                 +cmp edx, 5
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
    // 00425bb0  752e                   -jne 0x425be0
    if (!cpu.flags.zf)
    {
        goto L_0x00425be0;
    }
    // 00425bb2  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00425bb5  d94354                 -fld dword ptr [ebx + 0x54]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(84) /* 0x54 */));
    // 00425bb8  dd1c24                 -fstp qword ptr [esp]
    app->getMemory<double>(cpu.esp) = double(fp0);
    // 00425bbb  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00425bbe  d945fc                 -fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00425bc1  dd1c24                 -fstp qword ptr [esp]
    app->getMemory<double>(cpu.esp) = double(fp0);
    // 00425bc4  68246f5300             -push 0x536f24
    app->getMemory<x86::reg32>(cpu.esp-4) = 5467940 /*0x536f24*/;
    cpu.esp -= 4;
    // 00425bc9  8d85fcfeffff           -lea eax, [ebp - 0x104]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-260) /* -0x104 */);
    // 00425bcf  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00425bd0  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00425bd2  e8b99a0b00             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425bd7  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 00425bda  891d28305500           -mov dword ptr [0x553028], ebx
    app->getMemory<x86::reg32>(x86::reg32(5582888) /* 0x553028 */) = cpu.ebx;
L_0x00425be0:
    // 00425be0  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00425be2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425be3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425be4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425be5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425be6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425be7  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* sub_425bf0: native, nfs3hp::frontend::simCameraOverRoad */

/* sub_425d50: native, nfs3hp::frontend::simCameraFlyover */

/* sub_426290: native, nfs3hp::frontend::simCameraStill */

}
