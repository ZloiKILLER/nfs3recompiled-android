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
    // 00424459  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0042445c  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 0042445f  8b1c85a0c47900         -mov ebx, dword ptr [eax*4 + 0x79c4a0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7980192) /* 0x79c4a0 */ + cpu.eax * 4);
    // 00424466  d95df8                 -fstp dword ptr [ebp - 8]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = cpu.fpu.st(0);
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fpu1);
    // 00424469  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    cpu.fpu.count -= 1;
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(704) /* 0x2c0 */)));
    // 0042448a  dc0dbc6e5300           +fmul qword ptr [0x536ebc]
    x86::Float fpu3 = cpu.fpu.mul(fpu2, x86::Float(app->getMemory<double>(x86::reg32(5467836) /* 0x536ebc */)));
    // 00424490  dd55e0                 +fst qword ptr [ebp - 0x20]
    app->getMemory<double>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = double(fpu3);
    // 00424493  dc1dc46e5300           +fcomp qword ptr [0x536ec4]
    cpu.fpu.compare(fpu3, x86::Float(app->getMemory<double>(x86::reg32(5467844) /* 0x536ec4 */)));
    // 00424499  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 0042449b  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu4 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */));
    // 004244bc  d9c0                   -fld st(0)
    x86::Float fpu5 = x86::Float(fpu4);
    // 004244be  dc45f0                 -fadd qword ptr [ebp - 0x10]
    x86::Float fpu6 = cpu.fpu.add(fpu5, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 004244c1  ddd9                   -fstp st(1)
    x86::Float fpu7 = x86::Float(fpu6);
    // 004244c3  d95da8                 -fstp dword ptr [ebp - 0x58]
    app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = float(fpu7);
    // 004244c6  d905cc6e5300           -fld dword ptr [0x536ecc]
    x86::Float fpu8 = x86::Float(app->getMemory<float>(x86::reg32(5467852) /* 0x536ecc */));
    // 004244cc  d98218010000           -fld dword ptr [edx + 0x118]
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(280) /* 0x118 */));
    // 004244d2  d8c9                   -fmul st(1)
    x86::Float fpu10 = cpu.fpu.mul(fpu9, x86::Float(fpu8));
    // 004244d4  d845a8                 -fadd dword ptr [ebp - 0x58]
    x86::Float fpu11 = cpu.fpu.add(fpu10, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */)));
    // 004244d7  d95da8                 -fstp dword ptr [ebp - 0x58]
    app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = float(fpu11);
    // 004244da  d88a1c010000           -fmul dword ptr [edx + 0x11c]
    x86::Float fpu12 = cpu.fpu.mul(fpu8, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(284) /* 0x11c */)));
    // 004244e0  d86dac                 -fsubr dword ptr [ebp - 0x54]
    x86::Float fpu13 = cpu.fpu.sub(x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-84) /* -0x54 */)), fpu12);
    // 004244e3  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004244e6  d95dac                 -fstp dword ptr [ebp - 0x54]
    app->getMemory<float>(cpu.ebp + x86::reg32(-84) /* -0x54 */) = float(fpu13);
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu14 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424512  d80a                   -fmul dword ptr [edx]
    x86::Float fpu15 = cpu.fpu.mul(fpu14, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00424514  d900                   -fld dword ptr [eax]
    x86::Float fpu16 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424516  d84a04                 -fmul dword ptr [edx + 4]
    x86::Float fpu17 = cpu.fpu.mul(fpu16, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00424519  d900                   -fld dword ptr [eax]
    x86::Float fpu18 = x86::Float(app->getMemory<float>(cpu.eax));
    // 0042451b  d84a08                 -fmul dword ptr [edx + 8]
    x86::Float fpu19 = cpu.fpu.mul(fpu18, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 0042451e  d9ca                   -fxch st(2)
    
    // 00424520  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu20 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424523  d84a0c                 -fmul dword ptr [edx + 0xc]
    x86::Float fpu21 = cpu.fpu.mul(fpu20, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00424526  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu22 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424529  d84a10                 -fmul dword ptr [edx + 0x10]
    x86::Float fpu23 = cpu.fpu.mul(fpu22, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 0042452c  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu24 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 0042452f  d84a14                 -fmul dword ptr [edx + 0x14]
    x86::Float fpu25 = cpu.fpu.mul(fpu24, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424532  d9ca                   -fxch st(2)
    
    // 00424534  dec3                   -faddp st(3)
    x86::Float fpu26 = cpu.fpu.add(fpu15, fpu21);
    // 00424536  dec3                   -faddp st(3)
    x86::Float fpu27 = cpu.fpu.add(fpu17, fpu23);
    // 00424538  dec3                   -faddp st(3)
    x86::Float fpu28 = cpu.fpu.add(fpu19, fpu25);
    // 0042453a  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu29 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 0042453d  d84a18                 -fmul dword ptr [edx + 0x18]
    x86::Float fpu30 = cpu.fpu.mul(fpu29, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424540  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu31 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424543  d84a1c                 -fmul dword ptr [edx + 0x1c]
    x86::Float fpu32 = cpu.fpu.mul(fpu31, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424546  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu33 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424549  d84a20                 -fmul dword ptr [edx + 0x20]
    x86::Float fpu34 = cpu.fpu.mul(fpu33, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 0042454c  d9ca                   -fxch st(2)
    
    // 0042454e  dec3                   -faddp st(3)
    x86::Float fpu35 = cpu.fpu.add(fpu26, fpu30);
    // 00424550  dec3                   -faddp st(3)
    x86::Float fpu36 = cpu.fpu.add(fpu27, fpu32);
    // 00424552  dec3                   -faddp st(3)
    x86::Float fpu37 = cpu.fpu.add(fpu28, fpu34);
    // 00424554  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424557  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fpu35);
    // 00424559  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fpu36);
    // 0042455c  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fpu37);
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
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu38 = x86::Float(app->getMemory<float>(cpu.eax));
    // 0042459b  d80a                   -fmul dword ptr [edx]
    x86::Float fpu39 = cpu.fpu.mul(fpu38, x86::Float(app->getMemory<float>(cpu.edx)));
    // 0042459d  d900                   -fld dword ptr [eax]
    x86::Float fpu40 = x86::Float(app->getMemory<float>(cpu.eax));
    // 0042459f  d84a04                 -fmul dword ptr [edx + 4]
    x86::Float fpu41 = cpu.fpu.mul(fpu40, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 004245a2  d900                   -fld dword ptr [eax]
    x86::Float fpu42 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004245a4  d84a08                 -fmul dword ptr [edx + 8]
    x86::Float fpu43 = cpu.fpu.mul(fpu42, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004245a7  d9ca                   -fxch st(2)
    
    // 004245a9  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu44 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004245ac  d84a0c                 -fmul dword ptr [edx + 0xc]
    x86::Float fpu45 = cpu.fpu.mul(fpu44, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 004245af  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu46 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004245b2  d84a10                 -fmul dword ptr [edx + 0x10]
    x86::Float fpu47 = cpu.fpu.mul(fpu46, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 004245b5  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu48 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004245b8  d84a14                 -fmul dword ptr [edx + 0x14]
    x86::Float fpu49 = cpu.fpu.mul(fpu48, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 004245bb  d9ca                   -fxch st(2)
    
    // 004245bd  dec3                   -faddp st(3)
    x86::Float fpu50 = cpu.fpu.add(fpu39, fpu45);
    // 004245bf  dec3                   -faddp st(3)
    x86::Float fpu51 = cpu.fpu.add(fpu41, fpu47);
    // 004245c1  dec3                   -faddp st(3)
    x86::Float fpu52 = cpu.fpu.add(fpu43, fpu49);
    // 004245c3  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu53 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004245c6  d84a18                 -fmul dword ptr [edx + 0x18]
    x86::Float fpu54 = cpu.fpu.mul(fpu53, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 004245c9  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu55 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004245cc  d84a1c                 -fmul dword ptr [edx + 0x1c]
    x86::Float fpu56 = cpu.fpu.mul(fpu55, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 004245cf  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu57 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004245d2  d84a20                 -fmul dword ptr [edx + 0x20]
    x86::Float fpu58 = cpu.fpu.mul(fpu57, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 004245d5  d9ca                   -fxch st(2)
    
    // 004245d7  dec3                   -faddp st(3)
    x86::Float fpu59 = cpu.fpu.add(fpu50, fpu54);
    // 004245d9  dec3                   -faddp st(3)
    x86::Float fpu60 = cpu.fpu.add(fpu51, fpu56);
    // 004245db  dec3                   -faddp st(3)
    x86::Float fpu61 = cpu.fpu.add(fpu52, fpu58);
    // 004245dd  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004245e0  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fpu59);
    // 004245e2  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fpu60);
    // 004245e5  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fpu61);
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */)));
    // 004246f0  d80dd06e5300           -fmul dword ptr [0x536ed0]
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5467856) /* 0x536ed0 */)));
    // 004246f6  8d5dfc                 -lea ebx, [ebp - 4]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004246f9  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004246fc  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004246ff  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fpu2);
    }
    // 00424702  e869620c00             -call 0x4ea970
    cpu.esp -= 4;
    sub_4ea970(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424707  d945fc                 -fld dword ptr [ebp - 4]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu3 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 0042470a  d80dd46e5300           -fmul dword ptr [0x536ed4]
    x86::Float fpu4 = cpu.fpu.mul(fpu3, x86::Float(app->getMemory<float>(x86::reg32(5467860) /* 0x536ed4 */)));
    // 00424710  d845e8                 -fadd dword ptr [ebp - 0x18]
    x86::Float fpu5 = cpu.fpu.add(fpu4, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */)));
    // 00424713  d95914                 -fstp dword ptr [ecx + 0x14]
    app->getMemory<float>(cpu.ecx + x86::reg32(20) /* 0x14 */) = float(fpu5);
    // 00424716  d945f4                 -fld dword ptr [ebp - 0xc]
    x86::Float fpu6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 00424719  d80dd86e5300           -fmul dword ptr [0x536ed8]
    x86::Float fpu7 = cpu.fpu.mul(fpu6, x86::Float(app->getMemory<float>(x86::reg32(5467864) /* 0x536ed8 */)));
    // 0042471f  d845ec                 -fadd dword ptr [ebp - 0x14]
    x86::Float fpu8 = cpu.fpu.add(fpu7, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */)));
    // 00424722  d95918                 -fstp dword ptr [ecx + 0x18]
    app->getMemory<float>(cpu.ecx + x86::reg32(24) /* 0x18 */) = float(fpu8);
    // 00424725  d945f4                 -fld dword ptr [ebp - 0xc]
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 00424728  d80ddc6e5300           -fmul dword ptr [0x536edc]
    x86::Float fpu10 = cpu.fpu.mul(fpu9, x86::Float(app->getMemory<float>(x86::reg32(5467868) /* 0x536edc */)));
    // 0042472e  d845f0                 -fadd dword ptr [ebp - 0x10]
    x86::Float fpu11 = cpu.fpu.add(fpu10, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 00424731  d9591c                 -fstp dword ptr [ecx + 0x1c]
    app->getMemory<float>(cpu.ecx + x86::reg32(28) /* 0x1c */) = float(fpu11);
    }
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
    // 00424842  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00424845  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424848  8b1c85a0c47900         -mov ebx, dword ptr [eax*4 + 0x79c4a0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7980192) /* 0x79c4a0 */ + cpu.eax * 4);
    // 0042484f  d95de4                 -fstp dword ptr [ebp - 0x1c]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = cpu.fpu.st(0);
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fpu1);
    // 00424852  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    cpu.fpu.count -= 1;
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu2 = x86::Float(app->getMemory<float>(x86::reg32(5467872) /* 0x536ee0 */));
    // 0042486d  d98218010000           +fld dword ptr [edx + 0x118]
    x86::Float fpu3 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(280) /* 0x118 */));
    // 00424873  d8c9                   +fmul st(1)
    x86::Float fpu4 = cpu.fpu.mul(fpu3, x86::Float(fpu2));
    // 00424875  d845a4                 +fadd dword ptr [ebp - 0x5c]
    x86::Float fpu5 = cpu.fpu.add(fpu4, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-92) /* -0x5c */)));
    // 00424878  d95da4                 +fstp dword ptr [ebp - 0x5c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-92) /* -0x5c */) = float(fpu5);
    // 0042487b  d88a1c010000           +fmul dword ptr [edx + 0x11c]
    x86::Float fpu6 = cpu.fpu.mul(fpu2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(284) /* 0x11c */)));
    // 00424881  d86da8                 +fsubr dword ptr [ebp - 0x58]
    x86::Float fpu7 = cpu.fpu.sub(x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */)), fpu6);
    // 00424884  d9ee                   +fldz 
    x86::Float fpu8 = 0.0;
    // 00424886  d9c9                   +fxch st(1)
    
    // 00424888  d95da8                 +fstp dword ptr [ebp - 0x58]
    app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = float(fpu7);
    // 0042488b  d89aec040000           +fcomp dword ptr [edx + 0x4ec]
    cpu.fpu.compare(fpu8, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */)));
    // 00424891  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00424893  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00424894  760d                   -jbe 0x4248a3
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004248a3;
    }
    // 00424896  d982ec040000           +fld dword ptr [edx + 0x4ec]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */));
    // 0042489c  d9e0                   +fchs 
    x86::Float fpu10 = -fpu9;
    // 0042489e  d95dec                 +fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fpu10);
    }
    // 004248a1  eb09                   -jmp 0x4248ac
    goto L_0x004248ac;
L_0x004248a3:
    // 004248a3  8b82ec040000           -mov eax, dword ptr [edx + 0x4ec]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1260) /* 0x4ec */);
    // 004248a9  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
L_0x004248ac:
    // 004248ac  d9ee                   +fldz 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu11 = 0.0;
    // 004248ae  d89af4040000           +fcomp dword ptr [edx + 0x4f4]
    cpu.fpu.compare(fpu11, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */)));
    // 004248b4  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 004248b6  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 004248b7  760d                   -jbe 0x4248c6
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004248c6;
    }
    // 004248b9  d982f4040000           +fld dword ptr [edx + 0x4f4]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu12 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */));
    // 004248bf  d9e0                   +fchs 
    x86::Float fpu13 = -fpu12;
    // 004248c1  d95df0                 +fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fpu13);
    }
    // 004248c4  eb09                   -jmp 0x4248cf
    goto L_0x004248cf;
L_0x004248c6:
    // 004248c6  8b82f4040000           -mov eax, dword ptr [edx + 0x4f4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1268) /* 0x4f4 */);
    // 004248cc  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
L_0x004248cf:
    // 004248cf  d945ec                 +fld dword ptr [ebp - 0x14]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu14 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */));
    // 004248d2  d85df0                 +fcomp dword ptr [ebp - 0x10]
    cpu.fpu.compare(fpu14, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 004248d5  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 004248d7  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 004248d8  765c                   -jbe 0x424936
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00424936;
    }
    // 004248da  d9ee                   +fldz 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu15 = 0.0;
    // 004248dc  d89af4040000           +fcomp dword ptr [edx + 0x4f4]
    cpu.fpu.compare(fpu15, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */)));
    // 004248e2  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 004248e4  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 004248e5  760d                   -jbe 0x4248f4
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004248f4;
    }
    // 004248e7  d982f4040000           +fld dword ptr [edx + 0x4f4]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu16 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */));
    // 004248ed  d9e0                   +fchs 
    x86::Float fpu17 = -fpu16;
    // 004248ef  d95de0                 +fstp dword ptr [ebp - 0x20]
    app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = float(fpu17);
    }
    // 004248f2  eb09                   -jmp 0x4248fd
    goto L_0x004248fd;
L_0x004248f4:
    // 004248f4  8b82f4040000           -mov eax, dword ptr [edx + 0x4f4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1268) /* 0x4f4 */);
    // 004248fa  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
L_0x004248fd:
    // 004248fd  d945e0                 +fld dword ptr [ebp - 0x20]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu18 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */));
    // 00424900  d80de46e5300           +fmul dword ptr [0x536ee4]
    x86::Float fpu19 = cpu.fpu.mul(fpu18, x86::Float(app->getMemory<float>(x86::reg32(5467876) /* 0x536ee4 */)));
    // 00424906  d9ee                   +fldz 
    x86::Float fpu20 = 0.0;
    // 00424908  d9c9                   +fxch st(1)
    
    // 0042490a  d95dd0                 +fstp dword ptr [ebp - 0x30]
    app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = float(fpu19);
    // 0042490d  d89aec040000           +fcomp dword ptr [edx + 0x4ec]
    cpu.fpu.compare(fpu20, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */)));
    // 00424913  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00424915  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00424916  760d                   -jbe 0x424925
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00424925;
    }
    // 00424918  d982ec040000           +fld dword ptr [edx + 0x4ec]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu21 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */));
    // 0042491e  d9e0                   +fchs 
    x86::Float fpu22 = -fpu21;
    // 00424920  d95dd8                 +fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fpu22);
    }
    // 00424923  eb09                   -jmp 0x42492e
    goto L_0x0042492e;
L_0x00424925:
    // 00424925  8b82ec040000           -mov eax, dword ptr [edx + 0x4ec]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1260) /* 0x4ec */);
    // 0042492b  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
L_0x0042492e:
    // 0042492e  d945d8                 +fld dword ptr [ebp - 0x28]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu23 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */));
    // 00424931  d845d0                 +fadd dword ptr [ebp - 0x30]
    x86::Float fpu24 = cpu.fpu.add(fpu23, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */)));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu24;
    }
    // 00424934  eb5a                   -jmp 0x424990
    goto L_0x00424990;
L_0x00424936:
    // 00424936  d9ee                   +fldz 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu25 = 0.0;
    // 00424938  d89aec040000           +fcomp dword ptr [edx + 0x4ec]
    cpu.fpu.compare(fpu25, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */)));
    // 0042493e  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00424940  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00424941  760d                   -jbe 0x424950
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00424950;
    }
    // 00424943  d982ec040000           +fld dword ptr [edx + 0x4ec]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu26 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1260) /* 0x4ec */));
    // 00424949  d9e0                   +fchs 
    x86::Float fpu27 = -fpu26;
    // 0042494b  d95de8                 +fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fpu27);
    }
    // 0042494e  eb09                   -jmp 0x424959
    goto L_0x00424959;
L_0x00424950:
    // 00424950  8b82ec040000           -mov eax, dword ptr [edx + 0x4ec]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1260) /* 0x4ec */);
    // 00424956  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
L_0x00424959:
    // 00424959  d945e8                 +fld dword ptr [ebp - 0x18]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu28 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */));
    // 0042495c  d80de46e5300           +fmul dword ptr [0x536ee4]
    x86::Float fpu29 = cpu.fpu.mul(fpu28, x86::Float(app->getMemory<float>(x86::reg32(5467876) /* 0x536ee4 */)));
    // 00424962  d9ee                   +fldz 
    x86::Float fpu30 = 0.0;
    // 00424964  d9c9                   +fxch st(1)
    
    // 00424966  d95dd4                 +fstp dword ptr [ebp - 0x2c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = float(fpu29);
    // 00424969  d89af4040000           +fcomp dword ptr [edx + 0x4f4]
    cpu.fpu.compare(fpu30, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */)));
    // 0042496f  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00424971  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00424972  760d                   -jbe 0x424981
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00424981;
    }
    // 00424974  d982f4040000           +fld dword ptr [edx + 0x4f4]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu31 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(1268) /* 0x4f4 */));
    // 0042497a  d9e0                   +fchs 
    x86::Float fpu32 = -fpu31;
    // 0042497c  d95ddc                 +fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fpu32);
    }
    // 0042497f  eb09                   -jmp 0x42498a
    goto L_0x0042498a;
