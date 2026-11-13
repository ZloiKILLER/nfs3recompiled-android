#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4a1090: native, nfs3hp::port::conv::fn4a1090 */

/* sub_4a1210: native, nfs3hp::frontend::simTrackProbeWalls */

/* sub_4a1610: native, nfs3hp::frontend::simTrackPredictContact */

/* sub_4a1ce0: native, nfs3hp::frontend::simTrackReactToSurface */

/* sub_4a20c0: native, nfs3hp::port::conv::fn4a20c0 */

/* sub_4a21a0: native, nfs3hp::frontend::simTrackTunnelRoof */

/* sub_4a2260: native, nfs3hp::frontend::simTrackDownforceAndDrag */

/* sub_4a2630: native, nfs3hp::port::conv::fn4a2630 */

/* sub_4a2690: native, nfs3hp::frontend::simTrackSettle */

/* sub_4a2830: native, nfs3hp::port::conv::fn4a2830 */

/* sub_4a2bc0: native, nfs3hp::frontend::simTrackOffsetAcross */

/* sub_4a2c60: native, nfs3hp::port::conv::fn4a2c60 */

/* sub_4a2cc0: native, nfs3hp::frontend::simTrackTurnMatrix */

/* sub_4a2e20: native, nfs3hp::port::conv::fn4a2e20 */

/* sub_4a2f10: native, nfs3hp::frontend::simTrackSpinThirtySecond */

/* sub_4a2f30: native, nfs3hp::frontend::simTrackSpinSixtyFourth */

/* sub_4a3050: native, nfs3hp::pieces::stub_0x4a3050 */

/* sub_4a3070: native, nfs3hp::frontend::exceptionName */

/* sub_4a31f0: native, nfs3hp::dead::stub_0x4a31f0 */

/* sub_4a32f0: native, nfs3hp::dead::stub_0x4a32f0 */

/* sub_4a3390: native, nfs3hp::frontend::loopGameStart */

/* sub_4a33a0: native, nfs3hp::frontend::loopGameEnd */

/* sub_4a33b0: native, nfs3hp::frontend::loopRaceRestart */

/* sub_4a33d0: native, nfs3hp::frontend::loadingStopCheck */

/* sub_4a3400: native, nfs3hp::frontend::loopRaceLoad */

/* sub_4a35d0: native, nfs3hp::graphics::takeDownRace */

/* sub_4a3690: native, nfs3hp::frontend::loopRaceSession */

/* sub_4a38c0: native, nfs3hp::frontend::winaAlreadyRunning */

/* sub_4a38f0: native, nfs3hp::frontend::winaVersionText */

/* sub_4a39f0: native, nfs3hp::graphics::gameMain */

/* sub_4a4020: native, nfs3hp::frontend::simOpenWebPage */

/* sub_4a4060: native, nfs3hp::frontend::rendSkyRandomBetween */

/* sub_4a40c0: native, nfs3hp::frontend::rendSkySpritePlace */

/* sub_4a4390: native, nfs3hp::frontend::effectsLogoTextureStart */

/* sub_4a4410: native, nfs3hp::render::bouncers */

/* sub_4a46f0: native, nfs3hp::frontend::carBadgeColour */

/* sub_4a47c0: native, nfs3hp::frontend::effectsBandDraw */

/* sub_4a49c0: native, nfs3hp::frontend::effectsScreenDraw */

/* sub_4a4a10: native, nfs3hp::frontend::effectsScreenStart */

/* sub_4a4ba0: native, nfs3hp::frontend::effectsLoadScreenArt */

/* sub_4a4c20: native, nfs3hp::frontend::effectsLogoDraw */

/* sub_4a5080: native, nfs3hp::frontend::commandLineRead */

/* sub_4a54f0: native, nfs3hp::frontend::npsLoadDll */

/* sub_4a5780: native, nfs3hp::frontend::npsDevelopMode */

/* sub_4a5880: native, nfs3hp::frontend::npsDllPath */

/* sub_4a59c0: native, nfs3hp::frontend::npsEnsureDll */

/* sub_4a5aa0: native, nfs3hp::dead::stub_0x4a5aa0 */

/* sub_4a5af0: native, nfs3hp::frontend::netThisLink */

/* sub_4a5b00: native, nfs3hp::frontend::netNpsStart */

