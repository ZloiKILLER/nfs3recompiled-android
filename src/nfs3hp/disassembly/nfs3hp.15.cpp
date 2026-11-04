#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4543e0: native, nfs3hp::frontend::showcaseHistoryOnExit */

/* sub_4543f0: native, nfs3hp::frontend::showcaseComparePicture */

/* sub_454500: native, nfs3hp::frontend::showcaseCompareOnEnter */

/* sub_454580: native, nfs3hp::frontend::showcaseCompare2OnEnter */

/* sub_454660: native, nfs3hp::frontend::showcaseCompare2OnFrame */

/* sub_454690: native, nfs3hp::frontend::showcaseCompareOnFrame */

/* sub_454710: native, nfs3hp::frontend::showcaseCompare2OnExit */

/* sub_454730: native, nfs3hp::frontend::carPassesFilter */

/* sub_454780: native, nfs3hp::frontend::carListDraw */

/* sub_4549c0: native, nfs3hp::frontend::carCompareDraw */

/* sub_454bf0: native, nfs3hp::frontend::carCompareLoadArt */

/* sub_454cf0: native, nfs3hp::frontend::showcaseZoomMoved */

/* sub_454d10: native, nfs3hp::frontend::showcaseInteriorOnEnter */

/* sub_454d40: native, nfs3hp::frontend::showcaseInteriorOnFrame */

/* sub_454d50: native, nfs3hp::frontend::showcaseInteriorOnExit */

/* sub_454d60: native, nfs3hp::frontend::credits1ShowPage */

/* sub_454e40: native, nfs3hp::frontend::credits1NextClicked */

/* sub_454e80: native, nfs3hp::frontend::credits1OnExit */

/* sub_454ea0: native, nfs3hp::frontend::credits1OnEnter */

/* sub_455020: native, nfs3hp::frontend::resultsHiddenRemote */

/* sub_455060: native, nfs3hp::frontend::resultsNetworkNothing */

/* sub_455070: native, nfs3hp::frontend::tournamentScore */

/* sub_455140: native, nfs3hp::frontend::knockoutEliminate */

/* sub_455210: native, nfs3hp::frontend::resultsCollectFinishers */

/* sub_455280: native, nfs3hp::frontend::resultsCollectLaps */

/* sub_4552f0: native, nfs3hp::frontend::resultsAssign */

/* sub_455410: native, nfs3hp::frontend::resultsBuild */

/* sub_4554e0: native, nfs3hp::frontend::resultsParticipantById */

/* sub_455530: native, nfs3hp::frontend::resultsParticipantRanks */

/* sub_455570: native, nfs3hp::frontend::resultsName */

/* sub_4555c0: native, nfs3hp::frontend::resultsPlaceText */

/* sub_455600: native, nfs3hp::frontend::resultsCarName */

/* sub_455660: native, nfs3hp::frontend::resultsCarLine0 */

/* sub_4556c0: native, nfs3hp::frontend::resultsCarLine1 */

/* sub_455720: native, nfs3hp::frontend::resultsTopSpeedText */

/* sub_4557b0: native, nfs3hp::frontend::resultsParticipantMatches */

/* sub_455840: native, nfs3hp::frontend::resultsCompare */

/* sub_455c10: native, nfs3hp::frontend::resultsSort */

/* sub_455cd0: native, nfs3hp::frontend::resultsNth */

/* sub_455dd0: native, nfs3hp::frontend::resultsNthSorted */

/* sub_455e30: native, nfs3hp::port::buttons::textBoxDraw */

/* sub_456020: native, nfs3hp::port::buttons::textBoxKey */

/* sub_456110: native, nfs3hp::port::buttons::textBoxUp */

/* sub_456170: native, nfs3hp::port::buttons::textBoxShown */

/* sub_4561d0: native, nfs3hp::port::buttons::textBoxHidden */

/* sub_456200: native, nfs3hp::frontend::infoPlaceTrackPlaces */

/* sub_456320: native, nfs3hp::port::buttons::slideTrackMap */

/* sub_456590: native, nfs3hp::port::buttons::drawTrackFacts */

