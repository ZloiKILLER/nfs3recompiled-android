#ifndef NFS3HP_H_
#define NFS3HP_H_
#include <x86.h>
#include <lib/winapp.h>
#include <lib/memmap.h>
#include <map>

namespace nfs3hp
{

/* The game's functions the port has natively, where their code was
 * (disassemble_nfs3hp.py): the generated code calls them by name. */
void Error(win32::WinApplication* app, x86::CPU& cpu);  // 0x401010
namespace airace { void nothingAtStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401220
namespace airace { void nothingAtLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401230
namespace airace { void releaseEdge(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401240
namespace airace { void setUpCops(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4012c0
namespace airace { void radioOn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401320
namespace airace { void radioLevelDown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401330
namespace airace { void radioLevelUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401350
namespace airace { void copTopSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401370
namespace airace { void nearestCopGap(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4013b0
namespace airace { void setWantedSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401430
namespace airace { void noteSpeeding(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4014b0
namespace airace { void pulledOver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401500
namespace airace { void copRadioCall(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401520
namespace airace { void ticketRadio(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401580
namespace airace { void stopIfCopNear(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4015b0
namespace airace { void stopForCop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401650
namespace airace { void sizeUpTarget(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401780
namespace airace { void joinPursuit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4018b0
namespace airace { void leaveCopList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401a20
namespace airace { void giveUpPursuit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401a80
namespace airace { void pullOverSpot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401b50
namespace airace { void planPullOver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401c20
namespace airace { void endPullOver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401e20
namespace airace { void copStopsBy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401e40
namespace airace { void stopOver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401fb0
namespace airace { void giveTicket(win32::WinApplication* app, x86::CPU& cpu); }  // 0x401ff0
namespace airace { void targetStopping(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402110
namespace airace { void chaseSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402420
namespace airace { void pursueTarget(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4025c0
namespace airace { void votePursuitLane(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402750
namespace airace { void sideTowardNode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4028e0
namespace airace { void keepSideOfTarget(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402910
namespace airace { void placeCopAtStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4029c0
namespace airace { void placeCop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402a00
namespace airace { void alwaysOne(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402c70
namespace airace { void carReversing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402c80
namespace airace { void copHitByCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402cb0
namespace airace { void voteRoadLane(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402d90
namespace airace { void spotSpeeder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402e10
namespace airace { void laneOffRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402f70
namespace airace { void atOrderNode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x402fd0
namespace airace { void sideOfOrder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403000
namespace airace { void takeOrder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403030
namespace airace { void parkCop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403180
namespace airace { void drivePatrol(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403270
namespace airace { void copDriver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4032e0
namespace airace { void giveOrder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403830
namespace airace { void spreadCops(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403870
namespace airace { void radioStopped(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403910
namespace airace { void radioStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403950
namespace airace { void initPursuits(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403970
namespace airace { void callRoadblock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4039c0
namespace airace { void callTwoCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403b20
namespace airace { void roadblockSiren(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403d30
namespace airace { void watchRoadblock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403df0
namespace airace { void watchSecondCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403ed0
namespace airace { void copsWanted(win32::WinApplication* app, x86::CPU& cpu); }  // 0x403f80
namespace airace { void radioJoined(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404070
namespace airace { void radioGiveUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4040b0
namespace airace { void releaseCops(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4040e0
namespace airace { void dropCop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4042b0
namespace dead { void stub_0x404310(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404310
namespace airace { void sendCopAhead(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404340
namespace airace { void callBackup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4043a0
namespace airace { void orderTaken(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404560
namespace airace { void nothingForCop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404580
namespace dead { void stub_0x404590(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404590
namespace dead { void stub_0x4045a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4045a0
namespace dead { void stub_0x4045b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4045b0
namespace dead { void stub_0x4045c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4045c0
namespace dead { void stub_0x4045d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4045d0
namespace dead { void stub_0x4045e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4045e0
namespace dead { void stub_0x4045f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4045f0
namespace dead { void stub_0x404600(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404600
namespace airace { void classRule(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404610
namespace airace { void nothingAtEnd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404650
namespace airace { void nothingAtEndToo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404660
namespace airace { void setRammingRules(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404690
namespace airace { void classValue(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4047f0
namespace airace { void classValueToo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404810
namespace airace { void rivalTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404840
namespace airace { void carContact(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404a00
namespace airace { void slowForClass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404b90
namespace airace { void nothingAtStartToo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404c40
namespace airace { void catchUpSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404c50
namespace airace { void findLeader(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404c80
namespace aidrive { void copStartPlace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404d30
namespace aidrive { void callRoadblock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404e40
namespace aidrive { void sirenSwitch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404f80
namespace aidrive { void watchRacers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x404ff0
namespace aidrive { void jumpBehindLeader(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405220
namespace aidrive { void clockRacers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4052e0
namespace aidrive { void startPursuit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4053b0
namespace aidrive { void bustCheck(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405430
namespace aidrive { void pursuitStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405630
namespace aidrive { void dropPursuit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4056e0
namespace aidrive { void copDriver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405740
namespace aidrive { void nearCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405a20
namespace aidrive { void nearHuman(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405af0
namespace dead { void stub_0x405b50(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405b50
namespace dead { void stub_0x405c20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405c20
namespace aidrive { void humanNearSlice(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405c80
namespace aidrive { void trafficUnwatched(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405cd0
namespace aidrive { void trafficStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405d40
namespace aidrive { void trafficStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405dc0
namespace aidrive { void extraCarStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405e20
namespace dead { void stub_0x405e50(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405e50
namespace aidrive { void setDrivingConstants(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405e60
namespace aidrive { void drivingNothing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405fb0
namespace aidrive { void setReverse(win32::WinApplication* app, x86::CPU& cpu); }  // 0x405fd0
namespace aidrive { void reverseCheck(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4060d0
namespace aidrive { void reverseWhenSlow(win32::WinApplication* app, x86::CPU& cpu); }  // 0x406120
namespace aidrive { void reverseWhenTipped(win32::WinApplication* app, x86::CPU& cpu); }  // 0x406160
namespace aidrive { void engineRevs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4061b0
namespace aidrive { void brakeForce(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4063e0
namespace frontend { void simEnginePull(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4064f0
namespace aidrive { void speedAlongRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4066c0
namespace aidrive { void laneError(win32::WinApplication* app, x86::CPU& cpu); }  // 0x406880
namespace aidrive { void glideAlongRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4068a0
namespace aidrive { void lookAheadSlice(win32::WinApplication* app, x86::CPU& cpu); }  // 0x406980
namespace aidrive { void sidewaysToRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x406a80
namespace aidrive { void towardTarget(win32::WinApplication* app, x86::CPU& cpu); }  // 0x406b00
namespace aidrive { void towardTargetAhead(win32::WinApplication* app, x86::CPU& cpu); }  // 0x406b60
namespace aidrive { void towardSlice(win32::WinApplication* app, x86::CPU& cpu); }  // 0x406c00
namespace aidrive { void towardSliceSlowing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x406c90
namespace aidrive { void towardLane(win32::WinApplication* app, x86::CPU& cpu); }  // 0x406d40
namespace aidrive { void wayToGo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x406e50
namespace frontend { void simHeadingError(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4070c0
namespace aidrive { void integrate(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4071a0
namespace aidrive { void coastAfterFinish(win32::WinApplication* app, x86::CPU& cpu); }  // 0x407380
namespace aidrive { void steerOnRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4076b0
namespace aidrive { void steerReversing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x407c30
namespace aidrive { void steerOffRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x407fa0
namespace aidrive { void rideWithTarget(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408520
namespace aidrive { void aiPhysics(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408600
namespace aidrive { void easeLane(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408870
namespace aidrive { void aiStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4088f0
namespace pursuit { void raceStartHook(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408ad0
namespace pursuit { void raceEndHook(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408ae0
namespace dead { void stub_0x408af0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408af0
namespace pursuit { void roadsideTarget(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408b30
namespace pursuit { void countLaneTime(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408bd0
namespace pursuit { void pulledOverTarget(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408c50
namespace pursuit { void classSpeedFactor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408dc0
namespace pursuit { void unitFactor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408f00
namespace pursuit { void radarFactor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x408f10
namespace pursuit { void unitFactorToo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x409090
namespace pursuit { void leaderFactor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4090a0
namespace pursuit { void nodeTargetSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x409290
namespace pursuit { void markChased(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4092f0
namespace pursuit { void targetSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x409330
namespace pursuit { void targetSpeedOnly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4094b0
namespace dead { void stub_0x4094ba(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4094ba
namespace pursuit { void matchChasedSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4094d0
namespace pursuit { void scoreCloseCops(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4095d0
namespace pursuit { void growLaneTimes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x409660
namespace pursuit { void scoreLane(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4096b0
namespace pursuit { void scoreChases(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4097c0
namespace pursuit { void radioChaseCount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x409840
namespace pursuit { void scoreSpeeds(win32::WinApplication* app, x86::CPU& cpu); }  // 0x409920
namespace pursuit { void alwaysOne(win32::WinApplication* app, x86::CPU& cpu); }  // 0x409a80
namespace pursuit { void takeOverCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x409a90
namespace pursuit { void startReset(win32::WinApplication* app, x86::CPU& cpu); }  // 0x409b40
namespace pursuit { void checkProgress(win32::WinApplication* app, x86::CPU& cpu); }  // 0x409b70
namespace pursuit { void copStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x409c00
namespace pursuit { void loadRacerBanks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40a010
namespace pursuit { void countRadioFrames(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40a330
namespace pursuit { void radioNoise(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40a380
namespace pursuit { void sayPhrase(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40a410
namespace pursuit { void sayCarPhrase(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40a580
namespace pursuit { void namePhrase(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40a6d0
namespace pursuit { void namePhrases(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40a6f0
namespace pursuit { void radioFree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40a910
namespace pursuit { void radioNothing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40a940
namespace pursuit { void quietVoice(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40a950
namespace pursuit { void sayVoicePhrase(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40a970
namespace pursuit { void sayCallSign(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40aa20
namespace pursuit { void sayPhraseOnly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40aa70
namespace pursuit { void sayUnit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40aa90
namespace pursuit { void sayDescription(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ab20
namespace pursuit { void radioNothingToo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40aea0
namespace pursuit { void sayLocation(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40aeb0
namespace pursuit { void saySpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b190
namespace pursuit { void radioSpotted(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b220
namespace pursuit { void radioPursuit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b330
namespace pursuit { void radioLost(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b480
namespace pursuit { void radioBackup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b530
namespace pursuit { void radioPatrol(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b670
namespace pursuit { void radioRoadblock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b720
namespace pursuit { void radioNothingThree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b7f0
namespace dead { void stub_0x40b7f9(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b7f9
namespace pursuit { void radioTicket(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b860
namespace pursuit { void radioArrest(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b910
namespace pursuit { void radioStopped(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40b9c0
namespace pursuit { void radioAmbulance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40bac0
namespace pursuit { void radioSpikes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40bba0
namespace pursuit { void radioHelp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40bc30
namespace pursuit { void radioResume(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40bca0
namespace pursuit { void radioAbort(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40bd00
namespace dead { void stub_0x40bd60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40bd60
namespace dead { void stub_0x40bd70(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40bd70
namespace pursuit { void resetOnTrack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40bd80
namespace pursuit { void copRestart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40bf50
namespace pursuit { void topSpeedFactor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40bfe0
namespace pursuit { void followChased(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40bff0
namespace pursuit { void raceEndHookToo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c140
namespace pursuit { void raceStartHookToo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c150
namespace pursuit { void followCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c160
namespace pursuit { void countdownDone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c210
namespace pursuit { void trafficTarget(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c270
namespace pursuit { void trafficStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c2f0
namespace aiplan { void leastTrafficPlayer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c4f0
namespace aiplan { void placeTrafficCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c590
namespace aiplan { void reactToTraffic(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c630
namespace aiplan { void slowForPolice(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c780
namespace aiplan { void countStopped(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c7f0
namespace aiplan { void nearPoliceCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c830
namespace aiplan { void weighLaneVotes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c8a0
namespace aiplan { void voteRoadside(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c8d0
namespace aiplan { void atRoadside(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c950
namespace aiplan { void advanceCarAi(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40c9f0
namespace aiplan { void clearSensedCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40cb20
namespace aiplan { void cacheTrafficSide(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40cb50
namespace aiplan { void resetTrafficCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40cb70
namespace aiplan { void clearTrafficRecord(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40cb90
namespace aiplan { void placeOnTrack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40cd80
namespace aiplan { void brakeToStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ce70
namespace aiplan { void voteCurrentLane(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40cee0
namespace aiplan { void senseTraffic(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40cfc0
namespace aiplan { void planLaneChange(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40d1f0
namespace aiplan { void edgeLaneCost(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40d2e0
namespace aiplan { void scoreLanesByRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40d3d0
namespace aiplan { void scoreLanesByObstacles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40d720
namespace aiplan { void scoreLanesByCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40d800
namespace aiplan { void edgePenalty(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40d9f0
namespace aiplan { void sumLaneVotes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40da30
namespace aiplan { void steerToLane(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40daf0
namespace aiplan { void beginLanePlanning(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40dd60
namespace aiplan { void chooseLane(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ddb0
namespace frontend { void simLaneSteer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40de30
namespace aiplan { void startHorn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40de50
namespace aiplan { void stopHorn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40deb0
namespace aiplan { void stepTrafficLoading(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40df10
namespace load { void loadOpponentSpeeds(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e000
namespace aiplan { void targetSpeedAt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e080
namespace aiplan { void roadKindAt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e0c0
namespace aiplan { void cacheRoadWidth(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e0f0
namespace aiplan { void nodeGap(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e120
namespace aiplan { void nodeOffset(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e160
namespace aiplan { void nodesAheadBySpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e1b0
namespace aiplan { void nodeLaneBits(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e200
namespace aiplan { void nodeHasLane(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e220
namespace aiplan { void laneBit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e270
namespace aiplan { void roadSliceIndex(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e2b0
namespace aiplan { void carSlices(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e350
namespace aiplan { void fillTrackTuning(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e3c0
namespace aiplan { void setupTrackTuning(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40e410
namespace aiplan { void freeSpeedTable(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ec40
namespace aiplan { void trafficSide(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ec70
namespace aiplan { void roadIsWide(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40eca0
namespace dead { void stub_0x40ece0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ece0
namespace aiplan { void trafficSpeedAt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ed40
namespace aiplan { void tuningSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ee80
namespace aiplan { void tuningDistance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40eed0
namespace aiplan { void tuningFirst(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ef10
namespace dead { void stub_0x40ef30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ef30
namespace aiplan { void nodeDirectionDot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ef70
namespace aiplan { void nodeWithinRange(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f000
namespace aiplan { void carWithinReach(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f050
namespace dead { void stub_0x40f0c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f0c0
namespace aiplan { void alwaysOne(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f100
namespace aiplan { void carBoxedIn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f110
namespace load { void clearTrackCameras(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f230
namespace load { void loadTrackCameras(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f280
namespace aiplan { void freeAnimationPaths(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f330
namespace aiplan { void findFreeAnimation(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f370
namespace aiplan { void startAnimation(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f3b0
namespace aiplan { void setAnimationMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f430
namespace aiplan { void stopAnimation(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f460
namespace aiplan { void animationPose(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f4b0
namespace aiplan { void animationPlace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f6d0
namespace aiplan { void animationProgress(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f800
namespace aiplan { void animationLastPose(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f880
namespace aiplan { void pathEndPose(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f8d0
namespace aiplan { void animationStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40f940
namespace dead { void stub_0x40fa10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40fa10
namespace aiplan { void animationPlaying(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40fac0
void Is3DSoundAvailable(win32::WinApplication* app, x86::CPU& cpu);  // 0x40faf0
namespace aiplan { void startSound(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40fb10
namespace aiplan { void restartSound(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40fcf0
namespace port { void soundExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40feb0
namespace port { void deleteMusicCopy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40fed0
namespace port { void raceTuneOf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x40ffd0
namespace port { void raceMusicPrepare(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410030
namespace port { void musicStreamMake(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410330
namespace port { void musicStreamMakeOf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410370
namespace port { void musicAfterSetting(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4103a0
namespace port { void musicMenuSix(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410460
namespace frontend { void carMenuMusicBack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4104b0
namespace port { void musicStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4104f0
namespace port { void raceMusicStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4107c0
namespace port { void musicStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4109d0
namespace dead { void stub_0x410a40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410a40
namespace port { void musicStreamEnd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410aa0
namespace port { void banksFree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410ad0
namespace port { void banksRemove(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410b20
namespace port { void bankLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410b50
namespace dead { void stub_0x410c30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410c30
namespace port { void listenerMove(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410c40
namespace port { void sourceDistance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410cb0
namespace port { void sourceAngle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410d30
namespace port { void carAngle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410dc0
namespace port { void trackSounds(win32::WinApplication* app, x86::CPU& cpu); }  // 0x410e30
namespace port { void raceSoundTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x411410
namespace port { void sourceReset(win32::WinApplication* app, x86::CPU& cpu); }  // 0x411620
namespace port { void sourcesReset(win32::WinApplication* app, x86::CPU& cpu); }  // 0x411680
namespace port { void copChaseSet(win32::WinApplication* app, x86::CPU& cpu); }  // 0x411750
namespace port { void playerCarSound(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4117a0
namespace port { void copSiren(win32::WinApplication* app, x86::CPU& cpu); }  // 0x411c50
namespace port { void copSirenStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x411ca0
namespace port { void copEngine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x411cb0
namespace port { void copEngineStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x411da0
namespace port { void copDistance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x411db0
namespace port { void copsCloseness(win32::WinApplication* app, x86::CPU& cpu); }  // 0x411e20
namespace port { void copCarSound(win32::WinApplication* app, x86::CPU& cpu); }  // 0x411e60
namespace port { void copRemove(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4121b0
namespace port { void copSourcesPick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x412220
namespace port { void placeAhead(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4124d0
namespace port { void placeAcross(win32::WinApplication* app, x86::CPU& cpu); }  // 0x412620
namespace port { void carFlagTest(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4127b0
namespace port { void carRadio(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4127c0
namespace port { void playerCopRadio(win32::WinApplication* app, x86::CPU& cpu); }  // 0x412880
namespace port { void raceValue(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4129a0
namespace port { void soundNothing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4129b0
namespace port { void raceSoundInit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4129c0
namespace port { void raceSoundEnd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x412b50
namespace port { void copLineFor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x412be0
namespace port { void raceAnnouncer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x412d60
namespace port { void raceBankLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4136e0
namespace port { void voiceBankLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x413750
namespace port { void carBankLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4138e0
namespace port { void raceBanksLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x413bc0
namespace port { void slotsEmpty(win32::WinApplication* app, x86::CPU& cpu); }  // 0x414800
namespace port { void engineSoundsStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x414830
namespace port { void crashPitch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x414940
namespace port { void crashEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4149e0
namespace port { void scrapeEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x414d70
namespace port { void farEffectAim(win32::WinApplication* app, x86::CPU& cpu); }  // 0x414e10
namespace port { void farEffectPlay(win32::WinApplication* app, x86::CPU& cpu); }  // 0x414ef0
namespace port { void farEffectTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x414f60
namespace port { void crowdEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x414f90
namespace port { void carEffectFifteen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x415010
namespace port { void zoneSound(win32::WinApplication* app, x86::CPU& cpu); }  // 0x415050
namespace port { void scenerySounds(win32::WinApplication* app, x86::CPU& cpu); }  // 0x415210
namespace port { void carKnock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x416d90
namespace port { void slotStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x416f60
namespace port { void slotEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x416fc0
namespace port { void slotPlay(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417080
namespace port { void carEngine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417390
namespace port { void noiseBySpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4179d0
namespace port { void noiseByDistance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417b80
namespace port { void playerNoise(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417c00
namespace port { void playerNoiseStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417c70
namespace port { void sirenIsOn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417ca0
namespace port { void sirenFirstTone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417cb0
namespace port { void sirenFirstDone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417d00
namespace dead { void stub_0x417d50(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417d50
namespace port { void sirenSecondTone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417db0
namespace port { void sirenSteadyTone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417e00
namespace port { void sirenSecondDone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417e60
namespace port { void sirenStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417eb0
namespace port { void sirenStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417f60
namespace port { void sirenTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x417fd0
namespace port { void playMenuEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4181d0
namespace port { void playCarEffectFive(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418280
namespace port { void playEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4182b0
namespace port { void playSpeechEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4184f0
namespace port { void panFromSide(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4185e0
namespace port { void masterSilent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418610
namespace port { void masterBack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418620
namespace dead { void stub_0x418670(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418670
namespace port { void silenceForMenus(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4186a0
namespace port { void silenceForRace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4186f0
namespace port { void busInit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418740
namespace port { void busUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418770
namespace port { void busDown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418790
namespace port { void slotsStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4187a0
namespace port { void musicMuteToggle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4187e0
namespace decompiled { void swapRaceVolumes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418840
namespace port { void copBeepTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4188a0
namespace port { void copBeepStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418930
namespace port { void chaseLevel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418990
namespace port { void copEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418c30
namespace port { void musicEventResume(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418c60
namespace track { void shellOpen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418cc0
namespace dead { void stub_0x418cf0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418cf0
namespace load { void swapBigEndian(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418d00
namespace load { void newFileNode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418d40
namespace load { void freeFileNode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418d70
namespace dead { void stub_0x418db0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418db0
namespace load { void findFileNode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418dd0
namespace dead { void stub_0x418e00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418e00
namespace dead { void stub_0x418e20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418e20
namespace dead { void stub_0x418e60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418e60
namespace load { void insertFileNode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418e90
namespace load { void newEmptyFileNode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418f00
namespace load { void freeFileNodes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418f40
namespace load { void countFileNodes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418f70
namespace dead { void stub_0x418fa0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418fa0
namespace load { void findCachedFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x418fe0
namespace load { void addCachedFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419000
namespace load { void newFileCache(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419080
namespace load { void freeFileCache(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4190e0
namespace load { void bigHeaderSize(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419120
namespace load { void cachedDataSize(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4191b0
namespace load { void writeBigFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4191e0
namespace load { void packFileCache(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419460
namespace load { void readBigFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419510
namespace load { void openFileCache(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4195f0
namespace track { void tintColours(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419660
namespace track { void tintObjectList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419720
namespace track { void tintTrack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419750
namespace track { void greyColours(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4197c0
namespace track { void greyObjectList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419840
namespace track { void greyTrack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419870
namespace track { void readObjectList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4198e0
namespace track { void readLights(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419ab0
namespace track { void newArena(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419b60
namespace track { void freeArena(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419b90
namespace track { void arenaTake(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419bc0
namespace track { void frdRead(win32::WinApplication* app, x86::CPU& cpu); }  // 0x419bf0
namespace track { void loadGeometry(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41a100
namespace track { void projectedTriangle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41a120
namespace vertices { void projectedTriangle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41a360
namespace track { void shadeVertex(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41a3e0
namespace vertices { void trackVertices(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41a550
namespace track { void setShadeDistances(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41a800
namespace track { void viewObjectVertices(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41a8d0
namespace vertices { void objectVerticesColoured(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41a970
namespace track { void viewTurnedObjectVertices(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41a9b0
namespace track { void modelByDistance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41aa60
namespace vertices { void groundDistanceOf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41aab0
namespace track { void viewOnBlock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41aaf0
namespace track { void lightObjectPolygons(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41ab40
namespace vertices { void trackPolygons(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41ae90
namespace vertices { void trackRecords(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41b300
namespace vertices { void objectRecords(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41b640
namespace track { void bracketList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41b8c0
namespace render { void viewPass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41b9b0
namespace track { void ambientSounds(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41bb50
namespace track { void ambientWanted(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41bc20
namespace track { void pointBehindView(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41bc50
namespace track { void litBoxOutside(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41bc90
namespace vertices { void boxOutside(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41bf30
namespace track { void flushBuckets(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41c1a0
namespace track { void headlightsOnObjects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41c310
namespace track { void headlightsOnBlock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41c670
namespace track { void blockLights(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41c9f0
namespace track { void drawObjectList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41ca40
namespace track { void drawTrack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41cdc0
namespace track { void ambientSource(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d210
namespace track { void selectView(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d310
namespace track { void objectAnimation(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d350
namespace track { void turnFromAngles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d510
namespace track { void turnFromPitchYaw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d5a0
namespace track { void setViewDistance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d610
namespace render { void viewDistances(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d620
namespace track { void nextViewDistance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d870
namespace load { void trackFilePath(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d8a0
namespace load { void trackFolderPath(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d8d0
namespace track { void modelsUnlimited(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d900
namespace render { void raceViews(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41d960
namespace track { void freeAt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41da40
namespace track { void unloadTrack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41da60
namespace track { void newPool(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41daa0
namespace track { void newObjectPool(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41dae0
namespace track { void freeTrackKinds(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41db00
namespace track { void loadTextures(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41db10
namespace load { void loadTrackTextures(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41dbc0
namespace track { void takeTextures(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41dc60
namespace track { void loadCollisions(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41dcc0
namespace track { void freeCollisions(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41de10
namespace track { void clearObjectStates(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41de40
namespace track { void resetTrackViews(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41de80
namespace track { void resetViewsOuter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41dea0
namespace track { void resetViews(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41deb0
namespace render { void viewRecord(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41df10
namespace track { void blockInView(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e070
namespace vertices { void nearestBlock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e0f0
namespace trackdata { void loadCcmPoints(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e180
namespace trackdata { void freeCcmPoints(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e2c0
namespace trackdata { void loadSimHeights(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e2f0
namespace trackdata { void freeSimHeights(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e340
namespace trackdata { void openTrackNodes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e370
namespace trackdata { void closeTrackNodes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e3b0
namespace trackdata { void nearestNodeCoarse(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e3c0
namespace trackdata { void followNode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e420
namespace trackdata { void nearestNode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e470
namespace trackdata { void quadVertices(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e6a0
namespace trackdata { void quadStepForward(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e720
namespace trackdata { void quadStepBack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e770
namespace trackdata { void quadToFixed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e7c0
namespace trackdata { void quadSet(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e8d0
namespace trackdata { void nodeSection(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e8f0
namespace trackdata { void resetPosition(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e940
namespace trackdata { void findQuad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41e990
namespace trackdata { void findQuadNearby(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41f090
namespace trackdata { void findQuadByLinks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41f330
namespace trackdata { void findQuadFallback(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41f500
namespace trackdata { void locateOnRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41f650
namespace trackdata { void updatePosition(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41f6e0
namespace trackdata { void findMatchingQuad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41f870
namespace trackdata { void quadEdgeFlags(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41fd60
namespace trackdata { void sectionItemCount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41ff10
namespace trackdata { void sectionItem(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41ff20
namespace trackdata { void setLaneOffsets(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41ffa0
namespace dead { void stub_0x41ffb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41ffb0
namespace trackdata { void lanePosition(win32::WinApplication* app, x86::CPU& cpu); }  // 0x41ffd0
namespace trackdata { void positionFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x420230
namespace trackdata { void snapToTrack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4205a0
namespace trackdata { void placeOnTrack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x420650
namespace trackdata { void positionColour(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4206f0
namespace trackdata { void positionSurface(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4207e0
namespace trackdata { void surfaceUnder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x420840
namespace trackdata { void inTunnel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x420bf0
namespace trackdata { void carFacingWay(win32::WinApplication* app, x86::CPU& cpu); }  // 0x420d60
namespace port { namespace view { void sectionAheadIsTwo(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x420ff0
namespace trackdata { void carSectionKind(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4211c0
namespace carstate { void idleHook(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421360
namespace dead { void stub_0x421370(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421370
namespace carstate { void viewFromCamera(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421380
namespace carstate { void viewFromScreenCamera(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4213b0
namespace carstate { void chooseView(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421450
namespace carstate { void idleHookToo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421480
namespace carstate { void pickChaseNode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421490
namespace carstate { void turnAround(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421570
namespace carstate { void attachCameras(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4215c0
namespace carstate { void headTurn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421730
namespace carstate { void cameraNode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4217e0
namespace carstate { void cameraTrackPlace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421880
namespace carstate { void keepOutOfCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421900
namespace carstate { void keepAboveRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421b90
namespace carstate { void keepOnTrack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421cc0
namespace carstate { void keepFromRightWall(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421e80
namespace carstate { void keepFromLeftWall(win32::WinApplication* app, x86::CPU& cpu); }  // 0x421f90
namespace carstate { void keepAboveGround(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4220a0
namespace carstate { void lookAtTarget(win32::WinApplication* app, x86::CPU& cpu); }  // 0x422290
namespace carstate { void blendCamera(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4223c0
namespace carstate { void placeCamera(win32::WinApplication* app, x86::CPU& cpu); }  // 0x422520
namespace carstate { void switchCamera(win32::WinApplication* app, x86::CPU& cpu); }  // 0x422610
namespace carstate { void cameraZoom(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4227e0
namespace carstate { void viewMatrix(win32::WinApplication* app, x86::CPU& cpu); }  // 0x422850
namespace carstate { void updateCamera(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4228e0
namespace carstate { void cameraView(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4229c0
namespace carstate { void cockpitShown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x422aa0
namespace dead { void stub_0x422b70(win32::WinApplication* app, x86::CPU& cpu); }  // 0x422b70
namespace carstate { void cameraCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x422ba0
namespace carstate { void cameraTarget(win32::WinApplication* app, x86::CPU& cpu); }  // 0x422bd0
namespace dead { void stub_0x422c40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x422c40
namespace dead { void stub_0x422c80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x422c80
namespace carstate { void initCameras(win32::WinApplication* app, x86::CPU& cpu); }  // 0x422cf0
namespace carstate { void setCameraMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4238b0
namespace carstate { void setOverrideMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x423960
namespace carstate { void nextCycleMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x423a20
namespace carstate { void buildCameraCycle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x423a60
namespace carstate { void modeInCycle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x423ab0
namespace carstate { void rememberMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x423ae0
namespace carstate { void cameraMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x423b00
namespace carstate { void setModeAnew(win32::WinApplication* app, x86::CPU& cpu); }  // 0x423b50
namespace carstate { void settleCamera(win32::WinApplication* app, x86::CPU& cpu); }  // 0x423b80
namespace carstate { void nearestSpot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x423d90
namespace carstate { void scaledIfSet(win32::WinApplication* app, x86::CPU& cpu); }  // 0x423fb0
namespace carstate { void bitsBroken(win32::WinApplication* app, x86::CPU& cpu); }  // 0x423fe0
namespace carstate { void dampVector(win32::WinApplication* app, x86::CPU& cpu); }  // 0x424030
namespace carstate { void chaseOffset(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4240c0
namespace carstate { void chaseOffsetLevel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x424260
namespace frontend { void simCameraBesideRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x425060
namespace frontend { void simCameraNearestSpot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4251f0
namespace frontend { void simCameraPlayPath(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4253d0
namespace frontend { void simCameraEndPath(win32::WinApplication* app, x86::CPU& cpu); }  // 0x425640
namespace frontend { void simCameraOverRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x425bf0
namespace frontend { void simCameraFlyover(win32::WinApplication* app, x86::CPU& cpu); }  // 0x425d50
namespace frontend { void simCameraStill(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426290
namespace frontend { void simCameraWaitThenHandOn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4262a0
namespace frontend { void simCameraBackFromWait(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426310
namespace frontend { void simCameraStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426450
namespace frontend { void simCameraForcedMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426560
namespace frontend { void simCamerasStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426680
namespace frontend { void carSaveFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4267a0
namespace frontend { void trackSaveFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4267f0
namespace frontend { void carRecord(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426870
namespace frontend { void trackRecord(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426b00
namespace frontend { void car1EditClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426c20
namespace frontend { void car2EditClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426c40
namespace frontend { void car1ColourClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426c60
namespace frontend { void car2ColourClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426ce0
namespace frontend { void car1OnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426d60
namespace frontend { void car1OnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426e20
namespace frontend { void car1OnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426ee0
namespace frontend { void car2OnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426ef0
namespace frontend { void car2OnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x426f80
namespace frontend { void car2OnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427040
namespace frontend { void opponentFitCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427050
namespace frontend { void opponentFitArt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4270c0
namespace frontend { void opponentOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427190
namespace frontend { void opponentOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4272a0
namespace frontend { void opponentOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4272e0
namespace frontend { void tuningLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4272f0
namespace frontend { void tuningSave(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427370
namespace frontend { void tuningEngineMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4273e0
namespace frontend { void tuningBrakesMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427470
namespace frontend { void tuningSteeringMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427500
namespace frontend { void tuningGearsMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427590
namespace frontend { void tuningSuspensionMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427620
namespace frontend { void tuningAeroMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4276b0
namespace frontend { void tuningTiresMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427740
namespace frontend { void tuningSetSliders(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4277d0
namespace frontend { void tuningRememberSliders(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4279b0
namespace frontend { void tuningSlidersMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427ac0
namespace frontend { void tuningBindSliders(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427c20
namespace frontend { void tuningLoadAdvanced(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427d00
namespace frontend { void tuningDefaultsAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427d80
namespace frontend { void tuningDoneKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427e10
namespace frontend { void tuningCancelKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427ea0
namespace frontend { void tuningCarListKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427f40
namespace frontend { void tuningDefaultsKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x427fb0
namespace frontend { void tuningBindDoneCancel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x428020
namespace frontend { void tuningFitButtons(win32::WinApplication* app, x86::CPU& cpu); }  // 0x428060
namespace frontend { void tuningPrepare(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4281e0
namespace frontend { void tuningOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x428360
namespace frontend { void tuningOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4283b0
namespace dead { void stub_0x428480(win32::WinApplication* app, x86::CPU& cpu); }  // 0x428480
namespace dead { void stub_0x4284b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4284b0
namespace racestart { void respawnCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x428f90
namespace racestart { void resetCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x429100
namespace racestart { void stuckCheck(win32::WinApplication* app, x86::CPU& cpu); }  // 0x429260
namespace dead { void stub_0x429490(win32::WinApplication* app, x86::CPU& cpu); }  // 0x429490
namespace racestart { void clearRespawn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4294c0
namespace racestart { void shakeAmplitude(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4294e0
namespace racestart { void startShake(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4295a0
namespace racestart { void startIdleShake(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4296c0
namespace racestart { void shakeWave(win32::WinApplication* app, x86::CPU& cpu); }  // 0x429790
namespace racestart { void objectContacts(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4297d0
namespace racestart { void wheelSlip(win32::WinApplication* app, x86::CPU& cpu); }  // 0x429880
namespace racestart { void wheelCells(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4299a0
namespace racestart { void wheelSurface(win32::WinApplication* app, x86::CPU& cpu); }  // 0x429b40
namespace racestart { void endSkidMark(win32::WinApplication* app, x86::CPU& cpu); }  // 0x429ba0
namespace racestart { void extendSkidMark(win32::WinApplication* app, x86::CPU& cpu); }  // 0x429d20
namespace racestart { void skidSounds(win32::WinApplication* app, x86::CPU& cpu); }  // 0x429f10
namespace racestart { void wheelSpray(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42a130
namespace racestart { void surfaceEffects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42a280
namespace racestart { void wheelSkid(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42a480
namespace racestart { void wheelEffects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42a560
namespace racestart { void updateShake(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42a7d0
namespace racestart { void crashEffects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42ac40
namespace racestart { void respawnEffects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42b480
namespace racestart { void stopCarEffects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42b660
namespace racestart { void carEffects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42b780
namespace racestart { void airborneSpin(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42b7b0
namespace racestart { void updateCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42b900
namespace racestart { void defaultGridPlace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42ba00
namespace racestart { void loadStartGrid(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42bb80
namespace racestart { void gridPlace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42bc70
namespace racestart { void placeOnGrid(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42bcb0
namespace raceload { void loadCarPerformance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c2a0
namespace raceload { void loadRaceCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c350
namespace raceload { void setCarHandlers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c400
namespace raceload { void freeCarShapes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c520
namespace raceload { void restartRaceCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c530
namespace decompiled { void resetHandlingTunings(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c590
namespace raceload { void clearCarSlots(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c660
namespace raceload { void clearRaceCounts(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c680
namespace raceload { void clearCarList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c6d0
namespace raceload { void loadRaceCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c700
namespace raceload { void setTimeStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c7f0
namespace raceload { void addCarTasks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c830
namespace raceload { void openCarViv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42c950
namespace raceload { void closeCarViv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42ca80
namespace raceload { void removeCarTasks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42cab0
namespace raceload { void countLaps(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42cc20
namespace raceload { void carAxesSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42cd00
namespace raceload { void carForwardSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42cdd0
namespace raceload { void carRaceDistance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42ce10
namespace raceload { void updateRaceCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42ce70
namespace raceload { void frameFraction(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42cff0
namespace raceload { void placeHeadlights(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d020
namespace raceload { void placeCarShadow(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d160
namespace raceload { void sortRaceCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d3e0
namespace dead { void stub_0x42d4f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d4f0
namespace raceload { void emptyHook(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d500
namespace ea { void gameTrace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d510
namespace raceload { void emptyHook2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d520
namespace dead { void stub_0x42d530(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d530
namespace dead { void stub_0x42d540(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d540
namespace raceload { void showMessage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d620
namespace raceload { void sendMessage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d710
namespace raceload { void openMessages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d790
namespace raceload { void closeMessages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d7f0
namespace raceload { void clearMessages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d800
namespace raceload { void sendTypedMessage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d810
namespace raceload { void newMessageLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d860
namespace raceload { void freeMessageLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d8a0
namespace raceload { void insertMessageLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d8b0
namespace raceload { void freeMessageLines(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d910
namespace raceload { void clearMessageLines(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d940
namespace raceload { void showMessageLines(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42d9b0
namespace raceload { void appendMessageLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42da30
namespace dead { void stub_0x42daa0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42daa0
namespace raceload { void closeMessageLines(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42daf0
namespace raceload { void openMessageLines(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42db70
namespace raceload { void raceControlsThread(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42dc20
namespace dead { void stub_0x42dc3e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42dc3e
namespace raceload { void wakeRaceControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42dc40
namespace raceload { void raceControlsTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42dc60
namespace raceload { void startRaceControlsTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42dcb0
namespace raceload { void stopRaceControlsTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42dd00
namespace raceload { void cornerInBox(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42dd60
namespace raceload { void cornerInBoxBack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42e010
namespace raceload { void cornerInBox2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42e2c0
namespace raceload { void cornerInBoxBack2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42e4f0
namespace raceload { void clampSpin(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42e720
namespace raceload { void applyImpulse(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42e820
namespace raceload { void bounceOffWall(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42e9c0
namespace raceload { void hitWall(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42ee10
namespace raceload { void boxHitsWall(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42efe0
namespace raceload { void carsCollide(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42f280
namespace collide { void findContact(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42f550
namespace collide { void contactNormal(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42f8c0
namespace collide { void pushCarsApart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42fe10
namespace collide { void separateCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x42fee0
namespace collide { void polygonEdgeNormal(win32::WinApplication* app, x86::CPU& cpu); }  // 0x430210
namespace collide { void clearOfWall(win32::WinApplication* app, x86::CPU& cpu); }  // 0x430350
namespace collide { void carAgainstWalls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4304b0
namespace collide { void collideCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x430c80
namespace collide { void clearCarList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x430cd0
namespace collide { void objectInReach(win32::WinApplication* app, x86::CPU& cpu); }  // 0x430ce0
namespace collide { void carAgainstObjects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x430da0
namespace collide { void addCarToList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4310c0
namespace collide { void carCollisions(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4311d0
namespace collide { void distanceSquared(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431250
namespace collide { void nearestPoint(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431280
namespace collide { void openCollFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431320
namespace collide { void closeCollFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4313b0
namespace collide { void collFileSize(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4313d0
namespace collide { void readCollFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4313e0
namespace collide { void collFileCount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431410
namespace collide { void collChunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431420
namespace collide { void playerControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431430
namespace controls { void clearJoysticks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431580
namespace controls { void readJoysticks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4315a0
namespace controls { void pollJoystick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431690
namespace controls { void buttonHeld(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4316c0
namespace controls { void hatPointing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4316f0
namespace controls { void axisAmount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431740
namespace controls { void keyHeld(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4317d0
namespace controls { void controlHeld(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431800
namespace controls { void controlHeldNo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431880
namespace controls { void controlHeldYes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431885
namespace controls { void controlHeldByKind(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43188d
namespace controls { void noControl(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4318b0
namespace collide { void forgetRenderStates(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4318c0
namespace graphics { void setRenderState(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431900
namespace collide { void statesForText(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431aa0
namespace collide { void statesForPictures(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431af0
namespace collide { void statesForHud(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431b40
namespace collide { void statesForScreens(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431bc0
namespace collide { void passLayerNothing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431c20
namespace collide { void passSecondLayer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431c30
namespace collide { void passFirstLayer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431c40
namespace collide { void makeBlendTables(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431c50
namespace collide { void blendSpan(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431d50
namespace dead { void stub_0x431e80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431e80
namespace dead { void stub_0x431fa0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x431fa0
namespace dead { void stub_0x432070(win32::WinApplication* app, x86::CPU& cpu); }  // 0x432070
namespace dead { void stub_0x432190(win32::WinApplication* app, x86::CPU& cpu); }  // 0x432190
namespace dead { void stub_0x432260(win32::WinApplication* app, x86::CPU& cpu); }  // 0x432260
namespace dead { void stub_0x432320(win32::WinApplication* app, x86::CPU& cpu); }  // 0x432320
namespace graphics { void clearSoftwareBlend(win32::WinApplication* app, x86::CPU& cpu); }  // 0x432370
namespace graphics { void setSoftwareBlendColour(win32::WinApplication* app, x86::CPU& cpu); }  // 0x432390
namespace graphics { void dead_0x432440(win32::WinApplication* app, x86::CPU& cpu); }  // 0x432440
namespace collide { void subdivisionDepth(win32::WinApplication* app, x86::CPU& cpu); }  // 0x432690
namespace render { void subdivideQuad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x432720
namespace render { void quadSubdivided(win32::WinApplication* app, x86::CPU& cpu); }  // 0x433060
namespace render { void quadRecords(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4332b0
namespace collide { void drawTexturedTriangles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x433530
namespace collide { void drawFlatTriangles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x433660
namespace collide { void drawTrianglesByZ(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4336e0
namespace collide { void drawQuadsByZ(win32::WinApplication* app, x86::CPU& cpu); }  // 0x433840
namespace collide { void drawFlatTrianglesAgain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4339d0
namespace vertices { void triangleList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x433a50
namespace vertices { void quadList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x433bb0
namespace graphics { void drawColouredQuadRecord(win32::WinApplication* app, x86::CPU& cpu); }  // 0x433d40
namespace render { void colouredTriangles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x433e30
namespace render { void triangleRecords(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434120
namespace vertices { void submitPolygons(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434380
namespace dead { void stub_0x434799(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434799
namespace collide { void drawColouredTriangles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4347b0
namespace graphics { void drawLineRecord(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4347d0
namespace collide { void stateRecord(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434850
namespace render { void polygonDrawers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434870
namespace collide { void drawRecords(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434950
namespace collide { void raceTextFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434990
namespace collide { void setMirrored(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4349a0
namespace collide { void drawMirroredRecords(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4349d0
namespace graphics { void endFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434a00
namespace collide { void countFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434a70
namespace collide { void raceViewDistance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434a80
namespace collide { void raceFrameEnd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434a90
namespace dead { void stub_0x434ad0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434ad0
namespace collide { void raceFrameNothing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434ae0
namespace collide { void raceEndNothing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434af0
namespace collide { void raceStartDrawing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434b00
namespace collide { void raceStartNothing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434b10
namespace graphics { void syncWindow(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434b20
namespace graphics { void flushAndFlip(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434b40
namespace collide { void drawPointer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434b60
namespace controls { void startDirectInput(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434f20
namespace controls { void stopDirectInput(win32::WinApplication* app, x86::CPU& cpu); }  // 0x434fd0
namespace controls { void openMouse(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4350a0
namespace frontend { void placePointer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435230
namespace controls { void setPointerBounds(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4352d0
namespace controls { void readMouse(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435310
namespace controls { void findJoysticks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435600
namespace controls { void describeJoystick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435690
namespace controls { void readJoystick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4358f0
namespace controls { void readAxisZone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435ae0
namespace controls { void setAxisZone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435b40
namespace controls { void resetForceFeedback(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435b60
namespace controls { void setAutocenter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435bc0
namespace controls { void pauseEffects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435c60
namespace controls { void continueEffects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435cc0
namespace controls { void actuatorsOn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435d20
namespace controls { void actuatorsOff(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435d90
namespace controls { void setGain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435df0
namespace controls { void createEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x435f10
namespace controls { void startEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4361e0
namespace controls { void stopEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436230
namespace controls { void stopEffects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436270
namespace controls { void downloadEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4362f0
namespace controls { void unloadEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436310
namespace controls { void updateEffect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436330
namespace controls { void setCooperativeLevel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436610
namespace controls { void acquireDevice(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4366e0
namespace controls { void unacquireDevice(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436800
namespace controls { void acquireResult(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4368c0
namespace controls { void effectPlaying(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4368f0
namespace controls { void hideCursor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436930
namespace controls { void joystickFound(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436980
namespace controls { void axisProperty(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436b10
namespace collide { void openMessages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436c40
namespace collide { void closeMessages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436cf0
namespace collide { void clearAllMessages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436d40
namespace collide { void clearMessages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436d60
namespace collide { void clearMessageLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436dc0
namespace collide { void addMessageText(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436e10
namespace collide { void addMessageCharacter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x436e90
namespace collide { void copyMessageLines(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437080
namespace collide { void messagesScrolled(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4371a0
namespace collide { void messagesNothing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4371c0
namespace collide { void seedHash(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4371d0
namespace frontend { void feedSeparate(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437250
namespace frontend { void feedTimeButtonKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437300
namespace frontend { void feedTimeListKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437350
namespace frontend { void feedPositionButtonKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437390
namespace frontend { void feedPositionListKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4373e0
namespace frontend { void feedSpeedButtonKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437420
namespace frontend { void feedSpeedListKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437470
namespace frontend { void feedOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4374b0
namespace frontend { void feedOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437590
namespace frontend { void funkeysOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437600
namespace frontend { void funkeysOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4377c0
namespace frontend { void hardkeysOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4377d0
namespace frontend { void audioMenuMusicMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437990
namespace frontend { void audioRaceMusicMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4379b0
namespace frontend { void audioEffectsMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4379d0
namespace frontend { void audioEngineMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437a20
namespace frontend { void audioSpeechMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437a70
namespace frontend { void audioVolumeDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437a90
namespace frontend { void audioOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x437e00
namespace frontend { void audioOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438070
namespace frontend { void audioOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438260
namespace load { void endWithSlash(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438340
namespace load { void folderWritable(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438350
namespace load { void firstFreeHexFolder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4383f0
namespace dead { void stub_0x4384a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4384a0
namespace load { void firstFreePlayerFolder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4384f0
namespace load { void freePlayerCarFolder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4385a0
namespace dead { void stub_0x4385f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4385f0
namespace dead { void stub_0x4386a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4386a0
namespace load { void firstSubfolder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4386b0
namespace load { void nextSubfolder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4387e0
namespace load { void firstFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438870
namespace load { void nextFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438970
namespace load { void removeFolder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4389d0
namespace load { void firstCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438a80
namespace load { void nextCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438aa0
namespace load { void makeCarId(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438ad0
namespace load { void firstCarGlobal(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438af0
namespace load { void nextCarGlobal(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438b10
namespace dead { void stub_0x438b40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438b40
namespace load { void appendCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438b90
namespace load { void carCount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438bd0
namespace load { void countCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438be0
namespace load { void unlinkCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438c10
namespace load { void insertCarByName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438c50
namespace load { void joinCarLists(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438cc0
namespace load { void carListOf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438d20
namespace load { void takePoliceCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438d60
namespace load { void takeClasslessCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438dc0
namespace load { void takeOrdinaryCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438e30
namespace load { void sortCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438eb0
namespace dead { void stub_0x438fc0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x438fc0
namespace dead { void stub_0x4390f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4390f0
namespace dead { void stub_0x4391d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4391d0
namespace load { void carName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439250
namespace frontend { void showcaseCarFileStem(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439290
namespace load { void carFolder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4392b0
namespace load { void carFilePrefix(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439300
namespace port { namespace obj3d { void carVivPath(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x439360
namespace dead { void stub_0x439450(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439450
namespace load { void carId(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439520
namespace load { void carFlag40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439570
namespace dead { void stub_0x439590(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439590
namespace load { void carFlag4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4395c0
namespace load { void carHasNumericId(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439610
namespace load { void idIsNumeric(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439620
namespace load { void numericId(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439660
namespace load { void carIsBonus(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439680
namespace dead { void stub_0x4396a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4396a0
namespace load { void carIsPolice(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4396b0
namespace dead { void stub_0x4396d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4396d0
namespace load { void carIsFerrariOrMercedes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4396e0
namespace dead { void stub_0x439700(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439700
namespace load { void carClass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439730
namespace dead { void stub_0x439740(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439740
namespace port { namespace fields { void carColourCount(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x439760
namespace port { namespace fields { void carCustomColour(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x439780
namespace port { namespace fields { void carColour(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x439790
namespace port { namespace fields { void carColourName(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4397c0
namespace port { namespace fields { void setCarColour(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4397d0
namespace port { namespace fields { void carColourRgb(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x439810
namespace port { namespace fields { void setCarCustomColour(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4398c0
namespace dead { void stub_0x439930(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439930
namespace dead { void stub_0x439940(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439940
namespace dead { void stub_0x439950(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439950
namespace dead { void stub_0x439960(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439960
namespace dead { void stub_0x439970(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439970
namespace dead { void stub_0x439980(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439980
namespace load { void findCarById(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439990
namespace load { void clearCarCache(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4399f0
namespace load { void carRecord(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439a50
namespace port { namespace fields { void carText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x439ab0
namespace load { void fecText(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439af0
namespace load { void findCarByName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439b20
namespace load { void attachFecTexts(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439bd0
namespace load { void bonusCarLocked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439c10
namespace frontend { void creditsHaveBonusCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439ce0
namespace load { void checkDuplicateCars(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439d20
namespace load { void loadCarList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x439d90
namespace load { void carSelectable(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43a4f0
namespace load { void buildCarClassLists(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43a580
namespace load { void copyCarAsNew(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43ae60
namespace dead { void stub_0x43b330(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43b330
namespace load { void loadCarSpecs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43b3f0
namespace load { void writeCarFec(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43b4f0
namespace load { void readCarFecBody(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43b570
namespace load { void readCarFec(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43b630
namespace load { void carFecPath(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43b680
namespace load { void loadCarFec(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43b740
namespace port { namespace fields { void saveCarColours(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x43b7f0
namespace ea { void openViv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43b830
namespace ea { void closeViv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43b9b0
namespace ea { void findInViv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43b9d0
namespace load { void placeCarVivPath(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43ba20
namespace load { void swapBytes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43ba30
namespace load { void writeBigEndian(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43ba70
namespace load { void appendCarpToViv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43baa0
namespace load { void lastCarLoaded(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43bbf0
namespace load { void setLastCarId(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43bc00
namespace load { void setLastCarName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43bc40
namespace load { void setLastCarFlags(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43bc70
namespace load { void markLastCarFlag8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43bc90
namespace frontend { void viewInteriorMapX(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43bcc0
namespace dead { void stub_0x43bde0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43bde0
namespace frontend { void controlsPollJoystickBinding(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43be40
namespace frontend { void bindingIsDigital(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43c000
namespace frontend { void noteDigitalControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43c040
namespace frontend { void controlsDrawBinding(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43c110
namespace frontend { void controlsDrawList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43c900
namespace frontend { void controlsKeyAsBinding(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43cda0
namespace frontend { void controlsWarnDigitalBinding(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43ce80
namespace frontend { void controlsListKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43cf80
namespace frontend { void controlsListFitBox(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d2b0
namespace frontend { void controlsListPrepare(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d310
namespace frontend { void controlsCancelKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d3f0
namespace frontend { void controlsResetAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d410
namespace frontend { void controlsResetClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d460
namespace frontend { void controlsOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d4d0
namespace frontend { void controlsOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d5f0
namespace frontend { void controlsWaitJoystickRelease(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d650
namespace frontend { void controlsClearConflicts(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d6a0
namespace frontend { void controlsBindingKeyName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d7f0
namespace frontend { void controlsSampleAxes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d920
namespace frontend { void deadZoneFillList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d9a0
namespace frontend { void controllerName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43d9f0
namespace frontend { void connectApplyPlayerVector(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43dab0
namespace frontend { void colourMakeHueRamp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43db20
namespace port { namespace fields { void wheelFromCarColour(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x43dbe0
namespace frontend { void colourApplyToCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43dc90
namespace port { namespace fields { void wheelForPlayer(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x43dd50
namespace frontend { void colourAccept(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43dd90
namespace frontend { void colourRevert(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43ddd0
namespace frontend { void colourHsvToArgb(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43de20
namespace frontend { void colourTriangleFromColour(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43de60
namespace frontend { void colourFromTriangle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43def0
namespace frontend { void colourDrawFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43dfb0
namespace frontend { void colourDrawSwatch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43e040
namespace frontend { void colourDrawButtons(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43e0d0
namespace frontend { void colourDrawMarkers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43e1f0
namespace frontend { void colourDrawTriangle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43e310
namespace frontend { void colourDrawValueBar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43e490
namespace frontend { void colourDragMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43e7a0
namespace frontend { void colourHitRegion(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43e8e0
namespace frontend { void colourMouse(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43ead0
namespace frontend { void colourDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43ebc0
namespace frontend { void colourKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43ec40
namespace frontend { void colourPointerKind(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43ef80
namespace frontend { void colourFitBox(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43eff0
namespace frontend { void colourPrepare(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f030
namespace frontend { void colourShown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f160
namespace frontend { void connectListingRedraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f1f0
namespace frontend { void ipxRefreshGames(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f220
namespace frontend { void connectOpenProtocolList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f320
namespace frontend { void connectOpenAddressDialog(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f4e0
namespace frontend { void connectModemLabel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f550
namespace frontend { void connectComPortLabel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f610
namespace frontend { void ipxGameNameAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f640
namespace frontend { void connectAddressAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f680
namespace frontend { void connectTcpAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f760
namespace frontend { void connectProtocolListAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f7e0
namespace frontend { void connectSerialAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43f960
namespace frontend { void connectPhoneAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43fa00
namespace frontend { void connectModemAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43faf0
namespace frontend { void connectRaceNetMissingAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43fc20
namespace frontend { void connectRaceNetAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43fc50
namespace frontend { void multiMenuLeaveAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43fcb0
namespace frontend { void connectRaceNetClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43fd10
namespace port { namespace buttons { void joinListedGame(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x43fdd0
namespace frontend { void connectTcpClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43fe50
namespace frontend { void connectSerialClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43fee0
namespace frontend { void connectModemClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x43ff50
namespace frontend { void ipxCreateGameClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440040
namespace frontend { void ipxRefreshClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4400b0
namespace frontend { void multiMenuOnBack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4400d0
namespace frontend { void multiMenuApplyRaceList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440160
namespace frontend { void multiMenuFollowRaceType(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440390
namespace frontend { void multiMenuOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4403f0
namespace frontend { void multiMenuOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440470
namespace frontend { void multiMenuOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4405b0
namespace frontend { void connectOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4405c0
namespace frontend { void connectOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440730
namespace frontend { void ipxOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440810
namespace frontend { void ipxOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440890
namespace frontend { void ipxOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440960
namespace frontend { void loadScreenArt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440970
namespace frontend { void lobbyDrawPlayerIcon(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440b30
namespace frontend { void lobbyDrawPulsingIcon(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440b50
namespace port { namespace buttons { void drawReadyMarks(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x440c40
namespace port { namespace buttons { void networkLeft(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x440d20
namespace frontend { void lobbyPointAtPanel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440d60
namespace frontend { void lobbyDrawRacePanel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x440d80
namespace frontend { void lobbyPanelFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x441040
namespace frontend { void lobbyPanelBusy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x441680
namespace frontend { void lobbyPanelKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x441700
namespace dead { void stub_0x4417f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4417f0
namespace frontend { void lobbyPanelLineWidth(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4418e0
namespace frontend { void lobbyPanelLineFits(win32::WinApplication* app, x86::CPU& cpu); }  // 0x441900
namespace frontend { void lobbyDrawMessages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x441950
namespace frontend { void lobbyTypeInPanel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x441b20
namespace frontend { void creditsRenderPage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x441cb0
namespace frontend { void creditsNextPage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442370
namespace frontend { void creditsPreviousPage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4423e0
namespace port { namespace buttons { void drawLicence(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x442450
namespace frontend { void creditsOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442580
namespace frontend { void creditsOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442850
namespace frontend { void creditsOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442980
namespace port { namespace menus { void codelinkMatches(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4429c0
namespace frontend { void menuFindItem(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442a40
namespace frontend { void optionsOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442aa0
namespace port { namespace menus { void assignHandlers(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x442b10
namespace frontend { void postgameUnlockTrack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442b60
namespace port { namespace buttons { void listColours(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x442b90
namespace port { namespace buttons { void listAllowed(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x442bf0
namespace port { namespace buttons { void listLength(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x442c50
namespace port { namespace buttons { void findList(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x442c80
namespace port { namespace buttons { void menuEdited(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x442d00
namespace port { namespace buttons { void menuRecordEdited(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x442d20
namespace dead { void stub_0x442d30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442d30
namespace dead { void stub_0x442d50(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442d50
namespace dead { void stub_0x442d60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442d60
namespace frontend { void deadZoneCancelKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442db0
namespace frontend { void deadZoneApplyAxis(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442dd0
namespace frontend { void deadZoneAxisIndex(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442e60
namespace frontend { void deadZoneAxisMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442ef0
namespace frontend { void deadZoneAxisDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x442ff0
namespace frontend { void deadZoneShowController(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443070
namespace frontend { void deadZoneChooseKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443260
namespace frontend { void deadZoneChooseKeyInRace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443290
namespace frontend { void deadZoneOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4432c0
namespace frontend { void deadZoneOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443430
namespace frontend { void typedCharacterInRange(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4434f0
namespace frontend { void typedByteInRange(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443510
namespace frontend { void typedTextInRange(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443520
namespace frontend { void typedCharacterOfAddress(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4435c0
namespace frontend { void typedTextOfAddress(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4435e0
namespace frontend { void typedByteInRangeAgain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443620
namespace frontend { void shortNameCharacter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443630
namespace frontend { void shortNameValid(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443660
namespace frontend { void typedLanguageTestFirst(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4436c0
namespace frontend { void languageNameValid(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443700
namespace frontend { void typedLanguageTestSecond(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443740
namespace frontend { void languageFileNameValid(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443780
namespace frontend { void typedLanguageTestThird(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4437c0
namespace frontend { void typedCharacterAllowed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443810
namespace frontend { void lineAllowed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443860
namespace port { namespace dialogs { void dialogShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4438b0
namespace port { namespace dialogs { void dialogKind(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4438c0
namespace port { namespace dialogs { void drawBox(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4438d0
namespace frontend { void dialogLinesBox(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443a90
namespace frontend { void dialogButtonRowBox(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443c00
namespace frontend { void dialogPlaceButtons(win32::WinApplication* app, x86::CPU& cpu); }  // 0x443d20
namespace port { namespace dialogs { void drawLines(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x443f40
namespace port { namespace dialogs { void drawButtons(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x444000
namespace port { namespace dialogs { void drawEditField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x444350
namespace frontend { void dialogPointAtButton(win32::WinApplication* app, x86::CPU& cpu); }  // 0x444570
namespace port { namespace dialogs { void buttonUnderMouse(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4445d0
namespace frontend { void dialogFocusedButton(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4446c0
namespace frontend { void dialogOpenQuestion(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4446d0
namespace port { namespace dialogs { void questionKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x444910
namespace port { namespace dialogs { void questionPass(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4449d0
namespace port { namespace dialogs { void endMessage(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x444a60
namespace frontend { void dialogOpenMessage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x444a80
namespace port { namespace dialogs { void messageKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x444b20
namespace port { namespace dialogs { void messagePass(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x444b60
namespace frontend { void dialogMessageById(win32::WinApplication* app, x86::CPU& cpu); }  // 0x444be0
namespace frontend { void dialogEditText(win32::WinApplication* app, x86::CPU& cpu); }  // 0x444c10
namespace frontend { void dialogOpenEdit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x444c20
namespace port { namespace dialogs { void editKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x444f50
namespace port { namespace dialogs { void editPass(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4450c0
namespace port { namespace dialogs { void listFocus(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x445150
namespace port { namespace dialogs { void pointAtEntry(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x445160
namespace port { namespace dialogs { void entryUnderMouse(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4451d0
namespace port { namespace dialogs { void placeEntries(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4452d0
namespace port { namespace dialogs { void drawList(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x445380
namespace port { namespace dialogs { void listKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x445520
namespace port { namespace dialogs { void listPass(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x445680
namespace port { namespace dialogs { void openList(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x445750
namespace port { namespace dialogs { void drawDialogHelp(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x445ac0
namespace port { namespace dialogs { void loadDialogArt(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x445de0
namespace port { namespace dialogs { void dialogKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x445e60
namespace frontend { void dialogFadedIn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x445f00
namespace port { namespace dialogs { void closeDialog(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x445f10
namespace port { namespace dialogs { void dialogPass(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x445f50
namespace load { void readReplayInfo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446060
namespace frontend { void loadSaveKnockoutInfo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4461d0
namespace frontend { void loadSaveTournamentInfo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4462e0
namespace frontend { void loadSaveGhostInfo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446420
namespace frontend { void pauseSeriesSaveExists(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446510
namespace frontend { void pauseWriteSeriesSave(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446550
namespace frontend { void loadSaveLoadKnockout(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4465e0
namespace frontend { void loadSaveLoadGhost(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4466a0
namespace frontend { void pauseGhostExists(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446740
namespace load { void saveGhostAs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446780
namespace frontend { void loadSaveLoadTournament(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446800
namespace frontend { void pauseWriteTournamentSave(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4468c0
namespace frontend { void pauseTournamentSaveExists(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446950
namespace load { void loadKeptReplay(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446990
namespace load { void saveReplayAs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446a10
namespace frontend { void pauseReplayExists(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446a90
namespace frontend { void loadSaveShowTournaments(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446ad0
namespace frontend { void loadSaveShowKnockouts(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446b10
namespace frontend { void loadSaveShowReplays(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446b60
namespace frontend { void loadSaveShowGhosts(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446ba0
namespace frontend { void loadSaveLoadFailed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446bf0
namespace frontend { void loadSaveLoadNow(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446c50
namespace frontend { void loadSaveDeleteSelected(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446ea0
namespace frontend { void loadSaveDeleteAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x446ff0
namespace frontend { void loadSaveDeleteClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447030
namespace frontend { void loadSaveDrawDescription(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4470e0
namespace frontend { void loadSaveSelectionMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4471a0
namespace frontend { void loadSaveOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447370
namespace frontend { void loadSaveOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447470
namespace frontend { void loadSaveOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447590
namespace load { void listReplays(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4475a0
namespace frontend { void pauseOverwriteAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447800
namespace frontend { void pauseSaveNameAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4478c0
namespace frontend { void pauseAskSaveName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447a30
namespace frontend { void forceCancelKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447b40
namespace frontend { void forceStrengthMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447b60
namespace frontend { void forceRoadEffectsMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447c40
namespace frontend { void forceCollisionMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447c70
namespace frontend { void forceEngineMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447ca0
namespace frontend { void forceBodyRollMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447cd0
namespace frontend { void forceRoadGripMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447d50
namespace frontend { void forceOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x447dd0
namespace frontend { void forceOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4481c0
namespace frontend { void screenModeListAdd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x448210
namespace frontend { void modesList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4482c0
namespace frontend { void modesChosenWidth(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4482f0
namespace frontend { void modesCopyChosen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x448330
namespace frontend { void modesChooseByWidth(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4483a0
namespace frontend { void displayModeName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x448400
namespace frontend { void graphicsOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x448430
namespace frontend { void graphicsOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x448440
namespace frontend { void graphicsOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4484e0
namespace frontend { void advancedIntensityMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x448570
namespace frontend { void advancedOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4485a0
namespace frontend { void advancedOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x448610
namespace frontend { void advancedOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x448770
namespace frontend { void locationOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4487d0
namespace frontend { void locationOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4487e0
namespace frontend { void locationOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4489e0
namespace port { namespace menus { void keywordIndex(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x448f00
namespace port { namespace menus { void storeString(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x448f50
namespace port { namespace menus { void widgetType(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x448fd0
namespace port { namespace menus { void widgetName(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449020
namespace port { namespace menus { void initItem(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449060
namespace port { namespace menus { void sectionType(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4490e0
namespace port { namespace menus { void parseInt(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449130
namespace port { namespace menus { void parseLong(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449170
namespace port { namespace menus { void parseFloat(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4491b0
namespace port { namespace menus { void setField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449240
namespace port { namespace menus { void readEntry(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4493b0
namespace port { namespace menus { void loadMenus(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449510
namespace port { namespace menus { void checkMenuFile(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4498a0
namespace port { namespace menus { void checkMenuFiles(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449960
namespace port { namespace menus { void callOnLoad(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449990
namespace port { namespace loop { void itemUsable(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4499b0
namespace port { namespace loop { void keyAllowed(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4499f0
namespace decompiled { void resetMenuHelp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x449a60
namespace port { namespace loop { void drawHelp(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449aa0
namespace port { namespace loop { void keysTaken(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449df0
namespace port { namespace loop { void selectable(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449e50
namespace port { namespace loop { void pointAtSelected(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449ea0
namespace port { namespace loop { void selectPrevious(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x449f30
namespace port { namespace loop { void selectNext(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44a0c0
namespace port { namespace loop { void escapeButton(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44a240
namespace port { namespace loop { void handleKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44a2d0
namespace port { namespace loop { void selectedIndex(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44a440
namespace port { namespace loop { void selectItem(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44a470
namespace port { namespace loop { void drawItems(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44a4f0
namespace frontend { void menuShownNow(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44a660
namespace dead { void stub_0x44a680(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44a680
namespace port { namespace loop { void releaseMouse(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44a690
namespace dead { void stub_0x44a6a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44a6a0
namespace port { namespace loop { void itemUnderMouse(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44a6b0
namespace port { namespace loop { void itemsShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44a860
namespace port { namespace loop { void passItems(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44a8b0
namespace frontend { void menuShowItems(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44a960
namespace port { namespace loop { void hideItems(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44a9e0
namespace port { namespace loop { void leaveScreen(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44aa30
namespace port { namespace loop { void resetItems(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44aad0
namespace port { namespace loop { void enterScreen(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44ab40
namespace port { namespace buttons { void loadShapes(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44ac90
namespace dead { void stub_0x44ad60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44ad60
namespace dead { void stub_0x44adb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44adb0
namespace frontend { void menuCurrentName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44ae00
namespace dead { void stub_0x44ae20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44ae20
namespace port { namespace loop { void menuByName(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44ae60
namespace port { namespace loop { void currentMenu(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44aeb0
namespace port { namespace loop { void popMenu(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44aec0
namespace port { namespace loop { void pushMenu(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44af10
namespace port { namespace loop { void setHistory(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44af50
namespace port { namespace loop { void player1NameAnswer(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44af90
namespace port { namespace loop { void player2NameAnswer(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44b040
namespace port { namespace loop { void quitAnswer(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44b0f0
namespace port { namespace buttons { void redrawMenu(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44b170
namespace port { namespace loop { void waitWhileSuspended(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44b1a0
namespace port { namespace loop { void menuLoop(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44b230
namespace frontend { void racePauseBegin(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44b950
namespace frontend { void racePauseAgain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44ba30
namespace frontend { void racePauseFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44bac0
namespace frontend { void menuDrawPass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44bdd0
namespace frontend { void menuLeave(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44be20
namespace port { namespace loop { void fitRaceSettings(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x44be60
namespace frontend { void pauseHideSpare(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44c2d0
namespace frontend { void pauseHideForCop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44c360
namespace frontend { void pauseHideForRacer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44c4e0
namespace frontend { void pauseLeaveAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44c610
namespace frontend { void pauseRestartAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44c670
namespace frontend { void pauseAskLeave(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44c6d0
namespace frontend { void pauseQuitChoiceAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44c730
namespace frontend { void pauseQuitClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44c7b0
namespace frontend { void pauseRestartClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44ca10
namespace frontend { void pauseSaveClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44ca80
namespace frontend { void pauseOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44cab0
namespace frontend { void pauseOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44cf60
namespace frontend { void pauseIntensityMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44cf70
namespace frontend { void pauseMapCopMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44cfa0
namespace frontend { void pauseMapRacerMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44cfe0
namespace frontend { void pauseMapFirstCopMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d020
namespace frontend { void pauseMapFirstRacerMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d060
namespace frontend { void pauseMapSecondCopMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d0a0
namespace frontend { void pauseMapSecondRacerMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d0e0
namespace frontend { void pauseOptionsOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d120
namespace frontend { void pauseOptionsOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d160
namespace frontend { void pauseControlsCancelKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d170
namespace frontend { void pauseControlsOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d190
namespace frontend { void pauseControlsOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d220
namespace frontend { void pauseWidescreenClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d270
namespace frontend { void pauseGraphicsOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d2a0
namespace frontend { void pauseGraphicsOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d3e0
namespace frontend { void pauseGraphicsOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d420
namespace frontend { void pauseHudOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d4c0
namespace frontend { void pauseHudOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d670
namespace frontend { void pauseHud1OnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d7a0
namespace frontend { void pauseHud1OnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44d9a0
namespace frontend { void pauseHud2OnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44dad0
namespace frontend { void pauseHud2OnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44dcd0
namespace frontend { void hudEditDoneKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44de00
namespace frontend { void hudEditResetKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44de20
namespace frontend { void hudEditLayoutOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44de80
namespace frontend { void hudEditLayoutOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44dfd0
namespace frontend { void pauseAiTunerOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e130
namespace frontend { void pauseAiTunerOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e140
namespace frontend { void postgameAwardMessage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e170
namespace frontend { void postgameCheckAwards(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e1a0
namespace frontend { void postgameSaveKnockoutClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e460
namespace frontend { void postgameSaveTournamentClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e480
namespace frontend { void postgameSaveGhostClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e4a0
namespace frontend { void postgameLeaveNetwork(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e4c0
namespace frontend { void postgameLeaveAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e510
namespace frontend { void postgameAskLeave(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e550
namespace frontend { void postgameMainMenuClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e5b0
namespace frontend { void postgameNetworkMainMenuClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e600
namespace frontend { void postgameQuitClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e690
namespace frontend { void postgameSaveReplayClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e6a0
namespace frontend { void postgameCannotSaveReplay(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e6c0
namespace frontend { void postgameViewReplayClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e710
namespace frontend { void postgameToggleSheets(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e7a0
namespace frontend { void awardsClear(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e7d0
namespace frontend { void awardsGrant(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e7f0
namespace frontend { void awardsForRace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44e820
namespace frontend { void awardsSpeak(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44ea60
namespace frontend { void awardsSpeakForSheet(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44ed30
namespace frontend { void postgameOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44eda0
namespace frontend { void postgameOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44efd0
namespace frontend { void postgameOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44f500
namespace load { void loadRecords(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44f510
namespace frontend { void recordsLoaded(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44f5d0
namespace load { void saveRecords(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44f5f0
namespace frontend { void recordsSelect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44f670
namespace frontend { void recordsCompare(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44f6e0
namespace frontend { void recordsSortBy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44f830
namespace frontend { void recordsCount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44f8d0
namespace frontend { void recordByRank(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44f910
namespace frontend { void recordsTimeQualifies(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44f960
namespace frontend { void recordsClearMarks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44f9b0
namespace frontend { void recordsInsert(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44fa10
namespace frontend { void recordsRead(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44fa90
namespace frontend { void recordsHolderMark(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44fb20
namespace frontend { void recordsSetHolder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44fb30
namespace frontend { void recordsClear(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44fbc0
namespace frontend { void recordsSheetSortMark(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44fc00
namespace frontend { void recordsSheetDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x44fc70
namespace frontend { void recordsSheetLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x450770
namespace frontend { void recordsSheetSave(win32::WinApplication* app, x86::CPU& cpu); }  // 0x450950
namespace frontend { void recordsSheetLayout(win32::WinApplication* app, x86::CPU& cpu); }  // 0x450970
namespace frontend { void recordsSheetKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x450a90
namespace frontend { void recordsSheetIdle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x450e30
namespace frontend { void recordsDeleteAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x450e50
namespace frontend { void recordsDeleteClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x450eb0
namespace frontend { void recordsOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x450f20
namespace frontend { void recordsOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x450f70
namespace frontend { void recordsOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x450fd0
namespace frontend { void recordsEnterResult(win32::WinApplication* app, x86::CPU& cpu); }  // 0x450fe0
namespace frontend { void recordsEnterPlayers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4512f0
namespace port { namespace text { void setTextColour(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4513c0
namespace port { namespace fields { void setCursorColour(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4513e0
namespace port { namespace text { void toScreenX(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451400
namespace port { namespace text { void toScreenY(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451440
namespace port { namespace text { void screenToMenuX(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451480
namespace port { namespace text { void screenToMenuY(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4514c0
namespace port { namespace text { void readMenuMouse(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451500
namespace port { namespace text { void dimBackground(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451530
namespace port { namespace text { void drawTextAt(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451590
namespace port { namespace fields { void drawFieldText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4515e0
namespace dead { void stub_0x451640(win32::WinApplication* app, x86::CPU& cpu); }  // 0x451640
namespace dead { void stub_0x451670(win32::WinApplication* app, x86::CPU& cpu); }  // 0x451670
namespace port { namespace text { void joystickKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451700
namespace dead { void stub_0x451715(win32::WinApplication* app, x86::CPU& cpu); }  // 0x451715
namespace dead { void stub_0x4517d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4517d0
namespace port { namespace text { void captureKeys(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451810
namespace port { namespace text { void scanCodeOfKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451840
namespace port { namespace text { void keyOfScanCode(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4518c0
namespace frontend { void connectFlushKeys(win32::WinApplication* app, x86::CPU& cpu); }  // 0x451940
namespace port { namespace text { void menuKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451960
namespace port { namespace text { void resetMenuKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451b20
namespace port { namespace text { void nextWord(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451b50
namespace port { namespace text { void drawWrappedText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451cd0
namespace port { namespace text { void drawTextBox(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451fc0
namespace port { namespace text { void measureTextBox(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x451ff0
namespace port { namespace text { void textWidth(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x452020
namespace frontend { void textFit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452090
namespace port { namespace text { void selectFont(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x452100
namespace port { namespace text { void menusEnd(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x452110
namespace frontend { void textDrawFitted(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452120
namespace port { namespace text { void drawMenuText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x452190
namespace port { namespace fields { void drawFieldLine(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x452220
namespace port { namespace fields { void drawSpacedText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4522c0
namespace frontend { void menuCaptureScreen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452410
namespace frontend { void cheatAdvance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452420
namespace frontend { void cheatKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4524e0
namespace dead { void stub_0x452910(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452910
namespace port { namespace menus { void appendBackslash(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4529a0
namespace frontend { void setupRaceAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452ac0
namespace frontend { void setupAskRace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452bd0
namespace frontend { void setupTwoPlayersClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452d40
namespace frontend { void setupWebSiteRefused(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452d80
namespace frontend { void setupIsCurrent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452db0
namespace frontend { void setupWebSiteAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452de0
namespace frontend { void setupWebSiteClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452e60
namespace frontend { void setupOnePlayerClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452f20
namespace frontend { void setupOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452f60
namespace frontend { void setupOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x452f80
namespace frontend { void carArtExists(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4530c0
namespace frontend { void showcaseHasInterior(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453160
namespace frontend { void showcaseHasSlides(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453200
namespace frontend { void showcaseHasHistory(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453290
namespace frontend { void showcaseLoadArt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453320
namespace dead { void stub_0x453440(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453440
namespace dead { void stub_0x453470(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453470
namespace dead { void stub_0x4534a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4534a0
namespace frontend { void carViewRemember(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4534d0
namespace frontend { void showcaseResolveCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453500
namespace frontend { void showcaseOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4535e0
namespace frontend { void showcaseOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4537d0
namespace frontend { void showcaseOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453830
namespace frontend { void showcaseNextSlide(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453840
namespace frontend { void showcasePreviousSlide(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453860
namespace frontend { void carStatsDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453940
namespace frontend { void showcaseSlidesOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453c00
namespace frontend { void showcaseSlidesOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453ef0
namespace frontend { void showcaseSlidesOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453f50
namespace frontend { void carSpecsDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x453f60
namespace frontend { void showcaseHistoryPage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454000
namespace frontend { void showcaseHistoryPageFrom(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454210
namespace frontend { void showcaseHistoryOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454260
namespace frontend { void showcaseHistoryOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454310
namespace frontend { void showcaseHistoryOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4543e0
namespace frontend { void showcaseComparePicture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4543f0
namespace frontend { void showcaseCompareOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454500
namespace frontend { void showcaseCompare2OnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454580
namespace frontend { void showcaseCompare2OnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454660
namespace frontend { void showcaseCompareOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454690
namespace frontend { void showcaseCompare2OnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454710
namespace frontend { void carPassesFilter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454730
namespace frontend { void carListDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454780
namespace frontend { void carCompareDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4549c0
namespace frontend { void carCompareLoadArt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454bf0
namespace frontend { void showcaseZoomMoved(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454cf0
namespace frontend { void showcaseInteriorOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454d10
namespace frontend { void showcaseInteriorOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454d40
namespace frontend { void showcaseInteriorOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454d50
namespace frontend { void credits1ShowPage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454d60
namespace frontend { void credits1NextClicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454e40
namespace frontend { void credits1OnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454e80
namespace frontend { void credits1OnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x454ea0
namespace frontend { void resultsHiddenRemote(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455020
namespace frontend { void resultsNetworkNothing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455060
namespace frontend { void tournamentScore(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455070
namespace frontend { void knockoutEliminate(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455140
namespace frontend { void resultsCollectFinishers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455210
namespace frontend { void resultsCollectLaps(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455280
namespace frontend { void resultsAssign(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4552f0
namespace frontend { void resultsBuild(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455410
namespace frontend { void resultsParticipantById(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4554e0
namespace frontend { void resultsParticipantRanks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455530
namespace frontend { void resultsName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455570
namespace frontend { void resultsPlaceText(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4555c0
namespace frontend { void resultsCarName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455600
namespace frontend { void resultsCarLine0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455660
namespace frontend { void resultsCarLine1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4556c0
namespace frontend { void resultsTopSpeedText(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455720
namespace frontend { void resultsParticipantMatches(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4557b0
namespace frontend { void resultsCompare(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455840
namespace frontend { void resultsSort(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455c10
namespace frontend { void resultsNth(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455cd0
namespace frontend { void resultsNthSorted(win32::WinApplication* app, x86::CPU& cpu); }  // 0x455dd0
namespace port { namespace buttons { void textBoxDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x455e30
namespace port { namespace buttons { void textBoxKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456020
namespace port { namespace buttons { void textBoxUp(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456110
namespace port { namespace buttons { void textBoxShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456170
namespace port { namespace buttons { void textBoxHidden(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4561d0
namespace frontend { void infoPlaceTrackPlaces(win32::WinApplication* app, x86::CPU& cpu); }  // 0x456200
namespace port { namespace buttons { void slideTrackMap(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456320
namespace port { namespace buttons { void drawTrackFacts(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456590
namespace port { namespace buttons { void drawGlow(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456670
namespace port { namespace buttons { void trackMapShape(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456800
namespace frontend { void infoOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x456890
namespace frontend { void infoOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4568d0
namespace frontend { void infoOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x456990
namespace port { namespace fields { void colourListWidth(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4569a0
namespace port { namespace fields { void colourListRefresh(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456a20
namespace port { namespace fields { void drawColourLine(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456b50
namespace port { namespace fields { void drawColourList(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456ce0
namespace port { namespace fields { void colourListShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456ea0
namespace port { namespace fields { void pointAtColour(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456f10
namespace port { namespace fields { void openColourList(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456f50
namespace port { namespace fields { void closeColourList(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456f70
namespace port { namespace fields { void chooseColour(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x456f90
namespace port { namespace fields { void colourListKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x457030
namespace port { namespace fields { void layoutColourList(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4571e0
namespace port { namespace fields { void colourListHidden(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x457250
namespace dead { void stub_0x457260(win32::WinApplication* app, x86::CPU& cpu); }  // 0x457260
namespace frontend { void dirPadUnderPointer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x457270
namespace frontend { void dirPadCentreUnderPointer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4572d0
namespace dead { void stub_0x457310(win32::WinApplication* app, x86::CPU& cpu); }  // 0x457310
namespace frontend { void dirPadChoose(win32::WinApplication* app, x86::CPU& cpu); }  // 0x457340
namespace frontend { void dirPadDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x457460
namespace frontend { void dirPadKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4575a0
namespace frontend { void dirPadAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x457620
namespace frontend { void dirPadLayout(win32::WinApplication* app, x86::CPU& cpu); }  // 0x457630
namespace frontend { void dirPadNothing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x457670
namespace frontend { void dirPadLoadShapes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x457680
namespace dead { void stub_0x4576e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4576e0
namespace port { namespace fields { void editKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x457ae0
namespace port { namespace fields { void drawIntField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x457c40
namespace port { namespace fields { void intRangeMessage(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x457df0
namespace port { namespace fields { void intFieldKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x457e30
namespace port { namespace fields { void layoutIntField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x457f70
namespace port { namespace fields { void intFieldShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458010
namespace port { namespace fields { void intFieldHidden(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458050
namespace port { namespace fields { void bindIntField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458080
namespace port { namespace fields { void drawStringField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4580b0
namespace port { namespace fields { void stringFieldKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4581f0
namespace port { namespace fields { void layoutStringField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458340
namespace port { namespace fields { void stringFieldShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4583b0
namespace port { namespace fields { void stringFieldHidden(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458410
namespace frontend { void connectSetEditField(win32::WinApplication* app, x86::CPU& cpu); }  // 0x458450
namespace port { namespace fields { void drawCharField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458480
namespace port { namespace fields { void charRangeMessage(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458640
namespace port { namespace fields { void charFieldKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458690
namespace port { namespace fields { void layoutCharField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4587e0
namespace port { namespace fields { void charFieldShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4588a0
namespace port { namespace fields { void charFieldHidden(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4588e0
namespace port { namespace fields { void bindCharField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458910
namespace port { namespace fields { void drawFloatField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458940
namespace port { namespace fields { void floatRangeMessage(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458b20
namespace port { namespace fields { void floatFieldKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458b80
namespace port { namespace fields { void layoutFloatField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458ce0
namespace port { namespace fields { void floatFieldShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458d90
namespace port { namespace fields { void floatFieldHidden(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458dd0
namespace port { namespace fields { void bindFloatField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458e00
namespace port { namespace fields { void drawFixedField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x458e30
namespace port { namespace fields { void fixedRangeMessage(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x459020
namespace port { namespace fields { void fixedFieldKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x459090
namespace port { namespace fields { void layoutFixedField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x459210
namespace port { namespace fields { void fixedFieldShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4592b0
namespace port { namespace fields { void fixedFieldHidden(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4592f0
namespace port { namespace fields { void bindFixedField(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x459320
namespace graphics { void drawFlatRectangle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x459350
namespace graphics { void hudEditorFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x459440
namespace frontend { void hudDrawElement(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45ab30
namespace frontend { void hudElementPickCursor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45b940
namespace frontend { void hudElementDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45bcb0
namespace frontend { void hudElementKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45bd20
namespace frontend { void hudElementToRecord(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45c9d0
namespace frontend { void hudEditPlaceElement(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45ca90
namespace frontend { void hudElementHidden(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45cb50
namespace frontend { void mainMenuPlayerNameAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45cdd0
namespace frontend { void splitMenuPlayer2NameAnswer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45ce60
namespace frontend { void mainMenuOnPlayerName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45cef0
namespace frontend { void splitMenuOnPlayer2Name(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45cf70
namespace frontend { void mainMenuOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45cff0
namespace frontend { void mainMenuOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45d050
namespace frontend { void mainMenuOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45d2f0
namespace frontend { void splitMenuOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45d300
namespace frontend { void splitMenuOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45d360
namespace frontend { void splitMenuOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45d620
namespace frontend { void levelOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45d630
namespace frontend { void pauseCameraAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45d770
namespace frontend { void pauseCameraKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45d840
namespace frontend { void pauseCameraOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45d920
namespace frontend { void pauseCameraEnsureOne(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45d9a0
namespace frontend { void pauseCameraOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45d9e0
namespace frontend { void raceTypeOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45dab0
namespace frontend { void hudEditShowSpeedometer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45dae0
namespace frontend { void hudEditShowTachometer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45deb0
namespace frontend { void hudEditGreyMirrors(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45e160
namespace frontend { void hudEditGreyOpponentNames(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45e210
namespace frontend { void hudEditPlaceImages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45e310
namespace frontend { void hudEditShowScreenItems(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45e3d0
namespace frontend { void hudEditOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45e960
namespace frontend { void hudEditOnExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45f160
namespace frontend { void hudEditSetScreenArt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45f300
namespace frontend { void hudEditOnFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x45f4c0
namespace port { namespace buttons { void rectDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x45fae0
namespace port { namespace buttons { void speechDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x45fc90
namespace port { namespace buttons { void speechShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x45fca0
namespace port { namespace buttons { void speechHidden(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x45fcb0
namespace port { namespace buttons { void textDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x45fcd0
namespace port { namespace buttons { void textUp(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x460070
namespace port { namespace buttons { void textShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4600b0
namespace port { namespace buttons { void imageDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x460100
namespace port { namespace buttons { void imageUp(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x460410
namespace port { namespace buttons { void imageShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x460440
namespace port { namespace buttons { void pulseOn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x460700
namespace frontend { void resetButtonPulse(win32::WinApplication* app, x86::CPU& cpu); }  // 0x460760
namespace port { namespace buttons { void pulseColour(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x460770
namespace port { namespace buttons { void buttonDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4607c0
namespace port { namespace buttons { void buttonKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x461270
namespace port { namespace buttons { void layoutMenuItem(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4613e0
namespace port { namespace buttons { void buttonShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x461900
namespace port { namespace buttons { void cascadeLayoutClosed(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x461a70
namespace port { namespace buttons { void cascadeLineText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x461aa0
namespace port { namespace buttons { void cascadeDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x461d00
namespace port { namespace buttons { void cascadePass(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x462320
namespace port { namespace buttons { void cascadeShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4623a0
namespace port { namespace buttons { void cascadeKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4624a0
namespace port { namespace buttons { void cascadeUp(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4628d0
namespace port { namespace buttons { void checkboxDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4629c0
namespace port { namespace buttons { void checkboxKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x462ad0
namespace port { namespace buttons { void checkboxShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x462b20
namespace port { namespace buttons { void checkboxUp(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x462b30
namespace port { namespace buttons { void boxOverlayDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x462b90
namespace port { namespace buttons { void selectListShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x462c30
namespace port { namespace buttons { void selectListHidden(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x462ce0
namespace port { namespace buttons { void selectListDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x462d20
namespace port { namespace buttons { void selectListPass(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x462f90
namespace port { namespace buttons { void selectListKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x462fa0
namespace port { namespace buttons { void selectListUp(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4633d0
namespace port { namespace buttons { void dropListDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4634d0
namespace port { namespace buttons { void dropListPass(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x463de0
namespace port { namespace buttons { void dropListShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x463e60
namespace port { namespace buttons { void listKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x463ff0
namespace port { namespace buttons { void layoutList(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x464560
namespace port { namespace view { void storeIncremented(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4646e0
namespace frontend { void viewMakeTiles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x464700
namespace frontend { void viewFillTiles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4647a0
namespace port { namespace view { void gridDrawn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x464840
namespace frontend { void viewLoadInterior(win32::WinApplication* app, x86::CPU& cpu); }  // 0x464eb0
namespace port { namespace view { void viewFrame(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x464fd0
namespace port { namespace view { void panKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4651e0
namespace frontend { void viewWidgetState(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4652f0
namespace frontend { void viewLaidOut(win32::WinApplication* app, x86::CPU& cpu); }  // 0x465320
namespace frontend { void viewNoEntry(win32::WinApplication* app, x86::CPU& cpu); }  // 0x465360
namespace frontend { void viewLoadCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x465370
namespace frontend { void setSliderComponents(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4654c0
namespace dead { void stub_0x4654e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4654e0
namespace dead { void stub_0x4654f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4654f0
namespace port { namespace fields { void drawListBox(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x465500
namespace port { namespace fields { void pointAtListBoxLine(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x465880
namespace port { namespace fields { void listBoxDown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4658f0
namespace port { namespace fields { void listBoxUp(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x465950
namespace port { namespace fields { void listBoxPartAt(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4659a0
namespace port { namespace fields { void listBoxArrowAt(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x465a10
namespace port { namespace fields { void listBoxKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x465a30
namespace port { namespace fields { void layoutListBox(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x465ba0
namespace port { namespace fields { void listBoxShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x465c10
namespace port { namespace fields { void listBoxHidden(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x465c60
namespace port { namespace fields { void listBoxChooseAt(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x465ca0
namespace frontend { void clearMenuArtSlots(win32::WinApplication* app, x86::CPU& cpu); }  // 0x465d60
namespace frontend { void loadMenuArtSlots(win32::WinApplication* app, x86::CPU& cpu); }  // 0x465d80
namespace port { namespace obj3d { void flareQuad(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x465eb0
namespace port { namespace obj3d { void lightPlaced(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x466330
namespace port { namespace obj3d { void headlightGlare(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4663a0
namespace port { namespace obj3d { void taillightGlare(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x466580
namespace port { namespace obj3d { void objectLights(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x466740
namespace port { namespace obj3d { void objectDragged(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4667d0
namespace port { namespace obj3d { void objectBounds(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4668b0
namespace port { namespace obj3d { void objectDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x466cf0
namespace port { namespace obj3d { void objectKeyed(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4675b0
namespace port { namespace obj3d { void objectPassed(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4675d0
namespace port { namespace obj3d { void objectReleased(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x467650
namespace port { namespace obj3d { void objectOpened(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x467700
namespace port { namespace obj3d { void objectClosed(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x467930
namespace ea { void findVivEntry(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4679f0
namespace port { namespace obj3d { void resetLoader(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x467ac0
namespace port { namespace obj3d { void genericVivPath(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x467bc0
namespace port { namespace obj3d { void genericModelName(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x467be0
namespace port { namespace obj3d { void trackVivPath(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x467c00
namespace port { namespace obj3d { void trackModelName(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x467c20
namespace port { namespace obj3d { void chooseFiles(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x467c40
namespace port { namespace obj3d { void readPrehead(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x467ea0
namespace port { namespace obj3d { void readHead(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x467ef0
namespace port { namespace obj3d { void readModel(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x468010
namespace port { namespace obj3d { void readTexture(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x468170
namespace port { namespace obj3d { void readShapes(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4682c0
namespace port { namespace obj3d { void takeModel(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x468410
namespace port { namespace obj3d { void fadeIn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x468580
namespace port { namespace obj3d { void fadeOut(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4685e0
namespace port { namespace obj3d { void wantedChanged(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x468670
namespace port { namespace obj3d { void runLoader(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x468860
namespace port { namespace obj3d { void carColour(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x468da0
namespace port { namespace obj3d { void makeTexture(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x468f00
namespace dead { void stub_0x469020(win32::WinApplication* app, x86::CPU& cpu); }  // 0x469020
namespace port { namespace fields { void sliderPartSizes(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x469080
namespace port { namespace fields { void sliderPartUnderPointer(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4692c0
namespace port { namespace fields { void sliderThumbPlace(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x469340
namespace port { namespace fields { void snapToStep(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4693c0
namespace port { namespace fields { void setSliderValue(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x469480
namespace frontend { void sliderStoredValue(win32::WinApplication* app, x86::CPU& cpu); }  // 0x469520
namespace port { namespace fields { void setSliderStep(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x469530
namespace port { namespace view { void recordFloat(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x469580
namespace port { namespace fields { void dragSliderTo(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x469590
namespace port { namespace fields { void drawSlider(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x469680
namespace port { namespace fields { void trackSlider(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46a5f0
namespace port { namespace fields { void sliderKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46a860
namespace port { namespace fields { void placeSliderParts(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46a980
namespace port { namespace fields { void setUpSlider(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46ab10
namespace port { namespace fields { void sliderBox(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46abb0
namespace port { namespace fields { void twoAxisThumbPlace(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46ac40
namespace port { namespace fields { void setTwoAxisValue(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46add0
namespace frontend { void twoAxisValue(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46afd0
namespace port { namespace fields { void setTwoAxisStep(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46afe0
namespace port { namespace view { void recordFloatAt(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46b030
namespace port { namespace fields { void dragTwoAxisTo(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46b040
namespace port { namespace fields { void drawTwoAxis(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46b3a0
namespace port { namespace fields { void trackTwoAxis(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46c320
namespace port { namespace fields { void twoAxisKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46c4a0
namespace port { namespace fields { void placeTwoAxisParts(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46c640
namespace port { namespace fields { void setUpTwoAxis(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46c820
namespace port { namespace fields { void twoAxisBox(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46c8b0
namespace port { namespace fields { void sliderSetValue(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46c950
namespace frontend { void sliderValue(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46c9b0
namespace port { namespace view { void recordFloatByKind(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46ca70
namespace frontend { void sliderSetOnMove(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46cac0
namespace port { namespace fields { void sliderDraw(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46cae0
namespace port { namespace fields { void sliderWidgetKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46cb80
namespace port { namespace fields { void sliderWidgetLayout(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46cbe0
namespace port { namespace fields { void sliderWidgetHidden(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46cc10
namespace port { namespace fields { void sliderWidgetShown(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46cc50
namespace frontend { void sliderSetColour(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46cca0
namespace frontend { void drawColumnSortMark(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46cce0
namespace frontend { void resultsSortMark(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46cdc0
namespace frontend { void resultsPopupColumn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46cdf0
namespace port { namespace view { void metricPopup(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x46cf30
namespace frontend { void resultsLapTimesPopup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46d180
namespace frontend { void resultsTableDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46d3c0
namespace frontend { void resultsTableShown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46df90
namespace frontend { void resultsTablePass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46e0b0
namespace frontend { void resultsTableLayout(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46e0d0
namespace frontend { void resultsTableKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46e270
namespace frontend { void resultsTableHidden(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46e640
namespace frontend { void standingsSortMark(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46e660
namespace frontend { void standingsTableDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46e690
namespace frontend { void standingsTableShown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46ef90
namespace frontend { void standingsTablePass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46f0a0
namespace frontend { void standingsTableLayout(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46f0c0
namespace frontend { void standingsTableKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46f200
namespace frontend { void standingsTableHidden(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46f4c0
namespace frontend { void roundsSortMark(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46f4e0
namespace frontend { void roundsTableDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46f510
namespace frontend { void roundsTableShown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46fe30
namespace frontend { void roundsTablePass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46fee0
namespace frontend { void roundsTableLayout(win32::WinApplication* app, x86::CPU& cpu); }  // 0x46ff00
namespace frontend { void roundsTableKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470040
namespace frontend { void roundsTableHidden(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470340
namespace frontend { void headToHeadShown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470360
namespace frontend { void headToHeadPass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470390
namespace frontend { void headToHeadDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4703a0
namespace dead { void stub_0x470a70(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470a70
namespace controls { void steadyEngineRumble(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470bc0
namespace controls { void startEngineRumble(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470c60
namespace controls { void silenceEngineRumble(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470cb0
namespace controls { void setForceGain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470d10
namespace controls { void findForceController(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470d30
namespace controls { void createForceEffects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470dc0
namespace controls { void stopForces(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470f40
namespace controls { void stopForcesAgain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470f80
namespace controls { void stopForcesOnceMore(win32::WinApplication* app, x86::CPU& cpu); }  // 0x470fc0
namespace controls { void stopForcesOrActuatorsOn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471000
namespace controls { void stopForcesOrActuatorsOff(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471070
namespace controls { void forcePlayerCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4710f0
namespace controls { void startRaceForces(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4711e0
namespace controls { void stopRaceEffects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471240
namespace controls { void crashJolt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471270
namespace controls { void bumpJolt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471440
namespace controls { void gearChangeKick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4714f0
namespace controls { void steeringAmount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471650
namespace controls { void playSteeringSpring(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4716e0
namespace controls { void playSlideSpring(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471860
namespace controls { void playRoadRumble(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4719f0
namespace controls { void playEngineRumble(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471af0
namespace controls { void isForceCar(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471dc0
namespace controls { void raceForcesFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471e10
namespace controls { void allActuatorsOff(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471fb0
namespace controls { void playJolt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x471fd0
namespace controls { void stopEngineRumble(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4720a0
namespace controls { void stopSteeringSpring(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4720e0
namespace controls { void carSpeedMph(win32::WinApplication* app, x86::CPU& cpu); }  // 0x472110
namespace frontend { void nameValid(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4721f0
namespace load { void languageStub(win32::WinApplication* app, x86::CPU& cpu); }  // 0x472230
namespace load { void setLanguage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x472250
namespace load { void setCameraViews(win32::WinApplication* app, x86::CPU& cpu); }  // 0x472300
namespace port { namespace fields { void refreshCustomColours(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x472360
namespace load { void defaultConfig(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4723f0
namespace load { void saveConfig(win32::WinApplication* app, x86::CPU& cpu); }  // 0x472820
namespace load { void fileWriteTime(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4728e0
namespace load { void fileOlder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x472940
namespace load { void loadConfig(win32::WinApplication* app, x86::CPU& cpu); }  // 0x472980
namespace controls { void loadControllers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x472b60
namespace render { void detailSettings(win32::WinApplication* app, x86::CPU& cpu); }  // 0x472d10
namespace frontend { void titleDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x472f20
namespace frontend { void titleWait(win32::WinApplication* app, x86::CPU& cpu); }  // 0x472fc0
namespace frontend { void skipIntroCompare(win32::WinApplication* app, x86::CPU& cpu); }  // 0x473010
namespace frontend { void menusEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x473020
namespace frontend { void menusLeave(win32::WinApplication* app, x86::CPU& cpu); }  // 0x473080
namespace frontend { void frontEndFirstStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4730d0
namespace frontend { void postgameRun(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4732b0
namespace port { namespace view { void colourNearest(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4733c0
namespace frontend { void textLookClearLights(win32::WinApplication* app, x86::CPU& cpu); }  // 0x474d50
namespace port { namespace view { void edgeTriangles(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x474db0
namespace port { namespace view { void glyphResource(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x475240
namespace render { void solidText(win32::WinApplication* app, x86::CPU& cpu); }  // 0x475b50
namespace dead { void stub_0x476f30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x476f30
namespace dead { void stub_0x476f60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x476f60
namespace dead { void stub_0x476f90(win32::WinApplication* app, x86::CPU& cpu); }  // 0x476f90
namespace dead { void stub_0x477030(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477030
namespace frontend { void textLookSetFlat(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477070
namespace dead { void stub_0x4770c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4770c0
namespace dead { void stub_0x477120(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477120
namespace frontend { void textLookSetDirectional(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477170
namespace frontend { void textLookAddLight(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4771c0
namespace dead { void stub_0x477380(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477380
namespace frontend { void textLookSetTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477470
namespace dead { void stub_0x477690(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477690
namespace dead { void stub_0x4776f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4776f0
namespace dead { void stub_0x477740(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477740
namespace dead { void stub_0x4777a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4777a0
namespace dead { void stub_0x4777b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4777b0
namespace dead { void stub_0x4777c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4777c0
namespace load { void saveGhost(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477a90
namespace load { void writeGhost(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477ac0
namespace load { void readGhost(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477b00
namespace frontend { void clearLoadFlag(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477c00
namespace dead { void stub_0x477df0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x477df0
namespace frontend { void raceEndEmpty(win32::WinApplication* app, x86::CPU& cpu); }  // 0x478000
namespace frontend { void pauseCheckSaveable(win32::WinApplication* app, x86::CPU& cpu); }  // 0x478010
namespace load { void loadHorizon(win32::WinApplication* app, x86::CPU& cpu); }  // 0x478340
namespace render { void sky(win32::WinApplication* app, x86::CPU& cpu); }  // 0x47a190
namespace render { void flatSky(win32::WinApplication* app, x86::CPU& cpu); }  // 0x47a1f0
namespace render { void viewSky(win32::WinApplication* app, x86::CPU& cpu); }  // 0x47a880
namespace load { void loadSky(win32::WinApplication* app, x86::CPU& cpu); }  // 0x47ac30
namespace dead { void stub_0x47b380(win32::WinApplication* app, x86::CPU& cpu); }  // 0x47b380
namespace render { void glareVertices(win32::WinApplication* app, x86::CPU& cpu); }  // 0x47b7d0
namespace render { void skyDome(win32::WinApplication* app, x86::CPU& cpu); }  // 0x47b850
namespace render { void hudFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x47f650
namespace render { void hudBand(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4800a0
namespace render { void hudRect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x480910
namespace render { void hudPicture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x480fc0
namespace render { void hudDial(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4812e0
namespace render { void hudPanel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x481aa0
namespace render { void hudTable(win32::WinApplication* app, x86::CPU& cpu); }  // 0x481c50
namespace dead { void stub_0x482610(win32::WinApplication* app, x86::CPU& cpu); }  // 0x482610
namespace render { void countdown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x482620
namespace graphics { void hudMessageBox(win32::WinApplication* app, x86::CPU& cpu); }  // 0x482940
namespace render { void hud(win32::WinApplication* app, x86::CPU& cpu); }  // 0x482b00
namespace port { namespace hud { void loadingTextPrepared(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x484040
namespace port { namespace hud { void hudStarted(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x484070
namespace frontend { void hudClearRebuildFlag(win32::WinApplication* app, x86::CPU& cpu); }  // 0x484220
namespace dead { void stub_0x484230(win32::WinApplication* app, x86::CPU& cpu); }  // 0x484230
namespace frontend { void hudToggleShown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x484260
namespace dead { void stub_0x484390(win32::WinApplication* app, x86::CPU& cpu); }  // 0x484390
namespace dead { void stub_0x4843f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4843f0
namespace frontend { void hudToggleRearView(win32::WinApplication* app, x86::CPU& cpu); }  // 0x484450
namespace port { namespace hud { void hudPaneSet(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4844c0
namespace port { namespace hud { void screenScalesSet(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x484b70
namespace port { namespace hud { void hudTablesStarted(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x484c10
namespace port { namespace hud { void labelTextSet(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x485180
namespace frontend { void hudRebuildElement(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4853c0
namespace frontend { void hudRebuildView(win32::WinApplication* app, x86::CPU& cpu); }  // 0x485640
namespace frontend { void hudEditLoadDefaults(win32::WinApplication* app, x86::CPU& cpu); }  // 0x485680
namespace port { namespace hud { void standingsGapMeasured(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x485a60
namespace port { namespace hud { void floatsSwapped(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x485c20
namespace port { namespace hud { void pointClipped(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x485c30
namespace port { namespace hud { void segmentClippedToRect(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x485f20
namespace port { namespace hud { void segmentCutIntoPieces(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4860b0
namespace port { namespace hud { void segmentsMet(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x486340
namespace port { namespace hud { void floatsQuadSet(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x486df0
namespace port { namespace hud { void stackQuadSet(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x486f70
namespace port { namespace hud { void rectangleQuadSet(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4870c0
namespace render { void hudInset(win32::WinApplication* app, x86::CPU& cpu); }  // 0x487210
namespace frontend { void hudGroupClearCount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x487340
namespace port { namespace hud { void fourIntegersAppended(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x487350
namespace frontend { void hudGroupDropLast(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4873c0
namespace port { namespace hud { void strokeClippedDrawn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4873e0
namespace port { namespace hud { void gaugeQuadClipped(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x487620
namespace port { namespace hud { void boxQuadsSet(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x487a30
namespace render { void needles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4880b0
namespace render { void barGauges(win32::WinApplication* app, x86::CPU& cpu); }  // 0x488470
namespace graphics { void loadCabinInstruments(win32::WinApplication* app, x86::CPU& cpu); }  // 0x488a80
namespace render { void cockpit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x489240
namespace frontend { void hudOrdinalLetters(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4896f0
namespace frontend { void hudFormatTime(win32::WinApplication* app, x86::CPU& cpu); }  // 0x489750
namespace port { namespace hud { void textAlignedDrawn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4897f0
namespace frontend { void hudDrawTime(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4898b0
namespace frontend { void hudDrawPercentAndCount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x489900
namespace frontend { void hudDrawLaps(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4899d0
namespace frontend { void hudDrawRatio(win32::WinApplication* app, x86::CPU& cpu); }  // 0x489a50
namespace dead { void stub_0x489ab0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x489ab0
namespace port { namespace hud { void listRowsDrawn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x489b30
namespace port { namespace hud { void timeTextDrawn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x489c40
namespace port { namespace hud { void percentTextDrawn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x489d70
namespace port { namespace hud { void indicatorQuadSet(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x489e00
namespace port { namespace hud { void indicatorGeometryUpdated(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48a220
namespace frontend { void hudDrawPlaceAmong(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48a320
namespace port { namespace hud { void rankTextDrawn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48a3a0
namespace port { namespace hud { void labelRowsDrawn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48a660
namespace port { namespace hud { void columnsStarted(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48aa40
namespace port { namespace hud { void standingsDrawn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48ae70
namespace frontend { void hudDrawTriangle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48b2a0
namespace frontend { void hudDrawLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48b4a0
namespace port { namespace hud { void markersSet(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48b5f0
namespace render { void segmentMeter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48c0e0
namespace render { void startLights(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48c7c0
namespace dead { void stub_0x48cae0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48cae0
namespace dead { void stub_0x48caf0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48caf0
namespace port { namespace hud { void pointTransformed(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48cb50
namespace port { namespace hud { void boxDrawn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48cc60
namespace frontend { void hudRefreshPane(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48ccf0
namespace port { namespace hud { void mapOutlineStarted(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48cdb0
namespace port { namespace hud { void hudViewsSet(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48d110
namespace port { namespace hud { void mapPointsDrawn(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48d4c0
namespace frontend { void hudFlushLines(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48e770
namespace frontend { void hudPrepareLines(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48e780
namespace port { namespace hud { void viewPaneStepped(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48e790
namespace frontend { void hudStepMapPanes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48e920
namespace port { namespace hud { void viewZoomStepped(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x48ea20
namespace frontend { void hudStepZoom(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48eb60
namespace port { void timerThread(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48eb90
namespace port { void soundThread(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48ec40
namespace port { void setSoundServer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48ec80
namespace port { void clearTimerCallbacks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48ed00
namespace port { void timerTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48ed30
namespace port { void addTimerCallback(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48ee30
namespace port { void removeTimerCallback(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48ee90
namespace dead { void stub_0x48eef0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48eef0
namespace dead { void stub_0x48ef20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48ef20
namespace dead { void stub_0x48ef30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48ef30
namespace port { void stopTimer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48ef50
namespace port { void startTimer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48eff0
namespace controls { void bindPlayersControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f1f0
namespace controls { void startRaceControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f4f0
namespace controls { void readRaceControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f550
namespace controls { void takeCarControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f6a0
namespace controls { void stepMenuChoice(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f6e0
namespace controls { void stepMenuChoiceOfThree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f710
namespace controls { void actionPressed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f740
namespace controls { void functionKeyPressed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f7b0
namespace controls { void pressFunctionKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f820
namespace controls { void functionKeyRepeats(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f860
namespace controls { void keyPressed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f8d0
namespace controls { void firstFunctionKeyHeld(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f920
namespace controls { void bindControl(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f940
namespace controls { void bindFunctionKeys(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48f970
namespace controls { void queueRaceControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48fa10
namespace dead { void stub_0x48faa0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x48faa0
namespace dead { void stub_0x4900b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4900b0
namespace dead { void stub_0x490110(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490110
namespace dead { void stub_0x490140(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490140
namespace dead { void stub_0x4902f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4902f0
namespace controls { void controlsFor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4904d0
namespace controls { void defaultControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4904f0
namespace controls { void setControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4905b0
namespace controls { void controllerNamed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490610
namespace controls { void knownControllerControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490640
namespace controls { void sideWinderGamePadControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4906b0
namespace controls { void gravisGripControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490770
namespace controls { void wingManForceControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490830
namespace controls { void combatStickControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4908f0
namespace controls { void sideWinderProControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4909b0
namespace controls { void sideWinderWheelControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490a70
namespace controls { void steeringWheelControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490b30
namespace controls { void keyboardControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490c40
namespace controls { void gamePadControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490cb0
namespace controls { void joystickControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490e30
namespace controls { void otherControllerControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x490fa0
namespace render { void glow(win32::WinApplication* app, x86::CPU& cpu); }  // 0x491190
namespace render { void roadLight(win32::WinApplication* app, x86::CPU& cpu); }  // 0x491bc0
namespace vertices { void lightGlow(win32::WinApplication* app, x86::CPU& cpu); }  // 0x492370
namespace vertices { void headlightBeam(win32::WinApplication* app, x86::CPU& cpu); }  // 0x492980
namespace render { void lightning(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4946e0
namespace graphics { void drawLoadingAborted(win32::WinApplication* app, x86::CPU& cpu); }  // 0x494bd0
namespace render { void loadingScreen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x494cb0
namespace graphics { void showLoadingScreen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x494fa0
namespace graphics { void drawLoadingProgress(win32::WinApplication* app, x86::CPU& cpu); }  // 0x495230
namespace frontend { void loadQfs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4956f0
namespace dead { void stub_0x495a00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x495a00
namespace frontend { void playMovie(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4960e0
namespace frontend { void resetAttractTimer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4961e0
namespace frontend { void attractMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496200
namespace dead { void stub_0x496370(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496370
namespace dead { void stub_0x496390(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496390
namespace dead { void stub_0x496440(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496440
namespace dead { void stub_0x496490(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496490
namespace frontend { void simQuickDistance(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4965b0
namespace dead { void stub_0x496690(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496690
namespace frontend { void simPlanarLength(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4967e0
namespace vertices { void groundHeight(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4968a0
namespace dead { void stub_0x496b10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496b10
namespace dead { void stub_0x496b60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496b60
namespace dead { void stub_0x496c40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496c40
namespace dead { void stub_0x496cb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496cb0
namespace dead { void stub_0x496df0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496df0
namespace dead { void stub_0x496fc0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x496fc0
namespace dead { void stub_0x497010(win32::WinApplication* app, x86::CPU& cpu); }  // 0x497010
namespace dead { void stub_0x497240(win32::WinApplication* app, x86::CPU& cpu); }  // 0x497240
namespace vertices { void normalise(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4972f0
namespace dead { void stub_0x497360(win32::WinApplication* app, x86::CPU& cpu); }  // 0x497360
namespace dead { void stub_0x497370(win32::WinApplication* app, x86::CPU& cpu); }  // 0x497370
namespace dead { void stub_0x497380(win32::WinApplication* app, x86::CPU& cpu); }  // 0x497380
namespace dead { void stub_0x4973e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4973e0
namespace dead { void stub_0x497490(win32::WinApplication* app, x86::CPU& cpu); }  // 0x497490
namespace frontend { void hudVectorLength(win32::WinApplication* app, x86::CPU& cpu); }  // 0x497590
namespace dead { void stub_0x4992f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4992f0
namespace dead { void stub_0x499300(win32::WinApplication* app, x86::CPU& cpu); }  // 0x499300
namespace frontend { void freeNetworkLocks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49a4d0
namespace frontend { void allPlayersReady(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49ab60
namespace frontend { void lobbyDrawPlayers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49ae30
namespace load { void beginCarpReceive(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49b540
namespace load { void receiveCarpChunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49b5f0
namespace load { void beginCarpSend(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49b800
namespace port { namespace text { void takeQuad(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x49bcd0
namespace dead { void stub_0x49bcf0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49bcf0
namespace port { namespace text { void queueQuad(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x49bd30
namespace port { namespace text { void unqueueQuad(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x49bdc0
namespace port { namespace text { void flushQuads(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x49be40
namespace frontend { void hudSurfaceReset(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49bef0
namespace frontend { void hudSurfaceTakeVertices(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49bf40
namespace graphics { void flushLineMesh(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49bf60
namespace frontend { void setPointer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49bfc0
namespace port { namespace text { void mouseMenuKey(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x49c000
namespace frontend { void mouseKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c080
namespace frontend { void connectSetPointer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c0e0
namespace frontend { void menuPointerLoadArt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c140
namespace dead { void stub_0x49c1b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c1b0
namespace frontend { void menuPointerListLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c1c0
namespace frontend { void pointerInBox(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c1f0
namespace frontend { void menuPointerOverRect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c240
namespace dead { void stub_0x49c280(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c280
namespace dead { void stub_0x49c290(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c290
namespace frontend { void menuPointerFreeArt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c2a0
namespace dead { void stub_0x49c2d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c2d0
namespace dead { void stub_0x49c310(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c310
namespace dead { void stub_0x49c3b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c3b0
namespace graphics { void showPicture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c570
namespace graphics { void textureFormatFor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c5c0
namespace graphics { void allocTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c600
namespace graphics { void updateTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c620
namespace load { void readCarColors(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c700
namespace load { void rgbToHsv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49c880
namespace load { void hsvToRgb(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49ca50
namespace load { void tintPixel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49cc50
namespace load { void scaleTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49cd00
namespace load { void fitTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49cea0
namespace load { void loadArtTextures(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49d130
namespace load { void nearestModelColor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49d240
namespace load { void loadTgaTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49d320
namespace load { void loadCarModel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49d450
namespace decompiled { void resetSunLighting(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49d640
namespace render { void glareColours(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49d800
namespace vertices { void objectVerticesNearest(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49d9d0
namespace vertices { void shadeNormals(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49da30
namespace vertices { void envMapCoords(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49db70
namespace vertices { void cullPolygons(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49dbf0
namespace frontend { void connectConnectionAvailable(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49e360
namespace frontend { void connectListSessions(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49e3d0
namespace frontend { void connectCreateSession(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49e5f0
namespace frontend { void connectJoinSession(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49e800
namespace frontend { void connectUseTcp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49e9d0
namespace frontend { void connectFindAddress(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49ea70
namespace ea { void dropMachine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49ecb0
namespace ea { void announceMachineLeft(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49ecf0
namespace ea { void endNetworkGame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49eed0
namespace frontend { void connectSetUpEndpoint(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49efb0
namespace frontend { void connectEnumerateServices(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f030
namespace frontend { void connectFindConnections(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f150
namespace ea { void sessionIndex(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f1c0
namespace frontend { void connectServiceFound(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f200
namespace frontend { void connectSessionFound(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f380
namespace frontend { void connectQueuePeerByName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f470
namespace frontend { void connectQueuePeer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f4b0
namespace frontend { void connectClearPeerSlots(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f520
namespace ea { void forgetPendingPlayer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f550
namespace frontend { void connectRememberPeer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f5c0
namespace frontend { void connectClearQueue(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f650
namespace frontend { void connectEnqueuePeer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f6a0
namespace dead { void stub_0x49f940(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49f940
namespace dead { void stub_0x49fa90(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49fa90
namespace dead { void stub_0x49fc80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49fc80
namespace dead { void stub_0x49fda0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49fda0
namespace dead { void stub_0x49fdd0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49fdd0
namespace frontend { void simCarTiltedOver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49fde0
namespace port { namespace conv { void fn49fe70(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x49fe70
namespace frontend { void simSliceHeading(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49fee0
namespace frontend { void simCarTrackFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x49ffa0
namespace frontend { void simNodeGap(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a00c0
namespace port { namespace conv { void fn4a00d0(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a00d0
namespace frontend { void simCarOntoRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a0140
namespace port { namespace conv { void fn4a0440(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a0440
namespace frontend { void simCarNearPlayers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a0580
namespace frontend { void simCarTrackEnvironment(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a08c0
namespace port { namespace conv { void fn4a0980(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a0980
namespace port { namespace conv { void fn4a0a10(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a0a10
namespace frontend { void simTrackPlaceOnRoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a0aa0
namespace port { namespace conv { void fn4a0d20(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a0d20
namespace port { namespace conv { void fn4a0f30(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a0f30
namespace port { namespace conv { void fn4a1090(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a1090
namespace frontend { void simTrackProbeWalls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a1210
namespace frontend { void simTrackPredictContact(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a1610
namespace frontend { void simTrackReactToSurface(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a1ce0
namespace port { namespace conv { void fn4a20c0(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a20c0
namespace frontend { void simTrackTunnelRoof(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a21a0
namespace frontend { void simTrackDownforceAndDrag(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a2260
namespace port { namespace conv { void fn4a2630(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a2630
namespace frontend { void simTrackSettle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a2690
namespace port { namespace conv { void fn4a2830(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a2830
namespace frontend { void simTrackOffsetAcross(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a2bc0
namespace port { namespace conv { void fn4a2c60(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a2c60
namespace frontend { void simTrackTurnMatrix(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a2cc0
namespace port { namespace conv { void fn4a2e20(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4a2e20
namespace frontend { void simTrackSpinThirtySecond(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a2f10
namespace frontend { void simTrackSpinSixtyFourth(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a2f30
namespace graphics { void takeDownRace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a35d0
namespace graphics { void gameMain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a39f0
namespace frontend { void effectsLogoTextureStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a4390
namespace render { void bouncers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a4410
namespace frontend { void effectsBandDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a47c0
namespace frontend { void effectsScreenDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a49c0
namespace frontend { void effectsScreenStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a4a10
namespace frontend { void effectsLoadScreenArt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a4ba0
namespace frontend { void effectsLogoDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a4c20
namespace dead { void stub_0x4a5aa0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a5aa0
namespace ea { void sendOverLink(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a5d10
namespace dead { void stub_0x4a5d70(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a5d70
namespace dead { void stub_0x4a5f20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a5f20
namespace dead { void stub_0x4a5f80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a5f80
namespace dead { void stub_0x4a5f90(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a5f90
namespace dead { void stub_0x4a5fb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a5fb0
namespace dead { void stub_0x4a5ff0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a5ff0
namespace dead { void stub_0x4a6020(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a6020
namespace dead { void stub_0x4a60a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a60a0
namespace dead { void stub_0x4a60ab(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a60ab
namespace dead { void stub_0x4a60d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a60d0
namespace ea { void sendPacket(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a60e0
namespace ea { void resetNetwork(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a6390
namespace ea { void closeNetwork(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a65e0
namespace ea { void machineNumber(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a6650
namespace ea { void playerMachine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a6690
namespace dead { void stub_0x4a78b6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a78b6
namespace handling { void applyTuning(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a78d6
namespace dead { void stub_0x4a83af(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a83af
namespace drivetrain { void computerDriver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a83d3
namespace dead { void stub_0x4a8a2b(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a8a2b
namespace drivetrain { void engineTorque(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a8a99
namespace handling { void automaticGear(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a8bb9
namespace drivetrain { void automaticGears(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a90a0
namespace drivetrain { void gearLimit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a9769
namespace drivetrain { void autoReverse(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a97cb
namespace drivetrain { void driverInputs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4a9bc4
namespace handling { void matchRevsToSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4aa4f4
namespace dead { void stub_0x4aa682(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4aa682
namespace handling { void resetDriveState(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4aa79f
namespace handling { void settleAtRest(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4aa8e7
namespace drivetrain { void wheelGrip(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4aab41
namespace drivetrain { void engineForce(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4aaed1
namespace handling { void setupGripLoss(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4abd9c
namespace handling { void setupSlipLimit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ac027
namespace handling { void tyreGrip(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ac094
namespace handling { void tyreSlide(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ac3af
namespace handling { void logisticCurve(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ac994
namespace handling { void steeringGrip(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ac9dd
namespace handling { void axleForces(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4aca4f
namespace physics { void clearWheelForces(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ad250
namespace physics { void steerAngle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ad2e1
namespace physics { void sideSlip(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ad672
namespace physics { void groundSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ad6eb
namespace physics { void physicsStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ad7a3
namespace physics { void carPhysics(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ae4c1
namespace physics { void settleLoadShift(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ae5c2
namespace physics { void driveForce(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ae68e
namespace physics { void holdStill(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ae79b
namespace physics { void sideGrip(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ae85b
namespace physics { void handbrakeDrag(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ae98b
namespace physics { void yawDamping(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4aea88
namespace physics { void idleDamping(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4aeffc
namespace physics { void coastDamping(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4af140
namespace physics { void spinTorque(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4af254
namespace dead { void stub_0x4b54f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b54f0
namespace render { void transformBuffer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b56e0
namespace dead { void stub_0x4b5710(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b5710
namespace render { void startDriver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b59c0
namespace graphics { void warnTextureMemory(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b5ea0
namespace graphics { void shutDown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b6250
namespace frontend { void pauseQuitToAnswered(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b67b0
namespace frontend { void pauseQuitToAsk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b6810
namespace frontend { void pauseQuitToReplay(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b6870
namespace frontend { void pauseQuitToPost(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b6890
namespace frontend { void pauseQuitToMain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b68d0
namespace frontend { void pauseQuitToSystem(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b6910
namespace frontend { void pauseQuitToOnEnter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b6930
namespace dead { void stub_0x4b6a10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b6a10
namespace dead { void stub_0x4b6bb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b6bb0
namespace dead { void stub_0x4b70b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b70b0
namespace frontend { void simPoseBlend(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b70f0
namespace dead { void stub_0x4b7180(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7180
namespace frontend { void simQuatSlerp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b71e0
namespace frontend { void simQuatNearer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7300
namespace frontend { void simQuatFromMatrix(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b73c0
namespace frontend { void simMatrixFromQuat(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7590
namespace frontend { void simMatrixSlerp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7680
namespace dead { void stub_0x4b76d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b76d0
namespace frontend { void simMatrixSubtract(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7720
namespace frontend { void simMatrixAdd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7780
namespace frontend { void simMatrixScale(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b77e0
namespace frontend { void simMatrixOrthonormalise(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7840
namespace dead { void stub_0x4b7920(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7920
namespace frontend { void simMatrixQuarterTurn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b79c0
namespace load { void loadVivCarModel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7a60
namespace load { void copyVivCarModel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7ba0
namespace render { void carTextures(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7d30
namespace dead { void stub_0x4b7f30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b7f30
namespace render { void nightColour(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b97b0
namespace render { void carLights(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4b9bc0
namespace render { void detailedCarLights(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bae00
namespace render { void mediumCarLights(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bb200
namespace vertices { void carDetail(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bb5d0
namespace dead { void stub_0x4bbcc0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bbcc0
namespace dead { void stub_0x4bbd00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bbd00
namespace dead { void stub_0x4bbd30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bbd30
namespace graphics { void raceFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bc160
namespace frontend { void simViewPass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bc470
namespace frontend { void pauseResetRaceMouse(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bc530
namespace frontend { void pauseMenuAgain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bc660
namespace frontend { void pauseEndRestorePointer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bc670
namespace load { void saveRaceReplay(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bd1e0
namespace load { void readRaceReplay(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bd2e0
namespace frontend { void simPanelPointer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bd4d0
namespace frontend { void simPanelKeys(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bd640
namespace frontend { void simTvSetCamera(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bdb60
namespace frontend { void simTvNextCamera(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bdbd0
namespace frontend { void simTvShotOver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bdc70
namespace frontend { void simTvStartShot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bdcc0
namespace frontend { void simTvBusiest(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bdd10
namespace frontend { void simTvDirect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bdd70
namespace frontend { void simIsNumberCharacter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4be000
namespace dead { void stub_0x4be090(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4be090
namespace frontend { void simOffBeat(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4be1a0
namespace frontend { void pauseFillViewQuads(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4be3d0
namespace graphics { void modeSize(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4be570
namespace graphics { void drawViewSurround(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4be8e0
namespace frontend { void pauseSetRectFirst(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4beb00
namespace frontend { void pauseReadRect(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4beb20
namespace frontend { void pauseSetRectSecond(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4beb40
namespace dead { void stub_0x4beb60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4beb60
namespace port { namespace text { void viewport(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4beb80
namespace graphics { void setClip(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bebe0
namespace dead { void stub_0x4becd0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4becd0
namespace graphics { void listScreenModes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bed90
namespace graphics { void findScreenMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bee80
namespace render { void screenMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bef50
namespace dead { void stub_0x4bf160(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bf160
namespace vertices { void objectVertices(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bf4c0
namespace render { void clipByZ(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4bf790
namespace render { void line(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c0b20
namespace graphics { void dead_0x4c0f60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c0f60
namespace vertices { void clipTriangle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c11b0
namespace vertices { void clipQuad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c12e0
namespace vertices { void clipTriangleByZ(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c1aa0
namespace vertices { void quadMesh(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c1e20
namespace render { void quadByZ(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c20e0
namespace vertices { void clipPolygon(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c2cd0
namespace render { void nearTriangle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c3ad0
namespace frontend { void connectStartConnection(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c3f90
namespace frontend { void connectListModems(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c41d0
namespace frontend { void connectSetUpLink(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c4230
namespace frontend { void connectShakeHands(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c4310
namespace dead { void stub_0x4c4480(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c4480
namespace graphics { void captureScreen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c4490
namespace graphics { void showWaitMessage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c5c00
namespace controls { void appendToPacket(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c6080
namespace controls { void sendOwnInput(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c60c0
namespace controls { void receiveInput(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c61e0
namespace controls { void queueInput(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c6530
namespace controls { void queueRemoteInput(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c66a0
namespace controls { void takeQueuedFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c67a0
namespace controls { void takenControls(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c6840
namespace controls { void repeatMissingInputs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c6870
namespace controls { void syncFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c6970
namespace controls { void framesTaken(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c69a0
namespace controls { void framesQueued(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c69b0
namespace controls { void queueLead(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c69c0
namespace controls { void noteLag(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c6a10
namespace controls { void setQueueDelays(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c6a40
namespace controls { void queueFull(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c6ae0
namespace vertices { void gatherSorted(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c7320
namespace vertices { void sortedBuckets(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c7610
namespace vertices { void depthBuckets(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c7790
namespace vertices { void gatherBuckets(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c79a0
namespace decompiled { void setDepthOffsets(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c7a60
namespace dead { void stub_0x4c8200(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c8200
namespace port { void speechInit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c8370
namespace port { void speechFree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c8440
namespace port { void speechLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c85a0
namespace port { void speechTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c86d0
namespace port { void radioTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c8810
namespace port { void radioStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c89b0
namespace port { void speechSaid(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c8ac0
namespace port { void copRadio(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c8b30
namespace port { void queueSpeechLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c8ca0
namespace port { void coDriverTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c8d10
namespace port { void queueRaceLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c8f90
namespace port { void raceEventLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9010
namespace port { void gapLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9090
namespace port { void lapTimeLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c91a0
namespace snd { void descriptionLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9240
namespace port { void descriptionSpeak(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9340
namespace port { void descriptionTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9380
namespace port { void descriptionStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9490
namespace frontend { void infoSpeak(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c94e0
namespace frontend { void infoStopSpeaking(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9590
namespace port { void spokenStatus(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c95e0
namespace port { void descriptionStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c95f0
namespace port { void descriptionService(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9860
namespace port { void raceVoiceLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9890
namespace port { void carLineSpeak(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9990
namespace port { void spokenStop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c99d0
namespace port { void spokenStatus(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c99f0
namespace port { void raceVoiceFree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9a00
namespace frontend { void simSirenOn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9dd0
namespace frontend { void simSirenOff(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4c9f50
namespace vertices { void moveParticles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cb2c0
namespace render { void clearBuffers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cb670
namespace vertices { void drops(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cb750
namespace vertices { void particles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cbc90
namespace dead { void stub_0x4cc820(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cc820
namespace dead { void stub_0x4cc840(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cc840
namespace frontend { void nextKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cc970
namespace dead { void stub_0x4cca00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cca00
namespace dead { void stub_0x4ccbb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ccbb0
namespace dead { void stub_0x4cdc20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cdc20
namespace dead { void stub_0x4cdcd0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cdcd0
namespace dead { void stub_0x4cebf0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cebf0
namespace dead { void stub_0x4cecc0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cecc0
namespace frontend { void resultsMetricText(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cedd0
namespace frontend { void resultsTimeText(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cee40
namespace frontend { void recordsHolder(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cef00
namespace frontend { void postgameFinishers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cef80
namespace dead { void stub_0x4cefc0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cefc0
namespace frontend { void postgameFinishersCount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cf000
namespace frontend { void postgameReplayExcluded(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cf040
namespace frontend { void postgameRaceAgain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cf090
namespace dead { void stub_0x4cf120(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cf120
namespace frontend { void lapsToOption(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cf160
namespace frontend { void optionToLaps(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cf190
namespace dead { void stub_0x4cfda0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cfda0
namespace dead { void stub_0x4cfec0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4cfec0
namespace dead { void stub_0x4d0540(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d0540
namespace dead { void stub_0x4d0570(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d0570
namespace dead { void stub_0x4d0610(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d0610
namespace port { namespace text { void drawCharacter(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d06f0
namespace render { void pictureTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d0a10
namespace port { namespace text { void drawText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1000
namespace frontend { void hudTextToRightEdge(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d10e0
namespace graphics { void drawTextBox(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d1120
namespace port { namespace fields { void drawCursorText3d(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d11f0
namespace frontend { void hudFontSurfacesBegin(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d1310
namespace port { namespace text { void flushFonts(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1340
namespace port { namespace text { void setTextScale(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1390
namespace port { namespace text { void keepTextScale(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1400
namespace port { namespace text { void restoreTextScale(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1410
namespace port { namespace text { void measureText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1430
namespace port { namespace text { void textWidthWithOverhang(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1480
namespace frontend { void hudCharacterDrop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d14f0
namespace port { namespace text { void characterWidth(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1540
namespace port { namespace text { void characterOverhang(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1590
namespace dead { void stub_0x4d15e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d15e0
namespace port { namespace text { void loadLanguageText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1750
namespace port { namespace text { void specialText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d17b0
namespace port { namespace text { void languageText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1850
namespace frontend { void languageTextsFree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d1880
namespace port { namespace shapes { void scan4444(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d18e0
namespace port { namespace shapes { void scan8888(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1950
namespace port { namespace shapes { void scanNibbles(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d19b0
namespace port { namespace shapes { void scanPaletted(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1a00
namespace port { namespace shapes { void scan1555(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1a90
namespace port { namespace shapes { void scan565(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1ad0
namespace port { namespace shapes { void convert4444(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1b20
namespace port { namespace shapes { void convert8888(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1c00
namespace port { namespace shapes { void convert8(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1ce0
namespace port { namespace shapes { void convert565(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1db0
namespace port { namespace shapes { void convert1555(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1e60
namespace port { namespace shapes { void nibblesTo1555(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d1f70
namespace port { namespace shapes { void nibblesTo565(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d2050
namespace port { namespace shapes { void nibblesTo4444(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d2150
namespace port { namespace shapes { void nibblesTo8888(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d21f0
namespace port { namespace shapes { void paletteTo1555(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d22b0
namespace port { namespace shapes { void paletteTo565(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d23f0
namespace port { namespace shapes { void paletteTo4444(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d2510
namespace port { namespace shapes { void paletteTo8888(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d2690
namespace port { namespace shapes { void argb8888To4444(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d27e0
namespace port { namespace shapes { void argb8888To565(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d2940
namespace port { namespace shapes { void argb8888To1555(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d2a80
namespace port { namespace shapes { void rgb565To1555(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d2be0
namespace port { namespace shapes { void rgb565To4444(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d2d20
namespace port { namespace shapes { void rgb565To8888(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d2e60
namespace port { namespace shapes { void argb1555To565(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d2fb0
namespace port { namespace shapes { void argb1555To4444(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d30f0
namespace port { namespace shapes { void argb1555To8888(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d3230
namespace dead { void stub_0x4d3410(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d3410
namespace dead { void stub_0x4d34b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d34b0
namespace port { namespace shapes { void shapeToPixels(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d37a0
namespace render { void squareTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d3f50
namespace render { void paletteTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d41a0
namespace dead { void stub_0x4d43e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d43e0
namespace render { void pieceTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d4580
namespace render { void cabinTextures(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d4ae0
namespace dead { void stub_0x4d5030(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d5030
namespace dead { void stub_0x4d5360(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d5360
namespace dead { void stub_0x4d56c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d56c0
namespace dead { void stub_0x4d56d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d56d0
namespace dead { void stub_0x4d56e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d56e0
namespace frontend { void simRaceFrameEnd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d5720
namespace port { namespace draw2d { void quadOutcodes(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d5840
namespace dead { void stub_0x4d5a80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d5a80
namespace port { namespace shapes { void resetTextures(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d5c40
namespace graphics { void clearWithDepth(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d5c90
namespace graphics { void beginDrawing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d5cc0
namespace graphics { void clearBothBuffers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d5d30
namespace graphics { void clearBothBuffersAndReset(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d5d70
namespace port { namespace shapes { void shapeByName(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d5db0
namespace port { namespace fields { void sliderShape(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d5e40
namespace graphics { void shapeIntoTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d5ed0
namespace graphics { void makeArtTexture(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d5f40
namespace port { namespace shapes { void shapeTexture(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d6070
namespace port { namespace shapes { void cutTile(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d6110
namespace port { namespace shapes { void shape565To1555(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d6190
namespace port { namespace shapes { void tileRoom(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d6210
namespace port { namespace shapes { void artShapeTextures(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d6390
namespace port { namespace shapes { void shapeSize(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d66f0
namespace port { namespace buttons { void drawTurnedShape(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d6730
namespace dead { void stub_0x4d6c70(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d6c70
namespace port { namespace shapes { void drawShapeQuad(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d70f0
namespace port { namespace shapes { void drawArtShape(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d7160
namespace frontend { void artTilesDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d7560
namespace port { namespace shapes { void drawShape(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d7990
namespace port { namespace obj3d { void drawLogo(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d79b0
namespace frontend { void bouncerCornerFade(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d7cc0
namespace frontend { void bouncerDraw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d7d90
namespace port { namespace draw2d { void drawShadedRectangle(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d81b0
namespace port { namespace draw2d { void gradientRectangle(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d8550
namespace port { namespace fields { void drawShadedTriangle(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d8570
namespace port { namespace draw2d { void drawShadedBox(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d8860
namespace frontend { void pickerShadedTriangle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d88c0
namespace port { namespace buttons { void drawColouredRectangle(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d88f0
namespace port { namespace draw2d { void filledRectangle(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d8910
namespace port { namespace draw2d { void frame(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d8930
namespace port { namespace draw2d { void drawPanel(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d89f0
namespace port { namespace fields { void drawFrameAt(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d8aa0
namespace port { namespace draw2d { void drawFieldBox(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d8ac0
namespace port { namespace draw2d { void drawTileRectangle(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d8b80
namespace port { namespace draw2d { void drawLine(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d8ed0
namespace graphics { void takeWindowAndClear(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d9120
namespace graphics { void flipAndWaitFrames(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d9140
namespace frontend { void effectsBandColourScaled(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4d9180
namespace port { namespace draw2d { void fontCharacter(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d9240
namespace port { namespace draw2d { void accentedCharacter(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d9320
namespace port { namespace draw2d { void buildScreenFont(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d93a0
namespace port { namespace draw2d { void drawScreenCharacter(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d95c0
namespace port { namespace draw2d { void drawTurnedCharacter(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4d9b90
namespace port { namespace draw2d { void drawTurnedText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4da120
namespace port { namespace draw2d { void drawTitleText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4da1e0
namespace port { namespace draw2d { void drawScreenText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4da4a0
namespace port { namespace fields { void drawCursor(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4da560
namespace port { namespace fields { void drawCursorText(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4da8c0
namespace frontend { void menuPointerLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4daa10
namespace frontend { void menuPointerHotSpot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dab10
namespace port { namespace draw2d { void drawPointer(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4dab90
namespace port { namespace draw2d { void startScreens(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4daf80
namespace port { namespace draw2d { void drawTexturedQuad(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4db000
namespace port { namespace draw2d { void drawShadedQuad(win32::WinApplication* app, x86::CPU& cpu); } }  // 0x4db380
namespace frontend { void simSetView(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dbde0
namespace vertices { void objectVerticesList(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dbef0
namespace dead { void stub_0x4dc220(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc220
namespace dead { void stub_0x4dc260(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc260
namespace dead { void stub_0x4dc2a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc2a0
namespace dead { void stub_0x4dc2e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc2e0
namespace dead { void stub_0x4dc300(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc300
namespace dead { void stub_0x4dc350(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc350
namespace load { void setSpecUsed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc3a0
namespace load { void specUsed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc3f0
namespace load { void specCarName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc440
namespace load { void findSpec(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc490
namespace load { void freeSpecSlot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc500
namespace load { void setSpec(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc5a0
namespace load { void defaultSpecs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc670
namespace load { void refreshSpecs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc710
namespace load { void specPath(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc800
namespace load { void readSpecs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc8b0
namespace load { void loadPlayerSpecs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dc940
namespace load { void savePlayerSpecs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dca50
namespace frontend { void tuningAdvancedDiffers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dcaf0
namespace frontend { void tuningCopyAdvanced(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dcb60
namespace load { void loadWeatherShapes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dcd20
namespace dead { void stub_0x4dd1f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dd1f0
namespace render { void sparks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ddb40
namespace render { void smoke(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4de070
namespace render { void streaks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4de700
namespace dead { void stub_0x4df350(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df350
namespace dead { void stub_0x4df380(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df380
namespace dead { void stub_0x4df390(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df390
namespace dead { void stub_0x4df3a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df3a0
namespace dead { void stub_0x4df420(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df420
namespace dead { void stub_0x4df530(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df530
namespace dead { void stub_0x4df570(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df570
namespace dead { void stub_0x4df5c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df5c0
namespace watcom { void mem_putc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df660
namespace watcom { void vsprintf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df674
namespace watcom { void sprintf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df690
void Message(win32::WinApplication* app, x86::CPU& cpu);  // 0x4df6c0
namespace ea { void waitForObjects(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df6f0
namespace ea { void threadStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df730
namespace ea { void threadsInit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df740
namespace ea { void createThread(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df7a0
namespace ea { void removeThread(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df870
namespace ea { void sleepMs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df8e0
namespace ea { void sleepTicks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df8f0
namespace ea { void isThread(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df930
namespace ea { void threadPriority(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4df980
namespace ea { void setThreadPriority(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfa30
namespace ea { void exitThread(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfaf0
namespace ea { void createEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfb00
namespace ea { void postEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfb20
namespace ea { void signalEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfb30
namespace ea { void pollEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfb40
namespace ea { void waitEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfb70
namespace ea { void waitEventTicks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfb90
namespace ea { void waitForAny(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfbf0
namespace ea { void closeEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfc00
namespace ea { void createManualEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfc10
namespace ea { void setEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfc30
namespace ea { void resetEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfc40
namespace ea { void pulseEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfc50
namespace ea { void waitManualEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfc60
namespace ea { void setAndCloseEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfc80
namespace ea { void lastErrorText(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfca0
namespace ea { void setLastError(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfce0
namespace ea { void lastError(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfcf0
namespace ea { void doNothing(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfd00
namespace watcom { void exit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfd04
namespace watcom { void _exit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfd30
namespace watcom { void CHP(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfd56
namespace ea { void xRotation(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfd80
namespace ea { void yRotation(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfde0
namespace ea { void zRotation(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfe40
namespace ea { void axisRotation(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dfea0
namespace vertices { void addVectors(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4dffb0
namespace vertices { void subtractVectors(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0000
namespace vertices { void scaleVectors(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0050
namespace dead { void stub_0x4e00c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e00c0
namespace vertices { void unitVector(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e01c0
namespace vertices { void dotPush(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e01f0
namespace vertices { void dotProducts(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0210
namespace dead { void stub_0x4e0250(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0250
namespace vertices { void multiplyMatrices(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0280
namespace vertices { void rotateVectors(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e03b0
namespace vertices { void transformPoints(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0430
namespace dead { void stub_0x4e04e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e04e0
namespace dead { void stub_0x4e05b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e05b0
namespace dead { void stub_0x4e0620(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0620
namespace watcom { void memset(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0640
namespace dead { void stub_0x4e0660(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0660
namespace vertices { void vectorLengthOf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e06a0
namespace dead { void stub_0x4e06c4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e06c4
namespace watcom { void IF_DATAN2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e06d9
namespace dead { void stub_0x4e06ee(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e06ee
namespace dead { void stub_0x4e06fa(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e06fa
void fillZero(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e070c
void fillByte(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e0716
void fillWord(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e0721
namespace watcom { void strstr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0820
namespace ea { void monoPrintf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e08f0
namespace ea { void monoPrintfAt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0980
namespace ea { void monoWrite(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0a60
namespace ea { void monoScroll(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0c10
namespace ea { void monoClear(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0ca0
namespace ea { void monoSetTop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0d00
namespace ea { void monoSetAttribute(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0d50
namespace ea { void monoSetTab(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0d60
namespace ea { void monoPage(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0d70
namespace ea { void monoNoCursor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0d80
namespace port { void eaRandom(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0d88
namespace ea { void seedRandom(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0dee
namespace watcom { void strncpy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0e30
namespace ea { void fileExists(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0e60
namespace ea { void loadFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0ec0
namespace ea { void loadFileQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e0fe0
namespace ea { void findFreeBlock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1000
namespace ea { void findLargestFreeBlock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1030
namespace ea { void linkFreeBlock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1080
namespace ea { void unlinkFreeBlock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e10e0
namespace ea { void clearMemClasses(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1100
namespace ea { void markMemBlock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1120
namespace ea { void memClassOverhead(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e11e0
namespace ea { void memBlockOverhead(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e11f0
namespace ea { void creatememclass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1230
namespace ea { void releasememclass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e14b0
namespace ea { void getfreememclass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1500
namespace ea { void largestmem(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1580
namespace ea { void largestmemclass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e15b0
namespace ea { void memBlockSize(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e15d0
namespace ea { void memBlockName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e15e0
namespace ea { void memBlockFlags(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1610
namespace ea { void reservememadr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1620
namespace ea { void purgememadr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1890
namespace ea { void purgememclass(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1990
namespace ea { void reservememStdcall(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e19d0
namespace ea { void purgememStdcall(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e19f0
void StopSoundBufferLambda(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e1a00
void AcquireSoundBufferPositionLambda(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e1a0c
void AcquireSoundDataLambda(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e1a24
void PrintSoundLambda(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e1a34
void TraceSoundLambda(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e1a64
void abortmessagewrapper(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e1a88
void TickSoundLambda(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e1adc
void iSNDconvertsample(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e1ae4
namespace port { void leaveAudioLocks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e1d1c
void audiofocushandler(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e1d48
void iSNDloaddll(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e1dd4
namespace port { void iSNDunloaddll(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e217c
void SNDcaps(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e21b0
namespace dead { void stub_0x4e2224(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2224
namespace port { void SNDsetrate(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2228
void SoundServeHandler(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e2234
namespace port { void iSNDmessageinit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e225c
namespace port { void iSNDinit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2450
namespace port { void iSNDrestore(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e253c
namespace port { void soundIdle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e257c
namespace port { void iSNDstop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e25c0
namespace port { void SNDinit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2630
namespace port { void SNDrestore(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e26b4
namespace port { void iSNDverifymode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2754
namespace watcom { void unlink(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e28a0
namespace ea { void initHandlesDefault(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e28c0
namespace ea { void openHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e28d0
namespace ea { void openHandleErrorsAs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2ab0
namespace ea { void openHandleToWrite(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2ae0
namespace ea { void openMapHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2ba0
namespace ea { void closeHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2da0
namespace ea { void readHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2de0
namespace ea { void writeHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2e20
namespace ea { void seekHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2e60
namespace ea { void isSlash(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2eb0
namespace ea { void notSlash(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2ebe
namespace ea { void volumeRoot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2ed0
namespace ea { void fullPathName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e2fd0
namespace ea { void logicalDrives(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3110
namespace ea { void driveOfPath(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3120
namespace ea { void driveLetter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3250
namespace ea { void diskFreeBytes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e32e0
namespace ea { void driveNumber(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3330
namespace ea { void setDirectory(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3340
namespace ea { void getDirectory(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e33a0
namespace ea { void handlesReady(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3410
namespace ea { void initHandles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3430
namespace ea { void closeAllHandles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e34c0
namespace ea { void takeHandleSlot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3570
namespace ea { void deleteFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3600
namespace ea { void openFileHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3620
namespace ea { void handleInfo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3980
namespace ea { void readHandleBytes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3ae0
namespace ea { void writeHandleBytes(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3cc0
namespace ea { void seekHandleTo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3e60
namespace ea { void handlePosition(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3f30
namespace ea { void closeHandleSlot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e3fe0
namespace ea { void lockHandleDrive(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e40a0
namespace ea { void unlockHandleDrive(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e40f0
namespace ea { void openMemoryHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4140
namespace ea { void handleMemory(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e41f0
namespace dead { void stub_0x4e42e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e42e0
namespace dead { void stub_0x4e43b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e43b0
namespace dead { void stub_0x4e43c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e43c0
namespace dead { void stub_0x4e43d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e43d0
namespace port { void initStreamStruct(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4420
namespace port { void initStreamStructSecond(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e45d0
namespace port { void initStreamStructFirst(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e45e0
namespace port { void initStream(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e45f0
namespace port { void initStreamSecond(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4780
namespace port { void initStreamFirst(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4790
namespace dead { void stub_0x4e4a10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4a10
namespace port { void deleteStreamStruct(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4a30
namespace port { void deleteStream(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4b50
namespace port { void streamCommand(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4c60
namespace port { void purgeStreamCommand(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4d70
namespace port { void streamOpen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4ea0
namespace port { void streamOpenQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4ec0
namespace port { void streamOpenNext(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4ee0
namespace port { void streamOpenNextQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4f00
namespace port { void streamContinue(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4f20
namespace port { void streamContinueQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4f40
namespace port { void streamSwitch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4f60
namespace port { void streamSwitchQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4f80
namespace port { void streamQueueFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4fa0
namespace port { void streamQueueFileQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4fc0
namespace port { void streamRestart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e4fe0
namespace port { void streamRestartQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e5000
namespace port { void streamSeek(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e5020
namespace port { void streamSeekQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e5040
namespace port { void streamReadOn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e5060
namespace port { void streamReadOnQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e5080
namespace port { void streamStopAt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e50a0
namespace port { void streamStopAtQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e50c0
namespace dead { void stub_0x4e50e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e50e0
namespace dead { void stub_0x4e5140(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e5140
namespace dead { void stub_0x4e5250(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e5250
namespace port { void streamReaderThread(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e53c0
namespace port { void passFreedChunks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6660
namespace port { void roomAhead(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e66e0
namespace port { void roomToEnd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6710
namespace port { void roomAtStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6720
namespace port { void getStreamChunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6740
namespace port { void freeStreamChunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e68a0
namespace port { void streamBytesRead(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e68c0
namespace dead { void stub_0x4e68d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e68d0
namespace port { void readerIdle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e68e0
namespace dead { void stub_0x4e6910(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6910
namespace port { void readerWaits(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6950
namespace dead { void stub_0x4e6960(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6960
namespace dead { void stub_0x4e69a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e69a0
namespace dead { void stub_0x4e69c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e69c0
namespace port { void chainCommandBlocks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e69e0
namespace port { void freeCommandBlock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6a20
namespace port { void commandBlocksFree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6a70
namespace dead { void stub_0x4e6ad0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6ad0
namespace port { void takeCommandBlock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6b30
namespace port { void pathStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6be0
namespace port { void pathTask(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6d68
namespace port { void pathHook(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6d94
namespace port { void SNDpathinit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6dc8
namespace port { void SNDpathstartstream(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e6f0c
namespace port { void SNDpathcontrol(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e705c
namespace port { void SNDpathevent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e70fc
namespace port { void SNDpatheventinterrupt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7208
namespace port { void SNDpathrestore(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7388
namespace port { void SNDmastervol(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e73d0
namespace port { void SNDstreamautovol(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7470
namespace port { void SNDstreamgetvol(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e74f0
namespace port { void addSystemTask(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7520
namespace port { void removeSystemTask(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e75e0
namespace port { void runSystemTasks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7630
namespace port { void waitSystemTasks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7720
namespace port { void SNDstreamsetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7750
namespace port { void SNDstreamrestore(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e784c
namespace port { void SNDstreamqueue(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e78a0
namespace port { void SNDstreaminterrupt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7a14
namespace port { void SNDstreampurge(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7b28
namespace port { void SNDstreamchunksleft(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7c58
namespace port { void SNDstreambuftime(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7ce0
namespace port { void streamToppedUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7dd0
namespace port { void iSNDstreamchecktopup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7df4
namespace port { void iSNDstreamsetbuftime(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7e98
namespace port { void parseStreamChunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7f30
namespace port { void dropQueuedChunks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e7ff8
namespace port { void queueDataChunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8078
namespace port { void iSNDparseheader(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e80cc
namespace port { void queueLoopChunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8258
namespace port { void streamSeekChunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e82c4
namespace port { void streamEndChunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e82ec
namespace port { void streamEndsSeen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8320
namespace port { void nextStreamChunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8328
namespace port { void iSNDstreamservice(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e85a8
namespace port { void resetStreamer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8944
namespace port { void iSNDstreamstart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e89f8
namespace port { void SNDstreamvol(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8b60
namespace port { void removeBankPatches(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8bd0
namespace port { void removeAllBanks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8c1c
namespace port { void SNDbankremove(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8c28
namespace port { void SNDbankadd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8d20
namespace port { void SNDbankheadersize(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8db0
namespace port { void SNDbankheadercopy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8e20
namespace port { void SNDangle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8f28
namespace port { void SNDstatus(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e8fd0
namespace port { void SNDstopall(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9010
namespace watcom { void update_buffer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9030
namespace watcom { void reset_buffer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9070
namespace watcom { void fseek(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9088
namespace watcom { void fread(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9200
namespace port { void SNDautovol(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9420
namespace port { void setSoundVolume(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9540
namespace port { void SNDvol(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e95fc
namespace port { void SNDstop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9670
namespace port { void setSoundBend(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9710
namespace port { void SNDpitchbend(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e97e4
namespace port { void SNDpos3d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9850
namespace port { void SNDpan(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9950
namespace port { void SNDplaysetdef(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9b20
namespace port { void SNDplay(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9ba0
namespace port { void playOptions(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9bf8
namespace port { void SNDfxinitbus(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9c70
namespace port { void SNDfxmasterlevel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9dfc
namespace watcom { void stricmp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9ec0
namespace load { void typeOfBigFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9f00
namespace load { void bigHeaderLength(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4e9fe0
namespace load { void findBigEntry(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea040
namespace load { void locateBigEntry(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea290
namespace dead { void stub_0x4ea330(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea330
namespace dead { void stub_0x4ea360(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea360
namespace dead { void stub_0x4ea390(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea390
namespace dead { void stub_0x4ea3b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea3b0
namespace ea { void memmove(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea4f0
namespace frontend { void simAngle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea570
namespace frontend { void simPackCount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea654
namespace frontend { void simPackEntry(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea658
namespace frontend { void simTurnAboutFirstRow(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea680
namespace dead { void stub_0x4ea750(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea750
namespace frontend { void simTurnAboutLastRow(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea820
namespace dead { void stub_0x4ea8f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea8f0
namespace dead { void stub_0x4ea900(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea900
namespace dead { void stub_0x4ea910(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea910
namespace frontend { void simSquareRoot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea920
namespace dead { void stub_0x4ea990(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ea990
namespace dead { void stub_0x4ead10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ead10
namespace dead { void stub_0x4ead80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ead80
namespace dead { void stub_0x4eaea0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eaea0
namespace watcom { void mkdir(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb1d0
namespace watcom { void initrandnext(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb1f0
namespace watcom { void rand(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb1fc
namespace watcom { void srand(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb220
namespace ea { void randomFraction(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb230
namespace ea { void growLocks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb260
namespace ea { void makeLock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb2e0
namespace ea { void enterLock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb340
namespace ea { void leaveLock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb350
namespace ea { void freeLock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb360
namespace watcom { void strncmp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb390
namespace graphics { void makeReciprocals(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb3c0
namespace dead { void stub_0x4eb810(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb810
namespace dead { void stub_0x4eb830(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eb830
namespace dead { void stub_0x4eba30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eba30
void AcquireWindow(win32::WinApplication* app, x86::CPU& cpu);  // 0x4eba70
namespace dead { void stub_0x4ebaa0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ebaa0
namespace dead { void stub_0x4ebae0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ebae0
namespace dead { void stub_0x4ebd50(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ebd50
namespace dead { void stub_0x4ece00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ece00
namespace dead { void stub_0x4ece20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ece20
namespace dead { void stub_0x4ece60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ece60
namespace dead { void stub_0x4eceb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eceb0
namespace dead { void stub_0x4ecf20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ecf20
namespace dead { void stub_0x4ed060(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ed060
namespace dead { void stub_0x4ed440(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ed440
namespace dead { void stub_0x4ed760(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ed760
namespace dead { void stub_0x4ed7a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ed7a0
namespace dead { void stub_0x4ed7d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ed7d0
namespace dead { void stub_0x4ed800(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ed800
namespace dead { void stub_0x4ed910(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ed910
namespace dead { void stub_0x4ed950(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ed950
namespace dead { void stub_0x4ed990(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ed990
namespace dead { void stub_0x4eda10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eda10
namespace dead { void stub_0x4edb80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4edb80
namespace watcom { void open_flags(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4edd80
namespace watcom { void doopen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4edebc
namespace watcom { void fsopen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4edfa4
namespace watcom { void fopen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4edfe8
namespace dead { void stub_0x4edff4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4edff4
namespace dead { void stub_0x4ee07c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee07c
namespace watcom { void fclose(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee100
namespace watcom { void shutdown_stream(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee144
namespace watcom { void hexDigit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee15c
namespace watcom { void MkTmpFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee168
namespace watcom { void doclose(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee214
namespace watcom { void remove(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee2e0
namespace watcom { void strrchr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee2f0
namespace watcom { void strcmp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee310
namespace watcom { void chmod(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee3c0
namespace watcom { void rmdir(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee400
namespace watcom { void ftell(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee420
namespace watcom { void med3(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee480
namespace watcom { void byteswap(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee4d0
namespace watcom { void qsort(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ee4f8
namespace load { void savefileadr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eeae0
namespace load { void savefileadrQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eebb0
namespace watcom { void fwrite(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eebd0
namespace watcom { void toupper(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eede0
namespace watcom { void strncat(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eedf0
namespace dead { void stub_0x4ef1a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ef1a0
namespace ea { void locateShape(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ef270
namespace dead { void stub_0x4ef4a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ef4a0
namespace dead { void stub_0x4ef4e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ef4e0
namespace dead { void stub_0x4ef500(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ef500
namespace watcom { void strtok(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4eff60
namespace ea { void keyCharacter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0010
namespace ea { void keyFromHook(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0100
namespace ea { void inkey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0120
namespace dead { void stub_0x4f0160(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0160
namespace dead { void stub_0x4f01c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f01c0
namespace ea { void flushKeys(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0240
namespace dead { void stub_0x4f0250(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0250
namespace watcom { void chdir(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0260
namespace watcom { void atoi(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0430
namespace watcom { void atol(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0490
namespace watcom { void cget_string(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f04f0
namespace watcom { void uncget_string(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f051c
namespace watcom { void vsscanf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0520
namespace watcom { void sscanf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0548
namespace watcom { void fgets(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0570
namespace watcom { void access(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f05f0
namespace dead { void stub_0x4f0640(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0640
namespace dead { void stub_0x4f0670(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0670
namespace dead { void stub_0x4f0690(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0690
namespace dead { void stub_0x4f06a1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f06a1
namespace dead { void stub_0x4f06c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f06c0
namespace watcom { void sleep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0710
namespace watcom { void getcwd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0730
namespace watcom { void getdiskfree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f07d0
namespace dead { void stub_0x4f0870(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0870
namespace dead { void stub_0x4f0a30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0a30
namespace dead { void stub_0x4f0a50(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0a50
namespace dead { void stub_0x4f0a70(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0a70
namespace watcom { void tolower(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f0ae0
namespace dead { void stub_0x4f1110(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f1110
namespace dead { void stub_0x4f1230(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f1230
namespace dead { void stub_0x4f1240(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f1240
namespace dead { void stub_0x4f1360(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f1360
namespace dead { void stub_0x4f14b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f14b0
namespace dead { void stub_0x4f14f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f14f0
namespace dead { void stub_0x4f15e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f15e0
namespace dead { void stub_0x4f15f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f15f0
namespace dead { void stub_0x4f1610(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f1610
namespace dead { void stub_0x4f1710(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f1710
namespace watcom { void floor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f1c30
namespace removed { void processorChecked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f1c70
namespace dead { void stub_0x4f1ea0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f1ea0
namespace removed { void processorSpeed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f1ef0
namespace dead { void stub_0x4f20b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f20b0
namespace dead { void stub_0x4f21c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f21c0
namespace watcom { void time(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f22b0
namespace dead { void stub_0x4f2360(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2360
namespace load { void fixedSine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2380
namespace load { void fixedCosine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f23f0
namespace ea { void unpackQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2560
namespace ea { void unpack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2620
namespace ea { void unpackSizeQuietly(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f26a0
namespace ea { void unpacksize(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2710
namespace ea { void stopLowTimer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2830
namespace ea { void removeLowTimer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f29b0
namespace dead { void stub_0x4f2d00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2d00
namespace dead { void stub_0x4f2d20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2d20
namespace dead { void stub_0x4f2d40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2d40
namespace dead { void stub_0x4f2d60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2d60
namespace watcom { void ceil(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2d78
namespace dead { void stub_0x4f2da0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2da0
namespace ea { void loadpackadr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2e20
namespace dead { void stub_0x4f2f30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2f30
namespace port { void SNDSTRM_requeststatus(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f2f50
namespace port { void streamMemory(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3a10
namespace dead { void stub_0x4f3a30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3a30
namespace port { void SNDSTRM_createqueued(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3a60
namespace port { void SNDSTRM_requestdata(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3a80
namespace port { void SNDSTRM_restore(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3a90
namespace port { void findStream(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3aa4
namespace port { void streamTaskSkipped(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3ab5
namespace port { void removeRequest(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3ab8
namespace port { void framesPlayedDone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3b31
namespace port { void packetFreed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3b44
namespace port { void framesPlayed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3b60
namespace port { void iSNDstreamparseheader(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3ba4
namespace port { void iSNDstreamparseloop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3cc8
namespace port { void iSNDstreamparsedata(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3d00
namespace port { void iSNDstreamparseend(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3dac
namespace port { void iSNDstreamparsechunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3dc4
namespace port { void streamWaits(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3e1c
namespace port { void iSNDstreamsystemtask(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3ebc
namespace port { void streamCount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3fd0
namespace port { void iSNDstreamcreate(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f3fec
namespace port { void iSNDstreamrequest(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4258
namespace dead { void stub_0x4f43ac(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f43ac
namespace port { void SNDSTRM_destroy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f43c0
namespace dead { void stub_0x4f4438(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4438
namespace port { void SNDSTRM_purge(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4440
namespace watcom { void stackavail(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f45c0
namespace dead { void stub_0x4f45d1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f45d1
namespace watcom { void GRO(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4610
namespace dead { void stub_0x4f4630(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4630
namespace dead { void stub_0x4f4810(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4810
namespace dead { void stub_0x4f48f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f48f0
namespace watcom { void fprintf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4b80
namespace ea { void netTrace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4c50
namespace dead { void stub_0x4f4ef0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4ef0
namespace ea { void netShutGame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4f50
namespace ea { void netEndGame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4f90
namespace dead { void stub_0x4f4fd0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4fd0
namespace ea { void netKillPlayer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f4ff0
namespace dead { void stub_0x4f5010(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f5010
namespace ea { void netCheckPlayer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f5040
namespace dead { void stub_0x4f5070(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f5070
namespace dead { void stub_0x4f5090(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f5090
namespace ea { void connRestore(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f5280
namespace ea { void connSend(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f5310
namespace ea { void connClose(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f5330
namespace watcom { void nmalloc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7900
namespace watcom { void nfree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f79f0
namespace watcom { void TryInit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7af8
namespace dead { void stub_0x4f7b1a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7b1a
namespace watcom { void TryHandler(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7b2a
namespace watcom { void TryScope(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7b61
namespace watcom { void Except(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7b7d
namespace watcom { void TryOuter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7b87
namespace watcom { void TryDone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7b8c
namespace watcom { void global_unwind(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7ba7
namespace watcom { void local_unwind(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7bc3
namespace dead { void stub_0x4f7c09(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7c09
namespace watcom { void atexit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7c20
namespace watcom { void control87(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7c90
namespace watcom { void kbhit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7ce0
namespace watcom { void fflush(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7d70
namespace watcom { void fputs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7d80
namespace watcom { void do_getch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7e20
namespace watcom { void getch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7f24
namespace watcom { void cstart_(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f7fa0
namespace watcom { void strupr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f8020
namespace watcom { void localtime_(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f8040
namespace watcom { void localtime(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f8094
namespace dead { void stub_0x4f80ac(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f80ac
namespace watcom { void IF_DEXP(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f80b4
namespace watcom { void exp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f8116
namespace watcom { void Strtold(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f8122
namespace watcom { void strtod(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f82b6
void THRASH_opendll(win32::WinApplication* app, x86::CPU& cpu);  // 0x4f8390
namespace graphics { void setScreenFunctions(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f8830
namespace dead { void stub_0x4f88c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f88c0
namespace watcom { void setenv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f8b50
namespace watcom { void setenv_(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f8ce8
namespace watcom { void findenv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f8f38
namespace ea { void surfaceLocked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f9070
namespace dead { void stub_0x4f9090(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f9090
namespace dead { void stub_0x4f90a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f90a0
namespace dead { void stub_0x4f90d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f90d0
namespace dead { void stub_0x4f9140(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f9140
namespace dead { void stub_0x4f9170(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f9170
namespace dead { void stub_0x4f9190(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f9190
namespace dead { void stub_0x4f91b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f91b0
namespace dead { void stub_0x4f92e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f92e0
namespace dead { void stub_0x4f92f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f92f0
namespace dead { void stub_0x4f9310(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f9310
namespace dead { void stub_0x4f9470(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f9470
namespace watcom { void IF_DACOS(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f9482
namespace dead { void stub_0x4f94bf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4f94bf
namespace dead { void stub_0x4fa830(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fa830
namespace dead { void stub_0x4faa40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4faa40
namespace dead { void stub_0x4fabe0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fabe0
namespace dead { void stub_0x4faea0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4faea0
namespace dead { void stub_0x4fb030(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb030
namespace dead { void stub_0x4fb080(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb080
namespace dead { void stub_0x4fb140(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb140
namespace dead { void stub_0x4fb170(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb170
namespace dead { void stub_0x4fb1a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb1a0
namespace port { void SNDbankpatpresent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb1e0
namespace watcom { void bsearch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb300
namespace dead { void stub_0x4fb560(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb560
namespace dead { void stub_0x4fb590(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb590
namespace dead { void stub_0x4fb5a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb5a0
namespace dead { void stub_0x4fb6f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb6f0
namespace dead { void stub_0x4fb850(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fb850
namespace dead { void stub_0x4fc400(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fc400
namespace dead { void stub_0x4fc480(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fc480
namespace dead { void stub_0x4fc510(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fc510
namespace dead { void stub_0x4fc860(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fc860
namespace dead { void stub_0x4fca00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fca00
namespace dead { void stub_0x4fca30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fca30
namespace dead { void stub_0x4fcb00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fcb00
namespace dead { void stub_0x4fcb50(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fcb50
namespace dead { void stub_0x4fcb70(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fcb70
namespace dead { void stub_0x4fcba0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fcba0
namespace dead { void stub_0x4fcbc0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fcbc0
namespace dead { void stub_0x4fcc90(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fcc90
namespace dead { void stub_0x4fcce0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fcce0
namespace dead { void stub_0x4fce20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fce20
namespace dead { void stub_0x4fce30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fce30
namespace dead { void stub_0x4fce40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fce40
namespace dead { void stub_0x4fce90(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fce90
namespace dead { void stub_0x4fcea0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fcea0
namespace dead { void stub_0x4fceb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fceb0
namespace dead { void stub_0x4fcf40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fcf40
namespace dead { void stub_0x4fcfe0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fcfe0
namespace dead { void stub_0x4fd020(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fd020
namespace dead { void stub_0x4fd040(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fd040
namespace dead { void stub_0x4fd090(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fd090
namespace dead { void stub_0x4fd0c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fd0c0
namespace dead { void stub_0x4fd0f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fd0f0
namespace dead { void stub_0x4fd130(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fd130
namespace dead { void stub_0x4fd160(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fd160
namespace dead { void stub_0x4fd190(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fd190
namespace dead { void stub_0x4fd420(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fd420
namespace watcom { void prtf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe1d0
namespace watcom { void prtfSpec(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe578
namespace watcom { void prtfFlags(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe6cc
namespace watcom { void prtfLength(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe734
namespace watcom { void prtfWideLength(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe758
namespace watcom { void prtfHex(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe7c4
namespace watcom { void prtfHexEnd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe81e
namespace watcom { void prtfFixed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe828
namespace watcom { void prtfFloat(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe924
namespace watcom { void prtfZeros(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe92c
namespace watcom { void prtfWideOut(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe970
namespace watcom { void prtfConvert(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fe9d4
namespace watcom { void prtfUpper(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fefdc
namespace watcom { void file_putc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff000
namespace watcom { void vfprintf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff014
namespace dead { void stub_0x4ff0e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff0e0
namespace watcom { void GetThreadPtr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff0ec
namespace watcom { void NullAccess(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff0f4
namespace watcom { void NTInit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff108
namespace watcom { void NTMain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff338
namespace watcom { void exit_(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff38c
namespace watcom { void freeEnvironment(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff3d0
namespace watcom { void callRoutine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff410
namespace watcom { void InitRtns(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff41c
namespace watcom { void FiniRtns(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff46c
namespace watcom { void STOSB(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff4c0
namespace watcom { void STOSD(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff4f7
namespace dead { void stub_0x4ff5d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff5d0
namespace dead { void stub_0x4ff624(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff624
namespace port { void setTopupFunction(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff660
namespace port { void topUpStream(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff670
namespace port { void waitTopupMain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff700
namespace port { void waitTopupEvent(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff76e
namespace port { void topupDone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff780
namespace port { void toppedUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff7b0
namespace port { void topUpBeforeLoad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff7c0
namespace port { void topupReturn(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff7e0
namespace port { void waitTopupStream(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff7f0
namespace port { void waitTopupDone(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff80f
namespace port { void loadBlockSize(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff820
namespace port { void loadFlags(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff830
namespace port { void voicePicked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff840
namespace port { void iSNDallocchan(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ff86c
namespace port { void iSNDfreechan(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ffb64
namespace port { void voiceOf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ffc9c
namespace port { void bufferPosition(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ffce0
namespace snd { void dead_0x4ffd80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ffd80
namespace port { void mixFunctions(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4ffd84
namespace port { void iSNDmixinit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x4fff18
namespace port { void iSNDmixreinit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500008
namespace port { void iSNDmixstart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5000b8
namespace port { void mixerRamp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5004c8
namespace port { void iSNDmixstop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500604
namespace port { void setSendVolume(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500654
namespace port { void setMixVolume(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5006a4
namespace snd { void dead_0x5006f8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5006f8
namespace port { void mixerServe(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500864
namespace port { void convert16(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5008a0
namespace port { void convertToWords(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500910
namespace port { void convertXaStereo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500990
namespace port { void convert8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500b10
namespace port { void convertXaMono(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500b90
namespace port { void soundTick(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500c80
namespace port { void iSNDenteraudio(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500e20
namespace port { void iSNDleaveaudio(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500e7c
namespace port { void iSNDserveradd100hzclient(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500e94
namespace port { void iSNDserverremove100hzclient(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500ef0
namespace snd { void dead_0x500f80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500f80
namespace watcom { void set_errno_dos(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500f8c
namespace watcom { void set_errno_nt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x500ff4
namespace ea { void checkSurfaceUnlocked(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501010
namespace ea { void fileReport(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501050
namespace ea { void virtualAllocPages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501110
namespace ea { void virtualFreePages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501180
namespace ea { void touchMemoryPages(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5011a0
namespace ea { void openBlockHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501270
namespace ea { void blockHandleFile(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501360
namespace ea { void closeBlockHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5013a0
namespace ea { void readBlockHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5013b0
namespace ea { void seekBlockHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501450
namespace port { void updateVoiceVolume(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5014a0
namespace port { void iSNDvol(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501540
namespace port { void soundVolume(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501670
namespace port { void iSNDclearchunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5016c0
namespace port { void platformChunks(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5017b4
namespace port { void timbreDefaults(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5017c0
namespace port { void resetTimbre(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5017d8
namespace port { void addTimbreDeltas(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5018dc
namespace port { void newGroup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501938
namespace port { void iSNDplaytaggedtimbre(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501978
namespace port { void iSNDplaytaggedpatch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x501de8
namespace port { void iSNDresolvetaggedpatch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5021ac
namespace port { void unresolveTaggedPatch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5022d0
namespace port { void iSNDextracthdrinfo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502330
namespace port { void iSNDbankremovepat(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5024a0
namespace port { void bankUsable(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502570
namespace port { void iSNDbankalloc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5025b0
namespace port { void iSNDdownloadbank(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502620
namespace watcom { void flush(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502760
namespace watcom { void set_errno(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502880
namespace watcom { void set_EDOM(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502890
namespace watcom { void set_ERANGE(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5028a4
namespace dead { void stub_0x5028b8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5028b8
namespace watcom { void set_doserrno(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5028c8
namespace watcom { void lseek(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5028e0
namespace watcom { void tell(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502960
namespace watcom { void ioalloc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5029c0
namespace watcom { void qread(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502a50
namespace watcom { void fgetc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502af0
namespace watcom { void filbuf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502bb8
namespace watcom { void fill_buffer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502be8
namespace port { void updateVoicePitch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502cb0
namespace port { void iSNDplatformdetune(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502d30
namespace port { void iSNDplatform3dpos(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502df0
namespace port { void iSNDpan(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502ef0
namespace port { void iSNDwinsetfx(win32::WinApplication* app, x86::CPU& cpu); }  // 0x502ff0
namespace port { void setBusFx(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5030ac
namespace port { void busLevelSet(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5030c4
namespace port { void SNDfxlevel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5030d0
namespace dead { void stub_0x503280(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503280
namespace dead { void stub_0x503281(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503281
namespace mix { void copySwapped(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5033e4
namespace mix { void copy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5033e5
namespace dead { void stub_0x50356c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50356c
namespace dead { void stub_0x50356d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50356d
namespace port { void mul64(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503670
namespace dead { void stub_0x503700(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503700
namespace port { void div64(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503760
namespace dead { void stub_0x5037a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5037a0
namespace dead { void stub_0x503800(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503800
namespace dead { void stub_0x503860(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503860
namespace dead { void stub_0x5038c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5038c0
namespace dead { void stub_0x503910(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503910
namespace dead { void stub_0x503990(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503990
namespace dead { void stub_0x503a10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503a10
namespace dead { void stub_0x503aa0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503aa0
namespace dead { void stub_0x503b60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503b60
namespace dead { void stub_0x503e80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503e80
namespace dead { void stub_0x503f10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503f10
namespace dead { void stub_0x503f50(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503f50
namespace dead { void stub_0x503f90(win32::WinApplication* app, x86::CPU& cpu); }  // 0x503f90
namespace dead { void stub_0x508500(win32::WinApplication* app, x86::CPU& cpu); }  // 0x508500
namespace dead { void stub_0x5085a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5085a0
namespace dead { void stub_0x5085c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5085c0
namespace dead { void stub_0x508b20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x508b20
namespace dead { void stub_0x508ec0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x508ec0
namespace dead { void stub_0x508ee0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x508ee0
namespace dead { void stub_0x508f00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x508f00
namespace dead { void stub_0x508fd0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x508fd0
namespace watcom { void sopen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x508ff4
namespace watcom { void sopen_(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50901c
namespace watcom { void allocfp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x509250
namespace watcom { void freefp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x509314
namespace watcom { void chktty(win32::WinApplication* app, x86::CPU& cpu); }  // 0x509380
namespace watcom { void getpid(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5093c0
namespace dead { void stub_0x5093d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5093d0
namespace watcom { void tmpdir(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5094e0
namespace watcom { void close(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5095b0
namespace watcom { void fixed_GetFileAttributesA(win32::WinApplication* app, x86::CPU& cpu); }  // 0x509640
namespace watcom { void qwrite(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5096c0
namespace watcom { void fputc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5097b0
namespace dead { void stub_0x509bd0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x509bd0
namespace dead { void stub_0x509ec0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x509ec0
namespace dead { void stub_0x50a700(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50a700
namespace dead { void stub_0x50af80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50af80
namespace dead { void stub_0x50afa0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50afa0
namespace watcom { void setbits(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50c540
namespace watcom { void cgetc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50c640
namespace watcom { void uncget(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50c648
namespace watcom { void scnf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50c650
namespace watcom { void get_opt(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50c904
namespace watcom { void scan_white(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50ca08
namespace watcom { void scan_char(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50ca48
namespace watcom { void scan_string(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50cb30
namespace watcom { void report_scan(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50cc88
namespace watcom { void makelist(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50cce8
namespace watcom { void scan_arb(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50cd2c
namespace watcom { void scan_float(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50ce18
namespace watcom { void scan_int(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50d0f0
namespace watcom { void radix_value(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50d510
namespace watcom { void chktrm(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50d538
namespace dead { void stub_0x50dbb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50dbb0
namespace dead { void stub_0x50dbd0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50dbd0
namespace ea { void qnth(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50dc00
namespace ea { void qunlink(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50dc20
namespace ea { void qfree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50dde0
namespace ea { void qremove(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50e150
namespace ea { void qlength(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50e1d0
namespace ea { void qposition(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50e220
namespace dead { void stub_0x50e360(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50e360
namespace ea { void qfindindex(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50e470
namespace dead { void stub_0x50e670(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50e670
namespace dead { void stub_0x50e740(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50e740
namespace watcom { void modf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50ea4e
namespace dead { void stub_0x50ea68(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50ea68
namespace dead { void stub_0x50ea85(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50ea85
namespace dead { void stub_0x50eaa7(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50eaa7
namespace watcom { void getenv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50ead0
namespace watcom { void getctime(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50eb30
namespace watcom { void mktime(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50eba0
namespace ea { void huffUnpack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x50ed50
namespace ea { void runUnpack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x510110
namespace ea { void dxtBlock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x510420
namespace dead { void stub_0x510740(win32::WinApplication* app, x86::CPU& cpu); }  // 0x510740
namespace ea { void dxtUnpack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x510760
namespace ea { void pairExpand(win32::WinApplication* app, x86::CPU& cpu); }  // 0x510820
namespace ea { void pairUnpack(win32::WinApplication* app, x86::CPU& cpu); }  // 0x510870
namespace port { void findRequest(win32::WinApplication* app, x86::CPU& cpu); }  // 0x510a10
namespace dead { void stub_0x511200(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511200
namespace port { void playerMemory(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511210
namespace port { void SNDPKTPLAY_create(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511228
namespace port { void SNDPKTPLAY_start(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5112d0
namespace port { void SNDPKTPLAY_submit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5114d4
namespace port { void SNDPKTPLAY_submitspace(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5115b8
namespace port { void framesOutstanding(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51162c
namespace port { void SNDPKTPLAY_framesoutstanding(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511680
namespace dead { void stub_0x5116ac(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5116ac
namespace port { void SNDPKTPLAY_purge(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511718
namespace port { void SNDPKTPLAY_stop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51181c
namespace port { void SNDPKTPLAY_destroy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5118d0
namespace port { void nextPacket(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5118fc
namespace port { void packetFramesPlayed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511998
namespace port { void patchDefaults(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5119b0
namespace port { void iSNDpatchtohdr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5119d8
namespace port { void iSNDplatformcalcdatarate(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511bf0
namespace port { void streamChunksAtOnce(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511ca0
namespace watcom { void fatalWrite(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511cd0
namespace watcom { void fatal_runtime_error(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511d0c
namespace dead { void stub_0x511d30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511d30
namespace dead { void stub_0x511dc0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x511dc0
namespace ea { void layerSend(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5123b0
namespace ea { void layerCallPacker(win32::WinApplication* app, x86::CPU& cpu); }  // 0x512530
namespace ea { void layerPackFromTop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x512630
namespace ea { void layerFree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5126c0
namespace dead { void stub_0x512710(win32::WinApplication* app, x86::CPU& cpu); }  // 0x512710
namespace ea { void layerSendPacket(win32::WinApplication* app, x86::CPU& cpu); }  // 0x512770
namespace dead { void stub_0x512790(win32::WinApplication* app, x86::CPU& cpu); }  // 0x512790
namespace dead { void stub_0x5127b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5127b0
namespace watcom { void MemAllocator(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515300
namespace watcom { void MemFree(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5153b0
namespace watcom { void LinkUpNewNHeap(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5154e0
namespace watcom { void CreateNewNHeap(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515558
namespace watcom { void ExpandDGROUP(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515614
namespace watcom { void AdjustAmount(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515628
namespace watcom { void nmemneed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515680
namespace watcom { void NTRealKey(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515690
namespace watcom { void openConsoles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5156b8
namespace watcom { void NTConsoleInput(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515728
namespace watcom { void NTConsoleOutput(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515734
namespace watcom { void flushall(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515850
namespace watcom { void flushall_(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515858
namespace watcom { void mainStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5158a0
namespace dead { void stub_0x515af0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515af0
namespace dead { void stub_0x515b0c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515b0c
namespace watcom { void tzWindows(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515b34
namespace watcom { void tzset(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515c2c
namespace watcom { void tzNumber(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515c90
namespace watcom { void tzName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515cbc
namespace watcom { void tzRule(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515dd4
namespace watcom { void tzDaylight(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515ec2
namespace watcom { void brktime(win32::WinApplication* app, x86::CPU& cpu); }  // 0x515fa0
namespace dead { void stub_0x5160b8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5160b8
namespace dead { void stub_0x5160d8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5160d8
namespace watcom { void leapyear(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5160f0
namespace watcom { void calc_yday(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51612c
namespace watcom { void check_order(win32::WinApplication* app, x86::CPU& cpu); }  // 0x516224
namespace watcom { void isindst(win32::WinApplication* app, x86::CPU& cpu); }  // 0x516270
namespace watcom { void time_less(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51652c
namespace watcom { void math87_err(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51655c
namespace watcom { void ZBuf2LD(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5165d8
namespace watcom { void normalize(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51663a
namespace watcom { void scalePowers(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51667b
namespace watcom { void scaleEnd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5166cf
namespace watcom { void scaleBy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5166d8
namespace watcom { void LDScale10x(win32::WinApplication* app, x86::CPU& cpu); }  // 0x516759
namespace graphics { void useDriverSlot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x517170
namespace graphics { void keepDriverSlot(win32::WinApplication* app, x86::CPU& cpu); }  // 0x517370
namespace dead { void stub_0x517560(win32::WinApplication* app, x86::CPU& cpu); }  // 0x517560
namespace dead { void stub_0x517570(win32::WinApplication* app, x86::CPU& cpu); }  // 0x517570
namespace graphics { void setVideoMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x517610
namespace graphics { void lockFrameBuffer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x517890
namespace graphics { void releaseFrameLock(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5178d0
namespace graphics { void releaseAndFlip(win32::WinApplication* app, x86::CPU& cpu); }  // 0x517990
namespace graphics { void setUpScreen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5179d0
namespace watcom { void mbslen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x518050
namespace watcom { void mbstowcs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x518080
namespace dead { void stub_0x518100(win32::WinApplication* app, x86::CPU& cpu); }  // 0x518100
namespace watcom { void wsetenv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x518290
namespace watcom { void wfindenv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x518498
namespace watcom { void nrealloc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5185c0
namespace watcom { void memmove(win32::WinApplication* app, x86::CPU& cpu); }  // 0x518640
namespace dead { void stub_0x519080(win32::WinApplication* app, x86::CPU& cpu); }  // 0x519080
namespace dead { void stub_0x5191b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5191b0
namespace dead { void stub_0x519560(win32::WinApplication* app, x86::CPU& cpu); }  // 0x519560
namespace dead { void stub_0x5195b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5195b0
namespace dead { void stub_0x519a10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x519a10
namespace dead { void stub_0x519ae0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x519ae0
namespace dead { void stub_0x519e30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x519e30
namespace dead { void stub_0x51a188(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51a188
namespace watcom { void DSQRT(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51a1ba
namespace dead { void stub_0x51a1f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51a1f0
namespace dead { void stub_0x51a200(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51a200
namespace dead { void stub_0x51a260(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51a260
namespace dead { void stub_0x51a2a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51a2a0
namespace dead { void stub_0x51a310(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51a310
namespace dead { void stub_0x51a3b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51a3b0
namespace dead { void stub_0x51a3e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51a3e0
namespace dead { void stub_0x51a400(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51a400
namespace dead { void stub_0x51a420(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51a420
namespace dead { void stub_0x51ab10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51ab10
namespace dead { void stub_0x51b060(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51b060
namespace dead { void stub_0x51b0d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51b0d0
namespace dead { void stub_0x51b140(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51b140
namespace dead { void stub_0x51b5a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51b5a0
namespace dead { void stub_0x51b830(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51b830
namespace dead { void stub_0x51bae0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51bae0
namespace dead { void stub_0x51bb60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51bb60
namespace dead { void stub_0x51db20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51db20
namespace dead { void stub_0x51dd50(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51dd50
namespace dead { void stub_0x51ddf0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51ddf0
namespace dead { void stub_0x51def0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51def0
namespace dead { void stub_0x51dfb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51dfb0
namespace dead { void stub_0x51e020(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e020
namespace watcom { void wctomb_(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e060
namespace watcom { void utoa(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e0a0
namespace watcom { void itoa(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e0ec
namespace watcom { void ulltoa(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e130
namespace dead { void stub_0x51e1c8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e1c8
namespace watcom { void prtfUtoa(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e220
namespace dead { void stub_0x51e26c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e26c
namespace dead { void stub_0x51e290(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e290
namespace watcom { void handlesFull(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e2b0
namespace watcom { void allocPOSIXHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e304
namespace watcom { void freePOSIXHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e45c
namespace watcom { void initPOSIXHandles(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e490
namespace watcom { void NTGetFakeHandle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e4f0
namespace watcom { void initThreadData(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51e7ec
namespace watcom { void moduleWideName(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51eb00
namespace watcom { void copyCommandLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51eba0
namespace watcom { void copyWideCommandLine(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51ebf0
namespace watcom { void stackLimits(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51ec20
namespace watcom { void InitExceptionHandling(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f0d0
namespace watcom { void DoneExceptionHandler(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f11c
namespace dead { void stub_0x51f272(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f272
namespace dead { void stub_0x51f320(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f320
namespace port { void panSides(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f520
namespace port { void volumeMonoPair(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f580
namespace port { void volumeMonoLevels(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f5a4
namespace mix { void addMono8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f5dd
namespace port { void volumeStereoPair(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f680
namespace port { void volumeStereoLevels(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f6a4
namespace mix { void addStereo8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f6c7
namespace port { void volumeMonoSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f750
namespace port { void volumeMonoLevelsMmx(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f774
namespace mix { void addMono16(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f7ad
namespace port { void volumeStereoSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f850
namespace port { void volumeStereoLevelsMmx(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f874
namespace mix { void addStereo16(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f897
namespace port { void resample8MonoNeededDown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f930
namespace port { void resample8MonoNeededSame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f978
namespace port { void resample8MonoNeededUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f980
namespace port { void resample8MonoDown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51f9ac
namespace port { void resample8MonoSame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51fa80
namespace port { void resample8MonoUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51fa8c
namespace port { void resample8MonoStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51fb94
namespace port { void resample8MonoSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51fc38
namespace port { void resample8StereoNeededDown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51fc80
namespace port { void resample8StereoNeededSame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51fcc8
namespace port { void resample8StereoNeededUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51fcd0
namespace port { void resample8StereoDown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51fcfc
namespace port { void resample8StereoSame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51fe24
namespace port { void resample8StereoUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51fe30
namespace port { void resample8StereoStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x51ff88
namespace port { void resample8StereoSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52002c
namespace port { void resample16MonoNeededDown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520070
namespace port { void resample16MonoNeededSame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5200b8
namespace port { void resample16MonoNeededUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5200c0
namespace port { void resample16MonoDown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5200ec
namespace port { void resample16MonoSame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5201d8
namespace port { void resample16MonoUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5201e4
namespace port { void resampleMonoStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5202fc
namespace port { void resampleMonoSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5203a0
namespace port { void resample16StereoNeededDown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5203f0
namespace port { void resample16StereoNeededSame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520438
namespace port { void resample16StereoNeededUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520440
namespace port { void resample16StereoDown(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52046c
namespace port { void resample16StereoSame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5205b0
namespace port { void resample16StereoUp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5205bc
namespace port { void resampleStereoStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520740
namespace port { void resampleStereoSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5207e4
namespace mix { void outputMono8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520830
namespace mix { void outputStereo8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520870
namespace mix { void outputStereo16(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520920
namespace mix { void clear(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5209a0
namespace mix { void finished(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520a00
namespace mix { void outputMono16(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520a10
namespace snd { void dead_0x520a40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520a40
namespace snd { void dead_0x520a5e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520a5e
namespace snd { void dead_0x520aee(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520aee
namespace snd { void dead_0x520b7e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520b7e
namespace snd { void dead_0x520bed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520bed
namespace snd { void dead_0x520c69(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520c69
namespace snd { void dead_0x520ce6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520ce6
namespace snd { void dead_0x520d6b(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520d6b
namespace snd { void dead_0x520df1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520df1
namespace snd { void dead_0x520e00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520e00
namespace snd { void dead_0x520e1e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520e1e
namespace snd { void dead_0x520eae(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520eae
namespace snd { void dead_0x520f40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520f40
namespace snd { void dead_0x520fb1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x520fb1
namespace snd { void dead_0x52102d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52102d
namespace snd { void dead_0x5210ac(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5210ac
namespace snd { void dead_0x521133(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521133
namespace snd { void dead_0x5211bb(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5211bb
namespace snd { void dead_0x5211c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5211c0
namespace snd { void dead_0x521200(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521200
namespace snd { void dead_0x521215(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521215
namespace snd { void dead_0x52125b(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52125b
namespace snd { void dead_0x521310(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521310
namespace snd { void dead_0x521350(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521350
namespace snd { void dead_0x521365(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521365
namespace snd { void dead_0x5213ab(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5213ab
namespace snd { void dead_0x521490(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521490
namespace snd { void dead_0x5214d8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5214d8
namespace snd { void dead_0x5214e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5214e0
namespace snd { void dead_0x52150c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52150c
namespace snd { void dead_0x521614(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521614
namespace snd { void dead_0x521620(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521620
namespace snd { void dead_0x52175c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52175c
namespace snd { void dead_0x521800(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521800
namespace snd { void dead_0x521840(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521840
namespace snd { void dead_0x5218c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5218c0
namespace snd { void dead_0x521908(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521908
namespace snd { void dead_0x521910(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521910
namespace snd { void dead_0x52193c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52193c
namespace snd { void dead_0x521a10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521a10
namespace snd { void dead_0x521a1c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521a1c
namespace snd { void dead_0x521b20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521b20
namespace snd { void dead_0x521bc4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521bc4
namespace snd { void dead_0x521c10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521c10
namespace snd { void dead_0x521c90(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521c90
namespace snd { void dead_0x521d00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521d00
namespace port { void pcmTablesFirst(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521d80
namespace port { void pcmTablesSecond(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521e60
namespace port { void xaFloatTable(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521f30
namespace port { void xaIntTable(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521f7c
namespace port { void xaTables(win32::WinApplication* app, x86::CPU& cpu); }  // 0x521fc0
namespace port { void lastTables(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5220a0
namespace snd { void dead_0x522100(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522100
namespace snd { void dead_0x522130(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522130
namespace ea { void physicalMemory(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5221f0
namespace snd { void dead_0x522220(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522220
namespace snd { void dead_0x522250(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522250
namespace port { void panSplit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5222f0
namespace port { void panSurround(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522320
namespace port { void panLawPositive(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5224b0
namespace port { void randomOffset(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5224e0
namespace port { void iSNDplatformplay(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522510
namespace port { void nextTag(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5229d0
namespace port { void iSNDplatformresolve(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522a80
namespace port { void platformUnresolve(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522d08
namespace watcom { void fsync(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522d80
namespace watcom { void GetIOMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522de0
namespace watcom { void SetIOMode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522e38
namespace watcom { void getche(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522e60
namespace port { void pitchFactor(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522e90
namespace port { void mixStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522f10
namespace port { void setMixStep(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522f50
namespace snd { void dead_0x522f90(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522f90
namespace snd { void dead_0x522fd4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x522fd4
namespace port { void reverbStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5230b8
namespace snd { void dead_0x5230dc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5230dc
namespace snd { void dead_0x523118(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523118
namespace port { void reverbLength(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523200
namespace port { void reverbInit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52325c
namespace port { void setVoiceSend(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523310
namespace dead { void stub_0x523488(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523488
namespace dead { void stub_0x5234a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5234a0
namespace watcom { void GetNTAccessAttr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5234d0
namespace watcom { void GetNTShareAttr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523504
namespace watcom { void isatty(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523560
namespace dead { void stub_0x5235d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5235d0
namespace dead { void stub_0x5235dc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5235dc
namespace dead { void stub_0x5235f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5235f0
namespace watcom { void fullpath(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523610
namespace watcom { void mbtowc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523c00
namespace watcom { void FDFS(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523ca0
namespace watcom { void U8M(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523cec
namespace watcom { void mbsnicmp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523d10
namespace port { void platformPacketSize(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523d90
namespace port { void iSNDplatformpacketplay(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523d94
namespace port { void iSNDcheckplayopts(win32::WinApplication* app, x86::CPU& cpu); }  // 0x523f90
namespace watcom { void EnterWVIDEO(win32::WinApplication* app, x86::CPU& cpu); }  // 0x524190
namespace watcom { void nheapshrink(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5241c0
namespace watcom { void ReturnMemToSystem(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5241fc
namespace watcom { void ReleaseNHeap(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52425c
namespace watcom { void callMain(win32::WinApplication* app, x86::CPU& cpu); }  // 0x524290
namespace watcom { void wcstombs(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5242f0
namespace watcom { void reporterror(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5243a7
namespace watcom { void reportError(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5243b1
namespace graphics { void driverIdle(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5244e0
namespace graphics { void closeDriverAtExit(win32::WinApplication* app, x86::CPU& cpu); }  // 0x524640
namespace graphics { void closeDriver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x524650
namespace graphics { void openDriver(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5246a0
namespace graphics { void dead_0x524820(win32::WinApplication* app, x86::CPU& cpu); }  // 0x524820
namespace graphics { void unlockFrameBuffer(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5248f0
namespace dead { void stub_0x524f80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x524f80
namespace watcom { void mbterm(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5250c0
namespace watcom { void mbsinc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525100
namespace dead { void stub_0x525130(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525130
namespace watcom { void create_wide_environment(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5252a0
namespace watcom { void wcslen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525300
namespace watcom { void memcpy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525320
namespace watcom { void wcscpy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525350
namespace watcom { void towupper(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525370
namespace watcom { void msize(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525380
namespace watcom { void nexpand(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525390
namespace watcom { void HeapManager_expand(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5253cc
namespace watcom { void nexpandFailed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525578
namespace dead { void stub_0x525590(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525590
namespace dead { void stub_0x5256b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5256b0
namespace dead { void stub_0x525ac0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525ac0
namespace dead { void stub_0x525b90(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525b90
namespace dead { void stub_0x525d04(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525d04
namespace watcom { void U8D(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525d59
namespace watcom { void calloc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525df0
namespace dead { void stub_0x525fd0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x525fd0
namespace watcom { void initThread(win32::WinApplication* app, x86::CPU& cpu); }  // 0x526030
namespace dead { void stub_0x5261cc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5261cc
namespace dead { void stub_0x526238(win32::WinApplication* app, x86::CPU& cpu); }  // 0x526238
namespace dead { void stub_0x526288(win32::WinApplication* app, x86::CPU& cpu); }  // 0x526288
namespace dead { void stub_0x5269d4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5269d4
namespace dead { void stub_0x526aeb(win32::WinApplication* app, x86::CPU& cpu); }  // 0x526aeb
namespace dead { void stub_0x526f9a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x526f9a
namespace dead { void stub_0x526fad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x526fad
namespace dead { void stub_0x526fc0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x526fc0
namespace dead { void stub_0x527058(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527058
namespace dead { void stub_0x5270a4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5270a4
namespace mix { void interpolateMono8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5270f0
namespace mix { void interpolateStereo8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527170
namespace mix { void interpolateMono16(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527210
namespace mix { void interpolateStereo16(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527290
namespace snd { void dead_0x527330(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527330
namespace snd { void dead_0x5273c0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5273c0
namespace port { void pcmOnceMono(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5274b0
namespace port { void pcmLoopedMono(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5274d0
namespace port { void pcmStreamedMono(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527500
namespace port { void pcmFileMono(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527520
namespace port { void pcmOnceStereo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527540
namespace port { void pcmLoopedStereo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527560
namespace port { void pcmStreamedStereo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527590
namespace port { void pcmFileStereo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5275b0
namespace snd { void dead_0x5275d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5275d0
namespace snd { void dead_0x5275f0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5275f0
namespace snd { void dead_0x527620(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527620
namespace snd { void dead_0x527640(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527640
namespace snd { void dead_0x527660(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527660
namespace snd { void dead_0x527680(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527680
namespace snd { void dead_0x5276b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5276b0
namespace snd { void dead_0x5276d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5276d0
namespace port { void pcmOnceStereo2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5276f0
namespace port { void pcmLoopedStereo2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527710
namespace port { void pcmStreamedStereo2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527740
namespace port { void pcmFileStereo2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527760
namespace port { void pcmOnceQuad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527780
namespace port { void pcmLoopedQuad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5277a0
namespace port { void pcmStreamedQuad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5277d0
namespace port { void pcmFileQuad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5277f0
namespace snd { void dead_0x527810(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527810
namespace snd { void dead_0x527840(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527840
namespace snd { void dead_0x527870(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527870
namespace snd { void dead_0x527890(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527890
namespace snd { void dead_0x5278b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5278b0
namespace snd { void dead_0x5278e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5278e0
namespace snd { void dead_0x527910(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527910
namespace snd { void dead_0x527930(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527930
namespace snd { void dead_0x527950(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527950
namespace snd { void dead_0x527954(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527954
namespace snd { void dead_0x527ae8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527ae8
namespace snd { void dead_0x527b30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527b30
namespace snd { void dead_0x527b8c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527b8c
namespace snd { void dead_0x527d54(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527d54
namespace snd { void dead_0x527f20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527f20
namespace snd { void dead_0x527f80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x527f80
namespace snd { void dead_0x52813c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52813c
namespace snd { void dead_0x528180(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528180
namespace snd { void dead_0x5283a4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5283a4
namespace snd { void dead_0x528400(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528400
namespace snd { void dead_0x528404(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528404
namespace snd { void dead_0x5285b8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5285b8
namespace snd { void dead_0x528610(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528610
namespace snd { void dead_0x528680(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528680
namespace snd { void dead_0x5288b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5288b0
namespace snd { void dead_0x528a98(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528a98
namespace snd { void dead_0x528b00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528b00
namespace snd { void dead_0x528cac(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528cac
namespace snd { void dead_0x528d00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528d00
namespace snd { void dead_0x528f0c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528f0c
namespace port { void xaPlayed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528f70
namespace port { void xaOnceDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x528f74
namespace port { void xaOnceSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529104
namespace port { void xaKeptCopy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529140
namespace port { void xaLoopDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5291a8
namespace port { void xaIntroDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529384
namespace port { void xaIntroSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529534
namespace port { void xaStreamDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529590
namespace port { void xaStreamSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529778
namespace port { void xaPacketDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5297c0
namespace port { void xaPacketSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5299f4
namespace port { void xaStereoPlayed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529a50
namespace port { void xaStereoOnceDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529a54
namespace port { void xaStereoOnceSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529c2c
namespace port { void xaStereoKeptCopy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529c80
namespace port { void xaStereoLoopDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529cfc
namespace port { void xaStereoIntroDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x529f24
namespace port { void xaStereoIntroSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a144
namespace port { void xaStereoStreamDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a1a0
namespace port { void xaStereoStreamSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a354
namespace port { void xaStereoPacketDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a3a0
namespace port { void xaStereoFileSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a5b4
namespace port { void framesPlayedCoded(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a610
namespace port { void codedToWords(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a618
namespace port { void codedDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a668
namespace port { void codedSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a790
namespace port { void codedStreamDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a7e0
namespace port { void codedStreamSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a970
namespace port { void codedPacketDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52a9c0
namespace port { void codedPacketSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ab70
namespace snd { void dead_0x52abd0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52abd0
namespace snd { void dead_0x52abd8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52abd8
namespace snd { void dead_0x52abf8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52abf8
namespace snd { void dead_0x52ad28(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ad28
namespace snd { void dead_0x52ad70(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ad70
namespace snd { void dead_0x52af30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52af30
namespace snd { void dead_0x52af80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52af80
namespace snd { void dead_0x52b160(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b160
namespace port { void panLaw(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b1c0
namespace port { void readBigEndian(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b230
namespace watcom { void putch(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b290
namespace snd { void dead_0x52b300(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b300
namespace snd { void dead_0x52b4d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b4d0
namespace port { void setSendLevel(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b500
namespace watcom { void mbclen(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b5b0
namespace watcom { void mbcicmp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b5e0
namespace watcom { void startHook(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b640
namespace dead { void stub_0x52b641(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b641
namespace watcom { void printMathError(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b647
namespace watcom { void callMatherr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b68f
namespace dead { void stub_0x52b7d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52b7d0
namespace dead { void stub_0x52bf70(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52bf70
namespace dead { void stub_0x52bfe0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52bfe0
namespace watcom { void wputenv(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c178
namespace watcom { void wputenvFind(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c300
namespace dead { void stub_0x52c410(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c410
namespace dead { void stub_0x52c420(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c420
namespace dead { void stub_0x52c430(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c430
namespace dead { void stub_0x52c580(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c580
namespace dead { void stub_0x52c5d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c5d0
namespace dead { void stub_0x52c720(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c720
namespace dead { void stub_0x52c7b0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c7b0
namespace dead { void stub_0x52c814(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c814
namespace dead { void stub_0x52c82c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c82c
namespace dead { void stub_0x52c8c7(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c8c7
namespace port { void pcmPlayed(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c8f0
namespace port { void pcmOnceDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c8f4
namespace port { void pcmOnceSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c9a0
namespace port { void pcmLoopedDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52c9d0
namespace port { void pcmLoopedSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ca48
namespace port { void pcmStreamedDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ca80
namespace port { void pcmStreamedSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52cb74
namespace port { void pcmFileDecode(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52cba0
namespace port { void pcmFileSetup(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ccb8
namespace snd { void dead_0x52ccf0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ccf0
namespace snd { void dead_0x52ccf4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ccf4
namespace snd { void dead_0x52cdd4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52cdd4
namespace snd { void dead_0x52ce20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ce20
namespace snd { void dead_0x52ceb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ceb0
namespace snd { void dead_0x52cef0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52cef0
namespace snd { void dead_0x52cfd8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52cfd8
namespace snd { void dead_0x52d010(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52d010
namespace snd { void dead_0x52d110(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52d110
namespace port { void packetPlayerOf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52d160
namespace mix { void xaMono(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52d1a0
namespace mix { void xaStereo(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52d250
namespace port { void codedBits(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52d370
namespace port { void codedSkip(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52d3cc
namespace port { void codedPulses(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52d428
namespace port { void codedInterpolate(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52d5cc
namespace port { void codedFilter(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52d618
namespace port { void codedSynthesis(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52d6d8
namespace port { void codedStart(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52de94
namespace port { void codedFrame(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52df80
namespace watcom { void mbccpy(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52e310
namespace watcom { void mbsupr(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52e350
namespace watcom { void mbccmp(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52e3a0
namespace watcom { void get_std_stream(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52e400
namespace dead { void stub_0x52e654(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52e654
namespace dead { void stub_0x52eb40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52eb40
namespace watcom { void mbsnextc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ed30
namespace watcom { void mbctoupper(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ed70
namespace watcom { void mbvtop(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ede0
namespace watcom { void mbcupper932(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52eec0
namespace watcom { void mbclower(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52eee0
namespace watcom { void mbclegal(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ef30
namespace watcom { void mbbtrail(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ef70
namespace dead { void stub_0x52f370(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f370
namespace dead { void stub_0x52f3bb(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f3bb
namespace dead { void stub_0x52f406(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f406
namespace dead { void stub_0x52f451(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f451
namespace dead { void stub_0x52f49c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f49c
namespace dead { void stub_0x52f4e3(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f4e3
namespace dead { void stub_0x52f580(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f580
namespace dead { void stub_0x52f5cb(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f5cb
namespace dead { void stub_0x52f616(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f616
namespace dead { void stub_0x52f6a4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f6a4
namespace dead { void stub_0x52f6f3(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f6f3
namespace dead { void stub_0x52f742(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f742
namespace dead { void stub_0x52f78d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f78d
namespace dead { void stub_0x52f7d8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f7d8
namespace dead { void stub_0x52f823(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f823
namespace dead { void stub_0x52f878(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f878
namespace dead { void stub_0x52f8cd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f8cd
namespace dead { void stub_0x52f922(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f922
namespace dead { void stub_0x52f9ac(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f9ac
namespace dead { void stub_0x52f9fe(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52f9fe
namespace dead { void stub_0x52fa4d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fa4d
namespace dead { void stub_0x52fa98(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fa98
namespace dead { void stub_0x52fae3(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fae3
namespace dead { void stub_0x52fb79(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fb79
namespace dead { void stub_0x52fbd1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fbd1
namespace dead { void stub_0x52fc29(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fc29
namespace dead { void stub_0x52fc81(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fc81
namespace dead { void stub_0x52fcd9(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fcd9
namespace dead { void stub_0x52fd31(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fd31
namespace dead { void stub_0x52fd89(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fd89
namespace dead { void stub_0x52fdd8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fdd8
namespace dead { void stub_0x52fe27(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fe27
namespace dead { void stub_0x52fe76(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fe76
namespace dead { void stub_0x52fec8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52fec8
namespace dead { void stub_0x52ff1d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ff1d
namespace dead { void stub_0x52ff72(win32::WinApplication* app, x86::CPU& cpu); }  // 0x52ff72
namespace dead { void stub_0x5300bd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5300bd
namespace dead { void stub_0x530108(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530108
namespace dead { void stub_0x530153(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530153
namespace dead { void stub_0x53019e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53019e
namespace dead { void stub_0x5301e9(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5301e9
namespace dead { void stub_0x530234(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530234
namespace dead { void stub_0x530286(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530286
namespace dead { void stub_0x5302d8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5302d8
namespace dead { void stub_0x530323(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530323
namespace dead { void stub_0x53036e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53036e
namespace dead { void stub_0x5303b9(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5303b9
namespace dead { void stub_0x530404(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530404
namespace dead { void stub_0x530459(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530459
namespace dead { void stub_0x5304ae(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5304ae
namespace dead { void stub_0x530503(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530503
namespace dead { void stub_0x53054a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53054a
namespace dead { void stub_0x530591(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530591
namespace dead { void stub_0x5305d8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5305d8
namespace dead { void stub_0x53061f(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53061f
namespace dead { void stub_0x530666(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530666
namespace dead { void stub_0x5306b1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5306b1
namespace dead { void stub_0x5306fc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5306fc
namespace dead { void stub_0x530799(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530799
namespace dead { void stub_0x5307eb(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5307eb
namespace dead { void stub_0x530888(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530888
namespace dead { void stub_0x5308d3(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5308d3
namespace dead { void stub_0x53091e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53091e
namespace dead { void stub_0x53096d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53096d
namespace dead { void stub_0x5309b8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5309b8
namespace dead { void stub_0x530a03(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530a03
namespace dead { void stub_0x530aa3(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530aa3
namespace dead { void stub_0x530af8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530af8
namespace dead { void stub_0x530b94(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530b94
namespace dead { void stub_0x530bdb(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530bdb
namespace dead { void stub_0x530c22(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530c22
namespace dead { void stub_0x530c6d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530c6d
namespace dead { void stub_0x530cb8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530cb8
namespace dead { void stub_0x530cff(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530cff
namespace dead { void stub_0x530d46(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530d46
namespace dead { void stub_0x530d8d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530d8d
namespace dead { void stub_0x530dd8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530dd8
namespace dead { void stub_0x530e23(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530e23
namespace dead { void stub_0x530e6e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530e6e
namespace dead { void stub_0x530eb9(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530eb9
namespace dead { void stub_0x530f04(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530f04
namespace dead { void stub_0x530f4f(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530f4f
namespace dead { void stub_0x530f9a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530f9a
namespace dead { void stub_0x530fe5(win32::WinApplication* app, x86::CPU& cpu); }  // 0x530fe5
namespace dead { void stub_0x531030(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531030
namespace dead { void stub_0x53107b(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53107b
namespace dead { void stub_0x531110(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531110
namespace dead { void stub_0x531168(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531168
namespace dead { void stub_0x531212(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531212
namespace dead { void stub_0x531264(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531264
namespace dead { void stub_0x5312b6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5312b6
namespace dead { void stub_0x5312fd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5312fd
namespace dead { void stub_0x531344(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531344
namespace dead { void stub_0x5313e4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5313e4
namespace dead { void stub_0x531497(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531497
namespace dead { void stub_0x5314f5(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5314f5
namespace dead { void stub_0x531553(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531553
namespace dead { void stub_0x5315a2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5315a2
namespace dead { void stub_0x5315f1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5315f1
namespace dead { void stub_0x531640(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531640
namespace dead { void stub_0x531692(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531692
namespace dead { void stub_0x5316e4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5316e4
namespace dead { void stub_0x531736(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531736
namespace dead { void stub_0x531781(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531781
namespace dead { void stub_0x5317cc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5317cc
namespace dead { void stub_0x531817(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531817
namespace dead { void stub_0x53186c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53186c
namespace dead { void stub_0x5318b7(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5318b7
namespace dead { void stub_0x531902(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531902
namespace dead { void stub_0x53194d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53194d
namespace dead { void stub_0x531998(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531998
namespace dead { void stub_0x5319e7(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5319e7
namespace dead { void stub_0x531a2a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531a2a
namespace dead { void stub_0x531a6d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531a6d
namespace dead { void stub_0x531ab4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531ab4
namespace dead { void stub_0x531af7(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531af7
namespace dead { void stub_0x531b42(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531b42
namespace dead { void stub_0x531b8d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531b8d
namespace dead { void stub_0x531bd8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531bd8
namespace dead { void stub_0x531c27(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531c27
namespace dead { void stub_0x531c7c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531c7c
namespace dead { void stub_0x531cd1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531cd1
namespace dead { void stub_0x531d26(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531d26
namespace dead { void stub_0x531d6d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531d6d
namespace dead { void stub_0x531db8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531db8
namespace dead { void stub_0x531e0a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531e0a
namespace dead { void stub_0x531e51(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531e51
namespace dead { void stub_0x531ea3(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531ea3
namespace dead { void stub_0x531eea(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531eea
namespace dead { void stub_0x531f80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531f80
namespace dead { void stub_0x531fcf(win32::WinApplication* app, x86::CPU& cpu); }  // 0x531fcf
namespace dead { void stub_0x53201e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53201e
namespace dead { void stub_0x532069(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532069
namespace dead { void stub_0x5320d0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5320d0
namespace dead { void stub_0x532117(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532117
namespace dead { void stub_0x5321ad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5321ad
namespace dead { void stub_0x532205(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532205
namespace dead { void stub_0x532254(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532254
namespace dead { void stub_0x5322a3(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5322a3
namespace dead { void stub_0x5322f2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5322f2
namespace dead { void stub_0x532347(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532347
namespace dead { void stub_0x53239c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53239c
namespace dead { void stub_0x5323f1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5323f1
namespace dead { void stub_0x53243c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53243c
namespace dead { void stub_0x532487(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532487
namespace dead { void stub_0x532515(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532515
namespace dead { void stub_0x53255c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53255c
namespace dead { void stub_0x5325b4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5325b4
namespace dead { void stub_0x53260c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53260c
namespace dead { void stub_0x532664(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532664
namespace dead { void stub_0x5326b6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5326b6
namespace dead { void stub_0x532708(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532708
namespace dead { void stub_0x53275a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53275a
namespace dead { void stub_0x5327a1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5327a1
namespace dead { void stub_0x5327e4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5327e4
namespace dead { void stub_0x532833(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532833
namespace dead { void stub_0x532882(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532882
namespace dead { void stub_0x5328d1(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5328d1
namespace dead { void stub_0x532914(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532914
namespace dead { void stub_0x53295f(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53295f
namespace dead { void stub_0x5329aa(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5329aa
namespace dead { void stub_0x5329f5(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5329f5
namespace dead { void stub_0x532a40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532a40
namespace dead { void stub_0x532a8b(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532a8b
namespace dead { void stub_0x532ad6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532ad6
namespace dead { void stub_0x532b21(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532b21
namespace dead { void stub_0x532b70(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532b70
namespace dead { void stub_0x532bc2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532bc2
namespace dead { void stub_0x532c14(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532c14
namespace dead { void stub_0x532c66(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532c66
namespace dead { void stub_0x532cad(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532cad
namespace dead { void stub_0x532cf8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532cf8
namespace dead { void stub_0x532d3f(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532d3f
namespace dead { void stub_0x532d8a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532d8a
namespace dead { void stub_0x532dd5(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532dd5
namespace dead { void stub_0x532e20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532e20
namespace dead { void stub_0x532e6b(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532e6b
namespace dead { void stub_0x532eb6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532eb6
namespace dead { void stub_0x532f01(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532f01
namespace dead { void stub_0x532f4c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532f4c
namespace dead { void stub_0x532f97(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532f97
namespace dead { void stub_0x532fe2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x532fe2
namespace dead { void stub_0x533029(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533029
namespace dead { void stub_0x533070(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533070
namespace dead { void stub_0x5330b7(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5330b7
namespace dead { void stub_0x533106(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533106
namespace dead { void stub_0x533151(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533151
namespace dead { void stub_0x5331a3(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5331a3
namespace dead { void stub_0x5331fb(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5331fb
namespace dead { void stub_0x533253(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533253
namespace dead { void stub_0x5332a8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5332a8
namespace dead { void stub_0x5332fd(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5332fd
namespace dead { void stub_0x533355(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533355
namespace dead { void stub_0x5333a0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5333a0
namespace dead { void stub_0x5333eb(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5333eb
namespace dead { void stub_0x533436(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533436
namespace dead { void stub_0x533485(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533485
namespace dead { void stub_0x5334d7(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5334d7
namespace dead { void stub_0x533522(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533522
namespace dead { void stub_0x53356d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53356d
namespace dead { void stub_0x5335b8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5335b8
namespace dead { void stub_0x533603(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533603
namespace dead { void stub_0x533652(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533652
namespace dead { void stub_0x53369d(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53369d
namespace dead { void stub_0x5336e0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5336e0
namespace dead { void stub_0x533727(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533727
namespace dead { void stub_0x53376e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53376e
namespace dead { void stub_0x5337b5(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5337b5
namespace dead { void stub_0x5337fc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5337fc
namespace dead { void stub_0x53384b(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53384b
namespace dead { void stub_0x53389a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53389a
namespace dead { void stub_0x5338e9(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5338e9
namespace dead { void stub_0x53394a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x53394a
namespace dead { void stub_0x5339ab(win32::WinApplication* app, x86::CPU& cpu); }  // 0x5339ab
namespace dead { void stub_0x533a0c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a0c
namespace dead { void stub_0x533a12(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a12
namespace dead { void stub_0x533a18(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a18
namespace dead { void stub_0x533a1e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a1e
namespace dead { void stub_0x533a24(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a24
namespace dead { void stub_0x533a2a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a2a
namespace dead { void stub_0x533a30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a30
namespace dead { void stub_0x533a36(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a36
namespace dead { void stub_0x533a3c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a3c
namespace dead { void stub_0x533a42(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a42
namespace dead { void stub_0x533a48(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a48
namespace dead { void stub_0x533a4e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a4e
namespace dead { void stub_0x533a54(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a54
namespace dead { void stub_0x533a5a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a5a
namespace dead { void stub_0x533a60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a60
namespace dead { void stub_0x533a66(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a66
namespace dead { void stub_0x533a6c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a6c
namespace dead { void stub_0x533a72(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a72
namespace dead { void stub_0x533a78(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a78
namespace dead { void stub_0x533a7e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a7e
namespace dead { void stub_0x533a84(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a84
namespace dead { void stub_0x533a8a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a8a
namespace dead { void stub_0x533a90(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a90
namespace dead { void stub_0x533a96(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a96
namespace dead { void stub_0x533a9c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533a9c
namespace dead { void stub_0x533aa2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533aa2
namespace dead { void stub_0x533aa8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533aa8
namespace dead { void stub_0x533aae(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533aae
namespace dead { void stub_0x533ab4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ab4
namespace dead { void stub_0x533aba(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533aba
namespace dead { void stub_0x533ac0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ac0
namespace dead { void stub_0x533ac6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ac6
namespace dead { void stub_0x533acc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533acc
namespace dead { void stub_0x533ad2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ad2
namespace dead { void stub_0x533ad8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ad8
namespace dead { void stub_0x533ade(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ade
namespace dead { void stub_0x533ae4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ae4
namespace dead { void stub_0x533aea(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533aea
namespace dead { void stub_0x533af0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533af0
namespace dead { void stub_0x533af6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533af6
namespace dead { void stub_0x533afc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533afc
namespace dead { void stub_0x533b02(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b02
namespace dead { void stub_0x533b08(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b08
namespace dead { void stub_0x533b0e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b0e
namespace dead { void stub_0x533b14(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b14
namespace dead { void stub_0x533b1a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b1a
namespace dead { void stub_0x533b20(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b20
namespace dead { void stub_0x533b26(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b26
namespace dead { void stub_0x533b2c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b2c
namespace dead { void stub_0x533b32(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b32
namespace dead { void stub_0x533b38(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b38
namespace dead { void stub_0x533b3e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b3e
namespace dead { void stub_0x533b44(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b44
namespace dead { void stub_0x533b4a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b4a
namespace dead { void stub_0x533b50(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b50
namespace dead { void stub_0x533b56(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b56
namespace dead { void stub_0x533b5c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b5c
namespace dead { void stub_0x533b62(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b62
namespace dead { void stub_0x533b68(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b68
namespace dead { void stub_0x533b6e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b6e
namespace dead { void stub_0x533b74(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b74
namespace dead { void stub_0x533b7a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b7a
namespace dead { void stub_0x533b80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b80
namespace dead { void stub_0x533b86(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b86
namespace dead { void stub_0x533b8c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b8c
namespace dead { void stub_0x533b92(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b92
namespace dead { void stub_0x533b98(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b98
namespace dead { void stub_0x533b9e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533b9e
namespace dead { void stub_0x533ba4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ba4
namespace dead { void stub_0x533baa(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533baa
namespace dead { void stub_0x533bb0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bb0
namespace dead { void stub_0x533bb6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bb6
namespace dead { void stub_0x533bbc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bbc
namespace dead { void stub_0x533bc2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bc2
namespace dead { void stub_0x533bc8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bc8
namespace dead { void stub_0x533bce(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bce
namespace dead { void stub_0x533bd4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bd4
namespace dead { void stub_0x533bda(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bda
namespace dead { void stub_0x533be0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533be0
namespace dead { void stub_0x533be6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533be6
namespace dead { void stub_0x533bec(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bec
namespace dead { void stub_0x533bf2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bf2
namespace dead { void stub_0x533bf8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bf8
namespace dead { void stub_0x533bfe(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533bfe
namespace dead { void stub_0x533c04(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c04
namespace dead { void stub_0x533c0a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c0a
namespace dead { void stub_0x533c10(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c10
namespace dead { void stub_0x533c16(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c16
namespace dead { void stub_0x533c1c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c1c
namespace dead { void stub_0x533c22(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c22
namespace dead { void stub_0x533c28(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c28
namespace dead { void stub_0x533c2e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c2e
namespace dead { void stub_0x533c34(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c34
namespace dead { void stub_0x533c3a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c3a
namespace dead { void stub_0x533c40(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c40
namespace dead { void stub_0x533c46(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c46
namespace dead { void stub_0x533c4c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c4c
namespace dead { void stub_0x533c52(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c52
namespace dead { void stub_0x533c58(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c58
namespace dead { void stub_0x533c5e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c5e
namespace dead { void stub_0x533c64(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c64
namespace dead { void stub_0x533c6a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c6a
namespace dead { void stub_0x533c70(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c70
namespace dead { void stub_0x533c76(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c76
namespace dead { void stub_0x533c7c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c7c
namespace dead { void stub_0x533c82(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c82
namespace dead { void stub_0x533c88(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c88
namespace dead { void stub_0x533c8e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c8e
namespace dead { void stub_0x533c94(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c94
namespace dead { void stub_0x533c9a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533c9a
namespace dead { void stub_0x533ca0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ca0
namespace dead { void stub_0x533ca6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ca6
namespace dead { void stub_0x533cac(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cac
namespace dead { void stub_0x533cb2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cb2
namespace dead { void stub_0x533cb8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cb8
namespace dead { void stub_0x533cbe(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cbe
namespace dead { void stub_0x533cc4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cc4
namespace dead { void stub_0x533cca(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cca
namespace dead { void stub_0x533cd0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cd0
namespace dead { void stub_0x533cd6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cd6
namespace dead { void stub_0x533cdc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cdc
namespace dead { void stub_0x533ce2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ce2
namespace dead { void stub_0x533ce8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ce8
namespace dead { void stub_0x533cee(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cee
namespace dead { void stub_0x533cf4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cf4
namespace dead { void stub_0x533cfa(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533cfa
namespace dead { void stub_0x533d00(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d00
namespace dead { void stub_0x533d06(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d06
namespace dead { void stub_0x533d0c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d0c
namespace dead { void stub_0x533d12(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d12
namespace dead { void stub_0x533d18(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d18
namespace dead { void stub_0x533d1e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d1e
namespace dead { void stub_0x533d24(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d24
namespace dead { void stub_0x533d2a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d2a
namespace dead { void stub_0x533d30(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d30
namespace dead { void stub_0x533d36(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d36
namespace dead { void stub_0x533d3c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d3c
namespace dead { void stub_0x533d42(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d42
namespace dead { void stub_0x533d48(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d48
namespace dead { void stub_0x533d4e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d4e
namespace dead { void stub_0x533d54(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d54
namespace dead { void stub_0x533d5a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d5a
namespace dead { void stub_0x533d60(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d60
namespace dead { void stub_0x533d66(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d66
namespace dead { void stub_0x533d6c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d6c
namespace dead { void stub_0x533d72(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d72
namespace dead { void stub_0x533d78(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d78
namespace dead { void stub_0x533d7e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d7e
namespace dead { void stub_0x533d84(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d84
namespace dead { void stub_0x533d8a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d8a
namespace dead { void stub_0x533d90(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d90
namespace dead { void stub_0x533d96(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d96
namespace dead { void stub_0x533d9c(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533d9c
namespace dead { void stub_0x533da2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533da2
namespace dead { void stub_0x533da8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533da8
namespace dead { void stub_0x533dae(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533dae
namespace dead { void stub_0x533db4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533db4
namespace dead { void stub_0x533dba(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533dba
namespace dead { void stub_0x533dc0(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533dc0
namespace dead { void stub_0x533dc6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533dc6
namespace dead { void stub_0x533dcc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533dcc
namespace dead { void stub_0x533dd2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533dd2
namespace dead { void stub_0x533dd8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533dd8
namespace watcom { void RtlUnwindThunk(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533de4
namespace dead { void stub_0x533e62(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533e62
namespace dead { void stub_0x533e68(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533e68
namespace dead { void stub_0x533e6e(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533e6e
namespace dead { void stub_0x533e74(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533e74
namespace dead { void stub_0x533e7a(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533e7a
namespace dead { void stub_0x533e80(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533e80
namespace dead { void stub_0x533eb6(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533eb6
namespace dead { void stub_0x533ebc(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ebc
namespace dead { void stub_0x533ec2(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ec2
namespace dead { void stub_0x533ec8(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ec8
namespace dead { void stub_0x533ece(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ece
namespace dead { void stub_0x533ed4(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533ed4
namespace dead { void stub_0x533eda(win32::WinApplication* app, x86::CPU& cpu); }  // 0x533eda

class Application : public win32::WinApplication
{
public:
    Application(const char* appName);
    void execute();
private:
    static const x86::reg8 s_AUTOSegment[];
    static const x86::reg8 s__idataSegment[];
    static const x86::reg8 s_DGROUPSegment[];
    static const x86::reg8 s__rsrcSegment[];
    static void sub_401000(WinApplication* app, x86::CPU& cpu);
    static void sub_405fc0(WinApplication* app, x86::CPU& cpu);
    static void sub_40ff20(WinApplication* app, x86::CPU& cpu);
    static void sub_419c20(WinApplication* app, x86::CPU& cpu);
    static void sub_422c00(WinApplication* app, x86::CPU& cpu);
    static void sub_4243a0(WinApplication* app, x86::CPU& cpu);
    static void sub_424690(WinApplication* app, x86::CPU& cpu);
    static void sub_424740(WinApplication* app, x86::CPU& cpu);
    static void sub_424b50(WinApplication* app, x86::CPU& cpu);
    static void sub_424ce0(WinApplication* app, x86::CPU& cpu);
    static void sub_424e30(WinApplication* app, x86::CPU& cpu);
    static void sub_424f30(WinApplication* app, x86::CPU& cpu);
    static void sub_424fd0(WinApplication* app, x86::CPU& cpu);
    static void sub_425990(WinApplication* app, x86::CPU& cpu);
    static void sub_425ab0(WinApplication* app, x86::CPU& cpu);
    static void sub_425b40(WinApplication* app, x86::CPU& cpu);
    static void sub_426370(WinApplication* app, x86::CPU& cpu);
    static void sub_4283d0(WinApplication* app, x86::CPU& cpu);
    static void sub_428460(WinApplication* app, x86::CPU& cpu);
    static void sub_4284e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4285f0(WinApplication* app, x86::CPU& cpu);
    static void sub_428700(WinApplication* app, x86::CPU& cpu);
    static void sub_428970(WinApplication* app, x86::CPU& cpu);
    static void sub_428ad0(WinApplication* app, x86::CPU& cpu);
    static void sub_428d60(WinApplication* app, x86::CPU& cpu);
    static void sub_436fc0(WinApplication* app, x86::CPU& cpu);
    static void sub_438b70(WinApplication* app, x86::CPU& cpu);
    static void sub_438f60(WinApplication* app, x86::CPU& cpu);
    static void sub_439510(WinApplication* app, x86::CPU& cpu);
    static void sub_439530(WinApplication* app, x86::CPU& cpu);
    static void sub_4395a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4395e0(WinApplication* app, x86::CPU& cpu);
    static void sub_439600(WinApplication* app, x86::CPU& cpu);
    static void sub_439710(WinApplication* app, x86::CPU& cpu);
    static void sub_439a10(WinApplication* app, x86::CPU& cpu);
    static void sub_439b70(WinApplication* app, x86::CPU& cpu);
    static void sub_439bb0(WinApplication* app, x86::CPU& cpu);
    static void sub_439bf0(WinApplication* app, x86::CPU& cpu);
    static void sub_439cc0(WinApplication* app, x86::CPU& cpu);
    static void sub_43a4d0(WinApplication* app, x86::CPU& cpu);
    static void sub_43a6b0(WinApplication* app, x86::CPU& cpu);
    static void sub_43a6e0(WinApplication* app, x86::CPU& cpu);
    static void sub_43ace0(WinApplication* app, x86::CPU& cpu);
    static void sub_43b2a0(WinApplication* app, x86::CPU& cpu);
    static void sub_43b390(WinApplication* app, x86::CPU& cpu);
    static void sub_43bcb0(WinApplication* app, x86::CPU& cpu);
    static void sub_43bcf0(WinApplication* app, x86::CPU& cpu);
    static void sub_43da60(WinApplication* app, x86::CPU& cpu);
    static void sub_4516a0(WinApplication* app, x86::CPU& cpu);
    static void sub_452a00(WinApplication* app, x86::CPU& cpu);
    static void sub_459410(WinApplication* app, x86::CPU& cpu);
    static void sub_459670(WinApplication* app, x86::CPU& cpu);
    static void sub_459db0(WinApplication* app, x86::CPU& cpu);
    static void sub_459fe0(WinApplication* app, x86::CPU& cpu);
    static void sub_45a120(WinApplication* app, x86::CPU& cpu);
    static void sub_46ca10(WinApplication* app, x86::CPU& cpu);
    static void sub_470a60(WinApplication* app, x86::CPU& cpu);
    static void sub_470aa0(WinApplication* app, x86::CPU& cpu);
    static void sub_470ad0(WinApplication* app, x86::CPU& cpu);
    static void sub_470b20(WinApplication* app, x86::CPU& cpu);
    static void sub_470b70(WinApplication* app, x86::CPU& cpu);
    static void sub_470b90(WinApplication* app, x86::CPU& cpu);
    static void sub_473340(WinApplication* app, x86::CPU& cpu);
    static void sub_473360(WinApplication* app, x86::CPU& cpu);
    static void sub_4733b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4734d0(WinApplication* app, x86::CPU& cpu);
    static void sub_473550(WinApplication* app, x86::CPU& cpu);
    static void sub_473570(WinApplication* app, x86::CPU& cpu);
    static void sub_4735c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4736c0(WinApplication* app, x86::CPU& cpu);
    static void sub_473710(WinApplication* app, x86::CPU& cpu);
    static void sub_473890(WinApplication* app, x86::CPU& cpu);
    static void sub_473960(WinApplication* app, x86::CPU& cpu);
    static void sub_473a70(WinApplication* app, x86::CPU& cpu);
    static void sub_473ac0(WinApplication* app, x86::CPU& cpu);
    static void sub_473b70(WinApplication* app, x86::CPU& cpu);
    static void sub_473bf0(WinApplication* app, x86::CPU& cpu);
    static void sub_473ee0(WinApplication* app, x86::CPU& cpu);
    static void sub_474310(WinApplication* app, x86::CPU& cpu);
    static void sub_474640(WinApplication* app, x86::CPU& cpu);
    static void sub_474c60(WinApplication* app, x86::CPU& cpu);
    static void sub_474ca0(WinApplication* app, x86::CPU& cpu);
    static void sub_474d20(WinApplication* app, x86::CPU& cpu);
    static void sub_475b10(WinApplication* app, x86::CPU& cpu);
    static void sub_476fb0(WinApplication* app, x86::CPU& cpu);
    static void sub_477290(WinApplication* app, x86::CPU& cpu);
    static void sub_4774b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4777d0(WinApplication* app, x86::CPU& cpu);
    static void sub_477830(WinApplication* app, x86::CPU& cpu);
    static void sub_477850(WinApplication* app, x86::CPU& cpu);
    static void sub_4778c0(WinApplication* app, x86::CPU& cpu);
    static void sub_477990(WinApplication* app, x86::CPU& cpu);
    static void sub_4779e0(WinApplication* app, x86::CPU& cpu);
    static void sub_477a10(WinApplication* app, x86::CPU& cpu);
    static void sub_477a50(WinApplication* app, x86::CPU& cpu);
    static void sub_477b50(WinApplication* app, x86::CPU& cpu);
    static void sub_477c20(WinApplication* app, x86::CPU& cpu);
    static void sub_477d90(WinApplication* app, x86::CPU& cpu);
    static void sub_477f60(WinApplication* app, x86::CPU& cpu);
    static void sub_477fd0(WinApplication* app, x86::CPU& cpu);
    static void sub_478090(WinApplication* app, x86::CPU& cpu);
    static void sub_478100(WinApplication* app, x86::CPU& cpu);
    static void sub_4781b0(WinApplication* app, x86::CPU& cpu);
    static void sub_478280(WinApplication* app, x86::CPU& cpu);
    static void sub_478290(WinApplication* app, x86::CPU& cpu);
    static void sub_4782b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4782c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4782d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4782e0(WinApplication* app, x86::CPU& cpu);
    static void sub_478320(WinApplication* app, x86::CPU& cpu);
    static void sub_478b70(WinApplication* app, x86::CPU& cpu);
    static void sub_478bb0(WinApplication* app, x86::CPU& cpu);
    static void sub_478c70(WinApplication* app, x86::CPU& cpu);
    static void sub_478c80(WinApplication* app, x86::CPU& cpu);
    static void sub_478cb0(WinApplication* app, x86::CPU& cpu);
    static void sub_478dc0(WinApplication* app, x86::CPU& cpu);
    static void sub_478e10(WinApplication* app, x86::CPU& cpu);
    static void sub_479080(WinApplication* app, x86::CPU& cpu);
    static void sub_4790d0(WinApplication* app, x86::CPU& cpu);
    static void sub_479100(WinApplication* app, x86::CPU& cpu);
    static void sub_479160(WinApplication* app, x86::CPU& cpu);
    static void sub_4792e0(WinApplication* app, x86::CPU& cpu);
    static void sub_479370(WinApplication* app, x86::CPU& cpu);
    static void sub_479500(WinApplication* app, x86::CPU& cpu);
    static void sub_479680(WinApplication* app, x86::CPU& cpu);
    static void sub_479860(WinApplication* app, x86::CPU& cpu);
    static void sub_4799d0(WinApplication* app, x86::CPU& cpu);
    static void sub_479b90(WinApplication* app, x86::CPU& cpu);
    static void sub_479ed0(WinApplication* app, x86::CPU& cpu);
    static void sub_47a4c0(WinApplication* app, x86::CPU& cpu);
    static void sub_47a970(WinApplication* app, x86::CPU& cpu);
    static void sub_47adc0(WinApplication* app, x86::CPU& cpu);
    static void sub_47ae10(WinApplication* app, x86::CPU& cpu);
    static void sub_47b350(WinApplication* app, x86::CPU& cpu);
    static void sub_47b390(WinApplication* app, x86::CPU& cpu);
    static void sub_47b3a0(WinApplication* app, x86::CPU& cpu);
    static void sub_47b4f0(WinApplication* app, x86::CPU& cpu);
    static void sub_47b5c0(WinApplication* app, x86::CPU& cpu);
    static void sub_47bbb8(WinApplication* app, x86::CPU& cpu);
    static void sub_47bc20(WinApplication* app, x86::CPU& cpu);
    static void sub_47bc40(WinApplication* app, x86::CPU& cpu);
    static void sub_47d1d0(WinApplication* app, x86::CPU& cpu);
    static void sub_47d480(WinApplication* app, x86::CPU& cpu);
    static void sub_47d580(WinApplication* app, x86::CPU& cpu);
    static void sub_47ee80(WinApplication* app, x86::CPU& cpu);
    static void sub_47f090(WinApplication* app, x86::CPU& cpu);
    static void sub_47f2e0(WinApplication* app, x86::CPU& cpu);
    static void sub_47f340(WinApplication* app, x86::CPU& cpu);
    static void sub_47f590(WinApplication* app, x86::CPU& cpu);
    static void sub_47fbe0(WinApplication* app, x86::CPU& cpu);
    static void sub_47fcb0(WinApplication* app, x86::CPU& cpu);
    static void sub_47fd80(WinApplication* app, x86::CPU& cpu);
    static void sub_4808b0(WinApplication* app, x86::CPU& cpu);
    static void sub_480a00(WinApplication* app, x86::CPU& cpu);
    static void sub_481170(WinApplication* app, x86::CPU& cpu);
    static void sub_481290(WinApplication* app, x86::CPU& cpu);
    static void sub_481510(WinApplication* app, x86::CPU& cpu);
    static void sub_481570(WinApplication* app, x86::CPU& cpu);
    static void sub_4815d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4818b0(WinApplication* app, x86::CPU& cpu);
    static void sub_481f90(WinApplication* app, x86::CPU& cpu);
    static void sub_482030(WinApplication* app, x86::CPU& cpu);
    static void sub_482190(WinApplication* app, x86::CPU& cpu);
    static void sub_482880(WinApplication* app, x86::CPU& cpu);
    static void sub_484030(WinApplication* app, x86::CPU& cpu);
    static void sub_484330(WinApplication* app, x86::CPU& cpu);
    static void sub_485880(WinApplication* app, x86::CPU& cpu);
    static void sub_48fb40(WinApplication* app, x86::CPU& cpu);
    static void sub_490430(WinApplication* app, x86::CPU& cpu);
    static void sub_491120(WinApplication* app, x86::CPU& cpu);
    static void sub_493380(WinApplication* app, x86::CPU& cpu);
    static void sub_4934c0(WinApplication* app, x86::CPU& cpu);
    static void sub_493640(WinApplication* app, x86::CPU& cpu);
    static void sub_493a30(WinApplication* app, x86::CPU& cpu);
    static void sub_493ad0(WinApplication* app, x86::CPU& cpu);
    static void sub_493e50(WinApplication* app, x86::CPU& cpu);
    static void sub_493ef0(WinApplication* app, x86::CPU& cpu);
    static void sub_493fb0(WinApplication* app, x86::CPU& cpu);
    static void sub_494010(WinApplication* app, x86::CPU& cpu);
    static void sub_494170(WinApplication* app, x86::CPU& cpu);
    static void sub_494360(WinApplication* app, x86::CPU& cpu);
    static void sub_494390(WinApplication* app, x86::CPU& cpu);
    static void sub_4943e0(WinApplication* app, x86::CPU& cpu);
    static void sub_494420(WinApplication* app, x86::CPU& cpu);
    static void sub_4945a0(WinApplication* app, x86::CPU& cpu);
    static void sub_494630(WinApplication* app, x86::CPU& cpu);
    static void sub_495190(WinApplication* app, x86::CPU& cpu);
    static void sub_495600(WinApplication* app, x86::CPU& cpu);
    static void sub_4957a0(WinApplication* app, x86::CPU& cpu);
    static void sub_495860(WinApplication* app, x86::CPU& cpu);
    static void sub_495890(WinApplication* app, x86::CPU& cpu);
    static void sub_4958c0(WinApplication* app, x86::CPU& cpu);
    static void sub_495920(WinApplication* app, x86::CPU& cpu);
    static void sub_495a10(WinApplication* app, x86::CPU& cpu);
    static void sub_495a50(WinApplication* app, x86::CPU& cpu);
    static void sub_495b10(WinApplication* app, x86::CPU& cpu);
    static void sub_495b30(WinApplication* app, x86::CPU& cpu);
    static void sub_495b60(WinApplication* app, x86::CPU& cpu);
    static void sub_495bc0(WinApplication* app, x86::CPU& cpu);
    static void sub_496300(WinApplication* app, x86::CPU& cpu);
    static void sub_496310(WinApplication* app, x86::CPU& cpu);
    static void sub_4964d0(WinApplication* app, x86::CPU& cpu);
    static void sub_496550(WinApplication* app, x86::CPU& cpu);
    static void sub_4969c0(WinApplication* app, x86::CPU& cpu);
    static void sub_496a50(WinApplication* app, x86::CPU& cpu);
    static void sub_496bf0(WinApplication* app, x86::CPU& cpu);
    static void sub_496f20(WinApplication* app, x86::CPU& cpu);
    static void sub_4970c0(WinApplication* app, x86::CPU& cpu);
    static void sub_497430(WinApplication* app, x86::CPU& cpu);
    static void sub_497500(WinApplication* app, x86::CPU& cpu);
    static void sub_497620(WinApplication* app, x86::CPU& cpu);
    static void sub_497ba0(WinApplication* app, x86::CPU& cpu);
    static void sub_497cb0(WinApplication* app, x86::CPU& cpu);
    static void sub_497cc0(WinApplication* app, x86::CPU& cpu);
    static void sub_497d20(WinApplication* app, x86::CPU& cpu);
    static void sub_497fe0(WinApplication* app, x86::CPU& cpu);
    static void sub_498090(WinApplication* app, x86::CPU& cpu);
    static void sub_498370(WinApplication* app, x86::CPU& cpu);
    static void sub_4989a0(WinApplication* app, x86::CPU& cpu);
    static void sub_498a40(WinApplication* app, x86::CPU& cpu);
    static void sub_498b70(WinApplication* app, x86::CPU& cpu);
    static void sub_498bf0(WinApplication* app, x86::CPU& cpu);
    static void sub_498e10(WinApplication* app, x86::CPU& cpu);
    static void sub_498e40(WinApplication* app, x86::CPU& cpu);
    static void sub_498f00(WinApplication* app, x86::CPU& cpu);
    static void sub_499150(WinApplication* app, x86::CPU& cpu);
    static void sub_499330(WinApplication* app, x86::CPU& cpu);
    static void sub_4993b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4993d0(WinApplication* app, x86::CPU& cpu);
    static void sub_499400(WinApplication* app, x86::CPU& cpu);
    static void sub_499460(WinApplication* app, x86::CPU& cpu);
    static void sub_499480(WinApplication* app, x86::CPU& cpu);
    static void sub_499490(WinApplication* app, x86::CPU& cpu);
    static void sub_4994a0(WinApplication* app, x86::CPU& cpu);
    static void sub_499580(WinApplication* app, x86::CPU& cpu);
    static void sub_4995b0(WinApplication* app, x86::CPU& cpu);
    static void sub_499b30(WinApplication* app, x86::CPU& cpu);
    static void sub_499d30(WinApplication* app, x86::CPU& cpu);
    static void sub_499e90(WinApplication* app, x86::CPU& cpu);
    static void sub_499ea0(WinApplication* app, x86::CPU& cpu);
    static void sub_499f70(WinApplication* app, x86::CPU& cpu);
    static void sub_49a120(WinApplication* app, x86::CPU& cpu);
    static void sub_49a400(WinApplication* app, x86::CPU& cpu);
    static void sub_49a490(WinApplication* app, x86::CPU& cpu);
    static void sub_49a520(WinApplication* app, x86::CPU& cpu);
    static void sub_49a8e0(WinApplication* app, x86::CPU& cpu);
    static void sub_49aa80(WinApplication* app, x86::CPU& cpu);
    static void sub_49abd0(WinApplication* app, x86::CPU& cpu);
    static void sub_49acd0(WinApplication* app, x86::CPU& cpu);
    static void sub_49b2b0(WinApplication* app, x86::CPU& cpu);
    static void sub_49b310(WinApplication* app, x86::CPU& cpu);
    static void sub_49b3e0(WinApplication* app, x86::CPU& cpu);
    static void sub_49b470(WinApplication* app, x86::CPU& cpu);
    static void sub_49b500(WinApplication* app, x86::CPU& cpu);
    static void sub_49b520(WinApplication* app, x86::CPU& cpu);
    static void sub_49b9b0(WinApplication* app, x86::CPU& cpu);
    static void sub_49bab0(WinApplication* app, x86::CPU& cpu);
    static void sub_49bc80(WinApplication* app, x86::CPU& cpu);
    static void sub_49bd10(WinApplication* app, x86::CPU& cpu);
    static void sub_49bfb0(WinApplication* app, x86::CPU& cpu);
    static void sub_49c410(WinApplication* app, x86::CPU& cpu);
    static void sub_49c420(WinApplication* app, x86::CPU& cpu);
    static void sub_49c470(WinApplication* app, x86::CPU& cpu);
    static void sub_49c4a0(WinApplication* app, x86::CPU& cpu);
    static void sub_49cfa0(WinApplication* app, x86::CPU& cpu);
    static void sub_49d5a0(WinApplication* app, x86::CPU& cpu);
    static void sub_49d5c0(WinApplication* app, x86::CPU& cpu);
    static void sub_49d720(WinApplication* app, x86::CPU& cpu);
    static void sub_49d750(WinApplication* app, x86::CPU& cpu);
    static void sub_49d760(WinApplication* app, x86::CPU& cpu);
    static void sub_49d780(WinApplication* app, x86::CPU& cpu);
    static void sub_49d8f0(WinApplication* app, x86::CPU& cpu);
    static void sub_49de20(WinApplication* app, x86::CPU& cpu);
    static void sub_49e080(WinApplication* app, x86::CPU& cpu);
    static void sub_49e220(WinApplication* app, x86::CPU& cpu);
    static void sub_49e2e0(WinApplication* app, x86::CPU& cpu);
    static void sub_49e320(WinApplication* app, x86::CPU& cpu);
    static void sub_49eae0(WinApplication* app, x86::CPU& cpu);
    static void sub_49ec70(WinApplication* app, x86::CPU& cpu);
    static void sub_49edb0(WinApplication* app, x86::CPU& cpu);
    static void sub_49ede0(WinApplication* app, x86::CPU& cpu);
    static void sub_49ee50(WinApplication* app, x86::CPU& cpu);
    static void sub_49f680(WinApplication* app, x86::CPU& cpu);
    static void sub_49f730(WinApplication* app, x86::CPU& cpu);
    static void sub_49f790(WinApplication* app, x86::CPU& cpu);
    static void sub_49f810(WinApplication* app, x86::CPU& cpu);
    static void sub_49f880(WinApplication* app, x86::CPU& cpu);
    static void sub_49fb10(WinApplication* app, x86::CPU& cpu);
    static void sub_49fb60(WinApplication* app, x86::CPU& cpu);
    static void sub_49fc50(WinApplication* app, x86::CPU& cpu);
    static void sub_4a3050(WinApplication* app, x86::CPU& cpu);
    static void sub_4a3070(WinApplication* app, x86::CPU& cpu);
    static void sub_4a31f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a32f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a3390(WinApplication* app, x86::CPU& cpu);
    static void sub_4a33a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a33b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a33d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a3400(WinApplication* app, x86::CPU& cpu);
    static void sub_4a3690(WinApplication* app, x86::CPU& cpu);
    static void sub_4a38c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a38f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a4020(WinApplication* app, x86::CPU& cpu);
    static void sub_4a4060(WinApplication* app, x86::CPU& cpu);
    static void sub_4a40c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a46f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a5080(WinApplication* app, x86::CPU& cpu);
    static void sub_4a54f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a5780(WinApplication* app, x86::CPU& cpu);
    static void sub_4a5880(WinApplication* app, x86::CPU& cpu);
    static void sub_4a59c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a5af0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a5b00(WinApplication* app, x86::CPU& cpu);
    static void sub_4a5dd0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a5e50(WinApplication* app, x86::CPU& cpu);
    static void sub_4a5ef0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a5f30(WinApplication* app, x86::CPU& cpu);
    static void sub_4a6410(WinApplication* app, x86::CPU& cpu);
    static void sub_4a6440(WinApplication* app, x86::CPU& cpu);
    static void sub_4a6470(WinApplication* app, x86::CPU& cpu);
    static void sub_4a64c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a6500(WinApplication* app, x86::CPU& cpu);
    static void sub_4a65b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a66c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a68d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a69c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a6a10(WinApplication* app, x86::CPU& cpu);
    static void sub_4a6c40(WinApplication* app, x86::CPU& cpu);
    static void sub_4a6ce0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a6df0(WinApplication* app, x86::CPU& cpu);
    static void sub_4a6e10(WinApplication* app, x86::CPU& cpu);
    static void sub_4a6f70(WinApplication* app, x86::CPU& cpu);
    static void sub_4a7080(WinApplication* app, x86::CPU& cpu);
    static void sub_4a7310(WinApplication* app, x86::CPU& cpu);
    static void sub_4a75e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b0700(WinApplication* app, x86::CPU& cpu);
    static void sub_4b0790(WinApplication* app, x86::CPU& cpu);
    static void sub_4b07b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b07c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b07d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b0880(WinApplication* app, x86::CPU& cpu);
    static void sub_4b08b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b08d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b0900(WinApplication* app, x86::CPU& cpu);
    static void sub_4b0940(WinApplication* app, x86::CPU& cpu);
    static void sub_4b09d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b0bb0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b1380(WinApplication* app, x86::CPU& cpu);
    static void sub_4b22d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b48d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b56d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b5700(WinApplication* app, x86::CPU& cpu);
    static void sub_4b5720(WinApplication* app, x86::CPU& cpu);
    static void sub_4b5860(WinApplication* app, x86::CPU& cpu);
    static void sub_4b5940(WinApplication* app, x86::CPU& cpu);
    static void sub_4b5e50(WinApplication* app, x86::CPU& cpu);
    static void sub_4b5f10(WinApplication* app, x86::CPU& cpu);
    static void sub_4b5ff0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b6020(WinApplication* app, x86::CPU& cpu);
    static void sub_4b62a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b6320(WinApplication* app, x86::CPU& cpu);
    static void sub_4b6400(WinApplication* app, x86::CPU& cpu);
    static void sub_4b64c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b6520(WinApplication* app, x86::CPU& cpu);
    static void sub_4b6590(WinApplication* app, x86::CPU& cpu);
    static void sub_4b6a30(WinApplication* app, x86::CPU& cpu);
    static void sub_4b6ca0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b6d90(WinApplication* app, x86::CPU& cpu);
    static void sub_4b6e80(WinApplication* app, x86::CPU& cpu);
    static void sub_4b7c20(WinApplication* app, x86::CPU& cpu);
    static void sub_4b7c60(WinApplication* app, x86::CPU& cpu);
    static void sub_4b7d00(WinApplication* app, x86::CPU& cpu);
    static void sub_4b7eb0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b7f80(WinApplication* app, x86::CPU& cpu);
    static void sub_4b81a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b8740(WinApplication* app, x86::CPU& cpu);
    static void sub_4b90e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b9140(WinApplication* app, x86::CPU& cpu);
    static void sub_4b9280(WinApplication* app, x86::CPU& cpu);
    static void sub_4b93c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4b9550(WinApplication* app, x86::CPU& cpu);
    static void sub_4b9df0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ba320(WinApplication* app, x86::CPU& cpu);
    static void sub_4ba350(WinApplication* app, x86::CPU& cpu);
    static void sub_4ba810(WinApplication* app, x86::CPU& cpu);
    static void sub_4ba960(WinApplication* app, x86::CPU& cpu);
    static void sub_4baa50(WinApplication* app, x86::CPU& cpu);
    static void sub_4bab70(WinApplication* app, x86::CPU& cpu);
    static void sub_4bb110(WinApplication* app, x86::CPU& cpu);
    static void sub_4bb4c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bba10(WinApplication* app, x86::CPU& cpu);
    static void sub_4bba50(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbad0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbba0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbd70(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbd80(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbdc0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbde0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbe40(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbe70(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbe90(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbec0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbf20(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbf40(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbf80(WinApplication* app, x86::CPU& cpu);
    static void sub_4bbfc0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc000(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc030(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc060(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc0b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc100(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc130(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc560(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc620(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc650(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc680(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc820(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc830(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc870(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc8b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc8e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc910(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc950(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc980(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc990(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc9a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bc9b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bca80(WinApplication* app, x86::CPU& cpu);
    static void sub_4bcb00(WinApplication* app, x86::CPU& cpu);
    static void sub_4bcb90(WinApplication* app, x86::CPU& cpu);
    static void sub_4bcd60(WinApplication* app, x86::CPU& cpu);
    static void sub_4bcda0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bcee0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bd000(WinApplication* app, x86::CPU& cpu);
    static void sub_4bd090(WinApplication* app, x86::CPU& cpu);
    static void sub_4bd380(WinApplication* app, x86::CPU& cpu);
    static void sub_4bd3f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bd9d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bda00(WinApplication* app, x86::CPU& cpu);
    static void sub_4be020(WinApplication* app, x86::CPU& cpu);
    static void sub_4be1f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4be240(WinApplication* app, x86::CPU& cpu);
    static void sub_4be250(WinApplication* app, x86::CPU& cpu);
    static void sub_4be2f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4be3a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4be5e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4be970(WinApplication* app, x86::CPU& cpu);
    static void sub_4bea40(WinApplication* app, x86::CPU& cpu);
    static void sub_4bebb0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bed20(WinApplication* app, x86::CPU& cpu);
    static void sub_4bed50(WinApplication* app, x86::CPU& cpu);
    static void sub_4bed70(WinApplication* app, x86::CPU& cpu);
    static void sub_4bf110(WinApplication* app, x86::CPU& cpu);
    static void sub_4bf1b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4bf260(WinApplication* app, x86::CPU& cpu);
    static void sub_4bf370(WinApplication* app, x86::CPU& cpu);
    static void sub_4c1900(WinApplication* app, x86::CPU& cpu);
    static void sub_4c19e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c4190(WinApplication* app, x86::CPU& cpu);
    static void sub_4c45d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c45f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c4740(WinApplication* app, x86::CPU& cpu);
    static void sub_4c4820(WinApplication* app, x86::CPU& cpu);
    static void sub_4c4830(WinApplication* app, x86::CPU& cpu);
    static void sub_4c48d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c4ac0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c4be0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c4d80(WinApplication* app, x86::CPU& cpu);
    static void sub_4c4de0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c4e40(WinApplication* app, x86::CPU& cpu);
    static void sub_4c5520(WinApplication* app, x86::CPU& cpu);
    static void sub_4c5540(WinApplication* app, x86::CPU& cpu);
    static void sub_4c5560(WinApplication* app, x86::CPU& cpu);
    static void sub_4c55a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c5720(WinApplication* app, x86::CPU& cpu);
    static void sub_4c5b00(WinApplication* app, x86::CPU& cpu);
    static void sub_4c5e80(WinApplication* app, x86::CPU& cpu);
    static void sub_4c62a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c63c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c63f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c64e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c6c70(WinApplication* app, x86::CPU& cpu);
    static void sub_4c6cb0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c6f00(WinApplication* app, x86::CPU& cpu);
    static void sub_4c70f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7130(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7150(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7180(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7260(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7380(WinApplication* app, x86::CPU& cpu);
    static void sub_4c73e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7400(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7480(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7580(WinApplication* app, x86::CPU& cpu);
    static void sub_4c75f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c76e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7750(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7940(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7b10(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7b20(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7b30(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7b50(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7bb0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7bd0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7d30(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7e50(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7e80(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7ea0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7f90(WinApplication* app, x86::CPU& cpu);
    static void sub_4c7fd0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c8040(WinApplication* app, x86::CPU& cpu);
    static void sub_4c81c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c8250(WinApplication* app, x86::CPU& cpu);
    static void sub_4c8310(WinApplication* app, x86::CPU& cpu);
    static void sub_4c8350(WinApplication* app, x86::CPU& cpu);
    static void sub_4c8470(WinApplication* app, x86::CPU& cpu);
    static void sub_4c8550(WinApplication* app, x86::CPU& cpu);
    static void sub_4c9a80(WinApplication* app, x86::CPU& cpu);
    static void sub_4c9bc0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c9d70(WinApplication* app, x86::CPU& cpu);
    static void sub_4c9d90(WinApplication* app, x86::CPU& cpu);
    static void sub_4c9db0(WinApplication* app, x86::CPU& cpu);
    static void sub_4c9f70(WinApplication* app, x86::CPU& cpu);
    static void sub_4ca160(WinApplication* app, x86::CPU& cpu);
    static void sub_4ca480(WinApplication* app, x86::CPU& cpu);
    static void sub_4ca4b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ca500(WinApplication* app, x86::CPU& cpu);
    static void sub_4ca850(WinApplication* app, x86::CPU& cpu);
    static void sub_4cab60(WinApplication* app, x86::CPU& cpu);
    static void sub_4cac20(WinApplication* app, x86::CPU& cpu);
    static void sub_4cae00(WinApplication* app, x86::CPU& cpu);
    static void sub_4caea0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cb010(WinApplication* app, x86::CPU& cpu);
    static void sub_4cb220(WinApplication* app, x86::CPU& cpu);
    static void sub_4cb590(WinApplication* app, x86::CPU& cpu);
    static void sub_4cb6f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cb730(WinApplication* app, x86::CPU& cpu);
    static void sub_4cc540(WinApplication* app, x86::CPU& cpu);
    static void sub_4cc5c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cc600(WinApplication* app, x86::CPU& cpu);
    static void sub_4cc650(WinApplication* app, x86::CPU& cpu);
    static void sub_4cc660(WinApplication* app, x86::CPU& cpu);
    static void sub_4cc6d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cc7c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cc860(WinApplication* app, x86::CPU& cpu);
    static void sub_4cc870(WinApplication* app, x86::CPU& cpu);
    static void sub_4cc890(WinApplication* app, x86::CPU& cpu);
    static void sub_4cc8e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccaa0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccbb7(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccbd7(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccbd0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccbc9(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccbde(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccbe5(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccbec(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccbf3(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccbfa(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc01(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc08(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc0f(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc16(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc1d(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc24(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc2b(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc32(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc39(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc40(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc47(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc4e(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc55(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc5c(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc63(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccc70(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccd60(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccdd0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cce40(WinApplication* app, x86::CPU& cpu);
    static void sub_4cce60(WinApplication* app, x86::CPU& cpu);
    static void sub_4cce80(WinApplication* app, x86::CPU& cpu);
    static void sub_4cce90(WinApplication* app, x86::CPU& cpu);
    static void sub_4cced0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccf50(WinApplication* app, x86::CPU& cpu);
    static void sub_4ccfc0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cd020(WinApplication* app, x86::CPU& cpu);
    static void sub_4cd0e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cd130(WinApplication* app, x86::CPU& cpu);
    static void sub_4cd190(WinApplication* app, x86::CPU& cpu);
    static void sub_4cd1f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cd2a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cd2d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cd350(WinApplication* app, x86::CPU& cpu);
    static void sub_4cd4e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cd6c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cdaa0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cdb10(WinApplication* app, x86::CPU& cpu);
    static void sub_4cdbd0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cdc50(WinApplication* app, x86::CPU& cpu);
    static void sub_4cdc90(WinApplication* app, x86::CPU& cpu);
    static void sub_4cdd00(WinApplication* app, x86::CPU& cpu);
    static void sub_4cdd30(WinApplication* app, x86::CPU& cpu);
    static void sub_4cde00(WinApplication* app, x86::CPU& cpu);
    static void sub_4cded0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cdee0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cdef0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cdf10(WinApplication* app, x86::CPU& cpu);
    static void sub_4ce280(WinApplication* app, x86::CPU& cpu);
    static void sub_4ce5a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ce750(WinApplication* app, x86::CPU& cpu);
    static void sub_4ce800(WinApplication* app, x86::CPU& cpu);
    static void sub_4ce840(WinApplication* app, x86::CPU& cpu);
    static void sub_4ce8e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ce960(WinApplication* app, x86::CPU& cpu);
    static void sub_4ceae0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ceb50(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf1c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf1f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf220(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf280(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf2f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf340(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf380(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf3f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf440(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf570(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf5a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf5e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf6b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cf810(WinApplication* app, x86::CPU& cpu);
    static void sub_4cfa40(WinApplication* app, x86::CPU& cpu);
    static void sub_4cfa80(WinApplication* app, x86::CPU& cpu);
    static void sub_4cfca0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cfd30(WinApplication* app, x86::CPU& cpu);
    static void sub_4cfdd0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cfe30(WinApplication* app, x86::CPU& cpu);
    static void sub_4cfe70(WinApplication* app, x86::CPU& cpu);
    static void sub_4cfef0(WinApplication* app, x86::CPU& cpu);
    static void sub_4cffb0(WinApplication* app, x86::CPU& cpu);
    static void sub_4d0060(WinApplication* app, x86::CPU& cpu);
    static void sub_4d0220(WinApplication* app, x86::CPU& cpu);
    static void sub_4d0300(WinApplication* app, x86::CPU& cpu);
    static void sub_4d03c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4d0460(WinApplication* app, x86::CPU& cpu);
    static void sub_4d0490(WinApplication* app, x86::CPU& cpu);
    static void sub_4d04c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4d0500(WinApplication* app, x86::CPU& cpu);
    static void sub_4d0d50(WinApplication* app, x86::CPU& cpu);
    static void sub_4d18b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4d3380(WinApplication* app, x86::CPU& cpu);
    static void sub_4d4080(WinApplication* app, x86::CPU& cpu);
    static void sub_4d4120(WinApplication* app, x86::CPU& cpu);
    static void sub_4d4440(WinApplication* app, x86::CPU& cpu);
    static void sub_4d5060(WinApplication* app, x86::CPU& cpu);
    static void sub_4d5220(WinApplication* app, x86::CPU& cpu);
    static void sub_4d5370(WinApplication* app, x86::CPU& cpu);
    static void sub_4d56b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4d56f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4d5730(WinApplication* app, x86::CPU& cpu);
    static void sub_4d5d10(WinApplication* app, x86::CPU& cpu);
    static void sub_4db750(WinApplication* app, x86::CPU& cpu);
    static void sub_4db860(WinApplication* app, x86::CPU& cpu);
    static void sub_4db8b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4db940(WinApplication* app, x86::CPU& cpu);
    static void sub_4dbba0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dbbc0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dbbe0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dbbf0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dbcc0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dbce0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dbda0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dc0e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dc1b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dc1e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dc550(WinApplication* app, x86::CPU& cpu);
    static void sub_4dcbc0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dcd10(WinApplication* app, x86::CPU& cpu);
    static void sub_4dcdd0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dcdf0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dd230(WinApplication* app, x86::CPU& cpu);
    static void sub_4dd330(WinApplication* app, x86::CPU& cpu);
    static void sub_4dd3d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dd4c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dd510(WinApplication* app, x86::CPU& cpu);
    static void sub_4dd5e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dd930(WinApplication* app, x86::CPU& cpu);
    static void sub_4dece0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dedf0(WinApplication* app, x86::CPU& cpu);
    static void sub_4dee80(WinApplication* app, x86::CPU& cpu);
    static void sub_4df1f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4df2b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4df310(WinApplication* app, x86::CPU& cpu);
    static void sub_4df340(WinApplication* app, x86::CPU& cpu);
    static void sub_4e0090(WinApplication* app, x86::CPU& cpu);
    static void sub_4e0110(WinApplication* app, x86::CPU& cpu);
    static void sub_4e0160(WinApplication* app, x86::CPU& cpu);
    static void sub_4e01a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4e0658(WinApplication* app, x86::CPU& cpu);
    static void sub_4e0659(WinApplication* app, x86::CPU& cpu);
    static void sub_4e4230(WinApplication* app, x86::CPU& cpu);
    static void sub_4e4320(WinApplication* app, x86::CPU& cpu);
    static void sub_4e4360(WinApplication* app, x86::CPU& cpu);
    static void sub_4e43a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4e47a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ea440(WinApplication* app, x86::CPU& cpu);
    static void sub_4ea4a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ea665(WinApplication* app, x86::CPU& cpu);
    static void sub_4ea930(WinApplication* app, x86::CPU& cpu);
    static void sub_4ea950(WinApplication* app, x86::CPU& cpu);
    static void sub_4ea970(WinApplication* app, x86::CPU& cpu);
    static void sub_4ea9c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ea9e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4eaab0(WinApplication* app, x86::CPU& cpu);
    static void sub_4eab80(WinApplication* app, x86::CPU& cpu);
    static void sub_4eac60(WinApplication* app, x86::CPU& cpu);
    static void sub_4ead20(WinApplication* app, x86::CPU& cpu);
    static void sub_4ead90(WinApplication* app, x86::CPU& cpu);
    static void sub_4eada0(WinApplication* app, x86::CPU& cpu);
    static void sub_4eae90(WinApplication* app, x86::CPU& cpu);
    static void sub_4eaeb0(WinApplication* app, x86::CPU& cpu);
    static void sub_4eaf60(WinApplication* app, x86::CPU& cpu);
    static void sub_4eafc0(WinApplication* app, x86::CPU& cpu);
    static void sub_4eb160(WinApplication* app, x86::CPU& cpu);
    static void sub_4eb1c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4eb420(WinApplication* app, x86::CPU& cpu);
    static void sub_4eb430(WinApplication* app, x86::CPU& cpu);
    static void sub_4eb490(WinApplication* app, x86::CPU& cpu);
    static void sub_4eb5e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4eb880(WinApplication* app, x86::CPU& cpu);
    static void sub_4eb8a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4eba80(WinApplication* app, x86::CPU& cpu);
    static void sub_4eba90(WinApplication* app, x86::CPU& cpu);
    static void sub_4ebaf0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ebe50(WinApplication* app, x86::CPU& cpu);
    static void sub_4ec0d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ec1a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ec870(WinApplication* app, x86::CPU& cpu);
    static void sub_4ecd50(WinApplication* app, x86::CPU& cpu);
    static void sub_4ecef0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ecf60(WinApplication* app, x86::CPU& cpu);
    static void sub_4ed0f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ed2a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ed330(WinApplication* app, x86::CPU& cpu);
    static void sub_4ed480(WinApplication* app, x86::CPU& cpu);
    static void sub_4ed730(WinApplication* app, x86::CPU& cpu);
    static void sub_4ed770(WinApplication* app, x86::CPU& cpu);
    static void sub_4ed820(WinApplication* app, x86::CPU& cpu);
    static void sub_4edbc0(WinApplication* app, x86::CPU& cpu);
    static void sub_4eee30(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef190(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef1b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef1e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef2b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef320(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef340(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef370(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef3c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef3f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef4c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef520(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef6c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef760(WinApplication* app, x86::CPU& cpu);
    static void sub_4ef830(WinApplication* app, x86::CPU& cpu);
    static void sub_4efa10(WinApplication* app, x86::CPU& cpu);
    static void sub_4efa70(WinApplication* app, x86::CPU& cpu);
    static void sub_4efb10(WinApplication* app, x86::CPU& cpu);
    static void sub_4efb50(WinApplication* app, x86::CPU& cpu);
    static void sub_4efe30(WinApplication* app, x86::CPU& cpu);
    static void sub_4efe50(WinApplication* app, x86::CPU& cpu);
    static void sub_4efef0(WinApplication* app, x86::CPU& cpu);
    static void sub_4eff00(WinApplication* app, x86::CPU& cpu);
    static void sub_4f00d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f00e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f00f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f01f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0230(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0280(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0330(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0630(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0650(WinApplication* app, x86::CPU& cpu);
    static void sub_4f06e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0840(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0850(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0860(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0880(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0890(WinApplication* app, x86::CPU& cpu);
    static void sub_4f08a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f08c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f08d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f08e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f08f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0910(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0920(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0930(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0940(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0960(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0970(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0980(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0990(WinApplication* app, x86::CPU& cpu);
    static void sub_4f09b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f09c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0a10(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0a90(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0af0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0b60(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0c90(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0cb0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0cc0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0cd0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0d20(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0da0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f0f40(WinApplication* app, x86::CPU& cpu);
    static void sub_4f1370(WinApplication* app, x86::CPU& cpu);
    static void sub_4f1500(WinApplication* app, x86::CPU& cpu);
    static void sub_4f15d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f1790(WinApplication* app, x86::CPU& cpu);
    static void sub_4f1820(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2170(WinApplication* app, x86::CPU& cpu);
    static void sub_4f22e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2460(WinApplication* app, x86::CPU& cpu);
    static void sub_4f24e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2790(WinApplication* app, x86::CPU& cpu);
    static void sub_4f27a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2890(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2930(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2a60(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2a78(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2a7d(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2ad0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2bf0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2c40(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2c50(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2c60(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2c70(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2c90(WinApplication* app, x86::CPU& cpu);
    static void sub_4f2cb0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f30d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f3350(WinApplication* app, x86::CPU& cpu);
    static void sub_4f33b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f33e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f3420(WinApplication* app, x86::CPU& cpu);
    static void sub_4f34f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f3560(WinApplication* app, x86::CPU& cpu);
    static void sub_4f3600(WinApplication* app, x86::CPU& cpu);
    static void sub_4f44e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4544(WinApplication* app, x86::CPU& cpu);
    static void sub_4f45d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f45e1(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4650(WinApplication* app, x86::CPU& cpu);
    static void sub_4f46b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f46d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4700(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4740(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4760(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4780(WinApplication* app, x86::CPU& cpu);
    static void sub_4f47c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4920(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4a80(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4bb0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4c10(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4c70(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4d10(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4e60(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4eb0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4ed0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4f10(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4f30(WinApplication* app, x86::CPU& cpu);
    static void sub_4f4f70(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5050(WinApplication* app, x86::CPU& cpu);
    static void sub_4f50b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f50c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f50f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5110(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5130(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5230(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5390(WinApplication* app, x86::CPU& cpu);
    static void sub_4f53f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f54f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f55e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f56c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f56e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5700(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5710(WinApplication* app, x86::CPU& cpu);
    static void sub_4f57c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5850(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5880(WinApplication* app, x86::CPU& cpu);
    static void sub_4f58c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5900(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5950(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5a30(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5a80(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5ac0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5b10(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5ba0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5bf0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5c10(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5ea0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5f50(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5f70(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5f90(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5fa0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5fc0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5fd0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f5fe0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f6010(WinApplication* app, x86::CPU& cpu);
    static void sub_4f6030(WinApplication* app, x86::CPU& cpu);
    static void sub_4f60b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f61f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f6350(WinApplication* app, x86::CPU& cpu);
    static void sub_4f63d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f6490(WinApplication* app, x86::CPU& cpu);
    static void sub_4f6510(WinApplication* app, x86::CPU& cpu);
    static void sub_4f6570(WinApplication* app, x86::CPU& cpu);
    static void sub_4f6640(WinApplication* app, x86::CPU& cpu);
    static void sub_4f67b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f6860(WinApplication* app, x86::CPU& cpu);
    static void sub_4f6870(WinApplication* app, x86::CPU& cpu);
    static void sub_4f68c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f6c50(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7030(WinApplication* app, x86::CPU& cpu);
    static void sub_4f71a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7220(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7280(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7320(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7370(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7380(WinApplication* app, x86::CPU& cpu);
    static void sub_4f73a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7420(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7450(WinApplication* app, x86::CPU& cpu);
    static void sub_4f75d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7660(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7670(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7750(WinApplication* app, x86::CPU& cpu);
    static void sub_4f77c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f77f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7830(WinApplication* app, x86::CPU& cpu);
    static void sub_4f7c48(WinApplication* app, x86::CPU& cpu);
    static void sub_4f80e5(WinApplication* app, x86::CPU& cpu);
    static void sub_4f80b6(WinApplication* app, x86::CPU& cpu);
    static void sub_4f87a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f87e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f8930(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9060(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9088(WinApplication* app, x86::CPU& cpu);
    static void sub_4f90e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9120(WinApplication* app, x86::CPU& cpu);
    static void sub_4f91d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9260(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9270(WinApplication* app, x86::CPU& cpu);
    static void sub_4f92b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f92d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9320(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9360(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9390(WinApplication* app, x86::CPU& cpu);
    static void sub_4f93f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9410(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9500(WinApplication* app, x86::CPU& cpu);
    static void sub_4f97c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9920(WinApplication* app, x86::CPU& cpu);
    static void sub_4f99f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9a50(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9ac0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9d20(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9df0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9ea0(WinApplication* app, x86::CPU& cpu);
    static void sub_4f9f30(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa030(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa0f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa1d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa210(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa310(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa450(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa480(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa4e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa4f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa500(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa530(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa550(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa5f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa620(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa6d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa6e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa710(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa720(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa730(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa760(WinApplication* app, x86::CPU& cpu);
    static void sub_4fa780(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb060(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb380(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb3b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb3c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb3d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb440(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb536(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb527(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb51b(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb50b(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb4ea(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb4d0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb4c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb470(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb6a0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb6a7(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb6ae(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb6b5(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb6c2(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb6d7(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb700(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb710(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb790(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb820(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb82b(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb830(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb860(WinApplication* app, x86::CPU& cpu);
    static void sub_4fb990(WinApplication* app, x86::CPU& cpu);
    static void sub_4fbb60(WinApplication* app, x86::CPU& cpu);
    static void sub_4fbbe0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fbc30(WinApplication* app, x86::CPU& cpu);
    static void sub_4fbd10(WinApplication* app, x86::CPU& cpu);
    static void sub_4fbda0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fc170(WinApplication* app, x86::CPU& cpu);
    static void sub_4fc250(WinApplication* app, x86::CPU& cpu);
    static void sub_4fc340(WinApplication* app, x86::CPU& cpu);
    static void sub_4fc600(WinApplication* app, x86::CPU& cpu);
    static void sub_4fc780(WinApplication* app, x86::CPU& cpu);
    static void sub_4fc8e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fc9b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fca60(WinApplication* app, x86::CPU& cpu);
    static void sub_4fca90(WinApplication* app, x86::CPU& cpu);
    static void sub_4fcab0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fce50(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd060(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd1c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd280(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd2b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd3f0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd490(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd4b0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd4c0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd4e0(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd500(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd520(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd540(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd61c(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd6ef(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd7b6(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd892(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd8ee(WinApplication* app, x86::CPU& cpu);
    static void sub_4fd98c(WinApplication* app, x86::CPU& cpu);
    static void sub_4fda20(WinApplication* app, x86::CPU& cpu);
    static void sub_4fdc90(WinApplication* app, x86::CPU& cpu);
    static void sub_4feffc(WinApplication* app, x86::CPU& cpu);
    static void sub_4ff0f8(WinApplication* app, x86::CPU& cpu);
    static void sub_4ff100(WinApplication* app, x86::CPU& cpu);
    static void sub_4ff4f2(WinApplication* app, x86::CPU& cpu);
    static void sub_4ff570(WinApplication* app, x86::CPU& cpu);
    static void sub_4ff590(WinApplication* app, x86::CPU& cpu);
    static void sub_4ff598(WinApplication* app, x86::CPU& cpu);
    static void sub_4ff59c(WinApplication* app, x86::CPU& cpu);
    static void sub_4ff5dc(WinApplication* app, x86::CPU& cpu);
    static void sub_4ff620(WinApplication* app, x86::CPU& cpu);
    static void sub_503b00(WinApplication* app, x86::CPU& cpu);
    static void sub_503bc0(WinApplication* app, x86::CPU& cpu);
    static void sub_503cd0(WinApplication* app, x86::CPU& cpu);
    static void sub_503e50(WinApplication* app, x86::CPU& cpu);
    static void sub_503ec0(WinApplication* app, x86::CPU& cpu);
    static void sub_5040b8(WinApplication* app, x86::CPU& cpu);
    static void sub_504133(WinApplication* app, x86::CPU& cpu);
    static void sub_5041ae(WinApplication* app, x86::CPU& cpu);
    static void sub_5041d0(WinApplication* app, x86::CPU& cpu);
    static void sub_50425d(WinApplication* app, x86::CPU& cpu);
    static void sub_5042c4(WinApplication* app, x86::CPU& cpu);
    static void sub_504335(WinApplication* app, x86::CPU& cpu);
    static void sub_5043f0(WinApplication* app, x86::CPU& cpu);
    static void sub_5049c0(WinApplication* app, x86::CPU& cpu);
    static void sub_5049f0(WinApplication* app, x86::CPU& cpu);
    static void sub_504a30(WinApplication* app, x86::CPU& cpu);
    static void sub_504af0(WinApplication* app, x86::CPU& cpu);
    static void sub_504b70(WinApplication* app, x86::CPU& cpu);
    static void sub_504bf0(WinApplication* app, x86::CPU& cpu);
    static void sub_504c00(WinApplication* app, x86::CPU& cpu);
    static void sub_504c20(WinApplication* app, x86::CPU& cpu);
    static void sub_504c40(WinApplication* app, x86::CPU& cpu);
    static void sub_504c80(WinApplication* app, x86::CPU& cpu);
    static void sub_504e00(WinApplication* app, x86::CPU& cpu);
    static void sub_504f90(WinApplication* app, x86::CPU& cpu);
    static void sub_5050b0(WinApplication* app, x86::CPU& cpu);
    static void sub_5050e0(WinApplication* app, x86::CPU& cpu);
    static void sub_5050f0(WinApplication* app, x86::CPU& cpu);
    static void sub_5051c0(WinApplication* app, x86::CPU& cpu);
    static void sub_505250(WinApplication* app, x86::CPU& cpu);
    static void sub_505280(WinApplication* app, x86::CPU& cpu);
    static void sub_5052a0(WinApplication* app, x86::CPU& cpu);
    static void sub_505320(WinApplication* app, x86::CPU& cpu);
    static void sub_505500(WinApplication* app, x86::CPU& cpu);
    static void sub_505570(WinApplication* app, x86::CPU& cpu);
    static void sub_505650(WinApplication* app, x86::CPU& cpu);
    static void sub_5057d0(WinApplication* app, x86::CPU& cpu);
    static void sub_505830(WinApplication* app, x86::CPU& cpu);
    static void sub_505870(WinApplication* app, x86::CPU& cpu);
    static void sub_505920(WinApplication* app, x86::CPU& cpu);
    static void sub_505950(WinApplication* app, x86::CPU& cpu);
    static void sub_505970(WinApplication* app, x86::CPU& cpu);
    static void sub_505980(WinApplication* app, x86::CPU& cpu);
    static void sub_5059d0(WinApplication* app, x86::CPU& cpu);
    static void sub_505a50(WinApplication* app, x86::CPU& cpu);
    static void sub_505a80(WinApplication* app, x86::CPU& cpu);
    static void sub_505b30(WinApplication* app, x86::CPU& cpu);
    static void sub_506230(WinApplication* app, x86::CPU& cpu);
    static void sub_506310(WinApplication* app, x86::CPU& cpu);
    static void sub_506360(WinApplication* app, x86::CPU& cpu);
    static void sub_5063d0(WinApplication* app, x86::CPU& cpu);
    static void sub_506440(WinApplication* app, x86::CPU& cpu);
    static void sub_506690(WinApplication* app, x86::CPU& cpu);
    static void sub_506910(WinApplication* app, x86::CPU& cpu);
    static void sub_506ac0(WinApplication* app, x86::CPU& cpu);
    static void sub_506ca0(WinApplication* app, x86::CPU& cpu);
    static void sub_507240(WinApplication* app, x86::CPU& cpu);
    static void sub_5074b0(WinApplication* app, x86::CPU& cpu);
    static void sub_5075d0(WinApplication* app, x86::CPU& cpu);
    static void sub_507720(WinApplication* app, x86::CPU& cpu);
    static void sub_507900(WinApplication* app, x86::CPU& cpu);
    static void sub_5079a0(WinApplication* app, x86::CPU& cpu);
    static void sub_507a60(WinApplication* app, x86::CPU& cpu);
    static void sub_507b10(WinApplication* app, x86::CPU& cpu);
    static void sub_507bc0(WinApplication* app, x86::CPU& cpu);
    static void sub_507d00(WinApplication* app, x86::CPU& cpu);
    static void sub_507ed0(WinApplication* app, x86::CPU& cpu);
    static void sub_507f30(WinApplication* app, x86::CPU& cpu);
    static void sub_508170(WinApplication* app, x86::CPU& cpu);
    static void sub_5082f0(WinApplication* app, x86::CPU& cpu);
    static void sub_508390(WinApplication* app, x86::CPU& cpu);
    static void sub_5084e0(WinApplication* app, x86::CPU& cpu);
    static void sub_508510(WinApplication* app, x86::CPU& cpu);
    static void sub_508970(WinApplication* app, x86::CPU& cpu);
    static void sub_5089d0(WinApplication* app, x86::CPU& cpu);
    static void sub_508b80(WinApplication* app, x86::CPU& cpu);
    static void sub_508c70(WinApplication* app, x86::CPU& cpu);
    static void sub_508da0(WinApplication* app, x86::CPU& cpu);
    static void sub_508e70(WinApplication* app, x86::CPU& cpu);
    static void sub_509350(WinApplication* app, x86::CPU& cpu);
    static void sub_5098d0(WinApplication* app, x86::CPU& cpu);
    static void sub_509940(WinApplication* app, x86::CPU& cpu);
    static void sub_509a00(WinApplication* app, x86::CPU& cpu);
    static void sub_509a30(WinApplication* app, x86::CPU& cpu);
    static void sub_509b10(WinApplication* app, x86::CPU& cpu);
    static void sub_509c00(WinApplication* app, x86::CPU& cpu);
    static void sub_509c50(WinApplication* app, x86::CPU& cpu);
    static void sub_509c60(WinApplication* app, x86::CPU& cpu);
    static void sub_509c70(WinApplication* app, x86::CPU& cpu);
    static void sub_509cd0(WinApplication* app, x86::CPU& cpu);
    static void sub_509d20(WinApplication* app, x86::CPU& cpu);
    static void sub_509df0(WinApplication* app, x86::CPU& cpu);
    static void sub_509e50(WinApplication* app, x86::CPU& cpu);
    static void sub_509eb0(WinApplication* app, x86::CPU& cpu);
    static void sub_509ef0(WinApplication* app, x86::CPU& cpu);
    static void sub_509f50(WinApplication* app, x86::CPU& cpu);
    static void sub_509fb0(WinApplication* app, x86::CPU& cpu);
    static void sub_50a020(WinApplication* app, x86::CPU& cpu);
    static void sub_50a0a0(WinApplication* app, x86::CPU& cpu);
    static void sub_50a100(WinApplication* app, x86::CPU& cpu);
    static void sub_50a150(WinApplication* app, x86::CPU& cpu);
    static void sub_50a1c0(WinApplication* app, x86::CPU& cpu);
    static void sub_50a1e0(WinApplication* app, x86::CPU& cpu);
    static void sub_50a240(WinApplication* app, x86::CPU& cpu);
    static void sub_50a2b0(WinApplication* app, x86::CPU& cpu);
    static void sub_50a330(WinApplication* app, x86::CPU& cpu);
    static void sub_50a3c0(WinApplication* app, x86::CPU& cpu);
    static void sub_50a440(WinApplication* app, x86::CPU& cpu);
    static void sub_50a4c0(WinApplication* app, x86::CPU& cpu);
    static void sub_50a510(WinApplication* app, x86::CPU& cpu);
    static void sub_50a520(WinApplication* app, x86::CPU& cpu);
    static void sub_50a559(WinApplication* app, x86::CPU& cpu);
    static void sub_50a550(WinApplication* app, x86::CPU& cpu);
    static void sub_50a543(WinApplication* app, x86::CPU& cpu);
    static void sub_50a570(WinApplication* app, x86::CPU& cpu);
    static void sub_50a5d0(WinApplication* app, x86::CPU& cpu);
    static void sub_50a830(WinApplication* app, x86::CPU& cpu);
    static void sub_50a960(WinApplication* app, x86::CPU& cpu);
    static void sub_50aad0(WinApplication* app, x86::CPU& cpu);
    static void sub_50afc0(WinApplication* app, x86::CPU& cpu);
    static void sub_50b080(WinApplication* app, x86::CPU& cpu);
    static void sub_50b0b0(WinApplication* app, x86::CPU& cpu);
    static void sub_50b540(WinApplication* app, x86::CPU& cpu);
    static void sub_50b560(WinApplication* app, x86::CPU& cpu);
    static void sub_50b580(WinApplication* app, x86::CPU& cpu);
    static void sub_50b5c0(WinApplication* app, x86::CPU& cpu);
    static void sub_50ba90(WinApplication* app, x86::CPU& cpu);
    static void sub_50bb80(WinApplication* app, x86::CPU& cpu);
    static void sub_50bdf0(WinApplication* app, x86::CPU& cpu);
    static void sub_50bfd0(WinApplication* app, x86::CPU& cpu);
    static void sub_50c0d0(WinApplication* app, x86::CPU& cpu);
    static void sub_50c180(WinApplication* app, x86::CPU& cpu);
    static void sub_50c1e0(WinApplication* app, x86::CPU& cpu);
    static void sub_50c220(WinApplication* app, x86::CPU& cpu);
    static void sub_50c270(WinApplication* app, x86::CPU& cpu);
    static void sub_50c2c0(WinApplication* app, x86::CPU& cpu);
    static void sub_50c350(WinApplication* app, x86::CPU& cpu);
    static void sub_50c3a0(WinApplication* app, x86::CPU& cpu);
    static void sub_50c584(WinApplication* app, x86::CPU& cpu);
    static void sub_50c5d4(WinApplication* app, x86::CPU& cpu);
    static void sub_50d560(WinApplication* app, x86::CPU& cpu);
    static void sub_50d6d0(WinApplication* app, x86::CPU& cpu);
    static void sub_50d6e0(WinApplication* app, x86::CPU& cpu);
    static void sub_50d7e0(WinApplication* app, x86::CPU& cpu);
    static void sub_50d880(WinApplication* app, x86::CPU& cpu);
    static void sub_50d910(WinApplication* app, x86::CPU& cpu);
    static void sub_50d9c0(WinApplication* app, x86::CPU& cpu);
    static void sub_50daa0(WinApplication* app, x86::CPU& cpu);
    static void sub_50db30(WinApplication* app, x86::CPU& cpu);
    static void sub_50dbf0(WinApplication* app, x86::CPU& cpu);
    static void sub_50dcd0(WinApplication* app, x86::CPU& cpu);
    static void sub_50dd60(WinApplication* app, x86::CPU& cpu);
    static void sub_50de50(WinApplication* app, x86::CPU& cpu);
    static void sub_50ded0(WinApplication* app, x86::CPU& cpu);
    static void sub_50df60(WinApplication* app, x86::CPU& cpu);
    static void sub_50dff0(WinApplication* app, x86::CPU& cpu);
    static void sub_50e0a0(WinApplication* app, x86::CPU& cpu);
    static void sub_50e2d0(WinApplication* app, x86::CPU& cpu);
    static void sub_50e3f0(WinApplication* app, x86::CPU& cpu);
    static void sub_50e500(WinApplication* app, x86::CPU& cpu);
    static void sub_50e5a0(WinApplication* app, x86::CPU& cpu);
    static void sub_50e800(WinApplication* app, x86::CPU& cpu);
    static void sub_50e8e0(WinApplication* app, x86::CPU& cpu);
    static void sub_50e9b0(WinApplication* app, x86::CPU& cpu);
    static void sub_50ea20(WinApplication* app, x86::CPU& cpu);
    static void sub_50ea40(WinApplication* app, x86::CPU& cpu);
    static void sub_5102a4(WinApplication* app, x86::CPU& cpu);
    static void sub_5102f0(WinApplication* app, x86::CPU& cpu);
    static void sub_51039e(WinApplication* app, x86::CPU& cpu);
    static void sub_510337(WinApplication* app, x86::CPU& cpu);
    static void sub_510a50(WinApplication* app, x86::CPU& cpu);
    static void sub_510bb3(WinApplication* app, x86::CPU& cpu);
    static void sub_510cbd(WinApplication* app, x86::CPU& cpu);
    static void sub_510e48(WinApplication* app, x86::CPU& cpu);
    static void sub_510ff8(WinApplication* app, x86::CPU& cpu);
    static void sub_512180(WinApplication* app, x86::CPU& cpu);
    static void sub_512470(WinApplication* app, x86::CPU& cpu);
    static void sub_5125b0(WinApplication* app, x86::CPU& cpu);
    static void sub_512650(WinApplication* app, x86::CPU& cpu);
    static void sub_512660(WinApplication* app, x86::CPU& cpu);
    static void sub_5126f0(WinApplication* app, x86::CPU& cpu);
    static void sub_5127d0(WinApplication* app, x86::CPU& cpu);
    static void sub_512890(WinApplication* app, x86::CPU& cpu);
    static void sub_512940(WinApplication* app, x86::CPU& cpu);
    static void sub_512980(WinApplication* app, x86::CPU& cpu);
    static void sub_5129e0(WinApplication* app, x86::CPU& cpu);
    static void sub_512ad0(WinApplication* app, x86::CPU& cpu);
    static void sub_512c80(WinApplication* app, x86::CPU& cpu);
    static void sub_512d70(WinApplication* app, x86::CPU& cpu);
    static void sub_512dd0(WinApplication* app, x86::CPU& cpu);
    static void sub_512f60(WinApplication* app, x86::CPU& cpu);
    static void sub_512fc0(WinApplication* app, x86::CPU& cpu);
    static void sub_512fd0(WinApplication* app, x86::CPU& cpu);
    static void sub_512ff0(WinApplication* app, x86::CPU& cpu);
    static void sub_513000(WinApplication* app, x86::CPU& cpu);
    static void sub_513030(WinApplication* app, x86::CPU& cpu);
    static void sub_513190(WinApplication* app, x86::CPU& cpu);
    static void sub_5132a0(WinApplication* app, x86::CPU& cpu);
    static void sub_513320(WinApplication* app, x86::CPU& cpu);
    static void sub_5133a0(WinApplication* app, x86::CPU& cpu);
    static void sub_5140e0(WinApplication* app, x86::CPU& cpu);
    static void sub_514260(WinApplication* app, x86::CPU& cpu);
    static void sub_514280(WinApplication* app, x86::CPU& cpu);
    static void sub_514320(WinApplication* app, x86::CPU& cpu);
    static void sub_5143d0(WinApplication* app, x86::CPU& cpu);
    static void sub_5144b0(WinApplication* app, x86::CPU& cpu);
    static void sub_5145b0(WinApplication* app, x86::CPU& cpu);
    static void sub_514650(WinApplication* app, x86::CPU& cpu);
    static void sub_514820(WinApplication* app, x86::CPU& cpu);
    static void sub_514850(WinApplication* app, x86::CPU& cpu);
    static void sub_5148b0(WinApplication* app, x86::CPU& cpu);
    static void sub_514970(WinApplication* app, x86::CPU& cpu);
    static void sub_514af0(WinApplication* app, x86::CPU& cpu);
    static void sub_514dc0(WinApplication* app, x86::CPU& cpu);
    static void sub_514ec0(WinApplication* app, x86::CPU& cpu);
    static void sub_514f70(WinApplication* app, x86::CPU& cpu);
    static void sub_515050(WinApplication* app, x86::CPU& cpu);
    static void sub_5150f0(WinApplication* app, x86::CPU& cpu);
    static void sub_515740(WinApplication* app, x86::CPU& cpu);
    static void sub_5157e0(WinApplication* app, x86::CPU& cpu);
    static void sub_5157ec(WinApplication* app, x86::CPU& cpu);
    static void sub_5157f4(WinApplication* app, x86::CPU& cpu);
    static void sub_515900(WinApplication* app, x86::CPU& cpu);
    static void sub_5159e4(WinApplication* app, x86::CPU& cpu);
    static void sub_5167b3(WinApplication* app, x86::CPU& cpu);
    static void sub_5169ef(WinApplication* app, x86::CPU& cpu);
    static void sub_516a87(WinApplication* app, x86::CPU& cpu);
    static void sub_516ab4(WinApplication* app, x86::CPU& cpu);
    static void sub_516a9d(WinApplication* app, x86::CPU& cpu);
    static void sub_516acc(WinApplication* app, x86::CPU& cpu);
    static void sub_516ae2(WinApplication* app, x86::CPU& cpu);
    static void sub_516be2(WinApplication* app, x86::CPU& cpu);
    static void sub_516bf5(WinApplication* app, x86::CPU& cpu);
    static void sub_516dfa(WinApplication* app, x86::CPU& cpu);
    static void sub_516e13(WinApplication* app, x86::CPU& cpu);
    static void sub_517580(WinApplication* app, x86::CPU& cpu);
    static void sub_5175b0(WinApplication* app, x86::CPU& cpu);
    static void sub_517920(WinApplication* app, x86::CPU& cpu);
    static void sub_517b30(WinApplication* app, x86::CPU& cpu);
    static void sub_517b70(WinApplication* app, x86::CPU& cpu);
    static void sub_517ba0(WinApplication* app, x86::CPU& cpu);
    static void sub_517bf0(WinApplication* app, x86::CPU& cpu);
    static void sub_517d60(WinApplication* app, x86::CPU& cpu);
    static void sub_517d70(WinApplication* app, x86::CPU& cpu);
    static void sub_517da0(WinApplication* app, x86::CPU& cpu);
    static void sub_517de0(WinApplication* app, x86::CPU& cpu);
    static void sub_517e50(WinApplication* app, x86::CPU& cpu);
    static void sub_517eb0(WinApplication* app, x86::CPU& cpu);
    static void sub_517f10(WinApplication* app, x86::CPU& cpu);
    static void sub_518040(WinApplication* app, x86::CPU& cpu);
    static void sub_518690(WinApplication* app, x86::CPU& cpu);
    static void sub_518870(WinApplication* app, x86::CPU& cpu);
    static void sub_518d50(WinApplication* app, x86::CPU& cpu);
    static void sub_518df0(WinApplication* app, x86::CPU& cpu);
    static void sub_518e40(WinApplication* app, x86::CPU& cpu);
    static void sub_518eb0(WinApplication* app, x86::CPU& cpu);
    static void sub_518ee0(WinApplication* app, x86::CPU& cpu);
    static void sub_518ef0(WinApplication* app, x86::CPU& cpu);
    static void sub_518f80(WinApplication* app, x86::CPU& cpu);
    static void sub_518fc0(WinApplication* app, x86::CPU& cpu);
    static void sub_518fd0(WinApplication* app, x86::CPU& cpu);
    static void sub_519140(WinApplication* app, x86::CPU& cpu);
    static void sub_5199b0(WinApplication* app, x86::CPU& cpu);
    static void sub_519a50(WinApplication* app, x86::CPU& cpu);
    static void sub_519ea0(WinApplication* app, x86::CPU& cpu);
    static void sub_519f10(WinApplication* app, x86::CPU& cpu);
    static void sub_51a170(WinApplication* app, x86::CPU& cpu);
    static void sub_51a1b8(WinApplication* app, x86::CPU& cpu);
    static void sub_51a470(WinApplication* app, x86::CPU& cpu);
    static void sub_51a7b0(WinApplication* app, x86::CPU& cpu);
    static void sub_51a930(WinApplication* app, x86::CPU& cpu);
    static void sub_51ab40(WinApplication* app, x86::CPU& cpu);
    static void sub_51ab70(WinApplication* app, x86::CPU& cpu);
    static void sub_51abc0(WinApplication* app, x86::CPU& cpu);
    static void sub_51ad70(WinApplication* app, x86::CPU& cpu);
    static void sub_51aeb0(WinApplication* app, x86::CPU& cpu);
    static void sub_51af10(WinApplication* app, x86::CPU& cpu);
    static void sub_51b030(WinApplication* app, x86::CPU& cpu);
    static void sub_51b1c0(WinApplication* app, x86::CPU& cpu);
    static void sub_51b1e0(WinApplication* app, x86::CPU& cpu);
    static void sub_51b200(WinApplication* app, x86::CPU& cpu);
    static void sub_51b2f0(WinApplication* app, x86::CPU& cpu);
    static void sub_51b480(WinApplication* app, x86::CPU& cpu);
    static void sub_51b530(WinApplication* app, x86::CPU& cpu);
    static void sub_51b5f0(WinApplication* app, x86::CPU& cpu);
    static void sub_51b710(WinApplication* app, x86::CPU& cpu);
    static void sub_51b8a0(WinApplication* app, x86::CPU& cpu);
    static void sub_51b900(WinApplication* app, x86::CPU& cpu);
    static void sub_51b970(WinApplication* app, x86::CPU& cpu);
    static void sub_51ba50(WinApplication* app, x86::CPU& cpu);
    static void sub_51bbd0(WinApplication* app, x86::CPU& cpu);
    static void sub_51bc70(WinApplication* app, x86::CPU& cpu);
    static void sub_51bca0(WinApplication* app, x86::CPU& cpu);
    static void sub_51bcd0(WinApplication* app, x86::CPU& cpu);
    static void sub_51bcf0(WinApplication* app, x86::CPU& cpu);
    static void sub_51bd50(WinApplication* app, x86::CPU& cpu);
    static void sub_51bdc0(WinApplication* app, x86::CPU& cpu);
    static void sub_51be90(WinApplication* app, x86::CPU& cpu);
    static void sub_51bfd0(WinApplication* app, x86::CPU& cpu);
    static void sub_51c0a0(WinApplication* app, x86::CPU& cpu);
    static void sub_51c100(WinApplication* app, x86::CPU& cpu);
    static void sub_51c510(WinApplication* app, x86::CPU& cpu);
    static void sub_51c5c0(WinApplication* app, x86::CPU& cpu);
    static void sub_51c690(WinApplication* app, x86::CPU& cpu);
    static void sub_51c7a0(WinApplication* app, x86::CPU& cpu);
    static void sub_51c7c0(WinApplication* app, x86::CPU& cpu);
    static void sub_51c7f0(WinApplication* app, x86::CPU& cpu);
    static void sub_51c820(WinApplication* app, x86::CPU& cpu);
    static void sub_51c890(WinApplication* app, x86::CPU& cpu);
    static void sub_51c8f0(WinApplication* app, x86::CPU& cpu);
    static void sub_51ca50(WinApplication* app, x86::CPU& cpu);
    static void sub_51caf0(WinApplication* app, x86::CPU& cpu);
    static void sub_51cb50(WinApplication* app, x86::CPU& cpu);
    static void sub_51cc00(WinApplication* app, x86::CPU& cpu);
    static void sub_51cc80(WinApplication* app, x86::CPU& cpu);
    static void sub_51cd50(WinApplication* app, x86::CPU& cpu);
    static void sub_51cf30(WinApplication* app, x86::CPU& cpu);
    static void sub_51d000(WinApplication* app, x86::CPU& cpu);
    static void sub_51d010(WinApplication* app, x86::CPU& cpu);
    static void sub_51d040(WinApplication* app, x86::CPU& cpu);
    static void sub_51d050(WinApplication* app, x86::CPU& cpu);
    static void sub_51d270(WinApplication* app, x86::CPU& cpu);
    static void sub_51d810(WinApplication* app, x86::CPU& cpu);
    static void sub_51d8f0(WinApplication* app, x86::CPU& cpu);
    static void sub_51d9e0(WinApplication* app, x86::CPU& cpu);
    static void sub_51de60(WinApplication* app, x86::CPU& cpu);
    static void sub_51df60(WinApplication* app, x86::CPU& cpu);
    static void sub_51e110(WinApplication* app, x86::CPU& cpu);
    static void sub_51e2a0(WinApplication* app, x86::CPU& cpu);
    static void sub_51e388(WinApplication* app, x86::CPU& cpu);
    static void sub_51e51c(WinApplication* app, x86::CPU& cpu);
    static void sub_51e540(WinApplication* app, x86::CPU& cpu);
    static void sub_51e544(WinApplication* app, x86::CPU& cpu);
    static void sub_51e5e8(WinApplication* app, x86::CPU& cpu);
    static void sub_51e618(WinApplication* app, x86::CPU& cpu);
    static void sub_51e670(WinApplication* app, x86::CPU& cpu);
    static void sub_51e680(WinApplication* app, x86::CPU& cpu);
    static void sub_51e68c(WinApplication* app, x86::CPU& cpu);
    static void sub_51e698(WinApplication* app, x86::CPU& cpu);
    static void sub_51e6a8(WinApplication* app, x86::CPU& cpu);
    static void sub_51e6b8(WinApplication* app, x86::CPU& cpu);
    static void sub_51e6c0(WinApplication* app, x86::CPU& cpu);
    static void sub_51e6dc(WinApplication* app, x86::CPU& cpu);
    static void sub_51e6e4(WinApplication* app, x86::CPU& cpu);
    static void sub_51e6f0(WinApplication* app, x86::CPU& cpu);
    static void sub_51e6f8(WinApplication* app, x86::CPU& cpu);
    static void sub_51e704(WinApplication* app, x86::CPU& cpu);
    static void sub_51e70c(WinApplication* app, x86::CPU& cpu);
    static void sub_51e714(WinApplication* app, x86::CPU& cpu);
    static void sub_51e71c(WinApplication* app, x86::CPU& cpu);
    static void sub_51e778(WinApplication* app, x86::CPU& cpu);
    static void sub_51e780(WinApplication* app, x86::CPU& cpu);
    static void sub_51e7a8(WinApplication* app, x86::CPU& cpu);
    static void sub_51e828(WinApplication* app, x86::CPU& cpu);
    static void sub_51e888(WinApplication* app, x86::CPU& cpu);
    static void sub_51e8dc(WinApplication* app, x86::CPU& cpu);
    static void sub_51e934(WinApplication* app, x86::CPU& cpu);
    static void sub_51e940(WinApplication* app, x86::CPU& cpu);
    static void sub_51e964(WinApplication* app, x86::CPU& cpu);
    static void sub_51ea7c(WinApplication* app, x86::CPU& cpu);
    static void sub_51ec90(WinApplication* app, x86::CPU& cpu);
    static void sub_51eccc(WinApplication* app, x86::CPU& cpu);
    static void sub_51ed24(WinApplication* app, x86::CPU& cpu);
    static void sub_51ef08(WinApplication* app, x86::CPU& cpu);
    static void sub_51f140(WinApplication* app, x86::CPU& cpu);
    static void sub_51f254(WinApplication* app, x86::CPU& cpu);
    static void sub_51f270(WinApplication* app, x86::CPU& cpu);
    static void sub_51f2a0(WinApplication* app, x86::CPU& cpu);
    static void sub_523332(WinApplication* app, x86::CPU& cpu);
    static void sub_523465(WinApplication* app, x86::CPU& cpu);
    static void sub_5236c0(WinApplication* app, x86::CPU& cpu);
    static void sub_523760(WinApplication* app, x86::CPU& cpu);
    static void sub_5239ec(WinApplication* app, x86::CPU& cpu);
    static void sub_523b00(WinApplication* app, x86::CPU& cpu);
    static void sub_52448c(WinApplication* app, x86::CPU& cpu);
    static void sub_5244f0(WinApplication* app, x86::CPU& cpu);
    static void sub_524500(WinApplication* app, x86::CPU& cpu);
    static void sub_524550(WinApplication* app, x86::CPU& cpu);
    static void sub_5245a0(WinApplication* app, x86::CPU& cpu);
    static void sub_5245f0(WinApplication* app, x86::CPU& cpu);
    static void sub_524620(WinApplication* app, x86::CPU& cpu);
    static void sub_52468f(WinApplication* app, x86::CPU& cpu);
    static void sub_5247ab(WinApplication* app, x86::CPU& cpu);
    static void sub_5247c0(WinApplication* app, x86::CPU& cpu);
    static void sub_524930(WinApplication* app, x86::CPU& cpu);
    static void sub_524950(WinApplication* app, x86::CPU& cpu);
    static void sub_524970(WinApplication* app, x86::CPU& cpu);
    static void sub_524bb0(WinApplication* app, x86::CPU& cpu);
    static void sub_524c30(WinApplication* app, x86::CPU& cpu);
    static void sub_524d90(WinApplication* app, x86::CPU& cpu);
    static void sub_524dd0(WinApplication* app, x86::CPU& cpu);
    static void sub_524de0(WinApplication* app, x86::CPU& cpu);
    static void sub_524df0(WinApplication* app, x86::CPU& cpu);
    static void sub_524f88(WinApplication* app, x86::CPU& cpu);
    static void sub_52506c(WinApplication* app, x86::CPU& cpu);
    static void sub_5251a0(WinApplication* app, x86::CPU& cpu);
    static void sub_525600(WinApplication* app, x86::CPU& cpu);
    static void sub_525630(WinApplication* app, x86::CPU& cpu);
    static void sub_525650(WinApplication* app, x86::CPU& cpu);
    static void sub_525670(WinApplication* app, x86::CPU& cpu);
    static void sub_5257c0(WinApplication* app, x86::CPU& cpu);
    static void sub_5259d0(WinApplication* app, x86::CPU& cpu);
    static void sub_525b80(WinApplication* app, x86::CPU& cpu);
    static void sub_525ba0(WinApplication* app, x86::CPU& cpu);
    static void sub_525e10(WinApplication* app, x86::CPU& cpu);
    static void sub_525e4c(WinApplication* app, x86::CPU& cpu);
    static void sub_525f20(WinApplication* app, x86::CPU& cpu);
    static void sub_525f84(WinApplication* app, x86::CPU& cpu);
    static void sub_525ff8(WinApplication* app, x86::CPU& cpu);
    static void sub_526060(WinApplication* app, x86::CPU& cpu);
    static void sub_526070(WinApplication* app, x86::CPU& cpu);
    static void sub_526080(WinApplication* app, x86::CPU& cpu);
    static void sub_5260c8(WinApplication* app, x86::CPU& cpu);
    static void sub_5260ec(WinApplication* app, x86::CPU& cpu);
    static void sub_526110(WinApplication* app, x86::CPU& cpu);
    static void sub_52612c(WinApplication* app, x86::CPU& cpu);
    static void sub_526194(WinApplication* app, x86::CPU& cpu);
    static void sub_5261fc(WinApplication* app, x86::CPU& cpu);
    static void sub_526244(WinApplication* app, x86::CPU& cpu);
    static void sub_526338(WinApplication* app, x86::CPU& cpu);
    static void sub_5263a8(WinApplication* app, x86::CPU& cpu);
    static void sub_5263f0(WinApplication* app, x86::CPU& cpu);
    static void sub_526420(WinApplication* app, x86::CPU& cpu);
    static void sub_52643c(WinApplication* app, x86::CPU& cpu);
    static void sub_526642(WinApplication* app, x86::CPU& cpu);
    static void sub_526674(WinApplication* app, x86::CPU& cpu);
    static void sub_5267ff(WinApplication* app, x86::CPU& cpu);
    static void sub_52699c(WinApplication* app, x86::CPU& cpu);
    static void sub_526f87(WinApplication* app, x86::CPU& cpu);
    static void sub_52700c(WinApplication* app, x86::CPU& cpu);
    static void sub_52b523(WinApplication* app, x86::CPU& cpu);
    static void sub_52b6c0(WinApplication* app, x86::CPU& cpu);
    static void sub_52b720(WinApplication* app, x86::CPU& cpu);
    static void sub_52b860(WinApplication* app, x86::CPU& cpu);
    static void sub_52b8b0(WinApplication* app, x86::CPU& cpu);
    static void sub_52b8d0(WinApplication* app, x86::CPU& cpu);
    static void sub_52b8e0(WinApplication* app, x86::CPU& cpu);
    static void sub_52b8f0(WinApplication* app, x86::CPU& cpu);
    static void sub_52b910(WinApplication* app, x86::CPU& cpu);
    static void sub_52b970(WinApplication* app, x86::CPU& cpu);
    static void sub_52b9d0(WinApplication* app, x86::CPU& cpu);
    static void sub_52b9e0(WinApplication* app, x86::CPU& cpu);
    static void sub_52ba00(WinApplication* app, x86::CPU& cpu);
    static void sub_52ba70(WinApplication* app, x86::CPU& cpu);
    static void sub_52bac0(WinApplication* app, x86::CPU& cpu);
    static void sub_52baf0(WinApplication* app, x86::CPU& cpu);
    static void sub_52bb20(WinApplication* app, x86::CPU& cpu);
    static void sub_52bb50(WinApplication* app, x86::CPU& cpu);
    static void sub_52bb80(WinApplication* app, x86::CPU& cpu);
    static void sub_52bb90(WinApplication* app, x86::CPU& cpu);
    static void sub_52bbf0(WinApplication* app, x86::CPU& cpu);
    static void sub_52bc00(WinApplication* app, x86::CPU& cpu);
    static void sub_52bd00(WinApplication* app, x86::CPU& cpu);
    static void sub_52bd20(WinApplication* app, x86::CPU& cpu);
    static void sub_52bd50(WinApplication* app, x86::CPU& cpu);
    static void sub_52bdb0(WinApplication* app, x86::CPU& cpu);
    static void sub_52bed0(WinApplication* app, x86::CPU& cpu);
    static void sub_52c810(WinApplication* app, x86::CPU& cpu);
    static void sub_52c81c(WinApplication* app, x86::CPU& cpu);
    static void sub_52c824(WinApplication* app, x86::CPU& cpu);
    static void sub_52c870(WinApplication* app, x86::CPU& cpu);
    static void sub_52c878(WinApplication* app, x86::CPU& cpu);
    static void sub_52c88a(WinApplication* app, x86::CPU& cpu);
    static void sub_52c88f(WinApplication* app, x86::CPU& cpu);
    static void sub_52c8a7(WinApplication* app, x86::CPU& cpu);
    static void sub_52c89b(WinApplication* app, x86::CPU& cpu);
    static void sub_52e3f8(WinApplication* app, x86::CPU& cpu);
    static void sub_52e420(WinApplication* app, x86::CPU& cpu);
    static void sub_52e440(WinApplication* app, x86::CPU& cpu);
    static void sub_52e4e0(WinApplication* app, x86::CPU& cpu);
    static void sub_52e500(WinApplication* app, x86::CPU& cpu);
    static void sub_52e660(WinApplication* app, x86::CPU& cpu);
    static void sub_52e670(WinApplication* app, x86::CPU& cpu);
    static void sub_52e6d0(WinApplication* app, x86::CPU& cpu);
    static void sub_52e88c(WinApplication* app, x86::CPU& cpu);
    static void sub_52ea3c(WinApplication* app, x86::CPU& cpu);
    static void sub_52eb70(WinApplication* app, x86::CPU& cpu);
    static void sub_52ec04(WinApplication* app, x86::CPU& cpu);
    static void sub_52ecf0(WinApplication* app, x86::CPU& cpu);
    static void sub_52ed20(WinApplication* app, x86::CPU& cpu);
    static void sub_52edfa(WinApplication* app, x86::CPU& cpu);
    static void sub_52ee00(WinApplication* app, x86::CPU& cpu);
    static void sub_52ee60(WinApplication* app, x86::CPU& cpu);
    static void sub_52f318(WinApplication* app, x86::CPU& cpu);
    static void sub_52f535(WinApplication* app, x86::CPU& cpu);
    static void sub_52f661(WinApplication* app, x86::CPU& cpu);
    static void sub_52f969(WinApplication* app, x86::CPU& cpu);
    static void sub_52fb2e(WinApplication* app, x86::CPU& cpu);
    static void sub_52ffc7(WinApplication* app, x86::CPU& cpu);
    static void sub_530019(WinApplication* app, x86::CPU& cpu);
    static void sub_53006b(WinApplication* app, x86::CPU& cpu);
    static void sub_530747(WinApplication* app, x86::CPU& cpu);
    static void sub_53083d(WinApplication* app, x86::CPU& cpu);
    static void sub_530a4e(WinApplication* app, x86::CPU& cpu);
    static void sub_530b4d(WinApplication* app, x86::CPU& cpu);
    static void sub_5310be(WinApplication* app, x86::CPU& cpu);
    static void sub_5311c0(WinApplication* app, x86::CPU& cpu);
    static void sub_53138f(WinApplication* app, x86::CPU& cpu);
    static void sub_531439(WinApplication* app, x86::CPU& cpu);
    static void sub_531f31(WinApplication* app, x86::CPU& cpu);
    static void sub_532162(WinApplication* app, x86::CPU& cpu);
    static void sub_5324d2(WinApplication* app, x86::CPU& cpu);
    static void sub_533dde(WinApplication* app, x86::CPU& cpu);
    static void sub_533dea(WinApplication* app, x86::CPU& cpu);
    static void sub_533df0(WinApplication* app, x86::CPU& cpu);
    static void sub_533df6(WinApplication* app, x86::CPU& cpu);
    static void sub_533dfc(WinApplication* app, x86::CPU& cpu);
    static void sub_533e02(WinApplication* app, x86::CPU& cpu);
    static void sub_533e08(WinApplication* app, x86::CPU& cpu);
    static void sub_533e0e(WinApplication* app, x86::CPU& cpu);
    static void sub_533e14(WinApplication* app, x86::CPU& cpu);
    static void sub_533e1a(WinApplication* app, x86::CPU& cpu);
    static void sub_533e20(WinApplication* app, x86::CPU& cpu);
    static void sub_533e26(WinApplication* app, x86::CPU& cpu);
    static void sub_533e2c(WinApplication* app, x86::CPU& cpu);
    static void sub_533e32(WinApplication* app, x86::CPU& cpu);
    static void sub_533e38(WinApplication* app, x86::CPU& cpu);
    static void sub_533e3e(WinApplication* app, x86::CPU& cpu);
    static void sub_533e44(WinApplication* app, x86::CPU& cpu);
    static void sub_533e4a(WinApplication* app, x86::CPU& cpu);
    static void sub_533e50(WinApplication* app, x86::CPU& cpu);
    static void sub_533e56(WinApplication* app, x86::CPU& cpu);
    static void sub_533e5c(WinApplication* app, x86::CPU& cpu);
    static void sub_533e86(WinApplication* app, x86::CPU& cpu);
    static void sub_533e8c(WinApplication* app, x86::CPU& cpu);
    static void sub_533e92(WinApplication* app, x86::CPU& cpu);
    static void sub_533e98(WinApplication* app, x86::CPU& cpu);
    static void sub_533e9e(WinApplication* app, x86::CPU& cpu);
    static void sub_533ea4(WinApplication* app, x86::CPU& cpu);
    static void sub_533eaa(WinApplication* app, x86::CPU& cpu);
    static void sub_533eb0(WinApplication* app, x86::CPU& cpu);
    static void sub_533ee0(WinApplication* app, x86::CPU& cpu);
};

}

#endif /* !NFS3HP_H_ */