L_0x00424981:
    // 00424981  8b82f4040000           -mov eax, dword ptr [edx + 0x4f4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1268) /* 0x4f4 */);
    // 00424987  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
L_0x0042498a:
    // 0042498a  d945dc                 +fld dword ptr [ebp - 0x24]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu33 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */));
    // 0042498d  d845d4                 +fadd dword ptr [ebp - 0x2c]
    x86::Float fpu34 = cpu.fpu.add(fpu33, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */)));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu34;
    }
L_0x00424990:
    // 00424990  d95df4                 +fstp dword ptr [ebp - 0xc]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu35 = cpu.fpu.st(0);
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fpu35);
    // 00424993  d945f4                 +fld dword ptr [ebp - 0xc]
    x86::Float fpu36 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 00424996  d80de86e5300           +fmul dword ptr [0x536ee8]
    x86::Float fpu37 = cpu.fpu.mul(fpu36, x86::Float(app->getMemory<float>(x86::reg32(5467880) /* 0x536ee8 */)));
    // 0042499c  d955f8                 +fst dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fpu37);
    // 0042499f  d81dec6e5300           +fcomp dword ptr [0x536eec]
    cpu.fpu.compare(fpu37, x86::Float(app->getMemory<float>(x86::reg32(5467884) /* 0x536eec */)));
    // 004249a5  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 004249a7  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    cpu.fpu.count -= 1;
    }
    // 004249a8  7607                   -jbe 0x4249b1
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004249b1;
    }
    // 004249aa  c745f80000c040         -mov dword ptr [ebp - 8], 0x40c00000
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = 1086324736 /*0x40c00000*/;
L_0x004249b1:
    // 004249b1  d945a8                 -fld dword ptr [ebp - 0x58]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu38 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */));
    // 004249b4  d865f8                 -fsub dword ptr [ebp - 8]
    x86::Float fpu39 = cpu.fpu.sub(fpu38, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004249b7  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004249ba  d95da8                 -fstp dword ptr [ebp - 0x58]
    app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = float(fpu39);
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu40 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004249e8  d80a                   +fmul dword ptr [edx]
    x86::Float fpu41 = cpu.fpu.mul(fpu40, x86::Float(app->getMemory<float>(cpu.edx)));
    // 004249ea  d900                   +fld dword ptr [eax]
    x86::Float fpu42 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004249ec  d84a04                 +fmul dword ptr [edx + 4]
    x86::Float fpu43 = cpu.fpu.mul(fpu42, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 004249ef  d900                   +fld dword ptr [eax]
    x86::Float fpu44 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004249f1  d84a08                 +fmul dword ptr [edx + 8]
    x86::Float fpu45 = cpu.fpu.mul(fpu44, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004249f4  d9ca                   +fxch st(2)
    
    // 004249f6  d94004                 +fld dword ptr [eax + 4]
    x86::Float fpu46 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004249f9  d84a0c                 +fmul dword ptr [edx + 0xc]
    x86::Float fpu47 = cpu.fpu.mul(fpu46, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 004249fc  d94004                 +fld dword ptr [eax + 4]
    x86::Float fpu48 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004249ff  d84a10                 +fmul dword ptr [edx + 0x10]
    x86::Float fpu49 = cpu.fpu.mul(fpu48, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00424a02  d94004                 +fld dword ptr [eax + 4]
    x86::Float fpu50 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424a05  d84a14                 +fmul dword ptr [edx + 0x14]
    x86::Float fpu51 = cpu.fpu.mul(fpu50, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424a08  d9ca                   +fxch st(2)
    
    // 00424a0a  dec3                   +faddp st(3)
    x86::Float fpu52 = cpu.fpu.add(fpu41, fpu47);
    // 00424a0c  dec3                   +faddp st(3)
    x86::Float fpu53 = cpu.fpu.add(fpu43, fpu49);
    // 00424a0e  dec3                   +faddp st(3)
    x86::Float fpu54 = cpu.fpu.add(fpu45, fpu51);
    // 00424a10  d94008                 +fld dword ptr [eax + 8]
    x86::Float fpu55 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a13  d84a18                 +fmul dword ptr [edx + 0x18]
    x86::Float fpu56 = cpu.fpu.mul(fpu55, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424a16  d94008                 +fld dword ptr [eax + 8]
    x86::Float fpu57 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a19  d84a1c                 +fmul dword ptr [edx + 0x1c]
    x86::Float fpu58 = cpu.fpu.mul(fpu57, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424a1c  d94008                 +fld dword ptr [eax + 8]
    x86::Float fpu59 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a1f  d84a20                 +fmul dword ptr [edx + 0x20]
    x86::Float fpu60 = cpu.fpu.mul(fpu59, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00424a22  d9ca                   +fxch st(2)
    
    // 00424a24  dec3                   +faddp st(3)
    x86::Float fpu61 = cpu.fpu.add(fpu52, fpu56);
    // 00424a26  dec3                   +faddp st(3)
    x86::Float fpu62 = cpu.fpu.add(fpu53, fpu58);
    // 00424a28  dec3                   +faddp st(3)
    x86::Float fpu63 = cpu.fpu.add(fpu54, fpu60);
    // 00424a2a  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424a2d  d91b                   +fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fpu61);
    // 00424a2f  d95b04                 +fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fpu62);
    // 00424a32  d95b08                 +fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fpu63);
    // 00424a35  8d5db8                 -lea ebx, [ebp - 0x48]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 00424a38  8d55c4                 -lea edx, [ebp - 0x3c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00424a3b  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu64 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424a59  d80a                   -fmul dword ptr [edx]
    x86::Float fpu65 = cpu.fpu.mul(fpu64, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00424a5b  d900                   -fld dword ptr [eax]
    x86::Float fpu66 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424a5d  d84a04                 -fmul dword ptr [edx + 4]
    x86::Float fpu67 = cpu.fpu.mul(fpu66, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00424a60  d900                   -fld dword ptr [eax]
    x86::Float fpu68 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424a62  d84a08                 -fmul dword ptr [edx + 8]
    x86::Float fpu69 = cpu.fpu.mul(fpu68, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00424a65  d9ca                   -fxch st(2)
    
    // 00424a67  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu70 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424a6a  d84a0c                 -fmul dword ptr [edx + 0xc]
    x86::Float fpu71 = cpu.fpu.mul(fpu70, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00424a6d  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu72 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424a70  d84a10                 -fmul dword ptr [edx + 0x10]
    x86::Float fpu73 = cpu.fpu.mul(fpu72, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00424a73  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu74 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424a76  d84a14                 -fmul dword ptr [edx + 0x14]
    x86::Float fpu75 = cpu.fpu.mul(fpu74, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424a79  d9ca                   -fxch st(2)
    
    // 00424a7b  dec3                   -faddp st(3)
    x86::Float fpu76 = cpu.fpu.add(fpu65, fpu71);
    // 00424a7d  dec3                   -faddp st(3)
    x86::Float fpu77 = cpu.fpu.add(fpu67, fpu73);
    // 00424a7f  dec3                   -faddp st(3)
    x86::Float fpu78 = cpu.fpu.add(fpu69, fpu75);
    // 00424a81  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu79 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a84  d84a18                 -fmul dword ptr [edx + 0x18]
    x86::Float fpu80 = cpu.fpu.mul(fpu79, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424a87  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu81 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a8a  d84a1c                 -fmul dword ptr [edx + 0x1c]
    x86::Float fpu82 = cpu.fpu.mul(fpu81, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424a8d  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu83 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424a90  d84a20                 -fmul dword ptr [edx + 0x20]
    x86::Float fpu84 = cpu.fpu.mul(fpu83, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00424a93  d9ca                   -fxch st(2)
    
    // 00424a95  dec3                   -faddp st(3)
    x86::Float fpu85 = cpu.fpu.add(fpu76, fpu80);
    // 00424a97  dec3                   -faddp st(3)
    x86::Float fpu86 = cpu.fpu.add(fpu77, fpu82);
    // 00424a99  dec3                   -faddp st(3)
    x86::Float fpu87 = cpu.fpu.add(fpu78, fpu84);
    // 00424a9b  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424a9e  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fpu85);
    // 00424aa0  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fpu86);
    // 00424aa3  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fpu87);
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(284) /* 0x11c */));
    // 00424c11  d88728010000           -fadd dword ptr [edi + 0x128]
    x86::Float fpu2 = cpu.fpu.add(fpu1, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(296) /* 0x128 */)));
    // 00424c17  c7872401000000000000   -mov dword ptr [edi + 0x124], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(292) /* 0x124 */) = 0 /*0x0*/;
    // 00424c21  d99f28010000           -fstp dword ptr [edi + 0x128]
    app->getMemory<float>(cpu.edi + x86::reg32(296) /* 0x128 */) = float(fpu2);
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu3 = x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(272) /* 0x110 */));
    // 00424c3e  d84dfc                 -fmul dword ptr [ebp - 4]
    x86::Float fpu4 = cpu.fpu.mul(fpu3, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00424c41  d95dec                 -fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fpu4);
    // 00424c44  d98718010000           -fld dword ptr [edi + 0x118]
    x86::Float fpu5 = x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(280) /* 0x118 */));
    // 00424c4a  d84df8                 -fmul dword ptr [ebp - 8]
    x86::Float fpu6 = cpu.fpu.mul(fpu5, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00424c4d  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fpu6);
    // 00424c50  d98714010000           -fld dword ptr [edi + 0x114]
    x86::Float fpu7 = x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(276) /* 0x114 */));
    // 00424c56  d845f8                 -fadd dword ptr [ebp - 8]
    x86::Float fpu8 = cpu.fpu.add(fpu7, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00424c59  d95df0                 -fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fpu8);
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
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424c70  d80a                   -fmul dword ptr [edx]
    x86::Float fpu10 = cpu.fpu.mul(fpu9, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00424c72  d900                   -fld dword ptr [eax]
    x86::Float fpu11 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424c74  d84a04                 -fmul dword ptr [edx + 4]
    x86::Float fpu12 = cpu.fpu.mul(fpu11, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00424c77  d900                   -fld dword ptr [eax]
    x86::Float fpu13 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424c79  d84a08                 -fmul dword ptr [edx + 8]
    x86::Float fpu14 = cpu.fpu.mul(fpu13, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00424c7c  d9ca                   -fxch st(2)
    
    // 00424c7e  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu15 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424c81  d84a0c                 -fmul dword ptr [edx + 0xc]
    x86::Float fpu16 = cpu.fpu.mul(fpu15, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00424c84  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu17 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424c87  d84a10                 -fmul dword ptr [edx + 0x10]
    x86::Float fpu18 = cpu.fpu.mul(fpu17, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00424c8a  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu19 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424c8d  d84a14                 -fmul dword ptr [edx + 0x14]
    x86::Float fpu20 = cpu.fpu.mul(fpu19, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424c90  d9ca                   -fxch st(2)
    
    // 00424c92  dec3                   -faddp st(3)
    x86::Float fpu21 = cpu.fpu.add(fpu10, fpu16);
    // 00424c94  dec3                   -faddp st(3)
    x86::Float fpu22 = cpu.fpu.add(fpu12, fpu18);
    // 00424c96  dec3                   -faddp st(3)
    x86::Float fpu23 = cpu.fpu.add(fpu14, fpu20);
    // 00424c98  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu24 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424c9b  d84a18                 -fmul dword ptr [edx + 0x18]
    x86::Float fpu25 = cpu.fpu.mul(fpu24, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424c9e  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu26 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424ca1  d84a1c                 -fmul dword ptr [edx + 0x1c]
    x86::Float fpu27 = cpu.fpu.mul(fpu26, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424ca4  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu28 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424ca7  d84a20                 -fmul dword ptr [edx + 0x20]
    x86::Float fpu29 = cpu.fpu.mul(fpu28, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00424caa  d9ca                   -fxch st(2)
    
    // 00424cac  dec3                   -faddp st(3)
    x86::Float fpu30 = cpu.fpu.add(fpu21, fpu25);
    // 00424cae  dec3                   -faddp st(3)
    x86::Float fpu31 = cpu.fpu.add(fpu22, fpu27);
    // 00424cb0  dec3                   -faddp st(3)
    x86::Float fpu32 = cpu.fpu.add(fpu23, fpu29);
    // 00424cb2  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424cb5  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fpu30);
    // 00424cb7  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fpu31);
    // 00424cba  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fpu32);
    // 00424cbd  8d5de0                 -lea ebx, [ebp - 0x20]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00424cc0  8d55d4                 -lea edx, [ebp - 0x2c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 00424cc3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(288) /* 0x120 */));
    // 00424d41  d805f06e5300           +fadd dword ptr [0x536ef0]
    x86::Float fpu2 = cpu.fpu.add(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5467888) /* 0x536ef0 */)));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu2;
    }
    // 00424d47  eb20                   -jmp 0x424d69
    goto L_0x00424d69;
L_0x00424d49:
    // 00424d49  d98120010000           +fld dword ptr [ecx + 0x120]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu3 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(288) /* 0x120 */));
    // 00424d4f  d805f46e5300           +fadd dword ptr [0x536ef4]
    x86::Float fpu4 = cpu.fpu.add(fpu3, x86::Float(app->getMemory<float>(x86::reg32(5467892) /* 0x536ef4 */)));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu4;
    }
    // 00424d55  eb12                   -jmp 0x424d69
    goto L_0x00424d69;
L_0x00424d57:
    // 00424d57  db0520305500           -fild dword ptr [0x553020]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu5 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5582880) /* 0x553020 */)));
    // 00424d5d  d80df86e5300           -fmul dword ptr [0x536ef8]
    x86::Float fpu6 = cpu.fpu.mul(fpu5, x86::Float(app->getMemory<float>(x86::reg32(5467896) /* 0x536ef8 */)));
    // 00424d63  d88120010000           -fadd dword ptr [ecx + 0x120]
    x86::Float fpu7 = cpu.fpu.add(fpu6, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(288) /* 0x120 */)));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu7;
    }
L_0x00424d69:
    // 00424d69  d99920010000           -fstp dword ptr [ecx + 0x120]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu8 = cpu.fpu.st(0);
    app->getMemory<float>(cpu.ecx + x86::reg32(288) /* 0x120 */) = float(fpu8);
    cpu.fpu.count -= 1;
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(272) /* 0x110 */));
    // 00424d86  d84dfc                 -fmul dword ptr [ebp - 4]
    x86::Float fpu10 = cpu.fpu.mul(fpu9, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00424d89  d95de0                 -fstp dword ptr [ebp - 0x20]
    app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = float(fpu10);
    // 00424d8c  d98118010000           -fld dword ptr [ecx + 0x118]
    x86::Float fpu11 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(280) /* 0x118 */));
    // 00424d92  d84df8                 -fmul dword ptr [ebp - 8]
    x86::Float fpu12 = cpu.fpu.mul(fpu11, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00424d95  d95de8                 -fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fpu12);
    // 00424d98  d98114010000           -fld dword ptr [ecx + 0x114]
    x86::Float fpu13 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(276) /* 0x114 */));
    // 00424d9e  d845f8                 -fadd dword ptr [ebp - 8]
    x86::Float fpu14 = cpu.fpu.add(fpu13, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00424da1  d95de4                 -fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fpu14);
    // 00424da4  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00424da7  8d90c0000000           -lea edx, [eax + 0xc0]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(192) /* 0xc0 */);
    // 00424dad  8d5dec                 -lea ebx, [ebp - 0x14]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00424db0  8d45e0                 -lea eax, [ebp - 0x20]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00424db3  d900                   -fld dword ptr [eax]
    x86::Float fpu15 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424db5  d80a                   -fmul dword ptr [edx]
    x86::Float fpu16 = cpu.fpu.mul(fpu15, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00424db7  d900                   -fld dword ptr [eax]
    x86::Float fpu17 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424db9  d84a04                 -fmul dword ptr [edx + 4]
    x86::Float fpu18 = cpu.fpu.mul(fpu17, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00424dbc  d900                   -fld dword ptr [eax]
    x86::Float fpu19 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424dbe  d84a08                 -fmul dword ptr [edx + 8]
    x86::Float fpu20 = cpu.fpu.mul(fpu19, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00424dc1  d9ca                   -fxch st(2)
    
    // 00424dc3  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu21 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424dc6  d84a0c                 -fmul dword ptr [edx + 0xc]
    x86::Float fpu22 = cpu.fpu.mul(fpu21, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00424dc9  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu23 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424dcc  d84a10                 -fmul dword ptr [edx + 0x10]
    x86::Float fpu24 = cpu.fpu.mul(fpu23, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00424dcf  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu25 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424dd2  d84a14                 -fmul dword ptr [edx + 0x14]
    x86::Float fpu26 = cpu.fpu.mul(fpu25, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424dd5  d9ca                   -fxch st(2)
    
    // 00424dd7  dec3                   -faddp st(3)
    x86::Float fpu27 = cpu.fpu.add(fpu16, fpu22);
    // 00424dd9  dec3                   -faddp st(3)
    x86::Float fpu28 = cpu.fpu.add(fpu18, fpu24);
    // 00424ddb  dec3                   -faddp st(3)
    x86::Float fpu29 = cpu.fpu.add(fpu20, fpu26);
    // 00424ddd  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu30 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424de0  d84a18                 -fmul dword ptr [edx + 0x18]
    x86::Float fpu31 = cpu.fpu.mul(fpu30, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424de3  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu32 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424de6  d84a1c                 -fmul dword ptr [edx + 0x1c]
    x86::Float fpu33 = cpu.fpu.mul(fpu32, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424de9  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu34 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424dec  d84a20                 -fmul dword ptr [edx + 0x20]
    x86::Float fpu35 = cpu.fpu.mul(fpu34, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00424def  d9ca                   -fxch st(2)
    
    // 00424df1  dec3                   -faddp st(3)
    x86::Float fpu36 = cpu.fpu.add(fpu27, fpu31);
    // 00424df3  dec3                   -faddp st(3)
    x86::Float fpu37 = cpu.fpu.add(fpu28, fpu33);
    // 00424df5  dec3                   -faddp st(3)
    x86::Float fpu38 = cpu.fpu.add(fpu29, fpu35);
    // 00424df7  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424dfa  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fpu36);
    // 00424dfc  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fpu37);
    // 00424dff  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fpu38);
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
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1316) /* 0x524 */)));
    // 00424e48  d80dfc6e5300           -fmul dword ptr [0x536efc]
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<float>(x86::reg32(5467900) /* 0x536efc */)));
    // 00424e4e  d805006f5300           -fadd dword ptr [0x536f00]
    x86::Float fpu3 = cpu.fpu.add(fpu2, x86::Float(app->getMemory<float>(x86::reg32(5467904) /* 0x536f00 */)));
    // 00424e54  d99e20010000           -fstp dword ptr [esi + 0x120]
    app->getMemory<float>(cpu.esi + x86::reg32(288) /* 0x120 */) = float(fpu3);
    // 00424e5a  f6401c01               +test byte ptr [eax + 0x1c], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(28) /* 0x1c */) & 1 /*0x1*/));
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu4 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 00424e84  d84dfc                 -fmul dword ptr [ebp - 4]
    x86::Float fpu5 = cpu.fpu.mul(fpu4, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00424e87  d88910010000           -fmul dword ptr [ecx + 0x110]
    x86::Float fpu6 = cpu.fpu.mul(fpu5, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(272) /* 0x110 */)));
    // 00424e8d  d95ddc                 -fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fpu6);
    // 00424e90  d98118010000           -fld dword ptr [ecx + 0x118]
    x86::Float fpu7 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(280) /* 0x118 */));
    // 00424e96  d84df4                 -fmul dword ptr [ebp - 0xc]
    x86::Float fpu8 = cpu.fpu.mul(fpu7, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 00424e99  d95de4                 -fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fpu8);
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
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424eb6  d80a                   -fmul dword ptr [edx]
    x86::Float fpu10 = cpu.fpu.mul(fpu9, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00424eb8  d900                   -fld dword ptr [eax]
    x86::Float fpu11 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424eba  d84a04                 -fmul dword ptr [edx + 4]
    x86::Float fpu12 = cpu.fpu.mul(fpu11, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00424ebd  d900                   -fld dword ptr [eax]
    x86::Float fpu13 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00424ebf  d84a08                 -fmul dword ptr [edx + 8]
    x86::Float fpu14 = cpu.fpu.mul(fpu13, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00424ec2  d9ca                   -fxch st(2)
    
    // 00424ec4  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu15 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424ec7  d84a0c                 -fmul dword ptr [edx + 0xc]
    x86::Float fpu16 = cpu.fpu.mul(fpu15, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00424eca  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu17 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424ecd  d84a10                 -fmul dword ptr [edx + 0x10]
    x86::Float fpu18 = cpu.fpu.mul(fpu17, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00424ed0  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu19 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00424ed3  d84a14                 -fmul dword ptr [edx + 0x14]
    x86::Float fpu20 = cpu.fpu.mul(fpu19, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00424ed6  d9ca                   -fxch st(2)
    
    // 00424ed8  dec3                   -faddp st(3)
    x86::Float fpu21 = cpu.fpu.add(fpu10, fpu16);
    // 00424eda  dec3                   -faddp st(3)
    x86::Float fpu22 = cpu.fpu.add(fpu12, fpu18);
    // 00424edc  dec3                   -faddp st(3)
    x86::Float fpu23 = cpu.fpu.add(fpu14, fpu20);
    // 00424ede  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu24 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424ee1  d84a18                 -fmul dword ptr [edx + 0x18]
    x86::Float fpu25 = cpu.fpu.mul(fpu24, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00424ee4  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu26 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424ee7  d84a1c                 -fmul dword ptr [edx + 0x1c]
    x86::Float fpu27 = cpu.fpu.mul(fpu26, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00424eea  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu28 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00424eed  d84a20                 -fmul dword ptr [edx + 0x20]
    x86::Float fpu29 = cpu.fpu.mul(fpu28, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00424ef0  d9ca                   -fxch st(2)
    
    // 00424ef2  dec3                   -faddp st(3)
    x86::Float fpu30 = cpu.fpu.add(fpu21, fpu25);
    // 00424ef4  dec3                   -faddp st(3)
    x86::Float fpu31 = cpu.fpu.add(fpu22, fpu27);
    // 00424ef6  dec3                   -faddp st(3)
    x86::Float fpu32 = cpu.fpu.add(fpu23, fpu29);
    // 00424ef8  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00424efb  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fpu30);
    // 00424efd  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fpu31);
    // 00424f00  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fpu32);
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
    }
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
    sub_4b73c0(app, cpu.sync());
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
    sub_4b71e0(app, cpu.sync());
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
    sub_4b7590(app, cpu.sync());
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
    sub_4965b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00424ff3  d9ee                   +fldz 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = 0.0;
    // 00424ff5  ded9                   +fcompp 
    x86::Float fpu2 = cpu.fpu.st(0);
    cpu.fpu.compare(fpu1, fpu2);
    // 00424ff7  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00424ff9  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    cpu.fpu.count -= 1;
    }
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
    sub_4965b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042500b  d9e0                   +fchs 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu3 = cpu.fpu.st(0);
    x86::Float fpu4 = -fpu3;
    cpu.fpu.st(0) = fpu4;
    }
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
    sub_4965b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0042501e:
    // 0042501e  d95dfc                 +fstp dword ptr [ebp - 4]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu5 = cpu.fpu.st(0);
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fpu5);
    // 00425021  d945fc                 +fld dword ptr [ebp - 4]
    x86::Float fpu6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00425024  d85d18                 +fcomp dword ptr [ebp + 0x18]
    cpu.fpu.compare(fpu6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(24) /* 0x18 */)));
    // 00425027  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425029  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    cpu.fpu.count -= 1;
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu7 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00425038  d87518                 +fdiv dword ptr [ebp + 0x18]
    x86::Float fpu8 = cpu.fpu.div(fpu7, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(24) /* 0x18 */)));
    // 0042503b  d95b54                 +fstp dword ptr [ebx + 0x54]
    app->getMemory<float>(cpu.ebx + x86::reg32(84) /* 0x54 */) = float(fpu8);
    }
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

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_425060(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00425060  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00425061  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00425062  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00425063  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00425064  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00425065  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00425066  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00425068  83ec1c                 -sub esp, 0x1c
    (cpu.esp) -= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 0042506b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0042506d  8b15a8367d00           -mov edx, dword ptr [0x7d36a8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */);
    // 00425073  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00425075  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00425077  740d                   -je 0x425086
    if (cpu.flags.zf)
    {
        goto L_0x00425086;
    }
    // 00425079  833d60d97c0000         +cmp dword ptr [0x7cd960], 0
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
    // 00425080  0f8460010000           -je 0x4251e6
    if (cpu.flags.zf)
    {
        goto L_0x004251e6;
    }
L_0x00425086:
    // 00425086  833dec00560000         +cmp dword ptr [0x5600ec], 0
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
    // 0042508d  0f8553010000           -jne 0x4251e6
    if (!cpu.flags.zf)
    {
        goto L_0x004251e6;
    }
    // 00425093  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 00425096  8d780c                 -lea edi, [eax + 0xc]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00425099  8b410c                 -mov eax, dword ptr [ecx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 0042509c  e8cf8b0900             -call 0x4bdc70
    cpu.esp -= 4;
    sub_4bdc70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004250a1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004250a3  746a                   -je 0x42510f
    if (cpu.flags.zf)
    {
        goto L_0x0042510f;
    }
    // 004250a5  8b9184000000           -mov edx, dword ptr [ecx + 0x84]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(132) /* 0x84 */);
    // 004250ab  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 004250b2  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004250b4  a148d95d00             -mov eax, dword ptr [0x5dd948]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 004250b9  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004250bc  668b44020c             -mov ax, word ptr [edx + eax + 0xc]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(12) /* 0xc */ + cpu.eax * 1);
    // 004250c1  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 004250c6  e815efffff             -call 0x423fe0
    cpu.esp -= 4;
    nfs3hp::carstate::bitsBroken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004250cb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004250cd  7525                   -jne 0x4250f4
    if (!cpu.flags.zf)
    {
        goto L_0x004250f4;
    }
    // 004250cf  8b5710                 -mov edx, dword ptr [edi + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */);
    // 004250d2  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 004250d9  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004250db  8b1548d95d00           -mov edx, dword ptr [0x5dd948]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 004250e1  668b44820c             -mov ax, word ptr [edx + eax*4 + 0xc]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(12) /* 0xc */ + cpu.eax * 4);
    // 004250e6  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 004250eb  e8f0eeffff             -call 0x423fe0
    cpu.esp -= 4;
    nfs3hp::carstate::bitsBroken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004250f0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004250f2  741b                   -je 0x42510f
    if (cpu.flags.zf)
    {
        goto L_0x0042510f;
    }
L_0x004250f4:
    // 004250f4  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 004250f7  83b83806000007         +cmp dword ptr [eax + 0x638], 7
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1592) /* 0x638 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004250fe  7c07                   -jl 0x425107
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00425107;
    }
    // 00425100  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00425105  eb02                   -jmp 0x425109
    goto L_0x00425109;
L_0x00425107:
    // 00425107  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
L_0x00425109:
    // 00425109  898630010000           -mov dword ptr [esi + 0x130], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(304) /* 0x130 */) = cpu.eax;
L_0x0042510f:
    // 0042510f  8d55e4                 -lea edx, [ebp - 0x1c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00425112  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00425114  8b9e30010000           -mov ebx, dword ptr [esi + 0x130]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(304) /* 0x130 */);
    // 0042511a  e8b1aeffff             -call 0x41ffd0
    cpu.esp -= 4;
    nfs3hp::trackdata::lanePosition(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042511f  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00425121  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00425124  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00425127  db45fc                 +fild dword ptr [ebp - 4]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0042512a  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0042512d  dd05046f5300           +fld qword ptr [0x536f04]
    x86::Float fpu2 = x86::Float(app->getMemory<double>(x86::reg32(5467908) /* 0x536f04 */));
    // 00425133  d9c9                   +fxch st(1)
    
    // 00425135  d8c9                   +fmul st(1)
    x86::Float fpu3 = cpu.fpu.mul(fpu1, x86::Float(fpu2));
    // 00425137  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0042513a  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0042513d  db45fc                 +fild dword ptr [ebp - 4]
    x86::Float fpu4 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00425140  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00425143  d8ca                   +fmul st(2)
    x86::Float fpu5 = cpu.fpu.mul(fpu4, x86::Float(fpu2));
    // 00425145  db45fc                 +fild dword ptr [ebp - 4]
    x86::Float fpu6 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00425148  decb                   +fmulp st(3)
    x86::Float fpu7 = cpu.fpu.mul(fpu2, fpu6);
    // 0042514a  d9c9                   +fxch st(1)
    
    // 0042514c  d95df0                 +fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fpu3);
    // 0042514f  d95df4                 +fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fpu5);
    // 00425152  d95df8                 +fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fpu7);
    // 00425155  d94114                 +fld dword ptr [ecx + 0x14]
    x86::Float fpu8 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(20) /* 0x14 */));
    // 00425158  d85df0                 +fcomp dword ptr [ebp - 0x10]
    cpu.fpu.compare(fpu8, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 0042515b  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 0042515d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 0042515e  7516                   -jne 0x425176
    if (!cpu.flags.zf)
    {
        goto L_0x00425176;
    }
    // 00425160  d94118                 +fld dword ptr [ecx + 0x18]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(24) /* 0x18 */));
    // 00425163  d85df4                 +fcomp dword ptr [ebp - 0xc]
    cpu.fpu.compare(fpu9, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 00425166  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425168  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425169  750b                   -jne 0x425176
    if (!cpu.flags.zf)
    {
        goto L_0x00425176;
    }
    // 0042516b  d9411c                 +fld dword ptr [ecx + 0x1c]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu10 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(28) /* 0x1c */));
    // 0042516e  d85df8                 +fcomp dword ptr [ebp - 8]
    cpu.fpu.compare(fpu10, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00425171  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425173  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425174  7449                   -je 0x4251bf
    if (cpu.flags.zf)
    {
        goto L_0x004251bf;
    }
L_0x00425176:
    // 00425176  8b3d24305500           -mov edi, dword ptr [0x553024]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5582884) /* 0x553024 */);
    // 0042517c  47                     -inc edi
    (cpu.edi)++;
    // 0042517d  c7862401000000000000   -mov dword ptr [esi + 0x124], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(292) /* 0x124 */) = 0 /*0x0*/;
    // 00425187  893d24305500           -mov dword ptr [0x553024], edi
    app->getMemory<x86::reg32>(x86::reg32(5582884) /* 0x553024 */) = cpu.edi;
    // 0042518d  83ff03                 +cmp edi, 3
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
    // 00425190  7c1b                   -jl 0x4251ad
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004251ad;
    }
    // 00425192  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00425194  c7862c01000000000000   -mov dword ptr [esi + 0x12c], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(300) /* 0x12c */) = 0 /*0x0*/;
    // 0042519e  891524305500           -mov dword ptr [0x553024], edx
    app->getMemory<x86::reg32>(x86::reg32(5582884) /* 0x553024 */) = cpu.edx;
    // 004251a4  c7415400000000         -mov dword ptr [ecx + 0x54], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(84) /* 0x54 */) = 0 /*0x0*/;
    // 004251ab  eb0a                   -jmp 0x4251b7
    goto L_0x004251b7;
L_0x004251ad:
    // 004251ad  c7862c01000001000000   -mov dword ptr [esi + 0x12c], 1
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(300) /* 0x12c */) = 1 /*0x1*/;
L_0x004251b7:
    // 004251b7  8b410c                 -mov eax, dword ptr [ecx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 004251ba  e8118a0900             -call 0x4bdbd0
    cpu.esp -= 4;
    sub_4bdbd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004251bf:
    // 004251bf  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004251c2  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004251c4  ffb610010000           -push dword ptr [esi + 0x110]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(272) /* 0x110 */);
    cpu.esp -= 4;
    // 004251ca  e801feffff             -call 0x424fd0
    cpu.esp -= 4;
    sub_424fd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004251cf  8d5114                 -lea edx, [ecx + 0x14]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(20) /* 0x14 */);
    // 004251d2  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004251d4  8999ec000000           -mov dword ptr [ecx + 0xec], ebx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(236) /* 0xec */) = cpu.ebx;
    // 004251da  e8b1d0ffff             -call 0x422290
    cpu.esp -= 4;
    nfs3hp::carstate::lookAtTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004251df  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004251e1  e84afdffff             -call 0x424f30
    cpu.esp -= 4;
    sub_424f30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004251e6:
    // 004251e6  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004251e8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004251e9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004251ea  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004251eb  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004251ec  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004251ed  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004251ee  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4251f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004251f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004251f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004251f2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004251f3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004251f4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004251f5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004251f6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004251f8  83ec30                 -sub esp, 0x30
    (cpu.esp) -= x86::reg32(x86::sreg32(48 /*0x30*/));
    // 004251fb  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004251fe  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00425201  e88aebffff             -call 0x423d90
    cpu.esp -= 4;
    nfs3hp::carstate::nearestSpot(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425206  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00425208  83f8ff                 +cmp eax, -1
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
    // 0042520b  7525                   -jne 0x425232
    if (!cpu.flags.zf)
    {
        goto L_0x00425232;
    }
    // 0042520d  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00425210  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00425215  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00425218  e893e6ffff             -call 0x4238b0
    cpu.esp -= 4;
    nfs3hp::carstate::setCameraMode(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042521d  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00425220  8b500c                 -mov edx, dword ptr [eax + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00425223  b808000000             -mov eax, 8
    cpu.eax = 8 /*0x8*/;
    // 00425228  e823120000             -call 0x426450
    cpu.esp -= 4;
    sub_426450(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042522d  e986010000             -jmp 0x4253b8
    goto L_0x004253b8;
L_0x00425232:
    // 00425232  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00425235  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00425237  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0042523a  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0042523c  8b15502e5500           -mov edx, dword ptr [0x552e50]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5582416) /* 0x552e50 */);
    // 00425242  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00425244  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00425247  8b5018                 -mov edx, dword ptr [eax + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 0042524a  8991ec000000           -mov dword ptr [ecx + 0xec], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(236) /* 0xec */) = cpu.edx;
    // 00425250  dd050c6f5300           -fld qword ptr [0x536f0c]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<double>(x86::reg32(5467916) /* 0x536f0c */));
    // 00425256  db00                   -fild dword ptr [eax]
    x86::Float fpu2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax)));
    // 00425258  d8c9                   -fmul st(1)
    x86::Float fpu3 = cpu.fpu.mul(fpu2, x86::Float(fpu1));
    // 0042525a  d95914                 -fstp dword ptr [ecx + 0x14]
    app->getMemory<float>(cpu.ecx + x86::reg32(20) /* 0x14 */) = float(fpu3);
    // 0042525d  db4004                 -fild dword ptr [eax + 4]
    x86::Float fpu4 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */)));
    // 00425260  d8c9                   -fmul st(1)
    x86::Float fpu5 = cpu.fpu.mul(fpu4, x86::Float(fpu1));
    // 00425262  d95918                 -fstp dword ptr [ecx + 0x18]
    app->getMemory<float>(cpu.ecx + x86::reg32(24) /* 0x18 */) = float(fpu5);
    // 00425265  db4008                 -fild dword ptr [eax + 8]
    x86::Float fpu6 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 00425268  dec9                   -fmulp st(1)
    x86::Float fpu7 = cpu.fpu.mul(fpu1, fpu6);
    // 0042526a  d9591c                 -fstp dword ptr [ecx + 0x1c]
    app->getMemory<float>(cpu.ecx + x86::reg32(28) /* 0x1c */) = float(fpu7);
    // 0042526d  8a5016                 -mov dl, byte ptr [eax + 0x16]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(22) /* 0x16 */);
    // 00425270  f6c201                 +test dl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 1 /*0x1*/));
    }
    // 00425273  742d                   -je 0x4252a2
    if (cpu.flags.zf)
    {
        goto L_0x004252a2;
    }
    // 00425275  be98445600             -mov esi, 0x564498
    cpu.esi = 5653656 /*0x564498*/;
    // 0042527a  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0042527d  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0042527f  894108                 -mov dword ptr [ecx + 8], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 00425282  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00425284  c7810801000001000000   -mov dword ptr [ecx + 0x108], 1
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(264) /* 0x108 */) = 1 /*0x1*/;
    // 0042528e  b909000000             -mov ecx, 9
    cpu.ecx = 9 /*0x9*/;
    // 00425293  8d782c                 -lea edi, [eax + 0x2c]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(44) /* 0x2c */);
    // 00425296  83c214                 +add edx, 0x14
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00425299  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0042529b  e8f0cfffff             -call 0x422290
    cpu.esp -= 4;
    nfs3hp::carstate::lookAtTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004252a0  eb74                   -jmp 0x425316
    goto L_0x00425316;
