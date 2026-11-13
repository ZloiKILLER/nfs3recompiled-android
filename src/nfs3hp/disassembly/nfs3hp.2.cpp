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

/* sub_40ff20: native, nfs3hp::pieces::stub_0x40ff20 */

/* sub_40ffd0: native, nfs3hp::port::raceTuneOf */

/* sub_410030: native, nfs3hp::port::raceMusicPrepare */

/* sub_410330: native, nfs3hp::port::musicStreamMake */

/* sub_410370: native, nfs3hp::port::musicStreamMakeOf */

}
