#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_44f670: native, nfs3hp::frontend::recordsSelect */

/* sub_44f6e0: native, nfs3hp::frontend::recordsCompare */

/* sub_44f830: native, nfs3hp::frontend::recordsSortBy */

/* sub_44f8d0: native, nfs3hp::frontend::recordsCount */

/* sub_44f910: native, nfs3hp::frontend::recordByRank */

/* sub_44f960: native, nfs3hp::frontend::recordsTimeQualifies */

/* sub_44f9b0: native, nfs3hp::frontend::recordsClearMarks */

/* sub_44fa10: native, nfs3hp::frontend::recordsInsert */

/* sub_44fa90: native, nfs3hp::frontend::recordsRead */

/* sub_44fb20: native, nfs3hp::frontend::recordsHolderMark */

/* sub_44fb30: native, nfs3hp::frontend::recordsSetHolder */

/* sub_44fbc0: native, nfs3hp::frontend::recordsClear */

/* sub_44fc00: native, nfs3hp::frontend::recordsSheetSortMark */

/* sub_44fc70: native, nfs3hp::frontend::recordsSheetDraw */

/* sub_450770: native, nfs3hp::frontend::recordsSheetLoad */

/* sub_450950: native, nfs3hp::frontend::recordsSheetSave */

/* sub_450970: native, nfs3hp::frontend::recordsSheetLayout */

/* sub_450a90: native, nfs3hp::frontend::recordsSheetKey */

/* sub_450e30: native, nfs3hp::frontend::recordsSheetIdle */

/* sub_450e50: native, nfs3hp::frontend::recordsDeleteAnswered */

/* sub_450eb0: native, nfs3hp::frontend::recordsDeleteClicked */

/* sub_450f20: native, nfs3hp::frontend::recordsOnFrame */

/* sub_450f70: native, nfs3hp::frontend::recordsOnEnter */

/* sub_450fd0: native, nfs3hp::frontend::recordsOnExit */

/* sub_450fe0: native, nfs3hp::frontend::recordsEnterResult */

/* sub_4512f0: native, nfs3hp::frontend::recordsEnterPlayers */

/* sub_4513c0: native, nfs3hp::port::text::setTextColour */

/* sub_4513e0: native, nfs3hp::port::fields::setCursorColour */

/* sub_451400: native, nfs3hp::port::text::toScreenX */

/* sub_451440: native, nfs3hp::port::text::toScreenY */

/* sub_451480: native, nfs3hp::port::text::screenToMenuX */

/* sub_4514c0: native, nfs3hp::port::text::screenToMenuY */

/* sub_451500: native, nfs3hp::port::text::readMenuMouse */

/* sub_451530: native, nfs3hp::port::text::dimBackground */

/* sub_451590: native, nfs3hp::port::text::drawTextAt */

/* sub_4515e0: native, nfs3hp::port::fields::drawFieldText */

/* sub_451640: native, nfs3hp::dead::stub_0x451640 */

/* sub_451670: native, nfs3hp::dead::stub_0x451670 */

/* sub_4516a0: native, nfs3hp::frontend::loadingDiskHasRoom */

/* sub_451700: native, nfs3hp::port::text::joystickKey */

/* sub_451715: native, nfs3hp::dead::stub_0x451715 */

/* sub_4517d0: native, nfs3hp::dead::stub_0x4517d0 */

/* sub_451810: native, nfs3hp::port::text::captureKeys */

/* sub_451840: native, nfs3hp::port::text::scanCodeOfKey */

/* sub_4518c0: native, nfs3hp::port::text::keyOfScanCode */

/* sub_451940: native, nfs3hp::frontend::connectFlushKeys */

/* sub_451960: native, nfs3hp::port::text::menuKey */

/* sub_451b20: native, nfs3hp::port::text::resetMenuKey */

/* sub_451b50: native, nfs3hp::port::text::nextWord */

/* sub_451cd0: native, nfs3hp::port::text::drawWrappedText */

/* sub_451fc0: native, nfs3hp::port::text::drawTextBox */

