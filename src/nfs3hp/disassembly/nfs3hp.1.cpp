#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_405af0: native, nfs3hp::aidrive::nearHuman */

/* sub_405b50: native, nfs3hp::dead::stub_0x405b50 */

/* sub_405c20: native, nfs3hp::dead::stub_0x405c20 */

/* sub_405c80: native, nfs3hp::aidrive::humanNearSlice */

/* sub_405cd0: native, nfs3hp::aidrive::trafficUnwatched */

/* sub_405d40: native, nfs3hp::aidrive::trafficStart */

/* sub_405dc0: native, nfs3hp::aidrive::trafficStop */

/* sub_405e20: native, nfs3hp::aidrive::extraCarStep */

/* sub_405e50: native, nfs3hp::dead::stub_0x405e50 */

/* sub_405e60: native, nfs3hp::aidrive::setDrivingConstants */

/* sub_405fb0: native, nfs3hp::aidrive::drivingNothing */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_405fc0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00405fc0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00405fc1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00405fc3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00405fc4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_405fd0: native, nfs3hp::aidrive::setReverse */

/* sub_4060d0: native, nfs3hp::aidrive::reverseCheck */

/* sub_406120: native, nfs3hp::aidrive::reverseWhenSlow */

/* sub_406160: native, nfs3hp::aidrive::reverseWhenTipped */

/* sub_4061b0: native, nfs3hp::aidrive::engineRevs */

/* sub_4063e0: native, nfs3hp::aidrive::brakeForce */

/* sub_4064f0: native, nfs3hp::frontend::simEnginePull */

/* sub_4066c0: native, nfs3hp::aidrive::speedAlongRoad */

/* sub_406880: native, nfs3hp::aidrive::laneError */

/* sub_4068a0: native, nfs3hp::aidrive::glideAlongRoad */

/* sub_406980: native, nfs3hp::aidrive::lookAheadSlice */

/* sub_406a80: native, nfs3hp::aidrive::sidewaysToRoad */

/* sub_406b00: native, nfs3hp::aidrive::towardTarget */

/* sub_406b60: native, nfs3hp::aidrive::towardTargetAhead */

/* sub_406c00: native, nfs3hp::aidrive::towardSlice */

/* sub_406c90: native, nfs3hp::aidrive::towardSliceSlowing */

/* sub_406d40: native, nfs3hp::aidrive::towardLane */

/* sub_406e50: native, nfs3hp::aidrive::wayToGo */

/* sub_4070c0: native, nfs3hp::frontend::simHeadingError */

/* sub_4071a0: native, nfs3hp::aidrive::integrate */

/* sub_407380: native, nfs3hp::aidrive::coastAfterFinish */

/* sub_4076b0: native, nfs3hp::aidrive::steerOnRoad */

/* sub_407c30: native, nfs3hp::aidrive::steerReversing */

/* sub_407fa0: native, nfs3hp::aidrive::steerOffRoad */

/* sub_408520: native, nfs3hp::aidrive::rideWithTarget */

/* sub_408600: native, nfs3hp::aidrive::aiPhysics */

/* sub_408870: native, nfs3hp::aidrive::easeLane */

/* sub_4088f0: native, nfs3hp::aidrive::aiStep */

/* sub_408ad0: native, nfs3hp::pursuit::raceStartHook */

/* sub_408ae0: native, nfs3hp::pursuit::raceEndHook */

/* sub_408af0: native, nfs3hp::dead::stub_0x408af0 */

/* sub_408b30: native, nfs3hp::pursuit::roadsideTarget */

/* sub_408bd0: native, nfs3hp::pursuit::countLaneTime */

/* sub_408c50: native, nfs3hp::pursuit::pulledOverTarget */

/* sub_408dc0: native, nfs3hp::pursuit::classSpeedFactor */

/* sub_408f00: native, nfs3hp::pursuit::unitFactor */

/* sub_408f10: native, nfs3hp::pursuit::radarFactor */

/* sub_409090: native, nfs3hp::pursuit::unitFactorToo */

