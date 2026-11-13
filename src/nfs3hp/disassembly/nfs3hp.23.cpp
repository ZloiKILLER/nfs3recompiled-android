#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_49c1f0: native, nfs3hp::frontend::pointerInBox */

/* sub_49c240: native, nfs3hp::frontend::menuPointerOverRect */

/* sub_49c280: native, nfs3hp::dead::stub_0x49c280 */

/* sub_49c290: native, nfs3hp::dead::stub_0x49c290 */

/* sub_49c2a0: native, nfs3hp::frontend::menuPointerFreeArt */

/* sub_49c2d0: native, nfs3hp::dead::stub_0x49c2d0 */

/* sub_49c310: native, nfs3hp::dead::stub_0x49c310 */

/* sub_49c3b0: native, nfs3hp::dead::stub_0x49c3b0 */

/* sub_49c410: native, nfs3hp::frontend::rendObjectsBegin */

/* sub_49c420: native, nfs3hp::frontend::rendObjectsDraw */

/* sub_49c470: native, nfs3hp::frontend::rendObjectsEnd */

/* sub_49c4a0: native, nfs3hp::frontend::rendObjectsCopy */

/* sub_49c570: native, nfs3hp::graphics::showPicture */

/* sub_49c5c0: native, nfs3hp::graphics::textureFormatFor */

/* sub_49c600: native, nfs3hp::graphics::allocTexture */

/* sub_49c620: native, nfs3hp::graphics::updateTexture */

/* sub_49c700: native, nfs3hp::load::readCarColors */

/* sub_49c880: native, nfs3hp::load::rgbToHsv */

/* sub_49ca50: native, nfs3hp::load::hsvToRgb */

/* sub_49cc50: native, nfs3hp::load::tintPixel */

/* sub_49cd00: native, nfs3hp::load::scaleTexture */

/* sub_49cea0: native, nfs3hp::load::fitTexture */

/* sub_49cfa0: native, nfs3hp::loader::fceRecords */

/* sub_49d130: native, nfs3hp::load::loadArtTextures */

/* sub_49d240: native, nfs3hp::load::nearestModelColor */

/* sub_49d320: native, nfs3hp::load::loadTgaTexture */

/* sub_49d450: native, nfs3hp::load::loadCarModel */

/* sub_49d5a0: native, nfs3hp::frontend::rendModelFree */

/* sub_49d5c0: native, nfs3hp::frontend::rendModelTransformAll */

/* sub_49d640: native, nfs3hp::decompiled::resetSunLighting */

/* sub_49d720: native, nfs3hp::frontend::rendModelSetAlphas */

/* sub_49d750: native, nfs3hp::frontend::rendModelSetShading */

/* sub_49d760: native, nfs3hp::frontend::rendModelSetLight */

/* sub_49d780: native, nfs3hp::frontend::rendModelSetEnvironment */

/* sub_49d800: native, nfs3hp::render::glareColours */

/* sub_49d8f0: native, nfs3hp::frontend::rendModelSetColour */

/* sub_49d9d0: native, nfs3hp::vertices::objectVerticesNearest */

/* sub_49da30: native, nfs3hp::vertices::shadeNormals */

/* sub_49db70: native, nfs3hp::vertices::envMapCoords */

/* sub_49dbf0: native, nfs3hp::vertices::cullPolygons */

/* sub_49de20: native, nfs3hp::frontend::rendModelTransformPart */

/* sub_49e080: native, nfs3hp::frontend::netHostStart */

/* sub_49e220: native, nfs3hp::frontend::netJoinRaceNet */

/* sub_49e2e0: native, nfs3hp::frontend::netHostRaceNet */

/* sub_49e320: native, nfs3hp::frontend::netPendingAdd */

/* sub_49e360: native, nfs3hp::frontend::connectConnectionAvailable */

/* sub_49e3d0: native, nfs3hp::frontend::connectListSessions */

/* sub_49e5f0: native, nfs3hp::frontend::connectCreateSession */

/* sub_49e800: native, nfs3hp::frontend::connectJoinSession */

/* sub_49e9d0: native, nfs3hp::frontend::connectUseTcp */