/* sub_451ff0: native, nfs3hp::port::text::measureTextBox */

/* sub_452020: native, nfs3hp::port::text::textWidth */

/* sub_452090: native, nfs3hp::frontend::textFit */

/* sub_452100: native, nfs3hp::port::text::selectFont */

/* sub_452110: native, nfs3hp::port::text::menusEnd */

/* sub_452120: native, nfs3hp::frontend::textDrawFitted */

/* sub_452190: native, nfs3hp::port::text::drawMenuText */

/* sub_452220: native, nfs3hp::port::fields::drawFieldLine */

/* sub_4522c0: native, nfs3hp::port::fields::drawSpacedText */

/* sub_452410: native, nfs3hp::frontend::menuCaptureScreen */

/* sub_452420: native, nfs3hp::frontend::cheatAdvance */

/* sub_4524e0: native, nfs3hp::frontend::cheatKey */

/* sub_452910: native, nfs3hp::dead::stub_0x452910 */

/* sub_4529a0: native, nfs3hp::port::menus::appendBackslash */

/* sub_452a00: native, nfs3hp::frontend::loadingCopyFile */

/* sub_452ac0: native, nfs3hp::frontend::setupRaceAnswered */

/* sub_452bd0: native, nfs3hp::frontend::setupAskRace */

/* sub_452d40: native, nfs3hp::frontend::setupTwoPlayersClicked */

/* sub_452d80: native, nfs3hp::frontend::setupWebSiteRefused */

/* sub_452db0: native, nfs3hp::frontend::setupIsCurrent */

/* sub_452de0: native, nfs3hp::frontend::setupWebSiteAnswered */

/* sub_452e60: native, nfs3hp::frontend::setupWebSiteClicked */

/* sub_452f20: native, nfs3hp::frontend::setupOnePlayerClicked */

/* sub_452f60: native, nfs3hp::frontend::setupOnFrame */

/* sub_452f80: native, nfs3hp::frontend::setupOnEnter */

/* sub_4530c0: native, nfs3hp::frontend::carArtExists */

/* sub_453160: native, nfs3hp::frontend::showcaseHasInterior */

/* sub_453200: native, nfs3hp::frontend::showcaseHasSlides */

/* sub_453290: native, nfs3hp::frontend::showcaseHasHistory */

/* sub_453320: native, nfs3hp::frontend::showcaseLoadArt */

/* sub_453440: native, nfs3hp::dead::stub_0x453440 */

/* sub_453470: native, nfs3hp::dead::stub_0x453470 */

/* sub_4534a0: native, nfs3hp::dead::stub_0x4534a0 */

/* sub_4534d0: native, nfs3hp::frontend::carViewRemember */

/* sub_453500: native, nfs3hp::frontend::showcaseResolveCar */

/* sub_4535e0: native, nfs3hp::frontend::showcaseOnEnter */

/* sub_4537d0: native, nfs3hp::frontend::showcaseOnFrame */

/* sub_453830: native, nfs3hp::frontend::showcaseOnExit */

/* sub_453840: native, nfs3hp::frontend::showcaseNextSlide */

/* sub_453860: native, nfs3hp::frontend::showcasePreviousSlide */

/* sub_453940: native, nfs3hp::frontend::carStatsDraw */

/* sub_453c00: native, nfs3hp::frontend::showcaseSlidesOnEnter */

/* sub_453ef0: native, nfs3hp::frontend::showcaseSlidesOnFrame */

/* sub_453f50: native, nfs3hp::frontend::showcaseSlidesOnExit */

/* sub_453f60: native, nfs3hp::frontend::carSpecsDraw */

/* sub_454000: native, nfs3hp::frontend::showcaseHistoryPage */

/* sub_454210: native, nfs3hp::frontend::showcaseHistoryPageFrom */

/* sub_454260: native, nfs3hp::frontend::showcaseHistoryOnEnter */

/* sub_454310: native, nfs3hp::frontend::showcaseHistoryOnFrame */

}
