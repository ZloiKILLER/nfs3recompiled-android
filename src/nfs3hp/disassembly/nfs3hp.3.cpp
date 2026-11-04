#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4103a0: native, nfs3hp::port::musicAfterSetting */

/* sub_410460: native, nfs3hp::port::musicMenuSix */

/* sub_4104b0: native, nfs3hp::frontend::carMenuMusicBack */

/* sub_4104f0: native, nfs3hp::port::musicStart */

/* sub_4107c0: native, nfs3hp::port::raceMusicStart */

/* sub_4109d0: native, nfs3hp::port::musicStop */

/* sub_410a40: native, nfs3hp::dead::stub_0x410a40 */

/* sub_410aa0: native, nfs3hp::port::musicStreamEnd */

/* sub_410ad0: native, nfs3hp::port::banksFree */

/* sub_410b20: native, nfs3hp::port::banksRemove */

/* sub_410b50: native, nfs3hp::port::bankLoad */

/* sub_410c30: native, nfs3hp::dead::stub_0x410c30 */

/* sub_410c40: native, nfs3hp::port::listenerMove */

/* sub_410cb0: native, nfs3hp::port::sourceDistance */

/* sub_410d30: native, nfs3hp::port::sourceAngle */

/* sub_410dc0: native, nfs3hp::port::carAngle */

/* sub_410e30: native, nfs3hp::port::trackSounds */

/* sub_411410: native, nfs3hp::port::raceSoundTick */

/* sub_411620: native, nfs3hp::port::sourceReset */

/* sub_411680: native, nfs3hp::port::sourcesReset */

/* sub_411750: native, nfs3hp::port::copChaseSet */

/* sub_4117a0: native, nfs3hp::port::playerCarSound */

/* sub_411c50: native, nfs3hp::port::copSiren */

/* sub_411ca0: native, nfs3hp::port::copSirenStop */

/* sub_411cb0: native, nfs3hp::port::copEngine */

/* sub_411da0: native, nfs3hp::port::copEngineStop */

/* sub_411db0: native, nfs3hp::port::copDistance */

/* sub_411e20: native, nfs3hp::port::copsCloseness */

/* sub_411e60: native, nfs3hp::port::copCarSound */

/* sub_4121b0: native, nfs3hp::port::copRemove */

/* sub_412220: native, nfs3hp::port::copSourcesPick */

/* sub_4124d0: native, nfs3hp::port::placeAhead */

/* sub_412620: native, nfs3hp::port::placeAcross */

/* sub_4127b0: native, nfs3hp::port::carFlagTest */

/* sub_4127c0: native, nfs3hp::port::carRadio */

/* sub_412880: native, nfs3hp::port::playerCopRadio */

/* sub_4129a0: native, nfs3hp::port::raceValue */

/* sub_4129b0: native, nfs3hp::port::soundNothing */

/* sub_4129c0: native, nfs3hp::port::raceSoundInit */

/* sub_412b50: native, nfs3hp::port::raceSoundEnd */

/* sub_412be0: native, nfs3hp::port::copLineFor */

/* sub_412d60: native, nfs3hp::port::raceAnnouncer */

/* sub_4136e0: native, nfs3hp::port::raceBankLoad */

/* sub_413750: native, nfs3hp::port::voiceBankLoad */

/* sub_4138e0: native, nfs3hp::port::carBankLoad */

/* sub_413bc0: native, nfs3hp::port::raceBanksLoad */

/* sub_414800: native, nfs3hp::port::slotsEmpty */

/* sub_414830: native, nfs3hp::port::engineSoundsStart */

/* sub_414940: native, nfs3hp::port::crashPitch */

/* sub_4149e0: native, nfs3hp::port::crashEffect */

/* sub_414d70: native, nfs3hp::port::scrapeEffect */

/* sub_414e10: native, nfs3hp::port::farEffectAim */

/* sub_414ef0: native, nfs3hp::port::farEffectPlay */

/* sub_414f60: native, nfs3hp::port::farEffectTick */

/* sub_414f90: native, nfs3hp::port::crowdEffect */

/* sub_415010: native, nfs3hp::port::carEffectFifteen */

/* sub_415050: native, nfs3hp::port::zoneSound */

/* sub_415210: native, nfs3hp::port::scenerySounds */

/* sub_416d90: native, nfs3hp::port::carKnock */

/* sub_416f60: native, nfs3hp::port::slotStop */

/* sub_416fc0: native, nfs3hp::port::slotEffect */

/* sub_417080: native, nfs3hp::port::slotPlay */

/* sub_417390: native, nfs3hp::port::carEngine */

/* sub_4179d0: native, nfs3hp::port::noiseBySpeed */

/* sub_417b80: native, nfs3hp::port::noiseByDistance */

/* sub_417c00: native, nfs3hp::port::playerNoise */

/* sub_417c70: native, nfs3hp::port::playerNoiseStop */

/* sub_417ca0: native, nfs3hp::port::sirenIsOn */

/* sub_417cb0: native, nfs3hp::port::sirenFirstTone */

/* sub_417d00: native, nfs3hp::port::sirenFirstDone */

/* sub_417d50: native, nfs3hp::dead::stub_0x417d50 */

/* sub_417db0: native, nfs3hp::port::sirenSecondTone */

/* sub_417e00: native, nfs3hp::port::sirenSteadyTone */

/* sub_417e60: native, nfs3hp::port::sirenSecondDone */

/* sub_417eb0: native, nfs3hp::port::sirenStart */

/* sub_417f60: native, nfs3hp::port::sirenStop */

/* sub_417fd0: native, nfs3hp::port::sirenTick */

/* sub_4181d0: native, nfs3hp::port::playMenuEffect */

/* sub_418280: native, nfs3hp::port::playCarEffectFive */

/* sub_4182b0: native, nfs3hp::port::playEffect */

/* sub_4184f0: native, nfs3hp::port::playSpeechEffect */

/* sub_4185e0: native, nfs3hp::port::panFromSide */

/* sub_418610: native, nfs3hp::port::masterSilent */

/* sub_418620: native, nfs3hp::port::masterBack */

/* sub_418670: native, nfs3hp::dead::stub_0x418670 */

/* sub_4186a0: native, nfs3hp::port::silenceForMenus */

/* sub_4186f0: native, nfs3hp::port::silenceForRace */

/* sub_418740: native, nfs3hp::port::busInit */

/* sub_418770: native, nfs3hp::port::busUp */

/* sub_418790: native, nfs3hp::port::busDown */

/* sub_4187a0: native, nfs3hp::port::slotsStop */

/* sub_4187e0: native, nfs3hp::port::musicMuteToggle */

/* sub_418840: native, nfs3hp::decompiled::swapRaceVolumes */

/* sub_4188a0: native, nfs3hp::port::copBeepTick */

/* sub_418930: native, nfs3hp::port::copBeepStart */

/* sub_418990: native, nfs3hp::port::chaseLevel */

/* sub_418c30: native, nfs3hp::port::copEffect */

/* sub_418c60: native, nfs3hp::port::musicEventResume */

/* sub_418cc0: native, nfs3hp::track::shellOpen */

/* sub_418cf0: native, nfs3hp::dead::stub_0x418cf0 */

}
