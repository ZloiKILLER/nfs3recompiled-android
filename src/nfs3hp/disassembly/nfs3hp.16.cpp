#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_458b80: native, nfs3hp::port::fields::floatFieldKey */

/* sub_458ce0: native, nfs3hp::port::fields::layoutFloatField */

/* sub_458d90: native, nfs3hp::port::fields::floatFieldShown */

/* sub_458dd0: native, nfs3hp::port::fields::floatFieldHidden */

/* sub_458e00: native, nfs3hp::port::fields::bindFloatField */

/* sub_458e30: native, nfs3hp::port::fields::drawFixedField */

/* sub_459020: native, nfs3hp::port::fields::fixedRangeMessage */

/* sub_459090: native, nfs3hp::port::fields::fixedFieldKey */

/* sub_459210: native, nfs3hp::port::fields::layoutFixedField */

/* sub_4592b0: native, nfs3hp::port::fields::fixedFieldShown */

/* sub_4592f0: native, nfs3hp::port::fields::fixedFieldHidden */

/* sub_459320: native, nfs3hp::port::fields::bindFixedField */

/* sub_459350: native, nfs3hp::graphics::drawFlatRectangle */

/* sub_459410: native, nfs3hp::frontend::loopItemFrame */

/* sub_459440: native, nfs3hp::graphics::hudEditorFrame */

/* sub_459670: native, nfs3hp::frontend::hudEditElementFits */

/* sub_459db0: native, nfs3hp::frontend::hudEditElementMoved */

/* sub_459fe0: native, nfs3hp::frontend::hudEditorDrop */

/* sub_45a120: native, nfs3hp::frontend::hudEditorDrag */

/* sub_45ab30: native, nfs3hp::frontend::hudDrawElement */

/* sub_45b940: native, nfs3hp::frontend::hudElementPickCursor */

/* sub_45bcb0: native, nfs3hp::frontend::hudElementDraw */

/* sub_45bd20: native, nfs3hp::frontend::hudElementKey */

/* sub_45c9d0: native, nfs3hp::frontend::hudElementToRecord */

/* sub_45ca90: native, nfs3hp::frontend::hudEditPlaceElement */

/* sub_45cb50: native, nfs3hp::frontend::hudElementHidden */

/* sub_45cdd0: native, nfs3hp::frontend::mainMenuPlayerNameAnswer */

/* sub_45ce60: native, nfs3hp::frontend::splitMenuPlayer2NameAnswer */

/* sub_45cef0: native, nfs3hp::frontend::mainMenuOnPlayerName */

/* sub_45cf70: native, nfs3hp::frontend::splitMenuOnPlayer2Name */

/* sub_45cff0: native, nfs3hp::frontend::mainMenuOnFrame */

/* sub_45d050: native, nfs3hp::frontend::mainMenuOnEnter */

/* sub_45d2f0: native, nfs3hp::frontend::mainMenuOnExit */

/* sub_45d300: native, nfs3hp::frontend::splitMenuOnFrame */

/* sub_45d360: native, nfs3hp::frontend::splitMenuOnEnter */

/* sub_45d620: native, nfs3hp::frontend::splitMenuOnExit */

/* sub_45d630: native, nfs3hp::frontend::levelOnEnter */

/* sub_45d770: native, nfs3hp::frontend::pauseCameraAnswered */

/* sub_45d840: native, nfs3hp::frontend::pauseCameraKey */

/* sub_45d920: native, nfs3hp::frontend::pauseCameraOnEnter */

/* sub_45d9a0: native, nfs3hp::frontend::pauseCameraEnsureOne */

/* sub_45d9e0: native, nfs3hp::frontend::pauseCameraOnExit */

/* sub_45dab0: native, nfs3hp::frontend::raceTypeOnExit */

/* sub_45dae0: native, nfs3hp::frontend::hudEditShowSpeedometer */

/* sub_45deb0: native, nfs3hp::frontend::hudEditShowTachometer */

/* sub_45e160: native, nfs3hp::frontend::hudEditGreyMirrors */

/* sub_45e210: native, nfs3hp::frontend::hudEditGreyOpponentNames */

/* sub_45e310: native, nfs3hp::frontend::hudEditPlaceImages */