L_0x004252a2:
    // 004252a2  c7810801000000000000   -mov dword ptr [ecx + 0x108], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(264) /* 0x108 */) = 0 /*0x0*/;
    // 004252ac  8b500a                 -mov edx, dword ptr [eax + 0xa]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(10) /* 0xa */);
    // 004252af  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004252b2  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004252b5  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 004252b8  dd050c6f5300           -fld qword ptr [0x536f0c]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu8 = x86::Float(app->getMemory<double>(x86::reg32(5467916) /* 0x536f0c */));
    // 004252be  db45f8                 -fild dword ptr [ebp - 8]
    x86::Float fpu9 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004252c1  d8c9                   -fmul st(1)
    x86::Float fpu10 = cpu.fpu.mul(fpu9, x86::Float(fpu8));
    // 004252c3  d95dd0                 -fstp dword ptr [ebp - 0x30]
    app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = float(fpu10);
    // 004252c6  8b500c                 -mov edx, dword ptr [eax + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004252c9  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004252cc  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004252cf  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 004252d2  db45f8                 -fild dword ptr [ebp - 8]
    x86::Float fpu11 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004252d5  d8c9                   -fmul st(1)
    x86::Float fpu12 = cpu.fpu.mul(fpu11, x86::Float(fpu8));
    // 004252d7  d95dd4                 -fstp dword ptr [ebp - 0x2c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = float(fpu12);
    // 004252da  8b500e                 -mov edx, dword ptr [eax + 0xe]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(14) /* 0xe */);
    // 004252dd  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004252e0  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004252e3  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 004252e6  db45f8                 -fild dword ptr [ebp - 8]
    x86::Float fpu13 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004252e9  d8c9                   -fmul st(1)
    x86::Float fpu14 = cpu.fpu.mul(fpu13, x86::Float(fpu8));
    // 004252eb  d95dd8                 -fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fpu14);
    // 004252ee  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004252f1  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004252f4  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004252f7  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004252fa  83c12c                 -add ecx, 0x2c
    (cpu.ecx) += x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 004252fd  db45f8                 -fild dword ptr [ebp - 8]
    x86::Float fpu15 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00425300  dec9                   -fmulp st(1)
    x86::Float fpu16 = cpu.fpu.mul(fpu8, fpu15);
    // 00425302  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00425304  8d45d0                 -lea eax, [ebp - 0x30]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 00425307  d95ddc                 -fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fpu16);
    }
    // 0042530a  e881220900             -call 0x4b7590
    cpu.esp -= 4;
    sub_4b7590(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042530f  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00425311  e8aa260900             -call 0x4b79c0
    cpu.esp -= 4;
    sub_4b79c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00425316:
    // 00425316  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 0042531d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0042531f  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00425322  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00425324  8b15502e5500           -mov edx, dword ptr [0x552e50]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5582416) /* 0x552e50 */);
    // 0042532a  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0042532c  f6401602               +test byte ptr [eax + 0x16], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(22) /* 0x16 */) & 2 /*0x2*/));
    // 00425330  741c                   -je 0x42534e
    if (cpu.flags.zf)
    {
        goto L_0x0042534e;
    }
    // 00425332  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00425335  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00425338  83c214                 -add edx, 0x14
    (cpu.edx) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0042533b  680000c040             -push 0x40c00000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1086324736 /*0x40c00000*/;
    cpu.esp -= 4;
    // 00425340  e88bfcffff             -call 0x424fd0
    cpu.esp -= 4;
    sub_424fd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425345  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00425347  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425348  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425349  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042534a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042534b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042534c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042534d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0042534e:
    // 0042534e  8b5012                 -mov edx, dword ptr [eax + 0x12]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(18) /* 0x12 */);
    // 00425351  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00425354  c1e209                 -shl edx, 9
    cpu.edx <<= 9 /*0x9*/ % 32;
    // 00425357  b968010000             -mov ecx, 0x168
    cpu.ecx = 360 /*0x168*/;
    // 0042535c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0042535e  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00425361  f7f9                   +idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00425363  8d5de4                 -lea ebx, [ebp - 0x1c]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00425366  8d55e0                 -lea edx, [ebp - 0x20]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00425369  e8f2580c00             -call 0x4eac60
    cpu.esp -= 4;
    sub_4eac60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042536e  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00425371  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00425374  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00425377  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0042537a  db45f8                 +fild dword ptr [ebp - 8]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu17 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0042537d  db45fc                 +fild dword ptr [ebp - 4]
    x86::Float fpu18 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00425380  def9                   +fdivp st(1)
    x86::Float fpu19 = cpu.fpu.div(fpu17, fpu18);
    // 00425382  d955f0                 +fst dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fpu19);
    // 00425385  d81d146f5300           +fcomp dword ptr [0x536f14]
    cpu.fpu.compare(fpu19, x86::Float(app->getMemory<float>(x86::reg32(5467924) /* 0x536f14 */)));
    // 0042538b  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 0042538d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 0042538e  7309                   -jae 0x425399
    if (!cpu.flags.cf)
    {
        goto L_0x00425399;
    }
    // 00425390  c745ec3333333f         -mov dword ptr [ebp - 0x14], 0x3f333333
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = 1060320051 /*0x3f333333*/;
    // 00425397  eb16                   -jmp 0x4253af
    goto L_0x004253af;
L_0x00425399:
    // 00425399  d945f0                 -fld dword ptr [ebp - 0x10]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu20 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */));
    // 0042539c  d9c0                   -fld st(0)
    x86::Float fpu21 = x86::Float(fpu20);
    // 0042539e  ded9                   -fcompp 
    cpu.fpu.compare(fpu21, fpu20);
    // 004253a0  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 004253a2  9e                     -sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004253a3  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004253a6  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 004253a9  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004253ac  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    }
L_0x004253af:
    // 004253af  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004253b2  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004253b5  894254                 -mov dword ptr [edx + 0x54], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(84) /* 0x54 */) = cpu.eax;
L_0x004253b8:
    // 004253b8  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004253ba  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004253bb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004253bc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004253bd  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004253be  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004253bf  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004253c0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4253d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004253d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004253d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004253d2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004253d3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004253d4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004253d5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004253d6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004253d8  81ec84000000           -sub esp, 0x84
    (cpu.esp) -= x86::reg32(x86::sreg32(132 /*0x84*/));
    // 004253de  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 004253e4  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004253e7  8b9010010000           -mov edx, dword ptr [eax + 0x110]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(272) /* 0x110 */);
    // 004253ed  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004253ef  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004253f1  7553                   -jne 0x425446
    if (!cpu.flags.zf)
    {
        goto L_0x00425446;
    }
    // 004253f3  8b8014010000           -mov eax, dword ptr [eax + 0x114]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(276) /* 0x114 */);
    // 004253f9  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004253fb  e8b09ffeff             -call 0x40f3b0
    cpu.esp -= 4;
    nfs3hp::aiplan::startAnimation(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425400  898218010000           -mov dword ptr [edx + 0x118], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(280) /* 0x118 */) = cpu.eax;
    // 00425406  83f8ff                 +cmp eax, -1
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
    // 00425409  7531                   -jne 0x42543c
    if (!cpu.flags.zf)
    {
        goto L_0x0042543c;
    }
    // 0042540b  8b420c                 -mov eax, dword ptr [edx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    // 0042540e  8b0485b8bb6f00         -mov eax, dword ptr [eax*4 + 0x6fbbb8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7322552) /* 0x6fbbb8 */ + cpu.eax * 4);
    // 00425415  e896e6ffff             -call 0x423ab0
    cpu.esp -= 4;
    nfs3hp::carstate::modeInCycle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042541a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0042541c  740c                   -je 0x42542a
    if (cpu.flags.zf)
    {
        goto L_0x0042542a;
    }
    // 0042541e  8b520c                 -mov edx, dword ptr [edx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    // 00425421  8b0495b8bb6f00         -mov eax, dword ptr [edx*4 + 0x6fbbb8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7322552) /* 0x6fbbb8 */ + cpu.edx * 4);
    // 00425428  eb08                   -jmp 0x425432
    goto L_0x00425432;
L_0x0042542a:
    // 0042542a  a154105e00             -mov eax, dword ptr [0x5e1054]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6164564) /* 0x5e1054 */);
    // 0042542f  8b520c                 -mov edx, dword ptr [edx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
L_0x00425432:
    // 00425432  e8d9d1ffff             -call 0x422610
    cpu.esp -= 4;
    nfs3hp::carstate::switchCamera(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425437  e9f6010000             -jmp 0x425632
    goto L_0x00425632;
L_0x0042543c:
    // 0042543c  c7821001000001000000   -mov dword ptr [edx + 0x110], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(272) /* 0x110 */) = 1 /*0x1*/;
L_0x00425446:
    // 00425446  83b918010000ff         +cmp dword ptr [ecx + 0x118], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(280) /* 0x118 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0042544d  7511                   -jne 0x425460
    if (!cpu.flags.zf)
    {
        goto L_0x00425460;
    }
    // 0042544f  8b8114010000           -mov eax, dword ptr [ecx + 0x114]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(276) /* 0x114 */);
    // 00425455  e8569ffeff             -call 0x40f3b0
    cpu.esp -= 4;
    nfs3hp::aiplan::startAnimation(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042545a  898118010000           -mov dword ptr [ecx + 0x118], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(280) /* 0x118 */) = cpu.eax;
L_0x00425460:
    // 00425460  8bb118010000           -mov esi, dword ptr [ecx + 0x118]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(280) /* 0x118 */);
    // 00425466  83feff                 +cmp esi, -1
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
    // 00425469  750d                   -jne 0x425478
    if (!cpu.flags.zf)
    {
        goto L_0x00425478;
    }
    // 0042546b  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 00425471  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425472  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425473  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425474  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425475  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425476  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425477  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00425478:
    // 00425478  8d5d22                 -lea ebx, [ebp + 0x22]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(34) /* 0x22 */);
    // 0042547b  8d5552                 -lea edx, [ebp + 0x52]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 0042547e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00425480  e8bba4feff             -call 0x40f940
    cpu.esp -= 4;
    nfs3hp::aiplan::animationStep(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425485  8b8118010000           -mov eax, dword ptr [ecx + 0x118]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(280) /* 0x118 */);
    // 0042548b  e830a6feff             -call 0x40fac0
    cpu.esp -= 4;
    nfs3hp::aiplan::animationPlaying(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425490  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00425492  7557                   -jne 0x4254eb
    if (!cpu.flags.zf)
    {
        goto L_0x004254eb;
    }
    // 00425494  8b8118010000           -mov eax, dword ptr [ecx + 0x118]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(280) /* 0x118 */);
    // 0042549a  e8c19ffeff             -call 0x40f460
    cpu.esp -= 4;
    nfs3hp::aiplan::stopAnimation(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042549f  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004254a2  c78118010000ffffffff   -mov dword ptr [ecx + 0x118], 0xffffffff
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(280) /* 0x118 */) = 4294967295 /*0xffffffff*/;
    // 004254ac  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004254af  8b0485b8bb6f00         -mov eax, dword ptr [eax*4 + 0x6fbbb8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7322552) /* 0x6fbbb8 */ + cpu.eax * 4);
    // 004254b6  e8f5e5ffff             -call 0x423ab0
    cpu.esp -= 4;
    nfs3hp::carstate::modeInCycle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004254bb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004254bd  740f                   -je 0x4254ce
    if (cpu.flags.zf)
    {
        goto L_0x004254ce;
    }
    // 004254bf  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004254c2  8b500c                 -mov edx, dword ptr [eax + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004254c5  8b0495b8bb6f00         -mov eax, dword ptr [edx*4 + 0x6fbbb8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7322552) /* 0x6fbbb8 */ + cpu.edx * 4);
    // 004254cc  eb0b                   -jmp 0x4254d9
    goto L_0x004254d9;
L_0x004254ce:
    // 004254ce  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004254d1  8b500c                 -mov edx, dword ptr [eax + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004254d4  a154105e00             -mov eax, dword ptr [0x5e1054]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6164564) /* 0x5e1054 */);
L_0x004254d9:
    // 004254d9  e832d1ffff             -call 0x422610
    cpu.esp -= 4;
    nfs3hp::carstate::switchCamera(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004254de  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004254e4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004254e5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004254e6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004254e7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004254e8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004254e9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004254ea  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004254eb:
    // 004254eb  8bb914010000           -mov edi, dword ptr [ecx + 0x114]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(276) /* 0x114 */);
    // 004254f1  83ff05                 +cmp edi, 5
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004254f4  7409                   -je 0x4254ff
    if (cpu.flags.zf)
    {
        goto L_0x004254ff;
    }
    // 004254f6  83ff04                 +cmp edi, 4
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
    // 004254f9  0f85c1000000           -jne 0x4255c0
    if (!cpu.flags.zf)
    {
        goto L_0x004255c0;
    }
L_0x004254ff:
    // 004254ff  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00425502  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00425505  8d9024010000           -lea edx, [eax + 0x124]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(292) /* 0x124 */);
    // 0042550b  8d5dfe                 -lea ebx, [ebp - 2]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-2) /* -0x2 */);
    // 0042550e  8d4522                 -lea eax, [ebp + 0x22]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(34) /* 0x22 */);
    // 00425511  e86aad0b00             -call 0x4e0280
    cpu.esp -= 4;
    nfs3hp::vertices::multiplyMatrices(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425516  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00425519  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0042551c  8d7d6a                 -lea edi, [ebp + 0x6a]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 0042551f  8d9024010000           -lea edx, [eax + 0x124]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(292) /* 0x124 */);
    // 00425525  8d5d46                 -lea ebx, [ebp + 0x46]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 00425528  8d4552                 -lea eax, [ebp + 0x52]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 0042552b  8d7546                 -lea esi, [ebp + 0x46]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 0042552e  d900                   +fld dword ptr [eax]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425530  d80a                   +fmul dword ptr [edx]
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00425532  d900                   +fld dword ptr [eax]
    x86::Float fpu3 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425534  d84a04                 +fmul dword ptr [edx + 4]
    x86::Float fpu4 = cpu.fpu.mul(fpu3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00425537  d900                   +fld dword ptr [eax]
    x86::Float fpu5 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425539  d84a08                 +fmul dword ptr [edx + 8]
    x86::Float fpu6 = cpu.fpu.mul(fpu5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 0042553c  d9ca                   +fxch st(2)
    
    // 0042553e  d94004                 +fld dword ptr [eax + 4]
    x86::Float fpu7 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425541  d84a0c                 +fmul dword ptr [edx + 0xc]
    x86::Float fpu8 = cpu.fpu.mul(fpu7, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00425544  d94004                 +fld dword ptr [eax + 4]
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425547  d84a10                 +fmul dword ptr [edx + 0x10]
    x86::Float fpu10 = cpu.fpu.mul(fpu9, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 0042554a  d94004                 +fld dword ptr [eax + 4]
    x86::Float fpu11 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 0042554d  d84a14                 +fmul dword ptr [edx + 0x14]
    x86::Float fpu12 = cpu.fpu.mul(fpu11, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00425550  d9ca                   +fxch st(2)
    
    // 00425552  dec3                   +faddp st(3)
    x86::Float fpu13 = cpu.fpu.add(fpu2, fpu8);
    // 00425554  dec3                   +faddp st(3)
    x86::Float fpu14 = cpu.fpu.add(fpu4, fpu10);
    // 00425556  dec3                   +faddp st(3)
    x86::Float fpu15 = cpu.fpu.add(fpu6, fpu12);
    // 00425558  d94008                 +fld dword ptr [eax + 8]
    x86::Float fpu16 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 0042555b  d84a18                 +fmul dword ptr [edx + 0x18]
    x86::Float fpu17 = cpu.fpu.mul(fpu16, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 0042555e  d94008                 +fld dword ptr [eax + 8]
    x86::Float fpu18 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425561  d84a1c                 +fmul dword ptr [edx + 0x1c]
    x86::Float fpu19 = cpu.fpu.mul(fpu18, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00425564  d94008                 +fld dword ptr [eax + 8]
    x86::Float fpu20 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425567  d84a20                 +fmul dword ptr [edx + 0x20]
    x86::Float fpu21 = cpu.fpu.mul(fpu20, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 0042556a  d9ca                   +fxch st(2)
    
    // 0042556c  dec3                   +faddp st(3)
    x86::Float fpu22 = cpu.fpu.add(fpu13, fpu17);
    // 0042556e  dec3                   +faddp st(3)
    x86::Float fpu23 = cpu.fpu.add(fpu14, fpu19);
    // 00425570  dec3                   +faddp st(3)
    x86::Float fpu24 = cpu.fpu.add(fpu15, fpu21);
    // 00425572  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00425575  d91b                   +fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fpu22);
    // 00425577  d95b04                 +fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fpu23);
    // 0042557a  d95b08                 +fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fpu24);
    // 0042557d  8d456a                 -lea eax, [ebp + 0x6a]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 00425580  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00425581  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00425582  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    }
    // 00425583  e8681d0700             -call 0x4972f0
    cpu.esp -= 4;
    nfs3hp::vertices::normalise(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425588  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0042558b  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0042558e  d9e8                   +fld1 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu25 = 1.0;
    // 00425590  d88020010000           +fadd dword ptr [eax + 0x120]
    x86::Float fpu26 = cpu.fpu.add(fpu25, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(288) /* 0x120 */)));
    // 00425596  d95d7a                 +fstp dword ptr [ebp + 0x7a]
    app->getMemory<float>(cpu.ebp + x86::reg32(122) /* 0x7a */) = float(fpu26);
    // 00425599  d85d7a                 +fcomp dword ptr [ebp + 0x7a]
    x86::Float fpu27 = cpu.fpu.st(0);
    cpu.fpu.compare(fpu27, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(122) /* 0x7a */)));
    // 0042559c  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 0042559e  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    cpu.fpu.count -= 1;
    }
    // 0042559f  7335                   -jae 0x4255d6
    if (!cpu.flags.cf)
    {
        goto L_0x004255d6;
    }
    // 004255a1  8d456a                 -lea eax, [ebp + 0x6a]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 004255a4  8d556a                 -lea edx, [ebp + 0x6a]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 004255a7  8d7d46                 -lea edi, [ebp + 0x46]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 004255aa  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004255ab  8d756a                 -lea esi, [ebp + 0x6a]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 004255ae  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004255b3  ff757a                 -push dword ptr [ebp + 0x7a]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    cpu.esp -= 4;
    // 004255b6  e895aa0b00             -call 0x4e0050
    cpu.esp -= 4;
    nfs3hp::vertices::scaleVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004255bb  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004255bc  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004255bd  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004255be  eb16                   -jmp 0x4255d6
    goto L_0x004255d6;
L_0x004255c0:
    // 004255c0  b909000000             -mov ecx, 9
    cpu.ecx = 9 /*0x9*/;
    // 004255c5  8d7dfe                 -lea edi, [ebp - 2]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-2) /* -0x2 */);
    // 004255c8  8d7522                 -lea esi, [ebp + 0x22]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(34) /* 0x22 */);
    // 004255cb  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004255cd  8d7d46                 -lea edi, [ebp + 0x46]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 004255d0  8d7552                 -lea esi, [ebp + 0x52]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 004255d3  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004255d4  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004255d5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x004255d6:
    // 004255d6  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004255d9  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004255dc  8b8098000000           -mov eax, dword ptr [eax + 0x98]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(152) /* 0x98 */);
    // 004255e2  89455e                 -mov dword ptr [ebp + 0x5e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */) = cpu.eax;
    // 004255e5  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004255e8  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004255eb  8b809c000000           -mov eax, dword ptr [eax + 0x9c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(156) /* 0x9c */);
    // 004255f1  894562                 -mov dword ptr [ebp + 0x62], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */) = cpu.eax;
    // 004255f4  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004255f7  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004255fa  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004255fd  8d5d46                 -lea ebx, [ebp + 0x46]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 00425600  8d555e                 -lea edx, [ebp + 0x5e]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 00425603  8b80a0000000           -mov eax, dword ptr [eax + 0xa0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(160) /* 0xa0 */);
    // 00425609  83c114                 -add ecx, 0x14
    (cpu.ecx) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0042560c  894566                 -mov dword ptr [ebp + 0x66], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */) = cpu.eax;
    // 0042560f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00425614  e897a90b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425619  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0042561c  8d7546                 -lea esi, [ebp + 0x46]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 0042561f  8d7820                 -lea edi, [eax + 0x20]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(32) /* 0x20 */);
    // 00425622  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00425623  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00425624  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00425625  b909000000             -mov ecx, 9
    cpu.ecx = 9 /*0x9*/;
    // 0042562a  8d75fe                 -lea esi, [ebp - 2]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-2) /* -0x2 */);
    // 0042562d  8d782c                 -lea edi, [eax + 0x2c]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(44) /* 0x2c */);
    // 00425630  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x00425632:
    // 00425632  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 00425638  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425639  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042563a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042563b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042563c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042563d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042563e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_425640(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00425640  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00425641  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00425642  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00425643  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00425644  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00425645  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00425646  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00425648  81ec94000000           -sub esp, 0x94
    (cpu.esp) -= x86::reg32(x86::sreg32(148 /*0x94*/));
    // 0042564e  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 00425654  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00425656  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00425658  8b9018010000           -mov edx, dword ptr [eax + 0x118]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(280) /* 0x118 */);
    // 0042565e  c740549a99993f         -mov dword ptr [eax + 0x54], 0x3f99999a
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(84) /* 0x54 */) = 1067030938 /*0x3f99999a*/;
    // 00425665  83faff                 +cmp edx, -1
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
    // 00425668  0f85d7000000           -jne 0x425745
    if (!cpu.flags.zf)
    {
        goto L_0x00425745;
    }
    // 0042566e  8d45ee                 -lea eax, [ebp - 0x12]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-18) /* -0x12 */);
    // 00425671  8d4d66                 -lea ecx, [ebp + 0x66]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 00425674  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00425679  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0042567a  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 0042567f  8b8614010000           -mov eax, dword ptr [esi + 0x114]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(276) /* 0x114 */);
    // 00425685  e846a2feff             -call 0x40f8d0
    cpu.esp -= 4;
    nfs3hp::aiplan::pathEndPose(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042568a  8b4566                 -mov eax, dword ptr [ebp + 0x66]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 0042568d  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 00425690  db457e                 +fild dword ptr [ebp + 0x7e]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 00425693  8b456a                 -mov eax, dword ptr [ebp + 0x6a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 00425696  dc0d1c6f5300           +fmul qword ptr [0x536f1c]
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 0042569c  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 0042569f  8b456e                 -mov eax, dword ptr [ebp + 0x6e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */);
    // 004256a2  db457e                 +fild dword ptr [ebp + 0x7e]
    x86::Float fpu3 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004256a5  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004256a8  dc0d1c6f5300           +fmul qword ptr [0x536f1c]
    x86::Float fpu4 = cpu.fpu.mul(fpu3, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 004256ae  db457e                 +fild dword ptr [ebp + 0x7e]
    x86::Float fpu5 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004256b1  8b45ee                 -mov eax, dword ptr [ebp - 0x12]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-18) /* -0x12 */);
    // 004256b4  dc0d1c6f5300           +fmul qword ptr [0x536f1c]
    x86::Float fpu6 = cpu.fpu.mul(fpu5, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 004256ba  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004256bd  8b45f2                 -mov eax, dword ptr [ebp - 0xe]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-14) /* -0xe */);
    // 004256c0  db457e                 +fild dword ptr [ebp + 0x7e]
    x86::Float fpu7 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004256c3  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004256c6  dc0d1c6f5300           +fmul qword ptr [0x536f1c]
    x86::Float fpu8 = cpu.fpu.mul(fpu7, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 004256cc  db457e                 +fild dword ptr [ebp + 0x7e]
    x86::Float fpu9 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004256cf  8b45f6                 -mov eax, dword ptr [ebp - 0xa]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 004256d2  dc0d1c6f5300           +fmul qword ptr [0x536f1c]
    x86::Float fpu10 = cpu.fpu.mul(fpu9, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 004256d8  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004256db  8b45fa                 -mov eax, dword ptr [ebp - 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004256de  db457e                 +fild dword ptr [ebp + 0x7e]
    x86::Float fpu11 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004256e1  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004256e4  dc0d1c6f5300           +fmul qword ptr [0x536f1c]
    x86::Float fpu12 = cpu.fpu.mul(fpu11, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 004256ea  db457e                 +fild dword ptr [ebp + 0x7e]
    x86::Float fpu13 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004256ed  8b45fe                 -mov eax, dword ptr [ebp - 2]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-2) /* -0x2 */);
    // 004256f0  dc0d1c6f5300           +fmul qword ptr [0x536f1c]
    x86::Float fpu14 = cpu.fpu.mul(fpu13, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 004256f6  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004256f9  d9ce                   +fxch st(6)
    
    // 004256fb  d95d72                 +fstp dword ptr [ebp + 0x72]
    app->getMemory<float>(cpu.ebp + x86::reg32(114) /* 0x72 */) = float(fpu2);
    // 004256fe  db457e                 +fild dword ptr [ebp + 0x7e]
    x86::Float fpu15 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 00425701  8b4502                 -mov eax, dword ptr [ebp + 2]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(2) /* 0x2 */);
    // 00425704  dc0d1c6f5300           +fmul qword ptr [0x536f1c]
    x86::Float fpu16 = cpu.fpu.mul(fpu15, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 0042570a  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 0042570d  d9cd                   +fxch st(5)
    
    // 0042570f  d95d76                 +fstp dword ptr [ebp + 0x76]
    app->getMemory<float>(cpu.ebp + x86::reg32(118) /* 0x76 */) = float(fpu4);
    // 00425712  db457e                 +fild dword ptr [ebp + 0x7e]
    x86::Float fpu17 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 00425715  8b4506                 -mov eax, dword ptr [ebp + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(6) /* 0x6 */);
    // 00425718  dc0d1c6f5300           +fmul qword ptr [0x536f1c]
    x86::Float fpu18 = cpu.fpu.mul(fpu17, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 0042571e  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 00425721  d9cc                   +fxch st(4)
    
    // 00425723  d95d7a                 +fstp dword ptr [ebp + 0x7a]
    app->getMemory<float>(cpu.ebp + x86::reg32(122) /* 0x7a */) = float(fpu6);
    // 00425726  db457e                 +fild dword ptr [ebp + 0x7e]
    x86::Float fpu19 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 00425729  8b450a                 -mov eax, dword ptr [ebp + 0xa]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(10) /* 0xa */);
    // 0042572c  dc0d1c6f5300           +fmul qword ptr [0x536f1c]
    x86::Float fpu20 = cpu.fpu.mul(fpu19, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 00425732  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 00425735  d9cb                   +fxch st(3)
    
    // 00425737  d95d12                 +fstp dword ptr [ebp + 0x12]
    app->getMemory<float>(cpu.ebp + x86::reg32(18) /* 0x12 */) = float(fpu8);
    // 0042573a  db457e                 +fild dword ptr [ebp + 0x7e]
    x86::Float fpu21 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 0042573d  8b450e                 -mov eax, dword ptr [ebp + 0xe]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(14) /* 0xe */);
    cpu.fpu.count += 7;
    cpu.fpu.st(0) = fpu21;
    cpu.fpu.st(1) = fpu12;
    cpu.fpu.st(2) = fpu10;
    cpu.fpu.st(3) = fpu20;
    cpu.fpu.st(4) = fpu18;
    cpu.fpu.st(5) = fpu16;
    cpu.fpu.st(6) = fpu14;
    }
    // 00425740  e9df010000             -jmp 0x425924
    goto L_0x00425924;
L_0x00425745:
    // 00425745  8d5d12                 -lea ebx, [ebp + 0x12]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(18) /* 0x12 */);
    // 00425748  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0042574a  8d5572                 -lea edx, [ebp + 0x72]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 0042574d  e8eea1feff             -call 0x40f940
    cpu.esp -= 4;
    nfs3hp::aiplan::animationStep(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425752  8b8618010000           -mov eax, dword ptr [esi + 0x118]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(280) /* 0x118 */);
    // 00425758  e863a3feff             -call 0x40fac0
    cpu.esp -= 4;
    nfs3hp::aiplan::animationPlaying(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042575d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0042575f  0f85f3010000           -jne 0x425958
    if (!cpu.flags.zf)
    {
        goto L_0x00425958;
    }
    // 00425765  8b8618010000           -mov eax, dword ptr [esi + 0x118]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(280) /* 0x118 */);
    // 0042576b  e8f09cfeff             -call 0x40f460
    cpu.esp -= 4;
    nfs3hp::aiplan::stopAnimation(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425770  c78618010000ffffffff   -mov dword ptr [esi + 0x118], 0xffffffff
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(280) /* 0x118 */) = 4294967295 /*0xffffffff*/;
    // 0042577a  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 00425781  0f8ecb000000           -jle 0x425852
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00425852;
    }
    // 00425787  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00425789:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00425789  3b0518d56f00           +cmp eax, dword ptr [0x6fd518]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0042578f  0f8de9010000           -jge 0x42597e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0042597e;
    }
    // 00425795  8b3485c8fa5e00         -mov esi, dword ptr [eax*4 + 0x5efac8]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */ + cpu.eax * 4);
    // 0042579c  83bec402000000         +cmp dword ptr [esi + 0x2c4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004257a3  0f85a3000000           -jne 0x42584c
    if (!cpu.flags.zf)
    {
        goto L_0x0042584c;
    }
    // 004257a9  8a9600020000           -mov dl, byte ptr [esi + 0x200]
    cpu.dl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(512) /* 0x200 */);
    // 004257af  f6c280                 +test dl, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 128 /*0x80*/));
    // 004257b2  0f8594000000           -jne 0x42584c
    if (!cpu.flags.zf)
    {
        goto L_0x0042584c;
    }
    // 004257b8  f6c201                 +test dl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 1 /*0x1*/));
    // 004257bb  0f848b000000           -je 0x42584c
    if (cpu.flags.zf)
    {
        goto L_0x0042584c;
    }
    // 004257c1  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
    // 004257c6  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004257c8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004257ca  e8f1bdffff             -call 0x4215c0
    cpu.esp -= 4;
    nfs3hp::carstate::attachCameras(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004257cf  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004257d1  e86afe0500             -call 0x485640
    cpu.esp -= 4;
    sub_485640(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004257d6  833d94367d0000         +cmp dword ptr [0x7d3694], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205972) /* 0x7d3694 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004257dd  751e                   -jne 0x4257fd
    if (!cpu.flags.zf)
    {
        goto L_0x004257fd;
    }
    // 004257df  bba4010000             -mov ebx, 0x1a4
    cpu.ebx = 420 /*0x1a4*/;
    // 004257e4  ba28c66f00             -mov edx, 0x6fc628
    cpu.edx = 7325224 /*0x6fc628*/;
    // 004257e9  b850bc6f00             -mov eax, 0x6fbc50
    cpu.eax = 7322704 /*0x6fbc50*/;
    // 004257ee  e8fd4c0c00             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004257f3  c70594367d0001000000   -mov dword ptr [0x7d3694], 1
    app->getMemory<x86::reg32>(x86::reg32(8205972) /* 0x7d3694 */) = 1 /*0x1*/;
L_0x004257fd:
    // 004257fd  be0000803f             -mov esi, 0x3f800000
    cpu.esi = 1065353216 /*0x3f800000*/;
    // 00425802  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00425807  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00425809  ba11000000             -mov edx, 0x11
    cpu.edx = 17 /*0x11*/;
    // 0042580e  890d88bd6f00           -mov dword ptr [0x6fbd88], ecx
    app->getMemory<x86::reg32>(x86::reg32(7323016) /* 0x6fbd88 */) = cpu.ecx;
    // 00425814  890d84bd6f00           -mov dword ptr [0x6fbd84], ecx
    app->getMemory<x86::reg32>(x86::reg32(7323012) /* 0x6fbd84 */) = cpu.ecx;
    // 0042581a  893590bd6f00           -mov dword ptr [0x6fbd90], esi
    app->getMemory<x86::reg32>(x86::reg32(7323024) /* 0x6fbd90 */) = cpu.esi;
    // 00425820  89358cbd6f00           -mov dword ptr [0x6fbd8c], esi
    app->getMemory<x86::reg32>(x86::reg32(7323020) /* 0x6fbd8c */) = cpu.esi;
    // 00425826  a370bc6f00             -mov dword ptr [0x6fbc70], eax
    app->getMemory<x86::reg32>(x86::reg32(7322736) /* 0x6fbc70 */) = cpu.eax;
    // 0042582b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0042582d  893560bc6f00           -mov dword ptr [0x6fbc60], esi
    app->getMemory<x86::reg32>(x86::reg32(7322720) /* 0x6fbc60 */) = cpu.esi;
    // 00425833  e888fb0500             -call 0x4853c0
    cpu.esp -= 4;
    sub_4853c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425838  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0042583a  e881ec0500             -call 0x4844c0
    cpu.esp -= 4;
    nfs3hp::port::hud::hudPaneSet(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042583f  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 00425845  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425846  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425847  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425848  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425849  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042584a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042584b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0042584c:
    // 0042584c  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0042584d  e937ffffff             -jmp 0x425789
    goto L_0x00425789;
L_0x00425852:
    // 00425852  8d4536                 -lea eax, [ebp + 0x36]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(54) /* 0x36 */);
    // 00425855  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0042585a  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 0042585f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00425860  8b8114010000           -mov eax, dword ptr [ecx + 0x114]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(276) /* 0x114 */);
    // 00425866  8d4d5a                 -lea ecx, [ebp + 0x5a]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(90) /* 0x5a */);
    // 00425869  e862a0feff             -call 0x40f8d0
    cpu.esp -= 4;
    nfs3hp::aiplan::pathEndPose(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0042586e  8b455a                 -mov eax, dword ptr [ebp + 0x5a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */);
    // 00425871  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 00425874  db457e                 -fild dword ptr [ebp + 0x7e]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu22 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 00425877  8b455e                 -mov eax, dword ptr [ebp + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 0042587a  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    x86::Float fpu23 = cpu.fpu.mul(fpu22, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 00425880  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 00425883  8b4562                 -mov eax, dword ptr [ebp + 0x62]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 00425886  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu24 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 00425889  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 0042588c  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    x86::Float fpu25 = cpu.fpu.mul(fpu24, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 00425892  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu26 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 00425895  8b4536                 -mov eax, dword ptr [ebp + 0x36]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(54) /* 0x36 */);
    // 00425898  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    x86::Float fpu27 = cpu.fpu.mul(fpu26, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 0042589e  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004258a1  8b453a                 -mov eax, dword ptr [ebp + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(58) /* 0x3a */);
    // 004258a4  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu28 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004258a7  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004258aa  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    x86::Float fpu29 = cpu.fpu.mul(fpu28, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 004258b0  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu30 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004258b3  8b453e                 -mov eax, dword ptr [ebp + 0x3e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */);
    // 004258b6  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    x86::Float fpu31 = cpu.fpu.mul(fpu30, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 004258bc  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004258bf  8b4542                 -mov eax, dword ptr [ebp + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */);
    // 004258c2  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu32 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004258c5  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004258c8  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    x86::Float fpu33 = cpu.fpu.mul(fpu32, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 004258ce  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu34 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004258d1  8b4546                 -mov eax, dword ptr [ebp + 0x46]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 004258d4  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    x86::Float fpu35 = cpu.fpu.mul(fpu34, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 004258da  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004258dd  d9ce                   -fxch st(6)
    
    // 004258df  d95d72                 -fstp dword ptr [ebp + 0x72]
    app->getMemory<float>(cpu.ebp + x86::reg32(114) /* 0x72 */) = float(fpu23);
    // 004258e2  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu36 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004258e5  8b454a                 -mov eax, dword ptr [ebp + 0x4a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 004258e8  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    x86::Float fpu37 = cpu.fpu.mul(fpu36, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 004258ee  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004258f1  d9cd                   -fxch st(5)
    
    // 004258f3  d95d76                 -fstp dword ptr [ebp + 0x76]
    app->getMemory<float>(cpu.ebp + x86::reg32(118) /* 0x76 */) = float(fpu25);
    // 004258f6  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu38 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004258f9  8b454e                 -mov eax, dword ptr [ebp + 0x4e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */);
    // 004258fc  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    x86::Float fpu39 = cpu.fpu.mul(fpu38, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 00425902  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 00425905  d9cc                   -fxch st(4)
    
    // 00425907  d95d7a                 -fstp dword ptr [ebp + 0x7a]
    app->getMemory<float>(cpu.ebp + x86::reg32(122) /* 0x7a */) = float(fpu27);
    // 0042590a  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu40 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 0042590d  8b4552                 -mov eax, dword ptr [ebp + 0x52]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00425910  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    x86::Float fpu41 = cpu.fpu.mul(fpu40, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 00425916  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 00425919  d9cb                   -fxch st(3)
    
    // 0042591b  d95d12                 -fstp dword ptr [ebp + 0x12]
    app->getMemory<float>(cpu.ebp + x86::reg32(18) /* 0x12 */) = float(fpu29);
    // 0042591e  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu42 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 00425921  8b4556                 -mov eax, dword ptr [ebp + 0x56]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */);
    cpu.fpu.count += 7;
    cpu.fpu.st(0) = fpu42;
    cpu.fpu.st(1) = fpu33;
    cpu.fpu.st(2) = fpu31;
    cpu.fpu.st(3) = fpu41;
    cpu.fpu.st(4) = fpu39;
    cpu.fpu.st(5) = fpu37;
    cpu.fpu.st(6) = fpu35;
    }
L_0x00425924:
    // 00425924  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu43 = cpu.fpu.st(0);
    x86::Float fpu44 = cpu.fpu.mul(fpu43, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 0042592a  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 0042592d  d9ca                   -fxch st(2)
    
    // 0042592f  d95d16                 -fstp dword ptr [ebp + 0x16]
    x86::Float fpu45 = cpu.fpu.st(2);
    app->getMemory<float>(cpu.ebp + x86::reg32(22) /* 0x16 */) = float(fpu45);
    // 00425932  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu46 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 00425935  dc0d1c6f5300           -fmul qword ptr [0x536f1c]
    x86::Float fpu47 = cpu.fpu.mul(fpu46, x86::Float(app->getMemory<double>(x86::reg32(5467932) /* 0x536f1c */)));
    // 0042593b  d9c9                   -fxch st(1)
    
    // 0042593d  d95d1a                 -fstp dword ptr [ebp + 0x1a]
    x86::Float fpu48 = cpu.fpu.st(1);
    app->getMemory<float>(cpu.ebp + x86::reg32(26) /* 0x1a */) = float(fpu48);
    // 00425940  d9cd                   -fxch st(5)
    
    // 00425942  d95d1e                 -fstp dword ptr [ebp + 0x1e]
    x86::Float fpu49 = cpu.fpu.st(6);
    app->getMemory<float>(cpu.ebp + x86::reg32(30) /* 0x1e */) = float(fpu49);
    // 00425945  d9cb                   -fxch st(3)
    
    // 00425947  d95d22                 -fstp dword ptr [ebp + 0x22]
    x86::Float fpu50 = cpu.fpu.st(5);
    app->getMemory<float>(cpu.ebp + x86::reg32(34) /* 0x22 */) = float(fpu50);
    // 0042594a  d9c9                   -fxch st(1)
    
    // 0042594c  d95d26                 -fstp dword ptr [ebp + 0x26]
    x86::Float fpu51 = cpu.fpu.st(4);
    app->getMemory<float>(cpu.ebp + x86::reg32(38) /* 0x26 */) = float(fpu51);
    // 0042594f  d95d2a                 -fstp dword ptr [ebp + 0x2a]
    x86::Float fpu52 = cpu.fpu.st(3);
    app->getMemory<float>(cpu.ebp + x86::reg32(42) /* 0x2a */) = float(fpu52);
    // 00425952  d95d2e                 -fstp dword ptr [ebp + 0x2e]
    app->getMemory<float>(cpu.ebp + x86::reg32(46) /* 0x2e */) = float(fpu44);
    // 00425955  d95d32                 -fstp dword ptr [ebp + 0x32]
    app->getMemory<float>(cpu.ebp + x86::reg32(50) /* 0x32 */) = float(fpu47);
    cpu.fpu.count -= 7;
    }
L_0x00425958:
    // 00425958  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0042595b  8d5d72                 -lea ebx, [ebp + 0x72]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 0042595e  8d9098000000           -lea edx, [eax + 0x98]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(152) /* 0x98 */);
    // 00425964  8d4e14                 -lea ecx, [esi + 0x14]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 00425967  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0042596c  e83fa60b00             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425971  b909000000             -mov ecx, 9
    cpu.ecx = 9 /*0x9*/;
    // 00425976  8d7e2c                 -lea edi, [esi + 0x2c]
    cpu.edi = x86::reg32(cpu.esi + x86::reg32(44) /* 0x2c */);
    // 00425979  8d7512                 -lea esi, [ebp + 0x12]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(18) /* 0x12 */);
    // 0042597c  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x0042597e:
    // 0042597e  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 00425984  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425985  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425986  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425987  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425988  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425989  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0042598a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_425990(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425a43  d80a                   -fmul dword ptr [edx]
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00425a45  d900                   -fld dword ptr [eax]
    x86::Float fpu3 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425a47  d84a04                 -fmul dword ptr [edx + 4]
    x86::Float fpu4 = cpu.fpu.mul(fpu3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00425a4a  d900                   -fld dword ptr [eax]
    x86::Float fpu5 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425a4c  d84a08                 -fmul dword ptr [edx + 8]
    x86::Float fpu6 = cpu.fpu.mul(fpu5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00425a4f  d9ca                   -fxch st(2)
    
    // 00425a51  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu7 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425a54  d84a0c                 -fmul dword ptr [edx + 0xc]
    x86::Float fpu8 = cpu.fpu.mul(fpu7, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00425a57  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425a5a  d84a10                 -fmul dword ptr [edx + 0x10]
    x86::Float fpu10 = cpu.fpu.mul(fpu9, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00425a5d  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu11 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425a60  d84a14                 -fmul dword ptr [edx + 0x14]
    x86::Float fpu12 = cpu.fpu.mul(fpu11, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00425a63  d9ca                   -fxch st(2)
    
    // 00425a65  dec3                   -faddp st(3)
    x86::Float fpu13 = cpu.fpu.add(fpu2, fpu8);
    // 00425a67  dec3                   -faddp st(3)
    x86::Float fpu14 = cpu.fpu.add(fpu4, fpu10);
    // 00425a69  dec3                   -faddp st(3)
    x86::Float fpu15 = cpu.fpu.add(fpu6, fpu12);
    // 00425a6b  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu16 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425a6e  d84a18                 -fmul dword ptr [edx + 0x18]
    x86::Float fpu17 = cpu.fpu.mul(fpu16, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00425a71  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu18 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425a74  d84a1c                 -fmul dword ptr [edx + 0x1c]
    x86::Float fpu19 = cpu.fpu.mul(fpu18, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00425a77  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu20 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425a7a  d84a20                 -fmul dword ptr [edx + 0x20]
    x86::Float fpu21 = cpu.fpu.mul(fpu20, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00425a7d  d9ca                   -fxch st(2)
    
    // 00425a7f  dec3                   -faddp st(3)
    x86::Float fpu22 = cpu.fpu.add(fpu13, fpu17);
    // 00425a81  dec3                   -faddp st(3)
    x86::Float fpu23 = cpu.fpu.add(fpu14, fpu19);
    // 00425a83  dec3                   -faddp st(3)
    x86::Float fpu24 = cpu.fpu.add(fpu15, fpu21);
    // 00425a85  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00425a88  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fpu22);
    // 00425a8a  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fpu23);
    // 00425a8d  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fpu24);
    // 00425a90  8d5de8                 -lea ebx, [ebp - 0x18]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00425a93  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00425a96  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    }
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
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425acf  d80a                   -fmul dword ptr [edx]
    x86::Float fpu2 = cpu.fpu.mul(fpu1, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00425ad1  d900                   -fld dword ptr [eax]
    x86::Float fpu3 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425ad3  d84a04                 -fmul dword ptr [edx + 4]
    x86::Float fpu4 = cpu.fpu.mul(fpu3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00425ad6  d900                   -fld dword ptr [eax]
    x86::Float fpu5 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00425ad8  d84a08                 -fmul dword ptr [edx + 8]
    x86::Float fpu6 = cpu.fpu.mul(fpu5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00425adb  d9ca                   -fxch st(2)
    
    // 00425add  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu7 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425ae0  d84a0c                 -fmul dword ptr [edx + 0xc]
    x86::Float fpu8 = cpu.fpu.mul(fpu7, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00425ae3  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425ae6  d84a10                 -fmul dword ptr [edx + 0x10]
    x86::Float fpu10 = cpu.fpu.mul(fpu9, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00425ae9  d94004                 -fld dword ptr [eax + 4]
    x86::Float fpu11 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00425aec  d84a14                 -fmul dword ptr [edx + 0x14]
    x86::Float fpu12 = cpu.fpu.mul(fpu11, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00425aef  d9ca                   -fxch st(2)
    
    // 00425af1  dec3                   -faddp st(3)
    x86::Float fpu13 = cpu.fpu.add(fpu2, fpu8);
    // 00425af3  dec3                   -faddp st(3)
    x86::Float fpu14 = cpu.fpu.add(fpu4, fpu10);
    // 00425af5  dec3                   -faddp st(3)
    x86::Float fpu15 = cpu.fpu.add(fpu6, fpu12);
    // 00425af7  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu16 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425afa  d84a18                 -fmul dword ptr [edx + 0x18]
    x86::Float fpu17 = cpu.fpu.mul(fpu16, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00425afd  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu18 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425b00  d84a1c                 -fmul dword ptr [edx + 0x1c]
    x86::Float fpu19 = cpu.fpu.mul(fpu18, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00425b03  d94008                 -fld dword ptr [eax + 8]
    x86::Float fpu20 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00425b06  d84a20                 -fmul dword ptr [edx + 0x20]
    x86::Float fpu21 = cpu.fpu.mul(fpu20, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00425b09  d9ca                   -fxch st(2)
    
    // 00425b0b  dec3                   -faddp st(3)
    x86::Float fpu22 = cpu.fpu.add(fpu13, fpu17);
    // 00425b0d  dec3                   -faddp st(3)
    x86::Float fpu23 = cpu.fpu.add(fpu14, fpu19);
    // 00425b0f  dec3                   -faddp st(3)
    x86::Float fpu24 = cpu.fpu.add(fpu15, fpu21);
    // 00425b11  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00425b14  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fpu22);
    // 00425b16  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fpu23);
    // 00425b19  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fpu24);
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
    }
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
    sub_4965b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425b66  d9ee                   +fldz 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = 0.0;
    // 00425b68  ded9                   +fcompp 
    x86::Float fpu2 = cpu.fpu.st(0);
    cpu.fpu.compare(fpu1, fpu2);
    // 00425b6a  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425b6c  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    cpu.fpu.count -= 1;
    }
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
    sub_4965b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425b7e  d9e0                   +fchs 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu3 = cpu.fpu.st(0);
    x86::Float fpu4 = -fpu3;
    cpu.fpu.st(0) = fpu4;
    }
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
    sub_4965b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00425b91:
    // 00425b91  d95dfc                 -fstp dword ptr [ebp - 4]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu5 = cpu.fpu.st(0);
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fpu5);
    // 00425b94  d945fc                 -fld dword ptr [ebp - 4]
    x86::Float fpu6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00425b97  8b1528305500           -mov edx, dword ptr [0x553028]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5582888) /* 0x553028 */);
    // 00425b9d  d84d1c                 -fmul dword ptr [ebp + 0x1c]
    x86::Float fpu7 = cpu.fpu.mul(fpu6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(28) /* 0x1c */)));
    // 00425ba0  42                     -inc edx
    (cpu.edx)++;
    // 00425ba1  d84518                 -fadd dword ptr [ebp + 0x18]
    x86::Float fpu8 = cpu.fpu.add(fpu7, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(24) /* 0x18 */)));
    // 00425ba4  891528305500           -mov dword ptr [0x553028], edx
    app->getMemory<x86::reg32>(x86::reg32(5582888) /* 0x553028 */) = cpu.edx;
    // 00425baa  d95b54                 -fstp dword ptr [ebx + 0x54]
    app->getMemory<float>(cpu.ebx + x86::reg32(84) /* 0x54 */) = float(fpu8);
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
    cpu.fpu.count -= 1;
    }
    // 00425bb0  752e                   -jne 0x425be0
    if (!cpu.flags.zf)
    {
        goto L_0x00425be0;
    }
    // 00425bb2  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00425bb5  d94354                 -fld dword ptr [ebx + 0x54]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(84) /* 0x54 */));
    // 00425bb8  dd1c24                 -fstp qword ptr [esp]
    app->getMemory<double>(cpu.esp) = double(fpu9);
    // 00425bbb  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00425bbe  d945fc                 -fld dword ptr [ebp - 4]
    x86::Float fpu10 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00425bc1  dd1c24                 -fstp qword ptr [esp]
    app->getMemory<double>(cpu.esp) = double(fpu10);
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
    }
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

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_425bf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00425bf0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00425bf1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00425bf2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00425bf3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00425bf4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00425bf5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00425bf6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00425bf8  83ec1c                 -sub esp, 0x1c
    (cpu.esp) -= x86::reg32(x86::sreg32(28 /*0x1c*/));
    // 00425bfb  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00425bfd  8b15a8367d00           -mov edx, dword ptr [0x7d36a8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */);
    // 00425c03  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00425c05  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00425c07  740d                   -je 0x425c16
    if (cpu.flags.zf)
    {
        goto L_0x00425c16;
    }
    // 00425c09  833d60d97c0000         +cmp dword ptr [0x7cd960], 0
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
    // 00425c10  0f842f010000           -je 0x425d45
    if (cpu.flags.zf)
    {
        goto L_0x00425d45;
    }
L_0x00425c16:
    // 00425c16  833dec00560000         +cmp dword ptr [0x5600ec], 0
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
    // 00425c1d  0f8522010000           -jne 0x425d45
    if (!cpu.flags.zf)
    {
        goto L_0x00425d45;
    }
    // 00425c23  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 00425c26  8d780c                 -lea edi, [eax + 0xc]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00425c29  8b410c                 -mov eax, dword ptr [ecx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 00425c2c  e83f800900             -call 0x4bdc70
    cpu.esp -= 4;
    sub_4bdc70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425c31  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00425c33  746c                   -je 0x425ca1
    if (cpu.flags.zf)
    {
        goto L_0x00425ca1;
    }
    // 00425c35  8b9184000000           -mov edx, dword ptr [ecx + 0x84]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(132) /* 0x84 */);
    // 00425c3b  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 00425c42  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00425c44  a148d95d00             -mov eax, dword ptr [0x5dd948]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 00425c49  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00425c4c  668b44020c             -mov ax, word ptr [edx + eax + 0xc]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(12) /* 0xc */ + cpu.eax * 1);
    // 00425c51  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 00425c56  e885e3ffff             -call 0x423fe0
    cpu.esp -= 4;
    nfs3hp::carstate::bitsBroken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425c5b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00425c5d  7527                   -jne 0x425c86
    if (!cpu.flags.zf)
    {
        goto L_0x00425c86;
    }
    // 00425c5f  8b5710                 -mov edx, dword ptr [edi + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(16) /* 0x10 */);
    // 00425c62  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 00425c69  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00425c6b  a148d95d00             -mov eax, dword ptr [0x5dd948]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 00425c70  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00425c73  668b44020c             -mov ax, word ptr [edx + eax + 0xc]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(12) /* 0xc */ + cpu.eax * 1);
    // 00425c78  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 00425c7d  e85ee3ffff             -call 0x423fe0
    cpu.esp -= 4;
    nfs3hp::carstate::bitsBroken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425c82  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00425c84  741b                   -je 0x425ca1
    if (cpu.flags.zf)
    {
        goto L_0x00425ca1;
    }
L_0x00425c86:
    // 00425c86  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 00425c89  83b83806000007         +cmp dword ptr [eax + 0x638], 7
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(1592) /* 0x638 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00425c90  7c07                   -jl 0x425c99
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00425c99;
    }
    // 00425c92  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00425c97  eb02                   -jmp 0x425c9b
    goto L_0x00425c9b;
L_0x00425c99:
    // 00425c99  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
L_0x00425c9b:
    // 00425c9b  89861c010000           -mov dword ptr [esi + 0x11c], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(284) /* 0x11c */) = cpu.eax;
L_0x00425ca1:
    // 00425ca1  8d55e4                 -lea edx, [ebp - 0x1c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00425ca4  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00425ca6  8b9e1c010000           -mov ebx, dword ptr [esi + 0x11c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(284) /* 0x11c */);
    // 00425cac  e81fa3ffff             -call 0x41ffd0
    cpu.esp -= 4;
    nfs3hp::trackdata::lanePosition(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425cb1  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00425cb3  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00425cb6  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00425cb9  db45fc                 +fild dword ptr [ebp - 4]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00425cbc  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00425cbf  dd052c6f5300           +fld qword ptr [0x536f2c]
    x86::Float fpu2 = x86::Float(app->getMemory<double>(x86::reg32(5467948) /* 0x536f2c */));
    // 00425cc5  d9c9                   +fxch st(1)
    
    // 00425cc7  d8c9                   +fmul st(1)
    x86::Float fpu3 = cpu.fpu.mul(fpu1, x86::Float(fpu2));
    // 00425cc9  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00425ccc  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00425ccf  db45fc                 +fild dword ptr [ebp - 4]
    x86::Float fpu4 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00425cd2  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00425cd5  d8ca                   +fmul st(2)
    x86::Float fpu5 = cpu.fpu.mul(fpu4, x86::Float(fpu2));
    // 00425cd7  db45fc                 +fild dword ptr [ebp - 4]
    x86::Float fpu6 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00425cda  decb                   +fmulp st(3)
    x86::Float fpu7 = cpu.fpu.mul(fpu2, fpu6);
    // 00425cdc  d9c9                   +fxch st(1)
    
    // 00425cde  d95df0                 +fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fpu3);
    // 00425ce1  d805346f5300           +fadd dword ptr [0x536f34]
    x86::Float fpu8 = cpu.fpu.add(fpu5, x86::Float(app->getMemory<float>(x86::reg32(5467956) /* 0x536f34 */)));
    // 00425ce7  d9c9                   +fxch st(1)
    
    // 00425ce9  d95df8                 +fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fpu7);
    // 00425cec  d95df4                 +fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fpu8);
    // 00425cef  d94114                 +fld dword ptr [ecx + 0x14]
    x86::Float fpu9 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(20) /* 0x14 */));
    // 00425cf2  d85df0                 +fcomp dword ptr [ebp - 0x10]
    cpu.fpu.compare(fpu9, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 00425cf5  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425cf7  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425cf8  7516                   -jne 0x425d10
    if (!cpu.flags.zf)
    {
        goto L_0x00425d10;
    }
    // 00425cfa  d94118                 +fld dword ptr [ecx + 0x18]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu10 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(24) /* 0x18 */));
    // 00425cfd  d85df4                 +fcomp dword ptr [ebp - 0xc]
    cpu.fpu.compare(fpu10, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 00425d00  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425d02  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425d03  750b                   -jne 0x425d10
    if (!cpu.flags.zf)
    {
        goto L_0x00425d10;
    }
    // 00425d05  d9411c                 +fld dword ptr [ecx + 0x1c]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu11 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(28) /* 0x1c */));
    // 00425d08  d85df8                 +fcomp dword ptr [ebp - 8]
    cpu.fpu.compare(fpu11, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00425d0b  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425d0d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425d0e  7408                   -je 0x425d18
    if (cpu.flags.zf)
    {
        goto L_0x00425d18;
    }
L_0x00425d10:
    // 00425d10  8b410c                 -mov eax, dword ptr [ecx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 00425d13  e8b87e0900             -call 0x4bdbd0
    cpu.esp -= 4;
    sub_4bdbd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00425d18:
    // 00425d18  ffb614010000           -push dword ptr [esi + 0x114]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(276) /* 0x114 */);
    cpu.esp -= 4;
    // 00425d1e  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00425d21  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00425d23  ffb610010000           -push dword ptr [esi + 0x110]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(272) /* 0x110 */);
    cpu.esp -= 4;
    // 00425d29  e812feffff             -call 0x425b40
    cpu.esp -= 4;
    sub_425b40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425d2e  8d5114                 -lea edx, [ecx + 0x14]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(20) /* 0x14 */);
    // 00425d31  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00425d33  8999ec000000           -mov dword ptr [ecx + 0xec], ebx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(236) /* 0xec */) = cpu.ebx;
    // 00425d39  e852c5ffff             -call 0x422290
    cpu.esp -= 4;
    nfs3hp::carstate::lookAtTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425d3e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00425d40  e8ebf1ffff             -call 0x424f30
    cpu.esp -= 4;
    sub_424f30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00425d45:
    // 00425d45  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00425d47  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425d48  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425d49  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425d4a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425d4b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425d4c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00425d4d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_425d50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00425d50  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00425d51  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00425d52  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00425d53  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00425d54  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00425d55  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00425d56  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00425d58  81ecb0000000           -sub esp, 0xb0
    (cpu.esp) -= x86::reg32(x86::sreg32(176 /*0xb0*/));
    // 00425d5e  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 00425d64  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00425d66  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 00425d6b  8d7dd2                 -lea edi, [ebp - 0x2e]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-46) /* -0x2e */);
    // 00425d6e  be503d4200             -mov esi, 0x423d50
    cpu.esi = 4341072 /*0x423d50*/;
    // 00425d73  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00425d75  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 00425d7a  8d7df2                 -lea edi, [ebp - 0xe]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-14) /* -0xe */);
    // 00425d7d  be703d4200             -mov esi, 0x423d70
    cpu.esi = 4341104 /*0x423d70*/;
    // 00425d82  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00425d84  8b15a8367d00           -mov edx, dword ptr [0x7d36a8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */);
    // 00425d8a  8b4804                 -mov ecx, dword ptr [eax + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00425d8d  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00425d8f  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00425d91  740d                   -je 0x425da0
    if (cpu.flags.zf)
    {
        goto L_0x00425da0;
    }
    // 00425d93  833d60d97c0000         +cmp dword ptr [0x7cd960], 0
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
    // 00425d9a  0f84d9040000           -je 0x426279
    if (cpu.flags.zf)
    {
        goto L_0x00426279;
    }
L_0x00425da0:
    // 00425da0  833dec00560000         +cmp dword ptr [0x5600ec], 0
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
    // 00425da7  0f85cc040000           -jne 0x426279
    if (!cpu.flags.zf)
    {
        goto L_0x00426279;
    }
    // 00425dad  833d80367d0000         +cmp dword ptr [0x7d3680], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205952) /* 0x7d3680 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00425db4  740a                   -je 0x425dc0
    if (cpu.flags.zf)
    {
        goto L_0x00425dc0;
    }
    // 00425db6  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425db9  ff048504305500         -inc dword ptr [eax*4 + 0x553004]
    (app->getMemory<x86::reg32>(x86::reg32(5582852) /* 0x553004 */ + cpu.eax * 4))++;
L_0x00425dc0:
    // 00425dc0  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425dc3  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00425dc6  8bb804305500           -mov edi, dword ptr [eax + 0x553004]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5582852) /* 0x553004 */);
    // 00425dcc  8b9014305500           -mov edx, dword ptr [eax + 0x553014]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5582868) /* 0x553014 */);
    // 00425dd2  3b7c95f2               +cmp edi, dword ptr [ebp + edx*4 - 0xe]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-14) /* -0xe */ + cpu.edx * 4)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00425dd6  0f859b010000           -jne 0x425f77
    if (!cpu.flags.zf)
    {
        goto L_0x00425f77;
    }
    // 00425ddc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00425dde  899004305500           -mov dword ptr [eax + 0x553004], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5582852) /* 0x553004 */) = cpu.edx;
    // 00425de4  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425de7  8b04850c305500         -mov eax, dword ptr [eax*4 + 0x55300c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5582860) /* 0x55300c */ + cpu.eax * 4);
    // 00425dee  8b3d40d95d00           -mov edi, dword ptr [0x5dd940]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00425df4  40                     -inc eax
    (cpu.eax)++;
    // 00425df5  39f8                   +cmp eax, edi
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
    // 00425df7  7c02                   -jl 0x425dfb
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00425dfb;
    }
    // 00425df9  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
L_0x00425dfb:
    // 00425dfb  8b530c                 -mov edx, dword ptr [ebx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425dfe  8904950c305500         -mov dword ptr [edx*4 + 0x55300c], eax
    app->getMemory<x86::reg32>(x86::reg32(5582860) /* 0x55300c */ + cpu.edx * 4) = cpu.eax;
    // 00425e05  8b81c0070000           -mov eax, dword ptr [ecx + 0x7c0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(1984) /* 0x7c0 */);
    // 00425e0b  89455a                 -mov dword ptr [ebp + 0x5a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */) = cpu.eax;
    // 00425e0e  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425e11  8b511c                 -mov edx, dword ptr [ecx + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */);
    // 00425e14  8b3c850c305500         -mov edi, dword ptr [eax*4 + 0x55300c]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5582860) /* 0x55300c */ + cpu.eax * 4);
    // 00425e1b  8b048514305500         -mov eax, dword ptr [eax*4 + 0x553014]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5582868) /* 0x553014 */ + cpu.eax * 4);
    // 00425e22  d9455a                 -fld dword ptr [ebp + 0x5a]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */));
    // 00425e25  d86485d2               -fsub dword ptr [ebp + eax*4 - 0x2e]
    x86::Float fpu2 = cpu.fpu.sub(fpu1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-46) /* -0x2e */ + cpu.eax * 4)));
    // 00425e29  29fa                   -sub edx, edi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00425e2b  d95d72                 -fstp dword ptr [ebp + 0x72]
    app->getMemory<float>(cpu.ebp + x86::reg32(114) /* 0x72 */) = float(fpu2);
    // 00425e2e  83fa07                 +cmp edx, 7
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
    }
    // 00425e31  7f1d                   -jg 0x425e50
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00425e50;
    }
    // 00425e33  83fa04                 +cmp edx, 4
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
    // 00425e36  7e1f                   -jle 0x425e57
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00425e57;
    }
    // 00425e38  d94572                 +fld dword ptr [ebp + 0x72]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu3 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(114) /* 0x72 */));
    // 00425e3b  d81d386f5300           +fcomp dword ptr [0x536f38]
    cpu.fpu.compare(fpu3, x86::Float(app->getMemory<float>(x86::reg32(5467960) /* 0x536f38 */)));
    // 00425e41  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425e43  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425e44  7311                   -jae 0x425e57
    if (!cpu.flags.cf)
    {
        goto L_0x00425e57;
    }
    // 00425e46  d9ee                   +fldz 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu4 = 0.0;
    // 00425e48  d85d72                 +fcomp dword ptr [ebp + 0x72]
    cpu.fpu.compare(fpu4, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(114) /* 0x72 */)));
    // 00425e4b  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425e4d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425e4e  7307                   -jae 0x425e57
    if (!cpu.flags.cf)
    {
        goto L_0x00425e57;
    }
L_0x00425e50:
    // 00425e50  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 00425e55  eb27                   -jmp 0x425e7e
    goto L_0x00425e7e;
L_0x00425e57:
    // 00425e57  83fafa                 +cmp edx, -6
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-6 /*-0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00425e5a  7c1d                   -jl 0x425e79
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00425e79;
    }
    // 00425e5c  83fafc                 +cmp edx, -4
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-4 /*-0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00425e5f  7d1d                   -jge 0x425e7e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00425e7e;
    }
    // 00425e61  d94572                 +fld dword ptr [ebp + 0x72]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu5 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(114) /* 0x72 */));
    // 00425e64  dc1d3c6f5300           +fcomp qword ptr [0x536f3c]
    cpu.fpu.compare(fpu5, x86::Float(app->getMemory<double>(x86::reg32(5467964) /* 0x536f3c */)));
    // 00425e6a  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425e6c  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425e6d  760f                   -jbe 0x425e7e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00425e7e;
    }
    // 00425e6f  d9ee                   +fldz 
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu6 = 0.0;
    // 00425e71  d85d72                 +fcomp dword ptr [ebp + 0x72]
    cpu.fpu.compare(fpu6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(114) /* 0x72 */)));
    // 00425e74  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425e76  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425e77  7605                   -jbe 0x425e7e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00425e7e;
    }
