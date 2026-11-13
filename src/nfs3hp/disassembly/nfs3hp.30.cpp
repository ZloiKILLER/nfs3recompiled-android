#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4d03c0: native, nfs3hp::frontend::winaStreamFreeChunk */

/* sub_4d0460: native, nfs3hp::frontend::winaChannelBytes */

/* sub_4d0490: native, nfs3hp::frontend::winaChannelState */

/* sub_4d04c0: native, nfs3hp::frontend::winaChannelDone */

/* sub_4d0500: native, nfs3hp::frontend::winaChannelRingSize */

/* sub_4d0540: native, nfs3hp::dead::stub_0x4d0540 */

/* sub_4d0570: native, nfs3hp::dead::stub_0x4d0570 */

/* sub_4d0610: native, nfs3hp::dead::stub_0x4d0610 */

/* sub_4d06f0: native, nfs3hp::port::text::drawCharacter */

/* sub_4d0a10: native, nfs3hp::render::pictureTexture */

/* sub_4d0d50: native, nfs3hp::frontend::textLoadFonts */

/* sub_4d1000: native, nfs3hp::port::text::drawText */

/* sub_4d10e0: native, nfs3hp::frontend::hudTextToRightEdge */

/* sub_4d1120: native, nfs3hp::graphics::drawTextBox */

/* sub_4d11f0: native, nfs3hp::port::fields::drawCursorText3d */

/* sub_4d1310: native, nfs3hp::frontend::hudFontSurfacesBegin */

/* sub_4d1340: native, nfs3hp::port::text::flushFonts */

/* sub_4d1390: native, nfs3hp::port::text::setTextScale */

/* sub_4d1400: native, nfs3hp::port::text::keepTextScale */

/* sub_4d1410: native, nfs3hp::port::text::restoreTextScale */

/* sub_4d1430: native, nfs3hp::port::text::measureText */

/* sub_4d1480: native, nfs3hp::port::text::textWidthWithOverhang */

/* sub_4d14f0: native, nfs3hp::frontend::hudCharacterDrop */

/* sub_4d1540: native, nfs3hp::port::text::characterWidth */

/* sub_4d1590: native, nfs3hp::port::text::characterOverhang */

/* sub_4d15e0: native, nfs3hp::dead::stub_0x4d15e0 */

/* sub_4d1750: native, nfs3hp::port::text::loadLanguageText */

/* sub_4d17b0: native, nfs3hp::port::text::specialText */

/* sub_4d1850: native, nfs3hp::port::text::languageText */

/* sub_4d1880: native, nfs3hp::frontend::languageTextsFree */

/* sub_4d18b0: native, nfs3hp::frontend::rendFormatBits */

/* sub_4d18e0: native, nfs3hp::port::shapes::scan4444 */

/* sub_4d1950: native, nfs3hp::port::shapes::scan8888 */

/* sub_4d19b0: native, nfs3hp::port::shapes::scanNibbles */

/* sub_4d1a00: native, nfs3hp::port::shapes::scanPaletted */

/* sub_4d1a90: native, nfs3hp::port::shapes::scan1555 */

/* sub_4d1ad0: native, nfs3hp::port::shapes::scan565 */

/* sub_4d1b20: native, nfs3hp::port::shapes::convert4444 */

/* sub_4d1c00: native, nfs3hp::port::shapes::convert8888 */

/* sub_4d1ce0: native, nfs3hp::port::shapes::convert8 */

/* sub_4d1db0: native, nfs3hp::port::shapes::convert565 */

/* sub_4d1e60: native, nfs3hp::port::shapes::convert1555 */

/* sub_4d1f70: native, nfs3hp::port::shapes::nibblesTo1555 */

/* sub_4d2050: native, nfs3hp::port::shapes::nibblesTo565 */

/* sub_4d2150: native, nfs3hp::port::shapes::nibblesTo4444 */

/* sub_4d21f0: native, nfs3hp::port::shapes::nibblesTo8888 */

/* sub_4d22b0: native, nfs3hp::port::shapes::paletteTo1555 */

/* sub_4d23f0: native, nfs3hp::port::shapes::paletteTo565 */

/* sub_4d2510: native, nfs3hp::port::shapes::paletteTo4444 */

/* sub_4d2690: native, nfs3hp::port::shapes::paletteTo8888 */