/* sub_45e3d0: native, nfs3hp::frontend::hudEditShowScreenItems */

/* sub_45e960: native, nfs3hp::frontend::hudEditOnEnter */

/* sub_45f160: native, nfs3hp::frontend::hudEditOnExit */

/* sub_45f300: native, nfs3hp::frontend::hudEditSetScreenArt */

/* sub_45f4c0: native, nfs3hp::frontend::hudEditOnFrame */

/* sub_45fae0: native, nfs3hp::port::buttons::rectDraw */

/* sub_45fc90: native, nfs3hp::port::buttons::speechDraw */

/* sub_45fca0: native, nfs3hp::port::buttons::speechShown */

/* sub_45fcb0: native, nfs3hp::port::buttons::speechHidden */

/* sub_45fcd0: native, nfs3hp::port::buttons::textDraw */

/* sub_460070: native, nfs3hp::port::buttons::textUp */

/* sub_4600b0: native, nfs3hp::port::buttons::textShown */

/* sub_460100: native, nfs3hp::port::buttons::imageDraw */

/* sub_460410: native, nfs3hp::port::buttons::imageUp */

/* sub_460440: native, nfs3hp::port::buttons::imageShown */

/* sub_460700: native, nfs3hp::port::buttons::pulseOn */

/* sub_460760: native, nfs3hp::frontend::resetButtonPulse */

/* sub_460770: native, nfs3hp::port::buttons::pulseColour */

/* sub_4607c0: native, nfs3hp::port::buttons::buttonDraw */

/* sub_461270: native, nfs3hp::port::buttons::buttonKey */

/* sub_4613e0: native, nfs3hp::port::buttons::layoutMenuItem */

/* sub_461900: native, nfs3hp::port::buttons::buttonShown */

/* sub_461a70: native, nfs3hp::port::buttons::cascadeLayoutClosed */

/* sub_461aa0: native, nfs3hp::port::buttons::cascadeLineText */

/* sub_461d00: native, nfs3hp::port::buttons::cascadeDraw */

/* sub_462320: native, nfs3hp::port::buttons::cascadePass */

/* sub_4623a0: native, nfs3hp::port::buttons::cascadeShown */

/* sub_4624a0: native, nfs3hp::port::buttons::cascadeKey */

/* sub_4628d0: native, nfs3hp::port::buttons::cascadeUp */

/* sub_4629c0: native, nfs3hp::port::buttons::checkboxDraw */

/* sub_462ad0: native, nfs3hp::port::buttons::checkboxKey */

/* sub_462b20: native, nfs3hp::port::buttons::checkboxShown */

/* sub_462b30: native, nfs3hp::port::buttons::checkboxUp */

/* sub_462b90: native, nfs3hp::port::buttons::boxOverlayDraw */

/* sub_462c30: native, nfs3hp::port::buttons::selectListShown */

/* sub_462ce0: native, nfs3hp::port::buttons::selectListHidden */

/* sub_462d20: native, nfs3hp::port::buttons::selectListDraw */

/* sub_462f90: native, nfs3hp::port::buttons::selectListPass */

/* sub_462fa0: native, nfs3hp::port::buttons::selectListKey */

/* sub_4633d0: native, nfs3hp::port::buttons::selectListUp */

/* sub_4634d0: native, nfs3hp::port::buttons::dropListDraw */

/* sub_463de0: native, nfs3hp::port::buttons::dropListPass */

/* sub_463e60: native, nfs3hp::port::buttons::dropListShown */

/* sub_463ff0: native, nfs3hp::port::buttons::listKey */

/* sub_464560: native, nfs3hp::port::buttons::layoutList */

/* sub_4646e0: native, nfs3hp::port::view::storeIncremented */

/* sub_464700: native, nfs3hp::frontend::viewMakeTiles */

/* sub_4647a0: native, nfs3hp::frontend::viewFillTiles */

/* sub_464840: native, nfs3hp::port::view::gridDrawn */

/* sub_464eb0: native, nfs3hp::frontend::viewLoadInterior */

/* sub_464fd0: native, nfs3hp::port::view::viewFrame */

/* sub_4651e0: native, nfs3hp::port::view::panKey */

}