/* sub_456670: native, nfs3hp::port::buttons::drawGlow */

/* sub_456800: native, nfs3hp::port::buttons::trackMapShape */

/* sub_456890: native, nfs3hp::frontend::infoOnFrame */

/* sub_4568d0: native, nfs3hp::frontend::infoOnEnter */

/* sub_456990: native, nfs3hp::frontend::infoOnExit */

/* sub_4569a0: native, nfs3hp::port::fields::colourListWidth */

/* sub_456a20: native, nfs3hp::port::fields::colourListRefresh */

/* sub_456b50: native, nfs3hp::port::fields::drawColourLine */

/* sub_456ce0: native, nfs3hp::port::fields::drawColourList */

/* sub_456ea0: native, nfs3hp::port::fields::colourListShown */

/* sub_456f10: native, nfs3hp::port::fields::pointAtColour */

/* sub_456f50: native, nfs3hp::port::fields::openColourList */

/* sub_456f70: native, nfs3hp::port::fields::closeColourList */

/* sub_456f90: native, nfs3hp::port::fields::chooseColour */

/* sub_457030: native, nfs3hp::port::fields::colourListKey */

/* sub_4571e0: native, nfs3hp::port::fields::layoutColourList */

/* sub_457250: native, nfs3hp::port::fields::colourListHidden */

/* sub_457260: native, nfs3hp::dead::stub_0x457260 */

/* sub_457270: native, nfs3hp::frontend::dirPadUnderPointer */

/* sub_4572d0: native, nfs3hp::frontend::dirPadCentreUnderPointer */

/* sub_457310: native, nfs3hp::dead::stub_0x457310 */

/* sub_457340: native, nfs3hp::frontend::dirPadChoose */

/* sub_457460: native, nfs3hp::frontend::dirPadDraw */

/* sub_4575a0: native, nfs3hp::frontend::dirPadKey */

/* sub_457620: native, nfs3hp::frontend::dirPadAnswer */

/* sub_457630: native, nfs3hp::frontend::dirPadLayout */

/* sub_457670: native, nfs3hp::frontend::dirPadNothing */

/* sub_457680: native, nfs3hp::frontend::dirPadLoadShapes */

/* sub_4576e0: native, nfs3hp::dead::stub_0x4576e0 */

/* sub_457ae0: native, nfs3hp::port::fields::editKey */

/* sub_457c40: native, nfs3hp::port::fields::drawIntField */

/* sub_457df0: native, nfs3hp::port::fields::intRangeMessage */

/* sub_457e30: native, nfs3hp::port::fields::intFieldKey */

/* sub_457f70: native, nfs3hp::port::fields::layoutIntField */

/* sub_458010: native, nfs3hp::port::fields::intFieldShown */

/* sub_458050: native, nfs3hp::port::fields::intFieldHidden */

/* sub_458080: native, nfs3hp::port::fields::bindIntField */

/* sub_4580b0: native, nfs3hp::port::fields::drawStringField */

/* sub_4581f0: native, nfs3hp::port::fields::stringFieldKey */

/* sub_458340: native, nfs3hp::port::fields::layoutStringField */

/* sub_4583b0: native, nfs3hp::port::fields::stringFieldShown */

/* sub_458410: native, nfs3hp::port::fields::stringFieldHidden */

/* sub_458450: native, nfs3hp::frontend::connectSetEditField */

/* sub_458480: native, nfs3hp::port::fields::drawCharField */

/* sub_458640: native, nfs3hp::port::fields::charRangeMessage */

/* sub_458690: native, nfs3hp::port::fields::charFieldKey */

/* sub_4587e0: native, nfs3hp::port::fields::layoutCharField */

/* sub_4588a0: native, nfs3hp::port::fields::charFieldShown */

/* sub_4588e0: native, nfs3hp::port::fields::charFieldHidden */

/* sub_458910: native, nfs3hp::port::fields::bindCharField */

/* sub_458940: native, nfs3hp::port::fields::drawFloatField */

/* sub_458b20: native, nfs3hp::port::fields::floatRangeMessage */

}
