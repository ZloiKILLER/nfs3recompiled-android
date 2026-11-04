#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_44a660: native, nfs3hp::frontend::menuShownNow */

/* sub_44a680: native, nfs3hp::dead::stub_0x44a680 */

/* sub_44a690: native, nfs3hp::port::loop::releaseMouse */

/* sub_44a6a0: native, nfs3hp::dead::stub_0x44a6a0 */

/* sub_44a6b0: native, nfs3hp::port::loop::itemUnderMouse */

/* sub_44a860: native, nfs3hp::port::loop::itemsShown */

/* sub_44a8b0: native, nfs3hp::port::loop::passItems */

/* sub_44a960: native, nfs3hp::frontend::menuShowItems */

/* sub_44a9e0: native, nfs3hp::port::loop::hideItems */

/* sub_44aa30: native, nfs3hp::port::loop::leaveScreen */

/* sub_44aad0: native, nfs3hp::port::loop::resetItems */

/* sub_44ab40: native, nfs3hp::port::loop::enterScreen */

/* sub_44ac90: native, nfs3hp::port::buttons::loadShapes */

/* sub_44ad60: native, nfs3hp::dead::stub_0x44ad60 */

/* sub_44adb0: native, nfs3hp::dead::stub_0x44adb0 */

/* sub_44ae00: native, nfs3hp::frontend::menuCurrentName */

/* sub_44ae20: native, nfs3hp::dead::stub_0x44ae20 */

/* sub_44ae60: native, nfs3hp::port::loop::menuByName */

/* sub_44aeb0: native, nfs3hp::port::loop::currentMenu */

/* sub_44aec0: native, nfs3hp::port::loop::popMenu */

/* sub_44af10: native, nfs3hp::port::loop::pushMenu */

/* sub_44af50: native, nfs3hp::port::loop::setHistory */

/* sub_44af90: native, nfs3hp::port::loop::player1NameAnswer */

/* sub_44b040: native, nfs3hp::port::loop::player2NameAnswer */

/* sub_44b0f0: native, nfs3hp::port::loop::quitAnswer */

/* sub_44b170: native, nfs3hp::port::buttons::redrawMenu */

/* sub_44b1a0: native, nfs3hp::port::loop::waitWhileSuspended */

/* sub_44b230: native, nfs3hp::port::loop::menuLoop */

/* sub_44b950: native, nfs3hp::frontend::racePauseBegin */

/* sub_44ba30: native, nfs3hp::frontend::racePauseAgain */

/* sub_44bac0: native, nfs3hp::frontend::racePauseFrame */

/* sub_44bdd0: native, nfs3hp::frontend::menuDrawPass */

/* sub_44be20: native, nfs3hp::frontend::menuLeave */

/* sub_44be60: native, nfs3hp::port::loop::fitRaceSettings */

/* sub_44c2d0: native, nfs3hp::frontend::pauseHideSpare */

/* sub_44c360: native, nfs3hp::frontend::pauseHideForCop */

/* sub_44c4e0: native, nfs3hp::frontend::pauseHideForRacer */

/* sub_44c610: native, nfs3hp::frontend::pauseLeaveAnswered */

/* sub_44c670: native, nfs3hp::frontend::pauseRestartAnswered */

/* sub_44c6d0: native, nfs3hp::frontend::pauseAskLeave */

/* sub_44c730: native, nfs3hp::frontend::pauseQuitChoiceAnswered */

/* sub_44c7b0: native, nfs3hp::frontend::pauseQuitClicked */

/* sub_44ca10: native, nfs3hp::frontend::pauseRestartClicked */

/* sub_44ca80: native, nfs3hp::frontend::pauseSaveClicked */

/* sub_44cab0: native, nfs3hp::frontend::pauseOnEnter */

/* sub_44cf60: native, nfs3hp::frontend::pauseOnExit */

/* sub_44cf70: native, nfs3hp::frontend::pauseIntensityMoved */

/* sub_44cfa0: native, nfs3hp::frontend::pauseMapCopMoved */

/* sub_44cfe0: native, nfs3hp::frontend::pauseMapRacerMoved */

/* sub_44d020: native, nfs3hp::frontend::pauseMapFirstCopMoved */

/* sub_44d060: native, nfs3hp::frontend::pauseMapFirstRacerMoved */