L_0x00425e79:
    // 00425e79  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
L_0x00425e7e:
    // 00425e7e  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00425e80  0f84f1000000           -je 0x425f77
    if (cpu.flags.zf)
    {
        goto L_0x00425f77;
    }
    // 00425e86  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
L_0x00425e88:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00425e88  d9455a                 +fld dword ptr [ebp + 0x5a]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu7 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */));
    // 00425e8b  d85c95d2               +fcomp dword ptr [ebp + edx*4 - 0x2e]
    cpu.fpu.compare(fpu7, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-46) /* -0x2e */ + cpu.edx * 4)));
    // 00425e8f  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425e91  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425e92  7603                   -jbe 0x425e97
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00425e97;
    }
    // 00425e94  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00425e95  ebf1                   -jmp 0x425e88
    goto L_0x00425e88;
L_0x00425e97:
    // 00425e97  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425e9a  89148514305500         -mov dword ptr [eax*4 + 0x553014], edx
    app->getMemory<x86::reg32>(x86::reg32(5582868) /* 0x553014 */ + cpu.eax * 4) = cpu.edx;
    // 00425ea1  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425ea4  8b048514305500         -mov eax, dword ptr [eax*4 + 0x553014]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5582868) /* 0x553014 */ + cpu.eax * 4);
    // 00425eab  d9455a                 -fld dword ptr [ebp + 0x5a]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu8 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */));
    // 00425eae  d86485d2               -fsub dword ptr [ebp + eax*4 - 0x2e]
    x86::Float fpu9 = cpu.fpu.sub(fpu8, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-46) /* -0x2e */ + cpu.eax * 4)));
    // 00425eb2  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425eb5  d95d62                 -fstp dword ptr [ebp + 0x62]
    app->getMemory<float>(cpu.ebp + x86::reg32(98) /* 0x62 */) = float(fpu9);
    }
    // 00425eb8  e8b37d0900             -call 0x4bdc70
    cpu.esp -= 4;
    sub_4bdc70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425ebd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00425ebf  0f84b2000000           -je 0x425f77
    if (cpu.flags.zf)
    {
        goto L_0x00425f77;
    }
    // 00425ec5  83fe01                 +cmp esi, 1
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
    // 00425ec8  0f8562000000           -jne 0x425f30
    if (!cpu.flags.zf)
    {
        goto L_0x00425f30;
    }
    // 00425ece  d94562                 +fld dword ptr [ebp + 0x62]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu10 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(98) /* 0x62 */));
    // 00425ed1  d81d446f5300           +fcomp dword ptr [0x536f44]
    cpu.fpu.compare(fpu10, x86::Float(app->getMemory<float>(x86::reg32(5467972) /* 0x536f44 */)));
    // 00425ed7  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425ed9  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425eda  7314                   -jae 0x425ef0
    if (!cpu.flags.cf)
    {
        goto L_0x00425ef0;
    }
    // 00425edc  8b411c                 -mov eax, dword ptr [ecx + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */);
    // 00425edf  8b1540d95d00           -mov edx, dword ptr [0x5dd940]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00425ee5  83c003                 -add eax, 3
    (cpu.eax) += x86::reg32(x86::sreg32(3 /*0x3*/));
    // 00425ee8  39d0                   +cmp eax, edx
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
    // 00425eea  7c38                   -jl 0x425f24
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00425f24;
    }
    // 00425eec  29d0                   +sub eax, edx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00425eee  eb34                   -jmp 0x425f24
    goto L_0x00425f24;
