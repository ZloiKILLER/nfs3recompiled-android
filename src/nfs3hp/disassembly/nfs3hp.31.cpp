#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4d66f0: native, nfs3hp::port::shapes::shapeSize */

/* sub_4d6730: native, nfs3hp::port::buttons::drawTurnedShape */

/* sub_4d6c70: native, nfs3hp::dead::stub_0x4d6c70 */

/* sub_4d70f0: native, nfs3hp::port::shapes::drawShapeQuad */

/* sub_4d7160: native, nfs3hp::port::shapes::drawArtShape */

/* sub_4d7560: native, nfs3hp::frontend::artTilesDraw */

/* sub_4d7990: native, nfs3hp::port::shapes::drawShape */

/* sub_4d79b0: native, nfs3hp::port::obj3d::drawLogo */

/* sub_4d7cc0: native, nfs3hp::frontend::bouncerCornerFade */

/* sub_4d7d90: native, nfs3hp::frontend::bouncerDraw */

/* sub_4d81b0: native, nfs3hp::port::draw2d::drawShadedRectangle */

/* sub_4d8550: native, nfs3hp::port::draw2d::gradientRectangle */

/* sub_4d8570: native, nfs3hp::port::fields::drawShadedTriangle */

/* sub_4d8860: native, nfs3hp::port::draw2d::drawShadedBox */

/* sub_4d88c0: native, nfs3hp::frontend::pickerShadedTriangle */

/* sub_4d88f0: native, nfs3hp::port::buttons::drawColouredRectangle */

/* sub_4d8910: native, nfs3hp::port::draw2d::filledRectangle */

/* sub_4d8930: native, nfs3hp::port::draw2d::frame */

/* sub_4d89f0: native, nfs3hp::port::draw2d::drawPanel */

/* sub_4d8aa0: native, nfs3hp::port::fields::drawFrameAt */

/* sub_4d8ac0: native, nfs3hp::port::draw2d::drawFieldBox */

/* sub_4d8b80: native, nfs3hp::port::draw2d::drawTileRectangle */

/* sub_4d8ed0: native, nfs3hp::port::draw2d::drawLine */

/* sub_4d9120: native, nfs3hp::graphics::takeWindowAndClear */

/* sub_4d9140: native, nfs3hp::graphics::flipAndWaitFrames */

/* sub_4d9180: native, nfs3hp::frontend::effectsBandColourScaled */

/* sub_4d9240: native, nfs3hp::port::draw2d::fontCharacter */

/* sub_4d9320: native, nfs3hp::port::draw2d::accentedCharacter */

/* sub_4d93a0: native, nfs3hp::port::draw2d::buildScreenFont */

/* sub_4d95c0: native, nfs3hp::port::draw2d::drawScreenCharacter */

/* sub_4d9b90: native, nfs3hp::port::draw2d::drawTurnedCharacter */

/* sub_4da120: native, nfs3hp::port::draw2d::drawTurnedText */

/* sub_4da1e0: native, nfs3hp::port::draw2d::drawTitleText */

/* sub_4da4a0: native, nfs3hp::port::draw2d::drawScreenText */

/* sub_4da560: native, nfs3hp::port::fields::drawCursor */

/* sub_4da8c0: native, nfs3hp::port::fields::drawCursorText */

/* sub_4daa10: native, nfs3hp::frontend::menuPointerLoad */

/* sub_4dab10: native, nfs3hp::frontend::menuPointerHotSpot */

/* sub_4dab90: native, nfs3hp::port::draw2d::drawPointer */

/* sub_4daf80: native, nfs3hp::port::draw2d::startScreens */

/* sub_4db000: native, nfs3hp::port::draw2d::drawTexturedQuad */

/* sub_4db380: native, nfs3hp::port::draw2d::drawShadedQuad */

/* sub_4db750: native, nfs3hp::frontend::rendSkidSubdivide */

/* sub_4db860: native, nfs3hp::frontend::effectsWheelMarkCut */

/* sub_4db8b0: native, nfs3hp::frontend::rendSkidStrip */

/* sub_4db940: native, nfs3hp::frontend::rendSkidWheel */

/* sub_4dbba0: native, nfs3hp::frontend::effectsWheelMarksResetAtLoad */

/* sub_4dbbc0: native, nfs3hp::frontend::effectsWheelMarksReset */

