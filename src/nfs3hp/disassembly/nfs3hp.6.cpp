#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4262a0: native, nfs3hp::frontend::simCameraWaitThenHandOn */

/* sub_426310: native, nfs3hp::frontend::simCameraBackFromWait */

/* sub_426370: native, nfs3hp::frontend::simCameraPrepare */

/* sub_426450: native, nfs3hp::frontend::simCameraStep */

/* sub_426560: native, nfs3hp::frontend::simCameraForcedMode */

/* sub_426680: native, nfs3hp::frontend::simCamerasStep */

/* sub_4267a0: native, nfs3hp::frontend::carSaveFrame */

/* sub_4267f0: native, nfs3hp::frontend::trackSaveFrame */

/* sub_426870: native, nfs3hp::frontend::carRecord */

/* sub_426b00: native, nfs3hp::frontend::trackRecord */

/* sub_426c20: native, nfs3hp::frontend::car1EditClicked */

/* sub_426c40: native, nfs3hp::frontend::car2EditClicked */

/* sub_426c60: native, nfs3hp::frontend::car1ColourClicked */

/* sub_426ce0: native, nfs3hp::frontend::car2ColourClicked */

/* sub_426d60: native, nfs3hp::frontend::car1OnEnter */

/* sub_426e20: native, nfs3hp::frontend::car1OnFrame */

/* sub_426ee0: native, nfs3hp::frontend::car1OnExit */

/* sub_426ef0: native, nfs3hp::frontend::car2OnEnter */

/* sub_426f80: native, nfs3hp::frontend::car2OnFrame */

/* sub_427040: native, nfs3hp::frontend::car2OnExit */

/* sub_427050: native, nfs3hp::frontend::opponentFitCar */

/* sub_4270c0: native, nfs3hp::frontend::opponentFitArt */

/* sub_427190: native, nfs3hp::frontend::opponentOnFrame */

/* sub_4272a0: native, nfs3hp::frontend::opponentOnEnter */

/* sub_4272e0: native, nfs3hp::frontend::opponentOnExit */

/* sub_4272f0: native, nfs3hp::frontend::tuningLoad */

/* sub_427370: native, nfs3hp::frontend::tuningSave */

/* sub_4273e0: native, nfs3hp::frontend::tuningEngineMoved */

/* sub_427470: native, nfs3hp::frontend::tuningBrakesMoved */

/* sub_427500: native, nfs3hp::frontend::tuningSteeringMoved */

/* sub_427590: native, nfs3hp::frontend::tuningGearsMoved */

/* sub_427620: native, nfs3hp::frontend::tuningSuspensionMoved */

/* sub_4276b0: native, nfs3hp::frontend::tuningAeroMoved */

/* sub_427740: native, nfs3hp::frontend::tuningTiresMoved */

/* sub_4277d0: native, nfs3hp::frontend::tuningSetSliders */

/* sub_4279b0: native, nfs3hp::frontend::tuningRememberSliders */

/* sub_427ac0: native, nfs3hp::frontend::tuningSlidersMoved */

/* sub_427c20: native, nfs3hp::frontend::tuningBindSliders */

/* sub_427d00: native, nfs3hp::frontend::tuningLoadAdvanced */

/* sub_427d80: native, nfs3hp::frontend::tuningDefaultsAnswered */

/* sub_427e10: native, nfs3hp::frontend::tuningDoneKey */

/* sub_427ea0: native, nfs3hp::frontend::tuningCancelKey */

/* sub_427f40: native, nfs3hp::frontend::tuningCarListKey */

/* sub_427fb0: native, nfs3hp::frontend::tuningDefaultsKey */

/* sub_428020: native, nfs3hp::frontend::tuningBindDoneCancel */

/* sub_428060: native, nfs3hp::frontend::tuningFitButtons */

/* sub_4281e0: native, nfs3hp::frontend::tuningPrepare */

/* sub_428360: native, nfs3hp::frontend::tuningOnEnter */

/* sub_4283b0: native, nfs3hp::frontend::tuningOnExit */

