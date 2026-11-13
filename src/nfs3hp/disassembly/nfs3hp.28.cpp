#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4c7a60: native, nfs3hp::decompiled::setDepthOffsets */

/* sub_4c7b10: native, nfs3hp::frontend::simDepthOffsets */

/* sub_4c7b20: native, nfs3hp::frontend::raceEffectsEndEmpty */

/* sub_4c7b30: native, nfs3hp::frontend::inpaQueueLockMake */

/* sub_4c7b50: native, nfs3hp::frontend::raceQueuesClear */

/* sub_4c7bb0: native, nfs3hp::frontend::inpaQueueLockFree */

/* sub_4c7bd0: native, nfs3hp::frontend::simNetSendPlace */

/* sub_4c7d30: native, nfs3hp::frontend::netInputPacket */

/* sub_4c7e50: native, nfs3hp::frontend::netSendPlayerGone */

/* sub_4c7e80: native, nfs3hp::frontend::netPlayerLeftPacket */

/* sub_4c7ea0: native, nfs3hp::frontend::netSendRaceResults */

/* sub_4c7f90: native, nfs3hp::frontend::netResultsPacket */

/* sub_4c7fd0: native, nfs3hp::frontend::netClearRing */

/* sub_4c8040: native, nfs3hp::frontend::netPlayerControls */

/* sub_4c81c0: native, nfs3hp::frontend::netTakeInputsOnTick */

/* sub_4c8200: native, nfs3hp::dead::stub_0x4c8200 */

/* sub_4c8250: native, nfs3hp::frontend::netTakeInputs */

/* sub_4c8310: native, nfs3hp::frontend::netPackControls */

/* sub_4c8350: native, nfs3hp::frontend::netUnpackControls */

/* sub_4c8370: native, nfs3hp::port::speechInit */

/* sub_4c8440: native, nfs3hp::port::speechFree */

/* sub_4c8470: native, nfs3hp::frontend::trackPlaceNear */

/* sub_4c8550: native, nfs3hp::frontend::trackPlaceAt */

/* sub_4c85a0: native, nfs3hp::port::speechLine */

/* sub_4c86d0: native, nfs3hp::port::speechTick */

/* sub_4c8810: native, nfs3hp::port::radioTick */

/* sub_4c89b0: native, nfs3hp::port::radioStart */

/* sub_4c8ac0: native, nfs3hp::port::speechSaid */

/* sub_4c8b30: native, nfs3hp::port::copRadio */

/* sub_4c8ca0: native, nfs3hp::port::queueSpeechLine */

/* sub_4c8d10: native, nfs3hp::port::coDriverTick */

/* sub_4c8f90: native, nfs3hp::port::queueRaceLine */

/* sub_4c9010: native, nfs3hp::port::raceEventLine */

/* sub_4c9090: native, nfs3hp::port::gapLine */

/* sub_4c91a0: native, nfs3hp::port::lapTimeLine */

/* sub_4c9240: native, nfs3hp::snd::descriptionLoad */

/* sub_4c9340: native, nfs3hp::port::descriptionSpeak */

/* sub_4c9380: native, nfs3hp::port::descriptionTick */

/* sub_4c9490: native, nfs3hp::port::descriptionStop */

/* sub_4c94e0: native, nfs3hp::frontend::infoSpeak */

/* sub_4c9590: native, nfs3hp::frontend::infoStopSpeaking */

/* sub_4c95e0: native, nfs3hp::port::spokenStatus */

/* sub_4c95f0: native, nfs3hp::port::descriptionStart */

/* sub_4c9860: native, nfs3hp::port::descriptionService */

/* sub_4c9890: native, nfs3hp::port::raceVoiceLoad */

/* sub_4c9990: native, nfs3hp::port::carLineSpeak */

/* sub_4c99d0: native, nfs3hp::port::spokenStop */

/* sub_4c99f0: native, nfs3hp::port::spokenStatus */

/* sub_4c9a00: native, nfs3hp::port::raceVoiceFree */

/* sub_4c9a80: native, nfs3hp::frontend::trackLaneOffset */

/* sub_4c9bc0: native, nfs3hp::frontend::simPlaceInLane */

