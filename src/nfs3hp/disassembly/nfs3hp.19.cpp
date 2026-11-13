#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_475b50: native, nfs3hp::render::solidText */

/* sub_476f30: native, nfs3hp::dead::stub_0x476f30 */

/* sub_476f60: native, nfs3hp::dead::stub_0x476f60 */

/* sub_476f90: native, nfs3hp::dead::stub_0x476f90 */

/* sub_476fb0: native, nfs3hp::frontend::rendStringWidth */

/* sub_477030: native, nfs3hp::dead::stub_0x477030 */

/* sub_477070: native, nfs3hp::frontend::textLookSetFlat */

/* sub_4770c0: native, nfs3hp::dead::stub_0x4770c0 */

/* sub_477120: native, nfs3hp::dead::stub_0x477120 */

/* sub_477170: native, nfs3hp::frontend::textLookSetDirectional */

/* sub_4771c0: native, nfs3hp::frontend::textLookAddLight */

/* sub_477290: native, nfs3hp::pieces::stub_0x477290 */

/* sub_477380: native, nfs3hp::dead::stub_0x477380 */

/* sub_477470: native, nfs3hp::frontend::textLookSetTexture */

/* sub_4774b0: native, nfs3hp::frontend::rendSparksOnScreen */

/* sub_477690: native, nfs3hp::dead::stub_0x477690 */

/* sub_4776f0: native, nfs3hp::dead::stub_0x4776f0 */

/* sub_477740: native, nfs3hp::dead::stub_0x477740 */

/* sub_4777a0: native, nfs3hp::dead::stub_0x4777a0 */

/* sub_4777b0: native, nfs3hp::dead::stub_0x4777b0 */

/* sub_4777c0: native, nfs3hp::dead::stub_0x4777c0 */

/* sub_4777d0: native, nfs3hp::frontend::ghostSaveFile */

/* sub_477830: native, nfs3hp::frontend::ghostPoseSetRotation */

/* sub_477850: native, nfs3hp::frontend::ghostPosePosition */

/* sub_4778c0: native, nfs3hp::frontend::ghostPoseSetPosition */

/* sub_477990: native, nfs3hp::frontend::ghostPoseBetween */

/* sub_4779e0: native, nfs3hp::frontend::ghostRecordedAt */

/* sub_477a10: native, nfs3hp::frontend::ghostLoadedAt */

/* sub_477a50: native, nfs3hp::frontend::ghostRecordingStart */

/* sub_477a90: native, nfs3hp::load::saveGhost */

/* sub_477ac0: native, nfs3hp::load::writeGhost */

/* sub_477b00: native, nfs3hp::load::readGhost */

/* sub_477b50: native, nfs3hp::frontend::ghostLoadFile */

/* sub_477c00: native, nfs3hp::frontend::clearLoadFlag */

/* sub_477c20: native, nfs3hp::frontend::ghostRecordCar */

/* sub_477d90: native, nfs3hp::frontend::ghostPlaybackPose */

/* sub_477df0: native, nfs3hp::dead::stub_0x477df0 */

/* sub_477f60: native, nfs3hp::frontend::netRaceLoadStep */

/* sub_477fd0: native, nfs3hp::frontend::netRaceMusicStep */

/* sub_478000: native, nfs3hp::frontend::raceEndEmpty */

/* sub_478010: native, nfs3hp::frontend::pauseCheckSaveable */

/* sub_478090: native, nfs3hp::frontend::loadingSavedResolvePlayers */

/* sub_478100: native, nfs3hp::frontend::loadingSavedResolveEntries */

/* sub_4781b0: native, nfs3hp::frontend::loadingSavedNameEntries */

/* sub_478280: native, nfs3hp::frontend::loadingScreenSizeChosen */

/* sub_478290: native, nfs3hp::frontend::loadingScreenSizeNumber */

/* sub_4782b0: native, nfs3hp::frontend::loadingScreenSizeNumberOf */

/* sub_4782c0: native, nfs3hp::frontend::loadingHorizonStartCall */

/* sub_4782d0: native, nfs3hp::frontend::loadingHorizonFreeCall */