L_0x00425ef0:
    // 00425ef0  d94562                 +fld dword ptr [ebp + 0x62]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu11 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(98) /* 0x62 */));
    // 00425ef3  d81d486f5300           +fcomp dword ptr [0x536f48]
    cpu.fpu.compare(fpu11, x86::Float(app->getMemory<float>(x86::reg32(5467976) /* 0x536f48 */)));
    // 00425ef9  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425efb  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425efc  7314                   -jae 0x425f12
    if (!cpu.flags.cf)
    {
        goto L_0x00425f12;
    }
    // 00425efe  8b411c                 -mov eax, dword ptr [ecx + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */);
    // 00425f01  8b1540d95d00           -mov edx, dword ptr [0x5dd940]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00425f07  83c005                 -add eax, 5
    (cpu.eax) += x86::reg32(x86::sreg32(5 /*0x5*/));
    // 00425f0a  39d0                   +cmp eax, edx
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
    // 00425f0c  7c16                   -jl 0x425f24
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00425f24;
    }
    // 00425f0e  29d0                   +sub eax, edx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00425f10  eb12                   -jmp 0x425f24
    goto L_0x00425f24;
L_0x00425f12:
    // 00425f12  8b411c                 -mov eax, dword ptr [ecx + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */);
    // 00425f15  8b1540d95d00           -mov edx, dword ptr [0x5dd940]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00425f1b  83c007                 -add eax, 7
    (cpu.eax) += x86::reg32(x86::sreg32(7 /*0x7*/));
    // 00425f1e  39d0                   +cmp eax, edx
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
    // 00425f20  7c02                   -jl 0x425f24
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00425f24;
    }
    // 00425f22  29d0                   +sub eax, edx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