/* sub_44d0a0: native, nfs3hp::frontend::pauseMapSecondCopMoved */

/* sub_44d0e0: native, nfs3hp::frontend::pauseMapSecondRacerMoved */

/* sub_44d120: native, nfs3hp::frontend::pauseOptionsOnEnter */

/* sub_44d160: native, nfs3hp::frontend::pauseOptionsOnExit */

/* sub_44d170: native, nfs3hp::frontend::pauseControlsCancelKey */

/* sub_44d190: native, nfs3hp::frontend::pauseControlsOnEnter */

/* sub_44d220: native, nfs3hp::frontend::pauseControlsOnExit */

/* sub_44d270: native, nfs3hp::frontend::pauseWidescreenClicked */

/* sub_44d2a0: native, nfs3hp::frontend::pauseGraphicsOnEnter */

/* sub_44d3e0: native, nfs3hp::frontend::pauseGraphicsOnFrame */

/* sub_44d420: native, nfs3hp::frontend::pauseGraphicsOnExit */

/* sub_44d4c0: native, nfs3hp::frontend::pauseHudOnEnter */

/* sub_44d670: native, nfs3hp::frontend::pauseHudOnExit */

/* sub_44d7a0: native, nfs3hp::frontend::pauseHud1OnEnter */

/* sub_44d9a0: native, nfs3hp::frontend::pauseHud1OnExit */

/* sub_44dad0: native, nfs3hp::frontend::pauseHud2OnEnter */

/* sub_44dcd0: native, nfs3hp::frontend::pauseHud2OnExit */

/* sub_44de00: native, nfs3hp::frontend::hudEditDoneKey */

/* sub_44de20: native, nfs3hp::frontend::hudEditResetKey */

/* sub_44de80: native, nfs3hp::frontend::hudEditLayoutOnEnter */

/* sub_44dfd0: native, nfs3hp::frontend::hudEditLayoutOnExit */

/* sub_44e130: native, nfs3hp::frontend::pauseAiTunerOnEnter */

/* sub_44e140: native, nfs3hp::frontend::pauseAiTunerOnExit */

/* sub_44e170: native, nfs3hp::frontend::postgameAwardMessage */

/* sub_44e1a0: native, nfs3hp::frontend::postgameCheckAwards */

/* sub_44e460: native, nfs3hp::frontend::postgameSaveKnockoutClicked */

/* sub_44e480: native, nfs3hp::frontend::postgameSaveTournamentClicked */

/* sub_44e4a0: native, nfs3hp::frontend::postgameSaveGhostClicked */

/* sub_44e4c0: native, nfs3hp::frontend::postgameLeaveNetwork */

/* sub_44e510: native, nfs3hp::frontend::postgameLeaveAnswered */

/* sub_44e550: native, nfs3hp::frontend::postgameAskLeave */

/* sub_44e5b0: native, nfs3hp::frontend::postgameMainMenuClicked */

/* sub_44e600: native, nfs3hp::frontend::postgameNetworkMainMenuClicked */

/* sub_44e690: native, nfs3hp::frontend::postgameQuitClicked */

/* sub_44e6a0: native, nfs3hp::frontend::postgameSaveReplayClicked */

/* sub_44e6c0: native, nfs3hp::frontend::postgameCannotSaveReplay */

/* sub_44e710: native, nfs3hp::frontend::postgameViewReplayClicked */

/* sub_44e7a0: native, nfs3hp::frontend::postgameToggleSheets */

/* sub_44e7d0: native, nfs3hp::frontend::awardsClear */

/* sub_44e7f0: native, nfs3hp::frontend::awardsGrant */

/* sub_44e820: native, nfs3hp::frontend::awardsForRace */

/* sub_44ea60: native, nfs3hp::frontend::awardsSpeak */

/* sub_44ed30: native, nfs3hp::frontend::awardsSpeakForSheet */

/* sub_44eda0: native, nfs3hp::frontend::postgameOnFrame */

/* sub_44efd0: native, nfs3hp::frontend::postgameOnEnter */

/* sub_44f500: native, nfs3hp::frontend::postgameOnExit */

/* sub_44f510: native, nfs3hp::load::loadRecords */

/* sub_44f5d0: native, nfs3hp::frontend::recordsLoaded */

/* sub_44f5f0: native, nfs3hp::load::saveRecords */

}
