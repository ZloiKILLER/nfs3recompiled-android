#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_widescreen.py): defined in nfs3hp_main.cpp.
void dialogEmblem(bool drawing);

/* sub_440040: native, nfs3hp::frontend::ipxCreateGameClicked */

/* sub_4400b0: native, nfs3hp::frontend::ipxRefreshClicked */

/* sub_4400d0: native, nfs3hp::frontend::multiMenuOnBack */

/* sub_440160: native, nfs3hp::frontend::multiMenuApplyRaceList */

/* sub_440390: native, nfs3hp::frontend::multiMenuFollowRaceType */

/* sub_4403f0: native, nfs3hp::frontend::multiMenuOnFrame */

/* sub_440470: native, nfs3hp::frontend::multiMenuOnEnter */

/* sub_4405b0: native, nfs3hp::frontend::multiMenuOnExit */

/* sub_4405c0: native, nfs3hp::frontend::connectOnEnter */

/* sub_440730: native, nfs3hp::frontend::connectOnExit */

/* sub_440810: native, nfs3hp::frontend::ipxOnFrame */

/* sub_440890: native, nfs3hp::frontend::ipxOnEnter */

/* sub_440960: native, nfs3hp::frontend::ipxOnExit */

/* sub_440970: native, nfs3hp::frontend::loadScreenArt */

/* sub_440b30: native, nfs3hp::frontend::lobbyDrawPlayerIcon */

/* sub_440b50: native, nfs3hp::frontend::lobbyDrawPulsingIcon */

/* sub_440c40: native, nfs3hp::port::buttons::drawReadyMarks */

/* sub_440d20: native, nfs3hp::port::buttons::networkLeft */

/* sub_440d60: native, nfs3hp::frontend::lobbyPointAtPanel */

/* sub_440d80: native, nfs3hp::frontend::lobbyDrawRacePanel */

/* sub_441040: native, nfs3hp::frontend::lobbyPanelFrame */

/* sub_441680: native, nfs3hp::frontend::lobbyPanelBusy */

/* sub_441700: native, nfs3hp::frontend::lobbyPanelKey */

/* sub_4417f0: native, nfs3hp::dead::stub_0x4417f0 */

/* sub_4418e0: native, nfs3hp::frontend::lobbyPanelLineWidth */

/* sub_441900: native, nfs3hp::frontend::lobbyPanelLineFits */

/* sub_441950: native, nfs3hp::frontend::lobbyDrawMessages */

/* sub_441b20: native, nfs3hp::frontend::lobbyTypeInPanel */

/* sub_441cb0: native, nfs3hp::frontend::creditsRenderPage */

/* sub_442370: native, nfs3hp::frontend::creditsNextPage */

/* sub_4423e0: native, nfs3hp::frontend::creditsPreviousPage */

/* sub_442450: native, nfs3hp::port::buttons::drawLicence */

/* sub_442580: native, nfs3hp::frontend::creditsOnFrame */

/* sub_442850: native, nfs3hp::frontend::creditsOnEnter */

/* sub_442980: native, nfs3hp::frontend::creditsOnExit */

/* sub_4429c0: native, nfs3hp::port::menus::codelinkMatches */

/* sub_442a40: native, nfs3hp::frontend::menuFindItem */

/* sub_442aa0: native, nfs3hp::frontend::optionsOnEnter */

/* sub_442b10: native, nfs3hp::port::menus::assignHandlers */

/* sub_442b60: native, nfs3hp::frontend::postgameUnlockTrack */

/* sub_442b90: native, nfs3hp::port::buttons::listColours */

/* sub_442bf0: native, nfs3hp::port::buttons::listAllowed */

/* sub_442c50: native, nfs3hp::port::buttons::listLength */

/* sub_442c80: native, nfs3hp::port::buttons::findList */

/* sub_442d00: native, nfs3hp::port::buttons::menuEdited */

/* sub_442d20: native, nfs3hp::port::buttons::menuRecordEdited */

/* sub_442d30: native, nfs3hp::dead::stub_0x442d30 */

/* sub_442d50: native, nfs3hp::dead::stub_0x442d50 */

/* sub_442d60: native, nfs3hp::dead::stub_0x442d60 */