L_0x00425f24:
    // 00425f24  8b530c                 -mov edx, dword ptr [ebx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425f27  8904950c305500         -mov dword ptr [edx*4 + 0x55300c], eax
    app->getMemory<x86::reg32>(x86::reg32(5582860) /* 0x55300c */ + cpu.edx * 4) = cpu.eax;
    // 00425f2e  eb3a                   -jmp 0x425f6a
    goto L_0x00425f6a;
L_0x00425f30:
    // 00425f30  d94562                 +fld dword ptr [ebp + 0x62]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu12 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(98) /* 0x62 */));
    // 00425f33  dc1d4c6f5300           +fcomp qword ptr [0x536f4c]
    cpu.fpu.compare(fpu12, x86::Float(app->getMemory<double>(x86::reg32(5467980) /* 0x536f4c */)));
    // 00425f39  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00425f3b  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00425f3c  7612                   -jbe 0x425f50
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00425f50;
    }
    // 00425f3e  8b411c                 -mov eax, dword ptr [ecx + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */);
    // 00425f41  83e803                 -sub eax, 3
    (cpu.eax) -= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 00425f44  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00425f46  7d18                   -jge 0x425f60
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00425f60;
    }
    // 00425f48  030540d95d00           +add eax, dword ptr [0x5dd940]
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00425f4e  eb10                   -jmp 0x425f60
    goto L_0x00425f60;