/* sub_4dbbe0: native, nfs3hp::frontend::effectsWheelMarksEndEmpty */

/* sub_4dbbf0: native, nfs3hp::frontend::raceLoadTrackPieces */

/* sub_4dbcc0: native, nfs3hp::frontend::rendDefaultFieldOfView */

/* sub_4dbce0: native, nfs3hp::frontend::viewsSetProjection */

/* sub_4dbda0: native, nfs3hp::frontend::simViewProjection */

/* sub_4dbde0: native, nfs3hp::frontend::simSetView */

/* sub_4dbef0: native, nfs3hp::vertices::objectVerticesList */

/* sub_4dc0e0: native, nfs3hp::frontend::udffOpen */

/* sub_4dc1b0: native, nfs3hp::frontend::udffClose */

/* sub_4dc1e0: native, nfs3hp::frontend::udffReadWord */

/* sub_4dc220: native, nfs3hp::dead::stub_0x4dc220 */

/* sub_4dc260: native, nfs3hp::dead::stub_0x4dc260 */

/* sub_4dc2a0: native, nfs3hp::dead::stub_0x4dc2a0 */

/* sub_4dc2e0: native, nfs3hp::dead::stub_0x4dc2e0 */

/* sub_4dc300: native, nfs3hp::dead::stub_0x4dc300 */

/* sub_4dc350: native, nfs3hp::dead::stub_0x4dc350 */

/* sub_4dc3a0: native, nfs3hp::load::setSpecUsed */

/* sub_4dc3f0: native, nfs3hp::load::specUsed */

/* sub_4dc440: native, nfs3hp::load::specCarName */

/* sub_4dc490: native, nfs3hp::load::findSpec */

/* sub_4dc500: native, nfs3hp::load::freeSpecSlot */

/* sub_4dc550: native, nfs3hp::frontend::loadingTuningOf */

/* sub_4dc5a0: native, nfs3hp::load::setSpec */

/* sub_4dc670: native, nfs3hp::load::defaultSpecs */

/* sub_4dc710: native, nfs3hp::load::refreshSpecs */

/* sub_4dc800: native, nfs3hp::load::specPath */

/* sub_4dc8b0: native, nfs3hp::load::readSpecs */

/* sub_4dc940: native, nfs3hp::load::loadPlayerSpecs */

/* sub_4dca50: native, nfs3hp::load::savePlayerSpecs */

/* sub_4dcaf0: native, nfs3hp::frontend::tuningAdvancedDiffers */

/* sub_4dcb60: native, nfs3hp::frontend::tuningCopyAdvanced */

/* sub_4dcbc0: native, nfs3hp::frontend::loadingTuningForPlayers */

/* sub_4dcd10: native, nfs3hp::frontend::simWeatherStep */

/* sub_4dcd20: native, nfs3hp::load::loadWeatherShapes */

/* sub_4dcdd0: native, nfs3hp::frontend::simRainAmount */

/* sub_4dcdf0: native, nfs3hp::frontend::weatherSetUp */

/* sub_4dd1f0: native, nfs3hp::dead::stub_0x4dd1f0 */

/* sub_4dd230: native, nfs3hp::frontend::weatherScatterAtCut */

/* sub_4dd330: native, nfs3hp::frontend::weatherFadeByTime */

/* sub_4dd3d0: native, nfs3hp::frontend::weatherFallingAtNode */

/* sub_4dd4c0: native, nfs3hp::frontend::weatherShareSeen */

/* sub_4dd510: native, nfs3hp::frontend::weatherViewChanged */

/* sub_4dd5e0: native, nfs3hp::frontend::weatherMoveDrops */

/* sub_4dd930: native, nfs3hp::frontend::rendSmokeEmit */

/* sub_4ddb40: native, nfs3hp::render::sparks */

/* sub_4de070: native, nfs3hp::render::smoke */

/* sub_4de700: native, nfs3hp::render::streaks */

/* sub_4dece0: native, nfs3hp::frontend::weatherPassOverView */

/* sub_4dedf0: native, nfs3hp::frontend::rendSparksPass */

/* sub_4dee80: native, nfs3hp::dead::stub_0x4dee80 */

/* sub_4df1f0: native, nfs3hp::frontend::movieDisplayOpen */

/* sub_4df2b0: native, nfs3hp::movie::showFrameAt */

}