/* sub_442db0: native, nfs3hp::frontend::deadZoneCancelKey */

/* sub_442dd0: native, nfs3hp::frontend::deadZoneApplyAxis */

/* sub_442e60: native, nfs3hp::frontend::deadZoneAxisIndex */

/* sub_442ef0: native, nfs3hp::frontend::deadZoneAxisMoved */

/* sub_442ff0: native, nfs3hp::frontend::deadZoneAxisDraw */

/* sub_443070: native, nfs3hp::frontend::deadZoneShowController */

/* sub_443260: native, nfs3hp::frontend::deadZoneChooseKey */

/* sub_443290: native, nfs3hp::frontend::deadZoneChooseKeyInRace */

/* sub_4432c0: native, nfs3hp::frontend::deadZoneOnEnter */

/* sub_443430: native, nfs3hp::frontend::deadZoneOnExit */

/* sub_4434f0: native, nfs3hp::frontend::typedCharacterInRange */

/* sub_443510: native, nfs3hp::frontend::typedByteInRange */

/* sub_443520: native, nfs3hp::frontend::typedTextInRange */

/* sub_4435c0: native, nfs3hp::frontend::typedCharacterOfAddress */

/* sub_4435e0: native, nfs3hp::frontend::typedTextOfAddress */

/* sub_443620: native, nfs3hp::frontend::typedByteInRangeAgain */

/* sub_443630: native, nfs3hp::frontend::shortNameCharacter */

/* sub_443660: native, nfs3hp::frontend::shortNameValid */

/* sub_4436c0: native, nfs3hp::frontend::typedLanguageTestFirst */

/* sub_443700: native, nfs3hp::frontend::languageNameValid */

/* sub_443740: native, nfs3hp::frontend::typedLanguageTestSecond */

/* sub_443780: native, nfs3hp::frontend::languageFileNameValid */

/* sub_4437c0: native, nfs3hp::frontend::typedLanguageTestThird */

/* sub_443810: native, nfs3hp::frontend::typedCharacterAllowed */

/* sub_443860: native, nfs3hp::frontend::lineAllowed */

/* sub_4438b0: native, nfs3hp::port::dialogs::dialogShown */

/* sub_4438c0: native, nfs3hp::port::dialogs::dialogKind */

/* sub_4438d0: native, nfs3hp::port::dialogs::drawBox */

/* sub_443a90: native, nfs3hp::frontend::dialogLinesBox */

/* sub_443c00: native, nfs3hp::frontend::dialogButtonRowBox */

/* sub_443d20: native, nfs3hp::frontend::dialogPlaceButtons */

/* sub_443f40: native, nfs3hp::port::dialogs::drawLines */

/* sub_444000: native, nfs3hp::port::dialogs::drawButtons */

/* sub_444350: native, nfs3hp::port::dialogs::drawEditField */

/* sub_444570: native, nfs3hp::frontend::dialogPointAtButton */

/* sub_4445d0: native, nfs3hp::port::dialogs::buttonUnderMouse */

/* sub_4446c0: native, nfs3hp::frontend::dialogFocusedButton */

/* sub_4446d0: native, nfs3hp::frontend::dialogOpenQuestion */

/* sub_444910: native, nfs3hp::port::dialogs::questionKey */

/* sub_4449d0: native, nfs3hp::port::dialogs::questionPass */

/* sub_444a60: native, nfs3hp::port::dialogs::endMessage */

/* sub_444a80: native, nfs3hp::frontend::dialogOpenMessage */

/* sub_444b20: native, nfs3hp::port::dialogs::messageKey */

/* sub_444b60: native, nfs3hp::port::dialogs::messagePass */

/* sub_444be0: native, nfs3hp::frontend::dialogMessageById */

/* sub_444c10: native, nfs3hp::frontend::dialogEditText */

/* sub_444c20: native, nfs3hp::frontend::dialogOpenEdit */

/* sub_444f50: native, nfs3hp::port::dialogs::editKey */

/* sub_4450c0: native, nfs3hp::port::dialogs::editPass */

/* sub_445150: native, nfs3hp::port::dialogs::listFocus */

/* sub_445160: native, nfs3hp::port::dialogs::pointAtEntry */

}
