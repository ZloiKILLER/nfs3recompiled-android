#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_40bca0: native, nfs3hp::pursuit::radioResume */

/* sub_40bd00: native, nfs3hp::pursuit::radioAbort */

/* sub_40bd60: native, nfs3hp::dead::stub_0x40bd60 */

/* sub_40bd70: native, nfs3hp::dead::stub_0x40bd70 */

/* sub_40bd80: native, nfs3hp::pursuit::resetOnTrack */

/* sub_40bf50: native, nfs3hp::pursuit::copRestart */

/* sub_40bfe0: native, nfs3hp::pursuit::topSpeedFactor */

/* sub_40bff0: native, nfs3hp::pursuit::followChased */

/* sub_40c140: native, nfs3hp::pursuit::raceEndHookToo */

/* sub_40c150: native, nfs3hp::pursuit::raceStartHookToo */

/* sub_40c160: native, nfs3hp::pursuit::followCar */

/* sub_40c210: native, nfs3hp::pursuit::countdownDone */

/* sub_40c270: native, nfs3hp::pursuit::trafficTarget */

/* sub_40c2f0: native, nfs3hp::pursuit::trafficStep */

/* sub_40c4f0: native, nfs3hp::aiplan::leastTrafficPlayer */

/* sub_40c590: native, nfs3hp::aiplan::placeTrafficCar */

/* sub_40c630: native, nfs3hp::aiplan::reactToTraffic */

/* sub_40c780: native, nfs3hp::aiplan::slowForPolice */

/* sub_40c7f0: native, nfs3hp::aiplan::countStopped */

/* sub_40c830: native, nfs3hp::aiplan::nearPoliceCar */

/* sub_40c8a0: native, nfs3hp::aiplan::weighLaneVotes */

/* sub_40c8d0: native, nfs3hp::aiplan::voteRoadside */

/* sub_40c950: native, nfs3hp::aiplan::atRoadside */

/* sub_40c9f0: native, nfs3hp::aiplan::advanceCarAi */

/* sub_40cb20: native, nfs3hp::aiplan::clearSensedCars */

/* sub_40cb50: native, nfs3hp::aiplan::cacheTrafficSide */

/* sub_40cb70: native, nfs3hp::aiplan::resetTrafficCar */

/* sub_40cb90: native, nfs3hp::aiplan::clearTrafficRecord */

/* sub_40cd80: native, nfs3hp::aiplan::placeOnTrack */

/* sub_40ce70: native, nfs3hp::aiplan::brakeToStop */

/* sub_40cee0: native, nfs3hp::aiplan::voteCurrentLane */

/* sub_40cfc0: native, nfs3hp::aiplan::senseTraffic */

/* sub_40d1f0: native, nfs3hp::aiplan::planLaneChange */

/* sub_40d2e0: native, nfs3hp::aiplan::edgeLaneCost */

/* sub_40d3d0: native, nfs3hp::aiplan::scoreLanesByRoad */

/* sub_40d720: native, nfs3hp::aiplan::scoreLanesByObstacles */

/* sub_40d800: native, nfs3hp::aiplan::scoreLanesByCars */

/* sub_40d9f0: native, nfs3hp::aiplan::edgePenalty */

/* sub_40da30: native, nfs3hp::aiplan::sumLaneVotes */

/* sub_40daf0: native, nfs3hp::aiplan::steerToLane */

/* sub_40dd60: native, nfs3hp::aiplan::beginLanePlanning */

/* sub_40ddb0: native, nfs3hp::aiplan::chooseLane */

/* sub_40de30: native, nfs3hp::frontend::simLaneSteer */

/* sub_40de50: native, nfs3hp::aiplan::startHorn */

/* sub_40deb0: native, nfs3hp::aiplan::stopHorn */

/* sub_40df10: native, nfs3hp::aiplan::stepTrafficLoading */

/* sub_40e000: native, nfs3hp::load::loadOpponentSpeeds */

/* sub_40e080: native, nfs3hp::aiplan::targetSpeedAt */

/* sub_40e0c0: native, nfs3hp::aiplan::roadKindAt */

/* sub_40e0f0: native, nfs3hp::aiplan::cacheRoadWidth */

/* sub_40e120: native, nfs3hp::aiplan::nodeGap */

/* sub_40e160: native, nfs3hp::aiplan::nodeOffset */

/* sub_40e1b0: native, nfs3hp::aiplan::nodesAheadBySpeed */

/* sub_40e200: native, nfs3hp::aiplan::nodeLaneBits */

/* sub_40e220: native, nfs3hp::aiplan::nodeHasLane */

/* sub_40e270: native, nfs3hp::aiplan::laneBit */

/* sub_40e2b0: native, nfs3hp::aiplan::roadSliceIndex */

/* sub_40e350: native, nfs3hp::aiplan::carSlices */

/* sub_40e3c0: native, nfs3hp::aiplan::fillTrackTuning */

/* sub_40e410: native, nfs3hp::aiplan::setupTrackTuning */

/* sub_40ec40: native, nfs3hp::aiplan::freeSpeedTable */

/* sub_40ec70: native, nfs3hp::aiplan::trafficSide */

/* sub_40eca0: native, nfs3hp::aiplan::roadIsWide */

/* sub_40ece0: native, nfs3hp::dead::stub_0x40ece0 */

/* sub_40ed40: native, nfs3hp::aiplan::trafficSpeedAt */

/* sub_40ee80: native, nfs3hp::aiplan::tuningSpeed */

/* sub_40eed0: native, nfs3hp::aiplan::tuningDistance */