/* sub_4a5d10: native, nfs3hp::ea::sendOverLink */

/* sub_4a5d70: native, nfs3hp::dead::stub_0x4a5d70 */

/* sub_4a5dd0: native, nfs3hp::frontend::netNpsStop */

/* sub_4a5e50: native, nfs3hp::frontend::netNpsMessage */

/* sub_4a5ef0: native, nfs3hp::frontend::netNpsReceived */

/* sub_4a5f20: native, nfs3hp::dead::stub_0x4a5f20 */

/* sub_4a5f30: native, nfs3hp::frontend::netEndpointOfLink */

/* sub_4a5f80: native, nfs3hp::dead::stub_0x4a5f80 */

/* sub_4a5f90: native, nfs3hp::dead::stub_0x4a5f90 */

/* sub_4a5fb0: native, nfs3hp::dead::stub_0x4a5fb0 */

/* sub_4a5ff0: native, nfs3hp::dead::stub_0x4a5ff0 */

/* sub_4a6020: native, nfs3hp::dead::stub_0x4a6020 */

/* sub_4a60a0: native, nfs3hp::dead::stub_0x4a60a0 */

/* sub_4a60ab: native, nfs3hp::dead::stub_0x4a60ab */

/* sub_4a60d0: native, nfs3hp::dead::stub_0x4a60d0 */

/* sub_4a60e0: native, nfs3hp::ea::sendPacket */

/* sub_4a6390: native, nfs3hp::ea::resetNetwork */

/* sub_4a6410: native, nfs3hp::frontend::netEndpointHandlers */

/* sub_4a6440: native, nfs3hp::frontend::netDispatchPacket */

/* sub_4a6470: native, nfs3hp::frontend::netEndpointLost */

/* sub_4a64c0: native, nfs3hp::frontend::netEndpointTimedOut */

/* sub_4a6500: native, nfs3hp::frontend::netSessionReset */

/* sub_4a65b0: native, nfs3hp::frontend::netHostGone */

/* sub_4a65e0: native, nfs3hp::ea::closeNetwork */

/* sub_4a6650: native, nfs3hp::ea::machineNumber */

/* sub_4a6690: native, nfs3hp::ea::playerMachine */

/* sub_4a66c0: native, nfs3hp::frontend::netSendSetup */

/* sub_4a68d0: native, nfs3hp::frontend::netSetupPacket */

/* sub_4a69c0: native, nfs3hp::frontend::netClientLobby */

/* sub_4a6a10: native, nfs3hp::frontend::netBarrier */

/* sub_4a6c40: native, nfs3hp::frontend::netBarrierPacket */

/* sub_4a6ce0: native, nfs3hp::frontend::netCompareVersions */

/* sub_4a6df0: native, nfs3hp::frontend::netVersionPacket */

/* sub_4a6e10: native, nfs3hp::frontend::installFileRead */

/* sub_4a6f70: native, nfs3hp::frontend::loopNothingToEnd */

/* sub_4a7080: native, nfs3hp::frontend::loopPanelQuad */

/* sub_4a7310: native, nfs3hp::frontend::loopPanelTexturedQuad */

/* sub_4a75e0: native, nfs3hp::frontend::loopPanelDraw */

/* sub_4a78b6: native, nfs3hp::pieces::stub_0x4a78b6 */

/* sub_4a78d6: native, nfs3hp::handling::applyTuning */

/* sub_4a83af: native, nfs3hp::dead::stub_0x4a83af */

/* sub_4a83d3: native, nfs3hp::drivetrain::computerDriver */

/* sub_4a8a2b: native, nfs3hp::dead::stub_0x4a8a2b */

/* sub_4a8a99: native, nfs3hp::drivetrain::engineTorque */

/* sub_4a8bb9: native, nfs3hp::handling::automaticGear */

/* sub_4a90a0: native, nfs3hp::drivetrain::automaticGears */

/* sub_4a9769: native, nfs3hp::drivetrain::gearLimit */

/* sub_4a97cb: native, nfs3hp::drivetrain::autoReverse */

/* sub_4a9bc4: native, nfs3hp::drivetrain::driverInputs */

/* sub_4aa4f4: native, nfs3hp::handling::matchRevsToSpeed */

/* sub_4aa682: native, nfs3hp::dead::stub_0x4aa682 */

}