L_0x00425f50:
    // 00425f50  8b411c                 -mov eax, dword ptr [ecx + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */);
    // 00425f53  83e805                 -sub eax, 5
    (cpu.eax) -= x86::reg32(x86::sreg32(5 /*0x5*/));
    // 00425f56  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00425f58  7d06                   -jge 0x425f60
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00425f60;
    }
    // 00425f5a  030540d95d00           -add eax, dword ptr [0x5dd940]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
L_0x00425f60:
    // 00425f60  8b530c                 -mov edx, dword ptr [ebx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425f63  8904950c305500         -mov dword ptr [edx*4 + 0x55300c], eax
    app->getMemory<x86::reg32>(x86::reg32(5582860) /* 0x55300c */ + cpu.edx * 4) = cpu.eax;
L_0x00425f6a:
    // 00425f6a  ba09000000             -mov edx, 9
    cpu.edx = 9 /*0x9*/;
    // 00425f6f  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425f72  e8497d0900             -call 0x4bdcc0
    cpu.esp -= 4;
    sub_4bdcc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00425f77:
    // 00425f77  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425f7a  e8f17c0900             -call 0x4bdc70
    cpu.esp -= 4;
    sub_4bdc70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425f7f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00425f81  745e                   -je 0x425fe1
    if (cpu.flags.zf)
    {
        goto L_0x00425fe1;
    }
    // 00425f83  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00425f85  754d                   -jne 0x425fd4
    if (!cpu.flags.zf)
    {
        goto L_0x00425fd4;
    }
    // 00425f87  8b9384000000           -mov edx, dword ptr [ebx + 0x84]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(132) /* 0x84 */);
    // 00425f8d  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 00425f94  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00425f96  8b1548d95d00           -mov edx, dword ptr [0x5dd948]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 00425f9c  668b44820c             -mov ax, word ptr [edx + eax*4 + 0xc]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(12) /* 0xc */ + cpu.eax * 4);
    // 00425fa1  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 00425fa6  e835e0ffff             -call 0x423fe0
    cpu.esp -= 4;
    nfs3hp::carstate::bitsBroken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425fab  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00425fad  7525                   -jne 0x425fd4
    if (!cpu.flags.zf)
    {
        goto L_0x00425fd4;
    }
    // 00425faf  8b511c                 -mov edx, dword ptr [ecx + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */);
    // 00425fb2  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 00425fb9  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00425fbb  8b1548d95d00           -mov edx, dword ptr [0x5dd948]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 00425fc1  668b44820c             -mov ax, word ptr [edx + eax*4 + 0xc]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(12) /* 0xc */ + cpu.eax * 4);
    // 00425fc6  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 00425fcb  e810e0ffff             -call 0x423fe0
    cpu.esp -= 4;
    nfs3hp::carstate::bitsBroken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425fd0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00425fd2  740d                   -je 0x425fe1
    if (cpu.flags.zf)
    {
        goto L_0x00425fe1;
    }
L_0x00425fd4:
    // 00425fd4  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425fd7  e8f47b0900             -call 0x4bdbd0
    cpu.esp -= 4;
    sub_4bdbd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00425fdc  e998020000             -jmp 0x426279
    goto L_0x00426279;
L_0x00425fe1:
    // 00425fe1  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425fe4  8b04850c305500         -mov eax, dword ptr [eax*4 + 0x55300c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5582860) /* 0x55300c */ + cpu.eax * 4);
    // 00425feb  8b1540d95d00           -mov edx, dword ptr [0x5dd940]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00425ff1  40                     -inc eax
    (cpu.eax)++;
    // 00425ff2  39d0                   +cmp eax, edx
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
    // 00425ff4  7c02                   -jl 0x425ff8
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00425ff8;
    }
    // 00425ff6  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