/* sub_4090a0: native, nfs3hp::pursuit::leaderFactor */

/* sub_409290: native, nfs3hp::pursuit::nodeTargetSpeed */

/* sub_4092f0: native, nfs3hp::pursuit::markChased */

/* sub_409330: native, nfs3hp::pursuit::targetSpeed */

/* sub_4094b0: native, nfs3hp::pursuit::targetSpeedOnly */

/* sub_4094ba: native, nfs3hp::dead::stub_0x4094ba */

/* sub_4094d0: native, nfs3hp::pursuit::matchChasedSpeed */

/* sub_4095d0: native, nfs3hp::pursuit::scoreCloseCops */

/* sub_409660: native, nfs3hp::pursuit::growLaneTimes */

/* sub_4096b0: native, nfs3hp::pursuit::scoreLane */

/* sub_4097c0: native, nfs3hp::pursuit::scoreChases */

/* sub_409840: native, nfs3hp::pursuit::radioChaseCount */

/* sub_409920: native, nfs3hp::pursuit::scoreSpeeds */

/* sub_409a80: native, nfs3hp::pursuit::alwaysOne */

/* sub_409a90: native, nfs3hp::pursuit::takeOverCars */

/* sub_409b40: native, nfs3hp::pursuit::startReset */

/* sub_409b70: native, nfs3hp::pursuit::checkProgress */

/* sub_409c00: native, nfs3hp::pursuit::copStep */

/* sub_40a010: native, nfs3hp::pursuit::loadRacerBanks */

/* sub_40a330: native, nfs3hp::pursuit::countRadioFrames */

/* sub_40a380: native, nfs3hp::pursuit::radioNoise */

/* sub_40a410: native, nfs3hp::pursuit::sayPhrase */

/* sub_40a580: native, nfs3hp::pursuit::sayCarPhrase */

/* sub_40a6d0: native, nfs3hp::pursuit::namePhrase */

/* sub_40a6f0: native, nfs3hp::pursuit::namePhrases */

/* sub_40a910: native, nfs3hp::pursuit::radioFree */

/* sub_40a940: native, nfs3hp::pursuit::radioNothing */

/* sub_40a950: native, nfs3hp::pursuit::quietVoice */

/* sub_40a970: native, nfs3hp::pursuit::sayVoicePhrase */

/* sub_40aa20: native, nfs3hp::pursuit::sayCallSign */

/* sub_40aa70: native, nfs3hp::pursuit::sayPhraseOnly */

/* sub_40aa90: native, nfs3hp::pursuit::sayUnit */

/* sub_40ab20: native, nfs3hp::pursuit::sayDescription */

/* sub_40aea0: native, nfs3hp::pursuit::radioNothingToo */

/* sub_40aeb0: native, nfs3hp::pursuit::sayLocation */

/* sub_40b190: native, nfs3hp::pursuit::saySpeed */

/* sub_40b220: native, nfs3hp::pursuit::radioSpotted */

/* sub_40b330: native, nfs3hp::pursuit::radioPursuit */

/* sub_40b480: native, nfs3hp::pursuit::radioLost */

/* sub_40b530: native, nfs3hp::pursuit::radioBackup */

/* sub_40b670: native, nfs3hp::pursuit::radioPatrol */

/* sub_40b720: native, nfs3hp::pursuit::radioRoadblock */

/* sub_40b7f0: native, nfs3hp::pursuit::radioNothingThree */

/* sub_40b7f9: native, nfs3hp::dead::stub_0x40b7f9 */

/* sub_40b860: native, nfs3hp::pursuit::radioTicket */

/* sub_40b910: native, nfs3hp::pursuit::radioArrest */

/* sub_40b9c0: native, nfs3hp::pursuit::radioStopped */

/* sub_40bac0: native, nfs3hp::pursuit::radioAmbulance */

/* sub_40bba0: native, nfs3hp::pursuit::radioSpikes */

/* sub_40bc30: native, nfs3hp::pursuit::radioHelp */

}