/* sub_4c9d70: native, nfs3hp::frontend::simRaceClearA */

/* sub_4c9d90: native, nfs3hp::frontend::raceEndClearRecords */

/* sub_4c9db0: native, nfs3hp::frontend::simRaceClearB */

/* sub_4c9dd0: native, nfs3hp::frontend::simSirenOn */

/* sub_4c9f50: native, nfs3hp::frontend::simSirenOff */

/* sub_4c9f70: native, nfs3hp::frontend::rendBlinkingQuads */

/* sub_4ca160: native, nfs3hp::frontend::raceStripUnderWheels */

/* sub_4ca480: native, nfs3hp::pieces::stub_0x4ca480 */

/* sub_4ca4b0: native, nfs3hp::frontend::effectsParticleSlot */

/* sub_4ca500: native, nfs3hp::frontend::effectsDropsOnView */

/* sub_4ca850: native, nfs3hp::frontend::effectsParticleSpawn */

/* sub_4cab60: native, nfs3hp::frontend::effectsParticleChildren */

/* sub_4cac20: native, nfs3hp::frontend::effectsSparks */

/* sub_4cae00: native, nfs3hp::frontend::effectsCrashPuff */

/* sub_4caea0: native, nfs3hp::frontend::effectsRespawnSparkle */

/* sub_4cb010: native, nfs3hp::frontend::effectsWheelParticles */

/* sub_4cb220: native, nfs3hp::frontend::rendFadeParticles */

/* sub_4cb2c0: native, nfs3hp::vertices::moveParticles */

/* sub_4cb590: native, nfs3hp::frontend::rendLoadParticleTextures */

/* sub_4cb670: native, nfs3hp::render::clearBuffers */

/* sub_4cb6f0: native, nfs3hp::frontend::rendClearParticles */

/* sub_4cb730: native, nfs3hp::frontend::rendClearParticleStates */

/* sub_4cb750: native, nfs3hp::vertices::drops */

/* sub_4cbc90: native, nfs3hp::vertices::particles */

/* sub_4cc540: native, nfs3hp::frontend::rendParticlesCameraChanged */

/* sub_4cc5c0: native, nfs3hp::frontend::rendParticlesPass */

/* sub_4cc600: native, nfs3hp::frontend::rendDropsPass */

/* sub_4cc650: native, nfs3hp::frontend::winaCompareHandlers */

/* sub_4cc660: native, nfs3hp::frontend::winaHandlerOf */

/* sub_4cc6d0: native, nfs3hp::frontend::winaHandlerSet */

/* sub_4cc7c0: native, nfs3hp::frontend::winaPostMessage */

/* sub_4cc820: native, nfs3hp::dead::stub_0x4cc820 */

/* sub_4cc840: native, nfs3hp::dead::stub_0x4cc840 */

/* sub_4cc860: native, nfs3hp::frontend::winaSetFocusCallback */

/* sub_4cc870: native, nfs3hp::frontend::winaNoWindowTask */

/* sub_4cc890: native, nfs3hp::frontend::winaMovePointer */

/* sub_4cc8e0: native, nfs3hp::frontend::winaPushKey */

/* sub_4cc970: native, nfs3hp::frontend::nextKey */

/* sub_4cca00: native, nfs3hp::dead::stub_0x4cca00 */

/* sub_4ccaa0: native, nfs3hp::frontend::winaKeyOfMessage */

/* sub_4ccbb0: native, nfs3hp::dead::stub_0x4ccbb0 */

/* sub_4ccbb7: native, nfs3hp::dead::stub_0x4ccbb7 */

/* sub_4ccbd7: native, nfs3hp::dead::stub_0x4ccbd7 */

/* sub_4ccbd0: native, nfs3hp::dead::stub_0x4ccbd0 */

/* sub_4ccbc9: native, nfs3hp::dead::stub_0x4ccbc9 */

/* sub_4ccbde: native, nfs3hp::dead::stub_0x4ccbde */

/* sub_4ccbe5: native, nfs3hp::dead::stub_0x4ccbe5 */

/* sub_4ccbec: native, nfs3hp::dead::stub_0x4ccbec */

/* sub_4ccbf3: native, nfs3hp::dead::stub_0x4ccbf3 */

}
