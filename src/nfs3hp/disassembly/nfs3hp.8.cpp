#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_432190: native, nfs3hp::dead::stub_0x432190 */

/* sub_432260: native, nfs3hp::dead::stub_0x432260 */

/* sub_432320: native, nfs3hp::dead::stub_0x432320 */

/* sub_432370: native, nfs3hp::graphics::clearSoftwareBlend */

/* sub_432390: native, nfs3hp::graphics::setSoftwareBlendColour */

/* sub_432440: native, nfs3hp::graphics::dead_0x432440 */

/* sub_432690: native, nfs3hp::collide::subdivisionDepth */

/* sub_432720: native, nfs3hp::render::subdivideQuad */

/* sub_433060: native, nfs3hp::render::quadSubdivided */

/* sub_4332b0: native, nfs3hp::render::quadRecords */

/* sub_433530: native, nfs3hp::collide::drawTexturedTriangles */

/* sub_433660: native, nfs3hp::collide::drawFlatTriangles */

/* sub_4336e0: native, nfs3hp::collide::drawTrianglesByZ */

/* sub_433840: native, nfs3hp::collide::drawQuadsByZ */

/* sub_4339d0: native, nfs3hp::collide::drawFlatTrianglesAgain */

/* sub_433a50: native, nfs3hp::vertices::triangleList */

/* sub_433bb0: native, nfs3hp::vertices::quadList */

/* sub_433d40: native, nfs3hp::graphics::drawColouredQuadRecord */

/* sub_433e30: native, nfs3hp::render::colouredTriangles */

/* sub_434120: native, nfs3hp::render::triangleRecords */

/* sub_434380: native, nfs3hp::vertices::submitPolygons */

/* sub_434799: native, nfs3hp::dead::stub_0x434799 */

/* sub_4347b0: native, nfs3hp::collide::drawColouredTriangles */

/* sub_4347d0: native, nfs3hp::graphics::drawLineRecord */

/* sub_434850: native, nfs3hp::collide::stateRecord */

/* sub_434870: native, nfs3hp::render::polygonDrawers */

/* sub_434950: native, nfs3hp::collide::drawRecords */

/* sub_434990: native, nfs3hp::collide::raceTextFrame */

/* sub_4349a0: native, nfs3hp::collide::setMirrored */

/* sub_4349d0: native, nfs3hp::collide::drawMirroredRecords */

/* sub_434a00: native, nfs3hp::graphics::endFrame */

/* sub_434a70: native, nfs3hp::collide::countFrame */

/* sub_434a80: native, nfs3hp::collide::raceViewDistance */

/* sub_434a90: native, nfs3hp::collide::raceFrameEnd */

/* sub_434ad0: native, nfs3hp::dead::stub_0x434ad0 */

/* sub_434ae0: native, nfs3hp::collide::raceFrameNothing */

/* sub_434af0: native, nfs3hp::collide::raceEndNothing */

/* sub_434b00: native, nfs3hp::collide::raceStartDrawing */

/* sub_434b10: native, nfs3hp::collide::raceStartNothing */

/* sub_434b20: native, nfs3hp::graphics::syncWindow */

/* sub_434b40: native, nfs3hp::graphics::flushAndFlip */

/* sub_434b60: native, nfs3hp::collide::drawPointer */

/* sub_434f20: native, nfs3hp::controls::startDirectInput */

/* sub_434fd0: native, nfs3hp::controls::stopDirectInput */

/* sub_4350a0: native, nfs3hp::controls::openMouse */

/* sub_435230: native, nfs3hp::frontend::placePointer */

/* sub_4352d0: native, nfs3hp::controls::setPointerBounds */

/* sub_435310: native, nfs3hp::controls::readMouse */

/* sub_435600: native, nfs3hp::controls::findJoysticks */

/* sub_435690: native, nfs3hp::controls::describeJoystick */

/* sub_4358f0: native, nfs3hp::controls::readJoystick */

/* sub_435ae0: native, nfs3hp::controls::readAxisZone */

/* sub_435b40: native, nfs3hp::controls::setAxisZone */

/* sub_435b60: native, nfs3hp::controls::resetForceFeedback */

/* sub_435bc0: native, nfs3hp::controls::setAutocenter */

/* sub_435c60: native, nfs3hp::controls::pauseEffects */

/* sub_435cc0: native, nfs3hp::controls::continueEffects */

/* sub_435d20: native, nfs3hp::controls::actuatorsOn */

/* sub_435d90: native, nfs3hp::controls::actuatorsOff */

/* sub_435df0: native, nfs3hp::controls::setGain */

/* sub_435f10: native, nfs3hp::controls::createEffect */

/* sub_4361e0: native, nfs3hp::controls::startEffect */

/* sub_436230: native, nfs3hp::controls::stopEffect */

/* sub_436270: native, nfs3hp::controls::stopEffects */

/* sub_4362f0: native, nfs3hp::controls::downloadEffect */

/* sub_436310: native, nfs3hp::controls::unloadEffect */

/* sub_436330: native, nfs3hp::controls::updateEffect */

/* sub_436610: native, nfs3hp::controls::setCooperativeLevel */

/* sub_4366e0: native, nfs3hp::controls::acquireDevice */

/* sub_436800: native, nfs3hp::controls::unacquireDevice */

/* sub_4368c0: native, nfs3hp::controls::acquireResult */

/* sub_4368f0: native, nfs3hp::controls::effectPlaying */

/* sub_436930: native, nfs3hp::controls::hideCursor */

/* sub_436980: native, nfs3hp::controls::joystickFound */

/* sub_436b10: native, nfs3hp::controls::axisProperty */

/* sub_436c40: native, nfs3hp::collide::openMessages */

/* sub_436cf0: native, nfs3hp::collide::closeMessages */

/* sub_436d40: native, nfs3hp::collide::clearAllMessages */

/* sub_436d60: native, nfs3hp::collide::clearMessages */

/* sub_436dc0: native, nfs3hp::collide::clearMessageLine */

/* sub_436e10: native, nfs3hp::collide::addMessageText */

/* sub_436e90: native, nfs3hp::collide::addMessageCharacter */

/* sub_436fc0: native, nfs3hp::pieces::stub_0x436fc0 */

/* sub_437080: native, nfs3hp::collide::copyMessageLines */

/* sub_4371a0: native, nfs3hp::collide::messagesScrolled */

/* sub_4371c0: native, nfs3hp::collide::messagesNothing */

/* sub_4371d0: native, nfs3hp::collide::seedHash */

/* sub_437250: native, nfs3hp::frontend::feedSeparate */

/* sub_437300: native, nfs3hp::frontend::feedTimeButtonKey */

/* sub_437350: native, nfs3hp::frontend::feedTimeListKey */

/* sub_437390: native, nfs3hp::frontend::feedPositionButtonKey */

/* sub_4373e0: native, nfs3hp::frontend::feedPositionListKey */

/* sub_437420: native, nfs3hp::frontend::feedSpeedButtonKey */

/* sub_437470: native, nfs3hp::frontend::feedSpeedListKey */

/* sub_4374b0: native, nfs3hp::frontend::feedOnEnter */

/* sub_437590: native, nfs3hp::frontend::feedOnExit */

/* sub_437600: native, nfs3hp::frontend::funkeysOnEnter */

/* sub_4377c0: native, nfs3hp::frontend::funkeysOnExit */

/* sub_4377d0: native, nfs3hp::frontend::hardkeysOnEnter */

/* sub_437990: native, nfs3hp::frontend::audioMenuMusicMoved */

}
