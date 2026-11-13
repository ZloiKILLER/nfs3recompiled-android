#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_43a4f0: native, nfs3hp::load::carSelectable */

/* sub_43a580: native, nfs3hp::load::buildCarClassLists */

/* sub_43a6b0: native, nfs3hp::frontend::loadingCarInClass */

/* sub_43a6e0: native, nfs3hp::frontend::loadingCarFilterLists */

/* sub_43ace0: native, nfs3hp::pieces::stub_0x43ace0 */

/* sub_43ae60: native, nfs3hp::load::copyCarAsNew */

/* sub_43b2a0: native, nfs3hp::frontend::loadingCarRemoveFolder */

/* sub_43b330: native, nfs3hp::dead::stub_0x43b330 */

/* sub_43b390: native, nfs3hp::frontend::loadingRemoveKeptCars */

/* sub_43b3f0: native, nfs3hp::load::loadCarSpecs */

/* sub_43b4f0: native, nfs3hp::load::writeCarFec */

/* sub_43b570: native, nfs3hp::load::readCarFecBody */

/* sub_43b630: native, nfs3hp::load::readCarFec */

/* sub_43b680: native, nfs3hp::load::carFecPath */

/* sub_43b740: native, nfs3hp::load::loadCarFec */

/* sub_43b7f0: native, nfs3hp::port::fields::saveCarColours */

/* sub_43b830: native, nfs3hp::ea::openViv */

/* sub_43b9b0: native, nfs3hp::ea::closeViv */

/* sub_43b9d0: native, nfs3hp::ea::findInViv */

/* sub_43ba20: native, nfs3hp::load::placeCarVivPath */

/* sub_43ba30: native, nfs3hp::load::swapBytes */

/* sub_43ba70: native, nfs3hp::load::writeBigEndian */

/* sub_43baa0: native, nfs3hp::load::appendCarpToViv */

/* sub_43bbf0: native, nfs3hp::load::lastCarLoaded */

/* sub_43bc00: native, nfs3hp::load::setLastCarId */

/* sub_43bc40: native, nfs3hp::load::setLastCarName */

/* sub_43bc70: native, nfs3hp::load::setLastCarFlags */

/* sub_43bc90: native, nfs3hp::load::markLastCarFlag8 */

/* sub_43bcb0: native, nfs3hp::frontend::loopLoadingScreenFlag */

/* sub_43bcc0: native, nfs3hp::frontend::viewInteriorMapX */

/* sub_43bcf0: native, nfs3hp::frontend::carsRemoveNumberedFolders */

/* sub_43bde0: native, nfs3hp::dead::stub_0x43bde0 */

/* sub_43be40: native, nfs3hp::frontend::controlsPollJoystickBinding */

/* sub_43c000: native, nfs3hp::frontend::bindingIsDigital */

/* sub_43c040: native, nfs3hp::frontend::noteDigitalControls */

/* sub_43c110: native, nfs3hp::frontend::controlsDrawBinding */

/* sub_43c900: native, nfs3hp::frontend::controlsDrawList */

/* sub_43cda0: native, nfs3hp::frontend::controlsKeyAsBinding */

/* sub_43ce80: native, nfs3hp::frontend::controlsWarnDigitalBinding */

/* sub_43cf80: native, nfs3hp::frontend::controlsListKey */

/* sub_43d2b0: native, nfs3hp::frontend::controlsListFitBox */

/* sub_43d310: native, nfs3hp::frontend::controlsListPrepare */

/* sub_43d3f0: native, nfs3hp::frontend::controlsCancelKey */

/* sub_43d410: native, nfs3hp::frontend::controlsResetAnswered */

/* sub_43d460: native, nfs3hp::frontend::controlsResetClicked */

/* sub_43d4d0: native, nfs3hp::frontend::controlsOnEnter */

/* sub_43d5f0: native, nfs3hp::frontend::controlsOnExit */

/* sub_43d650: native, nfs3hp::frontend::controlsWaitJoystickRelease */

/* sub_43d6a0: native, nfs3hp::frontend::controlsClearConflicts */

/* sub_43d7f0: native, nfs3hp::frontend::controlsBindingKeyName */

