#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_401000: native, nfs3hp::pieces::stub_0x401000 */

/* sub_401010: native, nfs3hp::Error */

/* sub_401220: native, nfs3hp::airace::nothingAtStart */

/* sub_401230: native, nfs3hp::airace::nothingAtLoad */

/* sub_401240: native, nfs3hp::airace::releaseEdge */

/* sub_4012c0: native, nfs3hp::airace::setUpCops */

/* sub_401320: native, nfs3hp::airace::radioOn */

/* sub_401330: native, nfs3hp::airace::radioLevelDown */

/* sub_401350: native, nfs3hp::airace::radioLevelUp */

/* sub_401370: native, nfs3hp::airace::copTopSpeed */

/* sub_4013b0: native, nfs3hp::airace::nearestCopGap */

/* sub_401430: native, nfs3hp::airace::setWantedSpeed */

/* sub_4014b0: native, nfs3hp::airace::noteSpeeding */

/* sub_401500: native, nfs3hp::airace::pulledOver */

/* sub_401520: native, nfs3hp::airace::copRadioCall */

/* sub_401580: native, nfs3hp::airace::ticketRadio */

/* sub_4015b0: native, nfs3hp::airace::stopIfCopNear */

/* sub_401650: native, nfs3hp::airace::stopForCop */

/* sub_401780: native, nfs3hp::airace::sizeUpTarget */

/* sub_4018b0: native, nfs3hp::airace::joinPursuit */

/* sub_401a20: native, nfs3hp::airace::leaveCopList */

/* sub_401a80: native, nfs3hp::airace::giveUpPursuit */

/* sub_401b50: native, nfs3hp::airace::pullOverSpot */

/* sub_401c20: native, nfs3hp::airace::planPullOver */

/* sub_401e20: native, nfs3hp::airace::endPullOver */

/* sub_401e40: native, nfs3hp::airace::copStopsBy */

/* sub_401fb0: native, nfs3hp::airace::stopOver */

/* sub_401ff0: native, nfs3hp::airace::giveTicket */

/* sub_402110: native, nfs3hp::airace::targetStopping */

/* sub_402420: native, nfs3hp::airace::chaseSpeed */

/* sub_4025c0: native, nfs3hp::airace::pursueTarget */

/* sub_402750: native, nfs3hp::airace::votePursuitLane */

/* sub_4028e0: native, nfs3hp::airace::sideTowardNode */

/* sub_402910: native, nfs3hp::airace::keepSideOfTarget */

/* sub_4029c0: native, nfs3hp::airace::placeCopAtStart */

/* sub_402a00: native, nfs3hp::airace::placeCop */

/* sub_402c70: native, nfs3hp::airace::alwaysOne */

/* sub_402c80: native, nfs3hp::airace::carReversing */

/* sub_402cb0: native, nfs3hp::airace::copHitByCar */

/* sub_402d90: native, nfs3hp::airace::voteRoadLane */

/* sub_402e10: native, nfs3hp::airace::spotSpeeder */

/* sub_402f70: native, nfs3hp::airace::laneOffRoad */

/* sub_402fd0: native, nfs3hp::airace::atOrderNode */

/* sub_403000: native, nfs3hp::airace::sideOfOrder */

/* sub_403030: native, nfs3hp::airace::takeOrder */

/* sub_403180: native, nfs3hp::airace::parkCop */

/* sub_403270: native, nfs3hp::airace::drivePatrol */

/* sub_4032e0: native, nfs3hp::airace::copDriver */

/* sub_403830: native, nfs3hp::airace::giveOrder */

/* sub_403870: native, nfs3hp::airace::spreadCops */

/* sub_403910: native, nfs3hp::airace::radioStopped */

/* sub_403950: native, nfs3hp::airace::radioStop */

/* sub_403970: native, nfs3hp::airace::initPursuits */

/* sub_4039c0: native, nfs3hp::airace::callRoadblock */

/* sub_403b20: native, nfs3hp::airace::callTwoCars */

/* sub_403d30: native, nfs3hp::airace::roadblockSiren */

/* sub_403df0: native, nfs3hp::airace::watchRoadblock */

/* sub_403ed0: native, nfs3hp::airace::watchSecondCar */

/* sub_403f80: native, nfs3hp::airace::copsWanted */

/* sub_404070: native, nfs3hp::airace::radioJoined */

/* sub_4040b0: native, nfs3hp::airace::radioGiveUp */

/* sub_4040e0: native, nfs3hp::airace::releaseCops */

/* sub_4042b0: native, nfs3hp::airace::dropCop */

/* sub_404310: native, nfs3hp::dead::stub_0x404310 */

/* sub_404340: native, nfs3hp::airace::sendCopAhead */

/* sub_4043a0: native, nfs3hp::airace::callBackup */

/* sub_404560: native, nfs3hp::airace::orderTaken */

/* sub_404580: native, nfs3hp::airace::nothingForCop */

/* sub_404590: native, nfs3hp::dead::stub_0x404590 */

/* sub_4045a0: native, nfs3hp::dead::stub_0x4045a0 */

/* sub_4045b0: native, nfs3hp::dead::stub_0x4045b0 */

/* sub_4045c0: native, nfs3hp::dead::stub_0x4045c0 */

/* sub_4045d0: native, nfs3hp::dead::stub_0x4045d0 */

/* sub_4045e0: native, nfs3hp::dead::stub_0x4045e0 */

/* sub_4045f0: native, nfs3hp::dead::stub_0x4045f0 */

/* sub_404600: native, nfs3hp::dead::stub_0x404600 */

/* sub_404610: native, nfs3hp::airace::classRule */

/* sub_404650: native, nfs3hp::airace::nothingAtEnd */

/* sub_404660: native, nfs3hp::airace::nothingAtEndToo */

/* sub_404690: native, nfs3hp::airace::setRammingRules */

/* sub_4047f0: native, nfs3hp::airace::classValue */

/* sub_404810: native, nfs3hp::airace::classValueToo */

/* sub_404840: native, nfs3hp::airace::rivalTick */

/* sub_404a00: native, nfs3hp::airace::carContact */

/* sub_404b90: native, nfs3hp::airace::slowForClass */

/* sub_404c40: native, nfs3hp::airace::nothingAtStartToo */

/* sub_404c50: native, nfs3hp::airace::catchUpSpeed */

/* sub_404c80: native, nfs3hp::airace::findLeader */

/* sub_404d30: native, nfs3hp::aidrive::copStartPlace */

/* sub_404e40: native, nfs3hp::aidrive::callRoadblock */

/* sub_404f80: native, nfs3hp::aidrive::sirenSwitch */

/* sub_404ff0: native, nfs3hp::aidrive::watchRacers */

/* sub_405220: native, nfs3hp::aidrive::jumpBehindLeader */

/* sub_4052e0: native, nfs3hp::aidrive::clockRacers */

/* sub_4053b0: native, nfs3hp::aidrive::startPursuit */

/* sub_405430: native, nfs3hp::aidrive::bustCheck */

/* sub_405630: native, nfs3hp::aidrive::pursuitStep */

/* sub_4056e0: native, nfs3hp::aidrive::dropPursuit */

/* sub_405740: native, nfs3hp::aidrive::copDriver */

/* sub_405a20: native, nfs3hp::aidrive::nearCar */

}