/* sub_4782e0: native, nfs3hp::frontend::loadingSkyViewTo */

/* sub_478320: native, nfs3hp::frontend::loadingShapeSize */

/* sub_478340: native, nfs3hp::load::loadHorizon */

/* sub_478b70: native, nfs3hp::frontend::loadingHorizonSetPieces */

/* sub_478bb0: native, nfs3hp::frontend::loadingHorizonMeasure */

/* sub_478c70: native, nfs3hp::frontend::loadingHorizonStart */

/* sub_478c80: native, nfs3hp::frontend::loadingHorizonFree */

/* sub_478cb0: native, nfs3hp::frontend::rendMakeQuad */

/* sub_478dc0: native, nfs3hp::frontend::rendViewRectangle */

/* sub_478e10: native, nfs3hp::frontend::effectsFogAtCamera */

/* sub_479080: native, nfs3hp::frontend::rendSkyListStart */

/* sub_4790d0: native, nfs3hp::frontend::rendSkyListKeep */

/* sub_479100: native, nfs3hp::frontend::rendSkyListEnd */

/* sub_479160: native, nfs3hp::frontend::rendBandPoints */

/* sub_4792e0: native, nfs3hp::frontend::rendCurvePoints */

/* sub_479370: native, nfs3hp::frontend::rendBandVertices */

/* sub_479500: native, nfs3hp::frontend::rendRingPoints */

/* sub_479680: native, nfs3hp::frontend::rendSkyGradientRing */

/* sub_479860: native, nfs3hp::frontend::rendFoggedColour */

/* sub_4799d0: native, nfs3hp::frontend::rendSkyHazeRing */

/* sub_479b90: native, nfs3hp::frontend::rendSkyCloudRing */

/* sub_479ed0: native, nfs3hp::frontend::rendSun */

/* sub_47a190: native, nfs3hp::render::sky */

/* sub_47a1f0: native, nfs3hp::render::flatSky */

/* sub_47a4c0: native, nfs3hp::frontend::rendSkyFlat */

/* sub_47a880: native, nfs3hp::render::viewSky */

/* sub_47a970: native, nfs3hp::frontend::skyLoad */

/* sub_47ac30: native, nfs3hp::load::loadSky */

/* sub_47adc0: native, nfs3hp::frontend::rendPointHeight */

/* sub_47ae10: native, nfs3hp::frontend::rendCloudGrid */

/* sub_47b350: native, nfs3hp::frontend::rendFlashesReset */

/* sub_47b380: native, nfs3hp::dead::stub_0x47b380 */

/* sub_47b390: native, nfs3hp::frontend::rendFlashesResetAgain */

/* sub_47b3a0: native, nfs3hp::frontend::rendFlashOnGrid */

/* sub_47b4f0: native, nfs3hp::frontend::rendDomeVertices */

/* sub_47b5c0: native, nfs3hp::frontend::rendSkyWeather */

/* sub_47b7d0: native, nfs3hp::render::glareVertices */

/* sub_47b850: native, nfs3hp::render::skyDome */

/* sub_47bbb8: native, nfs3hp::pieces::stub_0x47bbb8 */

/* sub_47bc20: native, nfs3hp::frontend::rendKindIsText */

/* sub_47bc40: native, nfs3hp::frontend::rendSetUpHudVertices */

/* sub_47d1d0: native, nfs3hp::frontend::rendDrawBox */

/* sub_47d480: native, nfs3hp::frontend::rendUpdateCopMarkers */

/* sub_47d580: native, nfs3hp::frontend::rendDrawPursuitMark */

/* sub_47ee80: native, nfs3hp::frontend::rendPrepareStandings */

/* sub_47f090: native, nfs3hp::frontend::rendTurnMatrixToRoad */

/* sub_47f2e0: native, nfs3hp::frontend::rendCopyRectangle */

/* sub_47f340: native, nfs3hp::frontend::rendLayScalesIntoHud */

/* sub_47f590: native, nfs3hp::frontend::rendLoadHudPictures */

/* sub_47f650: native, nfs3hp::render::hudFrame */

/* sub_47fbe0: native, nfs3hp::frontend::rendQueueEightQuads */

}