/* sub_43d920: native, nfs3hp::frontend::controlsSampleAxes */

/* sub_43d9a0: native, nfs3hp::frontend::deadZoneFillList */

/* sub_43d9f0: native, nfs3hp::frontend::controllerName */

/* sub_43da60: native, nfs3hp::dead::stub_0x43da60 */

/* sub_43dab0: native, nfs3hp::frontend::connectApplyPlayerVector */

/* sub_43db20: native, nfs3hp::frontend::colourMakeHueRamp */

/* sub_43dbe0: native, nfs3hp::port::fields::wheelFromCarColour */

/* sub_43dc90: native, nfs3hp::frontend::colourApplyToCar */

/* sub_43dd50: native, nfs3hp::port::fields::wheelForPlayer */

/* sub_43dd90: native, nfs3hp::frontend::colourAccept */

/* sub_43ddd0: native, nfs3hp::frontend::colourRevert */

/* sub_43de20: native, nfs3hp::frontend::colourHsvToArgb */

/* sub_43de60: native, nfs3hp::frontend::colourTriangleFromColour */

/* sub_43def0: native, nfs3hp::frontend::colourFromTriangle */

/* sub_43dfb0: native, nfs3hp::frontend::colourDrawFrame */

/* sub_43e040: native, nfs3hp::frontend::colourDrawSwatch */

/* sub_43e0d0: native, nfs3hp::frontend::colourDrawButtons */

/* sub_43e1f0: native, nfs3hp::frontend::colourDrawMarkers */

/* sub_43e310: native, nfs3hp::frontend::colourDrawTriangle */

/* sub_43e490: native, nfs3hp::frontend::colourDrawValueBar */

/* sub_43e7a0: native, nfs3hp::frontend::colourDragMoved */

/* sub_43e8e0: native, nfs3hp::frontend::colourHitRegion */

/* sub_43ead0: native, nfs3hp::frontend::colourMouse */

/* sub_43ebc0: native, nfs3hp::frontend::colourDraw */

/* sub_43ec40: native, nfs3hp::frontend::colourKey */

/* sub_43ef80: native, nfs3hp::frontend::colourPointerKind */

/* sub_43eff0: native, nfs3hp::frontend::colourFitBox */

/* sub_43f030: native, nfs3hp::frontend::colourPrepare */

/* sub_43f160: native, nfs3hp::frontend::colourShown */

/* sub_43f1f0: native, nfs3hp::frontend::connectListingRedraw */

/* sub_43f220: native, nfs3hp::frontend::ipxRefreshGames */

/* sub_43f320: native, nfs3hp::frontend::connectOpenProtocolList */

/* sub_43f4e0: native, nfs3hp::frontend::connectOpenAddressDialog */

/* sub_43f550: native, nfs3hp::frontend::connectModemLabel */

/* sub_43f610: native, nfs3hp::frontend::connectComPortLabel */

/* sub_43f640: native, nfs3hp::frontend::ipxGameNameAnswer */

/* sub_43f680: native, nfs3hp::frontend::connectAddressAnswer */

/* sub_43f760: native, nfs3hp::frontend::connectTcpAnswer */

/* sub_43f7e0: native, nfs3hp::frontend::connectProtocolListAnswer */

/* sub_43f960: native, nfs3hp::frontend::connectSerialAnswer */

/* sub_43fa00: native, nfs3hp::frontend::connectPhoneAnswer */

/* sub_43faf0: native, nfs3hp::frontend::connectModemAnswer */

/* sub_43fc20: native, nfs3hp::frontend::connectRaceNetMissingAnswer */

/* sub_43fc50: native, nfs3hp::frontend::connectRaceNetAnswer */

/* sub_43fcb0: native, nfs3hp::frontend::multiMenuLeaveAnswer */

/* sub_43fd10: native, nfs3hp::frontend::connectRaceNetClicked */

/* sub_43fdd0: native, nfs3hp::port::buttons::joinListedGame */

/* sub_43fe50: native, nfs3hp::frontend::connectTcpClicked */

/* sub_43fee0: native, nfs3hp::frontend::connectSerialClicked */

/* sub_43ff50: native, nfs3hp::frontend::connectModemClicked */

}