/* sub_4d27e0: native, nfs3hp::port::shapes::argb8888To4444 */

/* sub_4d2940: native, nfs3hp::port::shapes::argb8888To565 */

/* sub_4d2a80: native, nfs3hp::port::shapes::argb8888To1555 */

/* sub_4d2be0: native, nfs3hp::port::shapes::rgb565To1555 */

/* sub_4d2d20: native, nfs3hp::port::shapes::rgb565To4444 */

/* sub_4d2e60: native, nfs3hp::port::shapes::rgb565To8888 */

/* sub_4d2fb0: native, nfs3hp::port::shapes::argb1555To565 */

/* sub_4d30f0: native, nfs3hp::port::shapes::argb1555To4444 */

/* sub_4d3230: native, nfs3hp::port::shapes::argb1555To8888 */

/* sub_4d3380: native, nfs3hp::frontend::rendExpandPalettedShape */

/* sub_4d3410: native, nfs3hp::dead::stub_0x4d3410 */

/* sub_4d34b0: native, nfs3hp::dead::stub_0x4d34b0 */

/* sub_4d37a0: native, nfs3hp::port::shapes::shapeToPixels */

/* sub_4d3f50: native, nfs3hp::render::squareTexture */

/* sub_4d4080: native, nfs3hp::frontend::rendLoadShapeTexture */

/* sub_4d4120: native, nfs3hp::frontend::rendFitPowerOfTwo */

/* sub_4d41a0: native, nfs3hp::render::paletteTexture */

/* sub_4d43e0: native, nfs3hp::dead::stub_0x4d43e0 */

/* sub_4d4440: native, nfs3hp::frontend::rendCopyCabinBar */

/* sub_4d4580: native, nfs3hp::render::pieceTexture */

/* sub_4d4ae0: native, nfs3hp::render::cabinTextures */

/* sub_4d5030: native, nfs3hp::dead::stub_0x4d5030 */

/* sub_4d5060: native, nfs3hp::pieces::stub_0x4d5060 */

/* sub_4d5220: native, nfs3hp::pieces::stub_0x4d5220 */

/* sub_4d5360: native, nfs3hp::dead::stub_0x4d5360 */

/* sub_4d5370: native, nfs3hp::frontend::rendLoadEffectTextures */

/* sub_4d56b0: native, nfs3hp::frontend::rendNothing */

/* sub_4d56c0: native, nfs3hp::dead::stub_0x4d56c0 */

/* sub_4d56d0: native, nfs3hp::dead::stub_0x4d56d0 */

/* sub_4d56e0: native, nfs3hp::dead::stub_0x4d56e0 */

/* sub_4d56f0: native, nfs3hp::frontend::rendLoadEffects */

/* sub_4d5720: native, nfs3hp::frontend::simRaceFrameEnd */

/* sub_4d5730: native, nfs3hp::frontend::raceEffectsEndNothing */

/* sub_4d5840: native, nfs3hp::port::draw2d::quadOutcodes */

/* sub_4d5a80: native, nfs3hp::dead::stub_0x4d5a80 */

/* sub_4d5c40: native, nfs3hp::port::shapes::resetTextures */

/* sub_4d5c90: native, nfs3hp::graphics::clearWithDepth */

/* sub_4d5cc0: native, nfs3hp::graphics::beginDrawing */

/* sub_4d5d10: native, nfs3hp::frontend::raceLoopClearDrawnFlag */

/* sub_4d5d30: native, nfs3hp::graphics::clearBothBuffers */

/* sub_4d5d70: native, nfs3hp::graphics::clearBothBuffersAndReset */

/* sub_4d5db0: native, nfs3hp::port::shapes::shapeByName */

/* sub_4d5e40: native, nfs3hp::port::fields::sliderShape */

/* sub_4d5ed0: native, nfs3hp::graphics::shapeIntoTexture */

/* sub_4d5f40: native, nfs3hp::graphics::makeArtTexture */

/* sub_4d6070: native, nfs3hp::port::shapes::shapeTexture */

/* sub_4d6110: native, nfs3hp::port::shapes::cutTile */

/* sub_4d6190: native, nfs3hp::port::shapes::shape565To1555 */

/* sub_4d6210: native, nfs3hp::port::shapes::tileRoom */

/* sub_4d6390: native, nfs3hp::port::shapes::artShapeTextures */

}