/* sub_49ea70: native, nfs3hp::frontend::connectFindAddress */

/* sub_49eae0: native, nfs3hp::frontend::netHostLobby */

/* sub_49ec70: native, nfs3hp::frontend::netDropGone */

/* sub_49ecb0: native, nfs3hp::ea::dropMachine */

/* sub_49ecf0: native, nfs3hp::ea::announceMachineLeft */

/* sub_49edb0: native, nfs3hp::frontend::netGonePacket */

/* sub_49ede0: native, nfs3hp::frontend::netLobbyOpen */

/* sub_49ee50: native, nfs3hp::frontend::netLobbyClose */

/* sub_49eed0: native, nfs3hp::ea::endNetworkGame */

/* sub_49efb0: native, nfs3hp::frontend::connectSetUpEndpoint */

/* sub_49f030: native, nfs3hp::frontend::connectEnumerateServices */

/* sub_49f150: native, nfs3hp::frontend::connectFindConnections */

/* sub_49f1c0: native, nfs3hp::ea::sessionIndex */

/* sub_49f200: native, nfs3hp::frontend::connectServiceFound */

/* sub_49f380: native, nfs3hp::frontend::connectSessionFound */

/* sub_49f470: native, nfs3hp::frontend::connectQueuePeerByName */

/* sub_49f4b0: native, nfs3hp::frontend::connectQueuePeer */

/* sub_49f520: native, nfs3hp::frontend::connectClearPeerSlots */

/* sub_49f550: native, nfs3hp::ea::forgetPendingPlayer */

/* sub_49f5c0: native, nfs3hp::frontend::connectRememberPeer */

/* sub_49f650: native, nfs3hp::frontend::connectClearQueue */

/* sub_49f680: native, nfs3hp::frontend::netPendingEmpty */

/* sub_49f6a0: native, nfs3hp::frontend::connectEnqueuePeer */

/* sub_49f730: native, nfs3hp::frontend::netNextPending */

/* sub_49f790: native, nfs3hp::frontend::netRasAvailable */

/* sub_49f810: native, nfs3hp::frontend::netNetworkThere */

/* sub_49f880: native, nfs3hp::frontend::netRasPhoneBook */

/* sub_49f940: native, nfs3hp::dead::stub_0x49f940 */

/* sub_49fa90: native, nfs3hp::dead::stub_0x49fa90 */

/* sub_49fb10: native, nfs3hp::frontend::netRasLoaded */

/* sub_49fb60: native, nfs3hp::frontend::netRasLoad */

/* sub_49fc50: native, nfs3hp::frontend::netRasFree */

/* sub_49fc80: native, nfs3hp::dead::stub_0x49fc80 */

/* sub_49fda0: native, nfs3hp::dead::stub_0x49fda0 */

/* sub_49fdd0: native, nfs3hp::pieces::stub_0x49fdd0 */

/* sub_49fde0: native, nfs3hp::frontend::simCarTiltedOver */

/* sub_49fe70: native, nfs3hp::port::conv::fn49fe70 */

/* sub_49fee0: native, nfs3hp::frontend::simSliceHeading */

/* sub_49ffa0: native, nfs3hp::frontend::simCarTrackFrame */

/* sub_4a00c0: native, nfs3hp::frontend::simNodeGap */

/* sub_4a00d0: native, nfs3hp::port::conv::fn4a00d0 */

/* sub_4a0140: native, nfs3hp::frontend::simCarOntoRoad */

/* sub_4a0440: native, nfs3hp::port::conv::fn4a0440 */

/* sub_4a0580: native, nfs3hp::frontend::simCarNearPlayers */

/* sub_4a08c0: native, nfs3hp::frontend::simCarTrackEnvironment */

/* sub_4a0980: native, nfs3hp::port::conv::fn4a0980 */

/* sub_4a0a10: native, nfs3hp::port::conv::fn4a0a10 */

/* sub_4a0aa0: native, nfs3hp::frontend::simTrackPlaceOnRoad */

/* sub_4a0d20: native, nfs3hp::port::conv::fn4a0d20 */

/* sub_4a0f30: native, nfs3hp::port::conv::fn4a0f30 */

}