/* sub_4283d0: native, nfs3hp::frontend::loadingRaceCarSteps */

/* sub_428460: native, nfs3hp::pieces::stub_0x428460 */

/* sub_428480: native, nfs3hp::dead::stub_0x428480 */

/* sub_4284b0: native, nfs3hp::dead::stub_0x4284b0 */

/* sub_4284e0: native, nfs3hp::frontend::loadingCarAddEvent */

/* sub_4285f0: native, nfs3hp::frontend::loadingCarClearDriving */

/* sub_428700: native, nfs3hp::frontend::loadingCarJoinLists */

/* sub_428970: native, nfs3hp::frontend::loadingCarReset */

/* sub_428ad0: native, nfs3hp::frontend::loadingCarPlaceIsClear */

/* sub_428d60: native, nfs3hp::frontend::loadingCarPlaceOnRoad */

/* sub_428f90: native, nfs3hp::racestart::respawnCar */

/* sub_429100: native, nfs3hp::racestart::resetCar */

/* sub_429260: native, nfs3hp::racestart::stuckCheck */

/* sub_429490: native, nfs3hp::dead::stub_0x429490 */

/* sub_4294c0: native, nfs3hp::racestart::clearRespawn */

/* sub_4294e0: native, nfs3hp::racestart::shakeAmplitude */

/* sub_4295a0: native, nfs3hp::racestart::startShake */

/* sub_4296c0: native, nfs3hp::racestart::startIdleShake */

/* sub_429790: native, nfs3hp::racestart::shakeWave */

/* sub_4297d0: native, nfs3hp::racestart::objectContacts */

/* sub_429880: native, nfs3hp::racestart::wheelSlip */

/* sub_4299a0: native, nfs3hp::racestart::wheelCells */

/* sub_429b40: native, nfs3hp::racestart::wheelSurface */

/* sub_429ba0: native, nfs3hp::racestart::endSkidMark */

/* sub_429d20: native, nfs3hp::racestart::extendSkidMark */

/* sub_429f10: native, nfs3hp::racestart::skidSounds */

/* sub_42a130: native, nfs3hp::racestart::wheelSpray */

/* sub_42a280: native, nfs3hp::racestart::surfaceEffects */

/* sub_42a480: native, nfs3hp::racestart::wheelSkid */

/* sub_42a560: native, nfs3hp::racestart::wheelEffects */

/* sub_42a7d0: native, nfs3hp::racestart::updateShake */

/* sub_42ac40: native, nfs3hp::racestart::crashEffects */

/* sub_42b480: native, nfs3hp::racestart::respawnEffects */

/* sub_42b660: native, nfs3hp::racestart::stopCarEffects */

/* sub_42b780: native, nfs3hp::racestart::carEffects */

/* sub_42b7b0: native, nfs3hp::racestart::airborneSpin */

/* sub_42b900: native, nfs3hp::racestart::updateCar */

/* sub_42ba00: native, nfs3hp::racestart::defaultGridPlace */

/* sub_42bb80: native, nfs3hp::racestart::loadStartGrid */

/* sub_42bc70: native, nfs3hp::racestart::gridPlace */

/* sub_42bcb0: native, nfs3hp::racestart::placeOnGrid */

/* sub_42c2a0: native, nfs3hp::raceload::loadCarPerformance */

/* sub_42c350: native, nfs3hp::raceload::loadRaceCar */

/* sub_42c400: native, nfs3hp::raceload::setCarHandlers */

/* sub_42c520: native, nfs3hp::raceload::freeCarShapes */

/* sub_42c530: native, nfs3hp::raceload::restartRaceCars */

/* sub_42c590: native, nfs3hp::decompiled::resetHandlingTunings */

/* sub_42c660: native, nfs3hp::raceload::clearCarSlots */

/* sub_42c680: native, nfs3hp::raceload::clearRaceCounts */

/* sub_42c6d0: native, nfs3hp::raceload::clearCarList */

/* sub_42c700: native, nfs3hp::raceload::loadRaceCars */

}
