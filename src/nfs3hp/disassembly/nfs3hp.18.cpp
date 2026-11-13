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

/* sub_470a60: native, nfs3hp::frontend::rendFogAmount */

/* sub_470a70: native, nfs3hp::dead::stub_0x470a70 */

/* sub_470aa0: native, nfs3hp::frontend::rendFogColour */

/* sub_470ad0: native, nfs3hp::frontend::rendFogDensity */

/* sub_470b20: native, nfs3hp::frontend::rendFogDensityTimes */

/* sub_470b70: native, nfs3hp::frontend::rendFogBoth */

/* sub_470b90: native, nfs3hp::frontend::rendFogReset */

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

/* sub_473340: native, nfs3hp::frontend::loadingRosterClearUsed */

/* sub_473360: native, nfs3hp::frontend::loadingRosterPickLeastUsed */

/* sub_4733b0: native, nfs3hp::frontend::loadingRosterMarkUsed */

/* sub_4733c0: native, nfs3hp::port::view::colourNearest */

/* sub_4734d0: native, nfs3hp::frontend::loadingRosterPickOpponentCar */

/* sub_473550: native, nfs3hp::frontend::loadingRosterPickOpponentDriver */

/* sub_473570: native, nfs3hp::frontend::loadingRosterPickCopDriver */

/* sub_4735c0: native, nfs3hp::frontend::loadingRosterPickTrafficCar */

/* sub_4736c0: native, nfs3hp::frontend::loadingRosterEntryFinalist */

/* sub_473710: native, nfs3hp::frontend::loadingRosterEntryHuman */

/* sub_473890: native, nfs3hp::frontend::loadingRosterEntryCop */

/* sub_473960: native, nfs3hp::frontend::loadingRosterEntryOpponent */

/* sub_473a70: native, nfs3hp::frontend::loadingRosterEntryTraffic */

/* sub_473ac0: native, nfs3hp::frontend::loadingRosterTournamentTracks */

/* sub_473b70: native, nfs3hp::frontend::loadingRosterGridOrder */

/* sub_473bf0: native, nfs3hp::frontend::loadingRosterBuildRegular */

/* sub_473ee0: native, nfs3hp::frontend::loadingRosterBuildTournament */

/* sub_474310: native, nfs3hp::frontend::loadingRosterBuildKnockout */

/* sub_474640: native, nfs3hp::frontend::loadingRosterBuildHotPursuit */

/* sub_474c60: native, nfs3hp::frontend::loadingRosterSeedByte */

/* sub_474ca0: native, nfs3hp::frontend::loadingRosterBuild */

/* sub_474d20: native, nfs3hp::pieces::stub_0x474d20 */

/* sub_474d50: native, nfs3hp::frontend::textLookClearLights */

/* sub_474db0: native, nfs3hp::port::view::edgeTriangles */

/* sub_475240: native, nfs3hp::port::view::glyphResource */

/* sub_475b10: native, nfs3hp::frontend::loopFreeLoadedBlock */

}