/* sub_40ef10: native, nfs3hp::aiplan::tuningFirst */

/* sub_40ef30: native, nfs3hp::dead::stub_0x40ef30 */

/* sub_40ef70: native, nfs3hp::aiplan::nodeDirectionDot */

/* sub_40f000: native, nfs3hp::aiplan::nodeWithinRange */

/* sub_40f050: native, nfs3hp::aiplan::carWithinReach */

/* sub_40f0c0: native, nfs3hp::dead::stub_0x40f0c0 */

/* sub_40f100: native, nfs3hp::aiplan::alwaysOne */

/* sub_40f110: native, nfs3hp::aiplan::carBoxedIn */

/* sub_40f230: native, nfs3hp::load::clearTrackCameras */

/* sub_40f280: native, nfs3hp::load::loadTrackCameras */

/* sub_40f330: native, nfs3hp::aiplan::freeAnimationPaths */

/* sub_40f370: native, nfs3hp::aiplan::findFreeAnimation */

/* sub_40f3b0: native, nfs3hp::aiplan::startAnimation */

/* sub_40f430: native, nfs3hp::aiplan::setAnimationMode */

/* sub_40f460: native, nfs3hp::aiplan::stopAnimation */

/* sub_40f4b0: native, nfs3hp::aiplan::animationPose */

/* sub_40f6d0: native, nfs3hp::aiplan::animationPlace */

/* sub_40f800: native, nfs3hp::aiplan::animationProgress */

/* sub_40f880: native, nfs3hp::aiplan::animationLastPose */

/* sub_40f8d0: native, nfs3hp::aiplan::pathEndPose */

/* sub_40f940: native, nfs3hp::aiplan::animationStep */

/* sub_40fa10: native, nfs3hp::dead::stub_0x40fa10 */

/* sub_40fac0: native, nfs3hp::aiplan::animationPlaying */

/* sub_40faf0: native, nfs3hp::Is3DSoundAvailable */

/* sub_40fb10: native, nfs3hp::aiplan::startSound */

/* sub_40fcf0: native, nfs3hp::aiplan::restartSound */

/* sub_40feb0: native, nfs3hp::port::soundExit */

/* sub_40fed0: native, nfs3hp::port::deleteMusicCopy */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_40ff20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0040ff20  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0040ff21  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0040ff22  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0040ff23  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0040ff24  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0040ff25  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0040ff27  81ec8c000000           -sub esp, 0x8c
    (cpu.esp) -= x86::reg32(x86::sreg32(140 /*0x8c*/));
    // 0040ff2d  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 0040ff33  6834277a00             -push 0x7a2734
    app->getMemory<x86::reg32>(cpu.esp-4) = 8005428 /*0x7a2734*/;
    cpu.esp -= 4;
    // 0040ff38  6880685300             -push 0x536880
    app->getMemory<x86::reg32>(cpu.esp-4) = 5466240 /*0x536880*/;
    cpu.esp -= 4;
    // 0040ff3d  8d45f6                 -lea eax, [ebp - 0xa]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 0040ff40  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0040ff41  e84af70c00             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0040ff46  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0040ff49  8d45f6                 -lea eax, [ebp - 0xa]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 0040ff4c  be0000f000             -mov esi, 0xf00000
    cpu.esi = 15728640 /*0xf00000*/;
    // 0040ff51  e80a0f0d00             -call 0x4e0e60
    cpu.esp -= 4;
    nfs3hp::ea::fileExists(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0040ff56  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0040ff58  7415                   -je 0x40ff6f
    if (cpu.flags.zf)
    {
        goto L_0x0040ff6f;
    }
    // 0040ff5a  6a20                   -push 0x20
    app->getMemory<x86::reg32>(cpu.esp-4) = 32 /*0x20*/;
    cpu.esp -= 4;
    // 0040ff5c  8d45f6                 -lea eax, [ebp - 0xa]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 0040ff5f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0040ff60  2eff15ec455300         -call dword ptr cs:[0x5345ec]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457388) /* 0x5345ec */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0040ff67  8d45f6                 -lea eax, [ebp - 0xa]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 0040ff6a  e831290d00             -call 0x4e28a0
    cpu.esp -= 4;
    nfs3hp::watcom::unlink(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0040ff6f:
    // 0040ff6f  8d4d7a                 -lea ecx, [ebp + 0x7a]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 0040ff72  8d5d7e                 -lea ebx, [ebp + 0x7e]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0040ff75  8d5576                 -lea edx, [ebp + 0x76]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 0040ff78  8d45f6                 -lea eax, [ebp - 0xa]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-10) /* -0xa */);
    // 0040ff7b  e8602b0d00             -call 0x4e2ae0
    cpu.esp -= 4;
    nfs3hp::ea::openHandleToWrite(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0040ff80  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 0040ff83  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0040ff85  e8a6420d00             -call 0x4e4230
    cpu.esp -= 4;
    sub_4e4230(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0040ff8a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0040ff8c  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 0040ff8f  e80c2e0d00             -call 0x4e2da0
    cpu.esp -= 4;
    nfs3hp::ea::closeHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0040ff94  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0040ff96  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 0040ff9c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0040ff9d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0040ff9e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0040ff9f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0040ffa0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0040ffa1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_40ffd0: native, nfs3hp::port::raceTuneOf */

/* sub_410030: native, nfs3hp::port::raceMusicPrepare */

/* sub_410330: native, nfs3hp::port::musicStreamMake */

/* sub_410370: native, nfs3hp::port::musicStreamMakeOf */

}
