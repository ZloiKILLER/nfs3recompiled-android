#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4b8740: native, nfs3hp::frontend::rendCarLights */

/* sub_4b90e0: native, nfs3hp::frontend::rendBodyShake */

/* sub_4b9140: native, nfs3hp::frontend::rendFrontWheelTurn */

/* sub_4b9280: native, nfs3hp::frontend::rendRearWheelTurn */

/* sub_4b93c0: native, nfs3hp::frontend::rendBodyLean */

/* sub_4b9550: native, nfs3hp::frontend::rendTrackLitColour */

/* sub_4b97b0: native, nfs3hp::render::nightColour */

/* sub_4b9bc0: native, nfs3hp::render::carLights */

/* sub_4b9df0: native, nfs3hp::frontend::rendReplayPose */

/* sub_4ba320: native, nfs3hp::frontend::rendCarStates */

/* sub_4ba350: native, nfs3hp::frontend::rendCarShade */

/* sub_4ba810: native, nfs3hp::frontend::rendCarAverageColour */

/* sub_4ba960: native, nfs3hp::frontend::rendCarFarthest */

/* sub_4baa50: native, nfs3hp::frontend::rendCarFar */

/* sub_4bab70: native, nfs3hp::frontend::rendCarNear */

/* sub_4bae00: native, nfs3hp::render::detailedCarLights */

/* sub_4bb110: native, nfs3hp::frontend::rendCarSimpleNear */

/* sub_4bb200: native, nfs3hp::render::mediumCarLights */

/* sub_4bb4c0: native, nfs3hp::frontend::rendDrawCar */

/* sub_4bb5d0: native, nfs3hp::vertices::carDetail */

/* sub_4bba10: native, nfs3hp::frontend::rendDetailInUse */

/* sub_4bba50: native, nfs3hp::frontend::rendLowerDetail */

/* sub_4bbad0: native, nfs3hp::frontend::rendFitDetailBudget */

/* sub_4bbba0: native, nfs3hp::frontend::rendCars */

/* sub_4bbcc0: native, nfs3hp::dead::stub_0x4bbcc0 */

/* sub_4bbd00: native, nfs3hp::dead::stub_0x4bbd00 */

/* sub_4bbd30: native, nfs3hp::dead::stub_0x4bbd30 */

/* sub_4bbd70: native, nfs3hp::frontend::rendArenaRewind */

/* sub_4bbd80: native, nfs3hp::frontend::rendArenaCreate */

/* sub_4bbdc0: native, nfs3hp::frontend::rendArenaFree */

/* sub_4bbde0: native, nfs3hp::frontend::rendArenaTake */

/* sub_4bbe40: native, nfs3hp::frontend::rendArenaHere */

/* sub_4bbe70: native, nfs3hp::frontend::rendEmptyList */

/* sub_4bbe90: native, nfs3hp::frontend::rendListOfOne */

/* sub_4bbec0: native, nfs3hp::frontend::rendJoinLists */

/* sub_4bbf20: native, nfs3hp::frontend::rendOpenListStart */

/* sub_4bbf40: native, nfs3hp::frontend::rendOpenListAppend */

/* sub_4bbf80: native, nfs3hp::frontend::rendOpenListTake */

/* sub_4bbfc0: native, nfs3hp::frontend::rendOpenListClose */

/* sub_4bc000: native, nfs3hp::frontend::rendFlushAndList */

/* sub_4bc030: native, nfs3hp::frontend::rendDrawListPlain */

/* sub_4bc060: native, nfs3hp::frontend::rendDrawListAt */

/* sub_4bc0b0: native, nfs3hp::frontend::rendDrawListMirrored */

/* sub_4bc100: native, nfs3hp::frontend::raceLoopViewForHud */

/* sub_4bc130: native, nfs3hp::frontend::raceLoopViewForScene */

/* sub_4bc160: native, nfs3hp::graphics::raceFrame */

/* sub_4bc470: native, nfs3hp::frontend::simViewPass */

/* sub_4bc530: native, nfs3hp::frontend::pauseResetRaceMouse */

/* sub_4bc560: native, nfs3hp::frontend::raceLoopPointerIdle */

/* sub_4bc620: native, nfs3hp::frontend::raceLoopEndFrame */