L_0x00425ff8:
    // 00425ff8  8b530c                 -mov edx, dword ptr [ebx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 00425ffb  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00425ffe  8b8a0c305500           -mov ecx, dword ptr [edx + 0x55300c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(5582860) /* 0x55300c */);
    // 00426004  895526                 -mov dword ptr [ebp + 0x26], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(38) /* 0x26 */) = cpu.edx;
    // 00426007  8d14cd00000000         -lea edx, [ecx*8]
    cpu.edx = x86::reg32(cpu.ecx * 8);
    // 0042600e  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00426010  8b3548d95d00           -mov esi, dword ptr [0x5dd948]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 00426016  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00426019  8d0c32                 -lea ecx, [edx + esi]
    cpu.ecx = x86::reg32(cpu.edx + cpu.esi * 1);
    // 0042601c  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0042601e  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00426021  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00426023  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00426026  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 00426028  89451e                 -mov dword ptr [ebp + 0x1e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(30) /* 0x1e */) = cpu.eax;
    // 0042602b  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 0042602d  8b4526                 -mov eax, dword ptr [ebp + 0x26]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(38) /* 0x26 */);
    // 00426030  8b8014305500           -mov eax, dword ptr [eax + 0x553014]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5582868) /* 0x553014 */);
    // 00426036  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00426039  8b39                   -mov edi, dword ptr [ecx]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx);
    // 0042603b  894522                 -mov dword ptr [ebp + 0x22], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(34) /* 0x22 */) = cpu.eax;
    // 0042603e  29fa                   -sub edx, edi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00426040  8b7d22                 -mov edi, dword ptr [ebp + 0x22]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(34) /* 0x22 */);
    // 00426043  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00426045  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00426048  f77c2ff2               -idiv dword ptr [edi + ebp - 0xe]
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(app->getMemory<x86::reg32>(cpu.edi + x86::reg32(-14) /* -0xe */ + cpu.ebp * 1));
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0042604c  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 0042604f  8b451e                 -mov eax, dword ptr [ebp + 0x1e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(30) /* 0x1e */);
    // 00426052  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00426055  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 00426058  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0042605a  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0042605c  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0042605f  f77c2ff2               -idiv dword ptr [edi + ebp - 0xe]
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(app->getMemory<x86::reg32>(cpu.edi + x86::reg32(-14) /* -0xe */ + cpu.ebp * 1));
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00426063  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 00426066  8b451e                 -mov eax, dword ptr [ebp + 0x1e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(30) /* 0x1e */);
    // 00426069  8b7908                 -mov edi, dword ptr [ecx + 8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0042606c  8b5008                 -mov edx, dword ptr [eax + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 0042606f  29fa                   -sub edx, edi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00426071  8b7d22                 -mov edi, dword ptr [ebp + 0x22]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(34) /* 0x22 */);
    // 00426074  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00426076  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00426079  f77c2ff2               -idiv dword ptr [edi + ebp - 0xe]
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(app->getMemory<x86::reg32>(cpu.edi + x86::reg32(-14) /* -0xe */ + cpu.ebp * 1));
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0042607d  dd05546f5300           -fld qword ptr [0x536f54]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu13 = x86::Float(app->getMemory<double>(x86::reg32(5467988) /* 0x536f54 */));
    // 00426083  db01                   -fild dword ptr [ecx]
    x86::Float fpu14 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx)));
    // 00426085  d8c9                   -fmul st(1)
    x86::Float fpu15 = cpu.fpu.mul(fpu14, x86::Float(fpu13));
    // 00426087  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu16 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 0042608a  d8ca                   -fmul st(2)
    x86::Float fpu17 = cpu.fpu.mul(fpu16, x86::Float(fpu13));
    // 0042608c  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 0042608f  8b4526                 -mov eax, dword ptr [ebp + 0x26]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(38) /* 0x26 */);
    // 00426092  d95d12                 -fstp dword ptr [ebp + 0x12]
    app->getMemory<float>(cpu.ebp + x86::reg32(18) /* 0x12 */) = float(fpu17);
    // 00426095  db8004305500           -fild dword ptr [eax + 0x553004]
    x86::Float fpu18 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5582852) /* 0x553004 */)));
    // 0042609b  d84d12                 -fmul dword ptr [ebp + 0x12]
    x86::Float fpu19 = cpu.fpu.mul(fpu18, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(18) /* 0x12 */)));
    // 0042609e  dec1                   -faddp st(1)
    x86::Float fpu20 = cpu.fpu.add(fpu15, fpu19);
    // 004260a0  8b4b0c                 -mov ecx, dword ptr [ebx + 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 004260a3  d95b14                 -fstp dword ptr [ebx + 0x14]
    app->getMemory<float>(cpu.ebx + x86::reg32(20) /* 0x14 */) = float(fpu20);
    // 004260a6  8b148d0c305500         -mov edx, dword ptr [ecx*4 + 0x55300c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5582860) /* 0x55300c */ + cpu.ecx * 4);
    // 004260ad  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 004260b4  db457a                 -fild dword ptr [ebp + 0x7a]
    x86::Float fpu21 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */)));
    // 004260b7  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004260b9  d8c9                   -fmul st(1)
    x86::Float fpu22 = cpu.fpu.mul(fpu21, x86::Float(fpu13));
    // 004260bb  db448604               -fild dword ptr [esi + eax*4 + 4]
    x86::Float fpu23 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */ + cpu.eax * 4)));
    // 004260bf  d8ca                   -fmul st(2)
    x86::Float fpu24 = cpu.fpu.mul(fpu23, x86::Float(fpu13));
    // 004260c1  d9c9                   -fxch st(1)
    
    // 004260c3  d95d16                 -fstp dword ptr [ebp + 0x16]
    app->getMemory<float>(cpu.ebp + x86::reg32(22) /* 0x16 */) = float(fpu22);
    // 004260c6  db048d04305500         -fild dword ptr [ecx*4 + 0x553004]
    x86::Float fpu25 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5582852) /* 0x553004 */ + cpu.ecx * 4)));
    // 004260cd  d84d16                 -fmul dword ptr [ebp + 0x16]
    x86::Float fpu26 = cpu.fpu.mul(fpu25, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(22) /* 0x16 */)));
    // 004260d0  dec1                   -faddp st(1)
    x86::Float fpu27 = cpu.fpu.add(fpu24, fpu26);
    // 004260d2  8b530c                 -mov edx, dword ptr [ebx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 004260d5  d95b18                 -fstp dword ptr [ebx + 0x18]
    app->getMemory<float>(cpu.ebx + x86::reg32(24) /* 0x18 */) = float(fpu27);
    // 004260d8  8b0c950c305500         -mov ecx, dword ptr [edx*4 + 0x55300c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5582860) /* 0x55300c */ + cpu.edx * 4);
    // 004260df  8d04cd00000000         -lea eax, [ecx*8]
    cpu.eax = x86::reg32(cpu.ecx * 8);
    // 004260e6  db457e                 -fild dword ptr [ebp + 0x7e]
    x86::Float fpu28 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004260e9  01c8                   +add eax, ecx
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
    // 004260eb  d8c9                   +fmul st(1)
    x86::Float fpu29 = cpu.fpu.mul(fpu28, x86::Float(fpu13));
    // 004260ed  db448608               +fild dword ptr [esi + eax*4 + 8]
    x86::Float fpu30 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */ + cpu.eax * 4)));
    // 004260f1  deca                   +fmulp st(2)
    x86::Float fpu31 = cpu.fpu.mul(fpu13, fpu30);
    // 004260f3  d95d1a                 +fstp dword ptr [ebp + 0x1a]
    app->getMemory<float>(cpu.ebp + x86::reg32(26) /* 0x1a */) = float(fpu29);
    // 004260f6  db049504305500         +fild dword ptr [edx*4 + 0x553004]
    x86::Float fpu32 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5582852) /* 0x553004 */ + cpu.edx * 4)));
    // 004260fd  d84d1a                 +fmul dword ptr [ebp + 0x1a]
    x86::Float fpu33 = cpu.fpu.mul(fpu32, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(26) /* 0x1a */)));
    // 00426100  dec1                   +faddp st(1)
    x86::Float fpu34 = cpu.fpu.add(fpu31, fpu33);
    // 00426102  d95b1c                 +fstp dword ptr [ebx + 0x1c]
    app->getMemory<float>(cpu.ebx + x86::reg32(28) /* 0x1c */) = float(fpu34);
    // 00426105  8b4304                 -mov eax, dword ptr [ebx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 00426108  d98098000000           +fld dword ptr [eax + 0x98]
    x86::Float fpu35 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(152) /* 0x98 */));
    // 0042610e  d86314                 +fsub dword ptr [ebx + 0x14]
    x86::Float fpu36 = cpu.fpu.sub(fpu35, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(20) /* 0x14 */)));
    // 00426111  d9ee                   +fldz 
    x86::Float fpu37 = 0.0;
    // 00426113  d9c9                   +fxch st(1)
    
    // 00426115  d95d5e                 +fstp dword ptr [ebp + 0x5e]
    app->getMemory<float>(cpu.ebp + x86::reg32(94) /* 0x5e */) = float(fpu36);
    // 00426118  d85d5e                 +fcomp dword ptr [ebp + 0x5e]
    cpu.fpu.compare(fpu37, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(94) /* 0x5e */)));
    // 0042611b  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 0042611d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 0042611e  760a                   -jbe 0x42612a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0042612a;
    }
    // 00426120  d9455e                 +fld dword ptr [ebp + 0x5e]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu38 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(94) /* 0x5e */));
    // 00426123  d9e0                   +fchs 
    x86::Float fpu39 = -fpu38;
    // 00426125  d95d42                 +fstp dword ptr [ebp + 0x42]
    app->getMemory<float>(cpu.ebp + x86::reg32(66) /* 0x42 */) = float(fpu39);
    }
    // 00426128  eb06                   -jmp 0x426130
    goto L_0x00426130;
L_0x0042612a:
    // 0042612a  8b455e                 -mov eax, dword ptr [ebp + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 0042612d  894542                 -mov dword ptr [ebp + 0x42], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */) = cpu.eax;
L_0x00426130:
    // 00426130  8b4304                 -mov eax, dword ptr [ebx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 00426133  d980a0000000           +fld dword ptr [eax + 0xa0]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu40 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(160) /* 0xa0 */));
    // 00426139  d8631c                 +fsub dword ptr [ebx + 0x1c]
    x86::Float fpu41 = cpu.fpu.sub(fpu40, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(28) /* 0x1c */)));
    // 0042613c  d9ee                   +fldz 
    x86::Float fpu42 = 0.0;
    // 0042613e  d9c9                   +fxch st(1)
    
    // 00426140  d95d56                 +fstp dword ptr [ebp + 0x56]
    app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */) = float(fpu41);
    // 00426143  d85d56                 +fcomp dword ptr [ebp + 0x56]
    cpu.fpu.compare(fpu42, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */)));
    // 00426146  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00426148  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00426149  760a                   -jbe 0x426155
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00426155;
    }
    // 0042614b  d94556                 +fld dword ptr [ebp + 0x56]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu43 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */));
    // 0042614e  d9e0                   +fchs 
    x86::Float fpu44 = -fpu43;
    // 00426150  d95d32                 +fstp dword ptr [ebp + 0x32]
    app->getMemory<float>(cpu.ebp + x86::reg32(50) /* 0x32 */) = float(fpu44);
    }
    // 00426153  eb06                   -jmp 0x42615b
    goto L_0x0042615b;
L_0x00426155:
    // 00426155  8b4556                 -mov eax, dword ptr [ebp + 0x56]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 00426158  894532                 -mov dword ptr [ebp + 0x32], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(50) /* 0x32 */) = cpu.eax;
L_0x0042615b:
    // 0042615b  d94542                 +fld dword ptr [ebp + 0x42]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu45 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(66) /* 0x42 */));
    // 0042615e  d85d32                 +fcomp dword ptr [ebp + 0x32]
    cpu.fpu.compare(fpu45, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(50) /* 0x32 */)));
    // 00426161  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00426163  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00426164  766c                   -jbe 0x4261d2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004261d2;
    }
    // 00426166  8b4304                 -mov eax, dword ptr [ebx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 00426169  d980a0000000           +fld dword ptr [eax + 0xa0]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu46 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(160) /* 0xa0 */));
    // 0042616f  d8631c                 +fsub dword ptr [ebx + 0x1c]
    x86::Float fpu47 = cpu.fpu.sub(fpu46, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(28) /* 0x1c */)));
    // 00426172  d9ee                   +fldz 
    x86::Float fpu48 = 0.0;
    // 00426174  d9c9                   +fxch st(1)
    
    // 00426176  d95d66                 +fstp dword ptr [ebp + 0x66]
    app->getMemory<float>(cpu.ebp + x86::reg32(102) /* 0x66 */) = float(fpu47);
    // 00426179  d85d66                 +fcomp dword ptr [ebp + 0x66]
    cpu.fpu.compare(fpu48, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(102) /* 0x66 */)));
    // 0042617c  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 0042617e  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 0042617f  760a                   -jbe 0x42618b
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0042618b;
    }
    // 00426181  d94566                 +fld dword ptr [ebp + 0x66]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu49 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(102) /* 0x66 */));
    // 00426184  d9e0                   +fchs 
    x86::Float fpu50 = -fpu49;
    // 00426186  d95d4a                 +fstp dword ptr [ebp + 0x4a]
    app->getMemory<float>(cpu.ebp + x86::reg32(74) /* 0x4a */) = float(fpu50);
    }
    // 00426189  eb06                   -jmp 0x426191
    goto L_0x00426191;
L_0x0042618b:
    // 0042618b  8b4566                 -mov eax, dword ptr [ebp + 0x66]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 0042618e  89454a                 -mov dword ptr [ebp + 0x4a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */) = cpu.eax;
L_0x00426191:
    // 00426191  d9454a                 +fld dword ptr [ebp + 0x4a]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu51 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(74) /* 0x4a */));
    // 00426194  d80d5c6f5300           +fmul dword ptr [0x536f5c]
    x86::Float fpu52 = cpu.fpu.mul(fpu51, x86::Float(app->getMemory<float>(x86::reg32(5467996) /* 0x536f5c */)));
    // 0042619a  8b4304                 -mov eax, dword ptr [ebx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0042619d  d9ee                   +fldz 
    x86::Float fpu53 = 0.0;
    // 0042619f  d98098000000           +fld dword ptr [eax + 0x98]
    x86::Float fpu54 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(152) /* 0x98 */));
    // 004261a5  d86314                 +fsub dword ptr [ebx + 0x14]
    x86::Float fpu55 = cpu.fpu.sub(fpu54, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(20) /* 0x14 */)));
    // 004261a8  d9ca                   +fxch st(2)
    
    // 004261aa  d95d2a                 +fstp dword ptr [ebp + 0x2a]
    app->getMemory<float>(cpu.ebp + x86::reg32(42) /* 0x2a */) = float(fpu52);
    // 004261ad  d9c9                   +fxch st(1)
    
    // 004261af  d95d6a                 +fstp dword ptr [ebp + 0x6a]
    app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */) = float(fpu55);
    // 004261b2  d85d6a                 +fcomp dword ptr [ebp + 0x6a]
    cpu.fpu.compare(fpu53, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 004261b5  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 004261b7  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 004261b8  760a                   -jbe 0x4261c4
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004261c4;
    }
    // 004261ba  d9456a                 +fld dword ptr [ebp + 0x6a]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu56 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */));
    // 004261bd  d9e0                   +fchs 
    x86::Float fpu57 = -fpu56;
    // 004261bf  d95d3a                 +fstp dword ptr [ebp + 0x3a]
    app->getMemory<float>(cpu.ebp + x86::reg32(58) /* 0x3a */) = float(fpu57);
    }
    // 004261c2  eb06                   -jmp 0x4261ca
    goto L_0x004261ca;
L_0x004261c4:
    // 004261c4  8b456a                 -mov eax, dword ptr [ebp + 0x6a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 004261c7  89453a                 -mov dword ptr [ebp + 0x3a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(58) /* 0x3a */) = cpu.eax;
L_0x004261ca:
    // 004261ca  d9453a                 +fld dword ptr [ebp + 0x3a]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu58 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(58) /* 0x3a */));
    // 004261cd  d8452a                 +fadd dword ptr [ebp + 0x2a]
    x86::Float fpu59 = cpu.fpu.add(fpu58, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(42) /* 0x2a */)));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu59;
    }
    // 004261d0  eb6a                   -jmp 0x42623c
    goto L_0x0042623c;
L_0x004261d2:
    // 004261d2  8b4304                 -mov eax, dword ptr [ebx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 004261d5  d98098000000           +fld dword ptr [eax + 0x98]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu60 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(152) /* 0x98 */));
    // 004261db  d86314                 +fsub dword ptr [ebx + 0x14]
    x86::Float fpu61 = cpu.fpu.sub(fpu60, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(20) /* 0x14 */)));
    // 004261de  d9ee                   +fldz 
    x86::Float fpu62 = 0.0;
    // 004261e0  d9c9                   +fxch st(1)
    
    // 004261e2  d95d6e                 +fstp dword ptr [ebp + 0x6e]
    app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */) = float(fpu61);
    // 004261e5  d85d6e                 +fcomp dword ptr [ebp + 0x6e]
    cpu.fpu.compare(fpu62, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */)));
    // 004261e8  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 004261ea  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 004261eb  760a                   -jbe 0x4261f7
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004261f7;
    }
    // 004261ed  d9456e                 +fld dword ptr [ebp + 0x6e]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu63 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */));
    // 004261f0  d9e0                   +fchs 
    x86::Float fpu64 = -fpu63;
    // 004261f2  d95d36                 +fstp dword ptr [ebp + 0x36]
    app->getMemory<float>(cpu.ebp + x86::reg32(54) /* 0x36 */) = float(fpu64);
    }
    // 004261f5  eb06                   -jmp 0x4261fd
    goto L_0x004261fd;
L_0x004261f7:
    // 004261f7  8b456e                 -mov eax, dword ptr [ebp + 0x6e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */);
    // 004261fa  894536                 -mov dword ptr [ebp + 0x36], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(54) /* 0x36 */) = cpu.eax;
L_0x004261fd:
    // 004261fd  d94536                 +fld dword ptr [ebp + 0x36]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu65 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(54) /* 0x36 */));
    // 00426200  d80d5c6f5300           +fmul dword ptr [0x536f5c]
    x86::Float fpu66 = cpu.fpu.mul(fpu65, x86::Float(app->getMemory<float>(x86::reg32(5467996) /* 0x536f5c */)));
    // 00426206  8b4304                 -mov eax, dword ptr [ebx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 00426209  d9ee                   +fldz 
    x86::Float fpu67 = 0.0;
    // 0042620b  d980a0000000           +fld dword ptr [eax + 0xa0]
    x86::Float fpu68 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(160) /* 0xa0 */));
    // 00426211  d8631c                 +fsub dword ptr [ebx + 0x1c]
    x86::Float fpu69 = cpu.fpu.sub(fpu68, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(28) /* 0x1c */)));
    // 00426214  d9ca                   +fxch st(2)
    
    // 00426216  d95d2e                 +fstp dword ptr [ebp + 0x2e]
    app->getMemory<float>(cpu.ebp + x86::reg32(46) /* 0x2e */) = float(fpu66);
    // 00426219  d9c9                   +fxch st(1)
    
    // 0042621b  d95d52                 +fstp dword ptr [ebp + 0x52]
    app->getMemory<float>(cpu.ebp + x86::reg32(82) /* 0x52 */) = float(fpu69);
    // 0042621e  d85d52                 +fcomp dword ptr [ebp + 0x52]
    cpu.fpu.compare(fpu67, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(82) /* 0x52 */)));
    // 00426221  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00426223  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00426224  760a                   -jbe 0x426230
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00426230;
    }
    // 00426226  d94552                 +fld dword ptr [ebp + 0x52]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu70 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(82) /* 0x52 */));
    // 00426229  d9e0                   +fchs 
    x86::Float fpu71 = -fpu70;
    // 0042622b  d95d46                 +fstp dword ptr [ebp + 0x46]
    app->getMemory<float>(cpu.ebp + x86::reg32(70) /* 0x46 */) = float(fpu71);
    }
    // 0042622e  eb06                   -jmp 0x426236
    goto L_0x00426236;
L_0x00426230:
    // 00426230  8b4552                 -mov eax, dword ptr [ebp + 0x52]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00426233  894546                 -mov dword ptr [ebp + 0x46], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */) = cpu.eax;
L_0x00426236:
    // 00426236  d94546                 +fld dword ptr [ebp + 0x46]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu72 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(70) /* 0x46 */));
    // 00426239  d8452e                 +fadd dword ptr [ebp + 0x2e]
    x86::Float fpu73 = cpu.fpu.add(fpu72, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(46) /* 0x2e */)));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu73;
    }
L_0x0042623c:
    // 0042623c  d95d4e                 +fstp dword ptr [ebp + 0x4e]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu74 = cpu.fpu.st(0);
    app->getMemory<float>(cpu.ebp + x86::reg32(78) /* 0x4e */) = float(fpu74);
    // 0042623f  d9454e                 +fld dword ptr [ebp + 0x4e]
    x86::Float fpu75 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(78) /* 0x4e */));
    // 00426242  d80d5c6f5300           +fmul dword ptr [0x536f5c]
    x86::Float fpu76 = cpu.fpu.mul(fpu75, x86::Float(app->getMemory<float>(x86::reg32(5467996) /* 0x536f5c */)));
    // 00426248  d905606f5300           +fld dword ptr [0x536f60]
    x86::Float fpu77 = x86::Float(app->getMemory<float>(x86::reg32(5468000) /* 0x536f60 */));
    // 0042624e  d9c9                   +fxch st(1)
    
    // 00426250  d95d76                 +fstp dword ptr [ebp + 0x76]
    app->getMemory<float>(cpu.ebp + x86::reg32(118) /* 0x76 */) = float(fpu76);
    // 00426253  d85d76                 +fcomp dword ptr [ebp + 0x76]
    cpu.fpu.compare(fpu77, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(118) /* 0x76 */)));
    // 00426256  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00426258  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    cpu.fpu.count -= 1;
    }
    // 00426259  7309                   -jae 0x426264
    if (!cpu.flags.cf)
    {
        goto L_0x00426264;
    }
    // 0042625b  c7453e00006040         -mov dword ptr [ebp + 0x3e], 0x40600000
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */) = 1080033280 /*0x40600000*/;
    // 00426262  eb06                   -jmp 0x42626a
    goto L_0x0042626a;
L_0x00426264:
    // 00426264  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 00426267  89453e                 -mov dword ptr [ebp + 0x3e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */) = cpu.eax;
L_0x0042626a:
    // 0042626a  d905386f5300           -fld dword ptr [0x536f38]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu78 = x86::Float(app->getMemory<float>(x86::reg32(5467960) /* 0x536f38 */));
    // 00426270  d8653e                 -fsub dword ptr [ebp + 0x3e]
    x86::Float fpu79 = cpu.fpu.sub(fpu78, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(62) /* 0x3e */)));
    // 00426273  d84318                 -fadd dword ptr [ebx + 0x18]
    x86::Float fpu80 = cpu.fpu.add(fpu79, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(24) /* 0x18 */)));
    // 00426276  d95b18                 -fstp dword ptr [ebx + 0x18]
    app->getMemory<float>(cpu.ebx + x86::reg32(24) /* 0x18 */) = float(fpu80);
    }
L_0x00426279:
    // 00426279  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 0042627f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00426280  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00426281  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00426282  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00426283  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00426284  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00426285  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_426290(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00426290  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00426291  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00426293  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00426294  c3                     -ret 
    cpu.esp += 4;
    return;
}

}
