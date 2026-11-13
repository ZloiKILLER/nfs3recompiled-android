#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_418d00: native, nfs3hp::load::swapBigEndian */

/* sub_418d40: native, nfs3hp::load::newFileNode */

/* sub_418d70: native, nfs3hp::load::freeFileNode */

/* sub_418db0: native, nfs3hp::dead::stub_0x418db0 */

/* sub_418dd0: native, nfs3hp::load::findFileNode */

/* sub_418e00: native, nfs3hp::dead::stub_0x418e00 */

/* sub_418e20: native, nfs3hp::dead::stub_0x418e20 */

/* sub_418e60: native, nfs3hp::dead::stub_0x418e60 */

/* sub_418e90: native, nfs3hp::load::insertFileNode */

/* sub_418f00: native, nfs3hp::load::newEmptyFileNode */

/* sub_418f40: native, nfs3hp::load::freeFileNodes */

/* sub_418f70: native, nfs3hp::load::countFileNodes */

/* sub_418fa0: native, nfs3hp::dead::stub_0x418fa0 */

/* sub_418fe0: native, nfs3hp::load::findCachedFile */

/* sub_419000: native, nfs3hp::load::addCachedFile */

/* sub_419080: native, nfs3hp::load::newFileCache */

/* sub_4190e0: native, nfs3hp::load::freeFileCache */

/* sub_419120: native, nfs3hp::load::bigHeaderSize */

/* sub_4191b0: native, nfs3hp::load::cachedDataSize */

/* sub_4191e0: native, nfs3hp::load::writeBigFile */

/* sub_419460: native, nfs3hp::load::packFileCache */

/* sub_419510: native, nfs3hp::load::readBigFile */

/* sub_4195f0: native, nfs3hp::load::openFileCache */

/* sub_419660: native, nfs3hp::track::tintColours */

/* sub_419720: native, nfs3hp::track::tintObjectList */

/* sub_419750: native, nfs3hp::track::tintTrack */

/* sub_4197c0: native, nfs3hp::track::greyColours */

/* sub_419840: native, nfs3hp::track::greyObjectList */

/* sub_419870: native, nfs3hp::track::greyTrack */

/* sub_4198e0: native, nfs3hp::track::readObjectList */

/* sub_419ab0: native, nfs3hp::track::readLights */

/* sub_419b60: native, nfs3hp::track::newArena */

/* sub_419b90: native, nfs3hp::track::freeArena */

/* sub_419bc0: native, nfs3hp::track::arenaTake */

/* sub_419bf0: native, nfs3hp::track::frdRead */

/* sub_419c20: native, nfs3hp::loader::frd */

/* sub_41a100: native, nfs3hp::track::loadGeometry */

/* sub_41a120: native, nfs3hp::track::projectedTriangle */

/* sub_41a360: native, nfs3hp::vertices::projectedTriangle */

/* sub_41a3e0: native, nfs3hp::track::shadeVertex */

/* sub_41a550: native, nfs3hp::vertices::trackVertices */

/* sub_41a800: native, nfs3hp::track::setShadeDistances */

/* sub_41a8d0: native, nfs3hp::track::viewObjectVertices */

/* sub_41a970: native, nfs3hp::vertices::objectVerticesColoured */

/* sub_41a9b0: native, nfs3hp::track::viewTurnedObjectVertices */

/* sub_41aa60: native, nfs3hp::track::modelByDistance */

/* sub_41aab0: native, nfs3hp::vertices::groundDistanceOf */

/* sub_41aaf0: native, nfs3hp::track::viewOnBlock */

/* sub_41ab40: native, nfs3hp::track::lightObjectPolygons */

/* sub_41ae90: native, nfs3hp::vertices::trackPolygons */

/* sub_41b300: native, nfs3hp::vertices::trackRecords */

/* sub_41b640: native, nfs3hp::vertices::objectRecords */

/* sub_41b8c0: native, nfs3hp::track::bracketList */

/* sub_41b9b0: native, nfs3hp::render::viewPass */

/* sub_41bb50: native, nfs3hp::track::ambientSounds */

/* sub_41bc20: native, nfs3hp::track::ambientWanted */

/* sub_41bc50: native, nfs3hp::track::pointBehindView */

/* sub_41bc90: native, nfs3hp::track::litBoxOutside */

/* sub_41bf30: native, nfs3hp::vertices::boxOutside */

/* sub_41c1a0: native, nfs3hp::track::flushBuckets */

/* sub_41c310: native, nfs3hp::track::headlightsOnObjects */

/* sub_41c670: native, nfs3hp::track::headlightsOnBlock */

/* sub_41c9f0: native, nfs3hp::track::blockLights */

/* sub_41ca40: native, nfs3hp::track::drawObjectList */

/* sub_41cdc0: native, nfs3hp::track::drawTrack */

/* sub_41d210: native, nfs3hp::track::ambientSource */

/* sub_41d310: native, nfs3hp::track::selectView */

/* sub_41d350: native, nfs3hp::track::objectAnimation */

/* sub_41d510: native, nfs3hp::track::turnFromAngles */

/* sub_41d5a0: native, nfs3hp::track::turnFromPitchYaw */

/* sub_41d610: native, nfs3hp::track::setViewDistance */

/* sub_41d620: native, nfs3hp::render::viewDistances */

/* sub_41d870: native, nfs3hp::track::nextViewDistance */

/* sub_41d8a0: native, nfs3hp::load::trackFilePath */

/* sub_41d8d0: native, nfs3hp::load::trackFolderPath */

/* sub_41d900: native, nfs3hp::track::modelsUnlimited */

/* sub_41d960: native, nfs3hp::render::raceViews */

/* sub_41da40: native, nfs3hp::track::freeAt */

/* sub_41da60: native, nfs3hp::track::unloadTrack */

/* sub_41daa0: native, nfs3hp::track::newPool */

/* sub_41dae0: native, nfs3hp::track::newObjectPool */

/* sub_41db00: native, nfs3hp::track::freeTrackKinds */

/* sub_41db10: native, nfs3hp::track::loadTextures */

/* sub_41dbc0: native, nfs3hp::load::loadTrackTextures */

/* sub_41dc60: native, nfs3hp::track::takeTextures */

/* sub_41dcc0: native, nfs3hp::track::loadCollisions */

/* sub_41de10: native, nfs3hp::track::freeCollisions */

/* sub_41de40: native, nfs3hp::track::clearObjectStates */

/* sub_41de80: native, nfs3hp::track::resetTrackViews */

/* sub_41dea0: native, nfs3hp::track::resetViewsOuter */

/* sub_41deb0: native, nfs3hp::track::resetViews */

/* sub_41df10: native, nfs3hp::render::viewRecord */

/* sub_41e070: native, nfs3hp::track::blockInView */

/* sub_41e0f0: native, nfs3hp::vertices::nearestBlock */

/* sub_41e180: native, nfs3hp::trackdata::loadCcmPoints */

/* sub_41e2c0: native, nfs3hp::trackdata::freeCcmPoints */

/* sub_41e2f0: native, nfs3hp::trackdata::loadSimHeights */

/* sub_41e340: native, nfs3hp::trackdata::freeSimHeights */

/* sub_41e370: native, nfs3hp::trackdata::openTrackNodes */

/* sub_41e3b0: native, nfs3hp::trackdata::closeTrackNodes */

}