/* sub_4bc650: native, nfs3hp::frontend::raceLoopTextAndCount */

/* sub_4bc660: native, nfs3hp::frontend::pauseMenuAgain */

/* sub_4bc670: native, nfs3hp::frontend::pauseEndRestorePointer */

/* sub_4bc680: native, nfs3hp::frontend::raceLoopPauseDraw */

/* sub_4bc820: native, nfs3hp::frontend::raceLoopNothing */

/* sub_4bc830: native, nfs3hp::frontend::loadingRaceSteps */

/* sub_4bc870: native, nfs3hp::frontend::loadingRaceResetSteps */

/* sub_4bc8b0: native, nfs3hp::frontend::raceEffectsReset */

/* sub_4bc8e0: native, nfs3hp::frontend::simRaceLeaveTasks */

/* sub_4bc910: native, nfs3hp::frontend::simRaceAddTasks */

/* sub_4bc950: native, nfs3hp::frontend::raceEffectsFree */

/* sub_4bc980: native, nfs3hp::frontend::raceLoopDrawStart */

/* sub_4bc990: native, nfs3hp::frontend::raceLoopDrawEnd */

/* sub_4bc9a0: native, nfs3hp::frontend::replayModeOff */

/* sub_4bc9b0: native, nfs3hp::frontend::replayPackRow */

/* sub_4bca80: native, nfs3hp::frontend::replayUnpackRow */

/* sub_4bcb00: native, nfs3hp::frontend::loadingReadReplayKeepingSettings */

/* sub_4bcb90: native, nfs3hp::frontend::replayStart */

/* sub_4bcd60: native, nfs3hp::frontend::replayEnd */

/* sub_4bcda0: native, nfs3hp::frontend::replayStoreBlock */

/* sub_4bcee0: native, nfs3hp::frontend::replayTakeBlock */

/* sub_4bd000: native, nfs3hp::frontend::replayRecordControls */

/* sub_4bd090: native, nfs3hp::frontend::replayPlayControls */

/* sub_4bd1e0: native, nfs3hp::load::saveRaceReplay */

/* sub_4bd2e0: native, nfs3hp::load::readRaceReplay */

/* sub_4bd380: native, nfs3hp::frontend::replayCarTask */

/* sub_4bd3f0: native, nfs3hp::frontend::raceLoopPointerAtChoice */

/* sub_4bd4d0: native, nfs3hp::frontend::simPanelPointer */

/* sub_4bd640: native, nfs3hp::frontend::simPanelKeys */

/* sub_4bd9d0: native, nfs3hp::frontend::replayNoteEvent */

/* sub_4bda00: native, nfs3hp::frontend::replayRecordEvents */

/* sub_4bdb60: native, nfs3hp::frontend::simTvSetCamera */

/* sub_4bdbd0: native, nfs3hp::frontend::simTvNextCamera */

/* sub_4bdc70: native, nfs3hp::frontend::simTvShotOver */

/* sub_4bdcc0: native, nfs3hp::frontend::simTvStartShot */

/* sub_4bdd10: native, nfs3hp::frontend::simTvBusiest */

/* sub_4bdd70: native, nfs3hp::frontend::simTvDirect */

/* sub_4be000: native, nfs3hp::frontend::simIsNumberCharacter */

/* sub_4be020: native, nfs3hp::frontend::raceLoopReadNumber */

/* sub_4be090: native, nfs3hp::dead::stub_0x4be090 */

/* sub_4be1a0: native, nfs3hp::frontend::simOffBeat */

/* sub_4be1f0: native, nfs3hp::frontend::simTaskListNew */

/* sub_4be240: native, nfs3hp::frontend::taskBlockFree */

/* sub_4be250: native, nfs3hp::frontend::simTaskListAdd */

/* sub_4be2f0: native, nfs3hp::frontend::loadingListRemove */

/* sub_4be3a0: native, nfs3hp::frontend::taskListRun */

/* sub_4be3d0: native, nfs3hp::frontend::pauseFillViewQuads */

/* sub_4be570: native, nfs3hp::graphics::modeSize */

/* sub_4be5e0: native, nfs3hp::frontend::viewsLayout */

/* sub_4be8e0: native, nfs3hp::graphics::drawViewSurround */

}
