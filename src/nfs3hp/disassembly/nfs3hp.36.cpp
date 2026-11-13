#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4ed800: native, nfs3hp::dead::stub_0x4ed800 */

/* sub_4ed820: native, nfs3hp::frontend::ddrawSurfaceSize */

/* sub_4ed910: native, nfs3hp::dead::stub_0x4ed910 */

/* sub_4ed950: native, nfs3hp::dead::stub_0x4ed950 */

/* sub_4ed990: native, nfs3hp::dead::stub_0x4ed990 */

/* sub_4eda10: native, nfs3hp::dead::stub_0x4eda10 */

/* sub_4edb80: native, nfs3hp::dead::stub_0x4edb80 */

/* sub_4edbc0: native, nfs3hp::frontend::ddrawEnumModeCallback */

/* sub_4edd80: native, nfs3hp::watcom::open_flags */

/* sub_4edebc: native, nfs3hp::watcom::doopen */

/* sub_4edfa4: native, nfs3hp::watcom::fsopen */

/* sub_4edfe8: native, nfs3hp::watcom::fopen */

/* sub_4edff4: native, nfs3hp::dead::stub_0x4edff4 */

/* sub_4ee07c: native, nfs3hp::dead::stub_0x4ee07c */

/* sub_4ee100: native, nfs3hp::watcom::fclose */

/* sub_4ee144: native, nfs3hp::watcom::shutdown_stream */

/* sub_4ee15c: native, nfs3hp::watcom::hexDigit */

/* sub_4ee168: native, nfs3hp::watcom::MkTmpFile */

/* sub_4ee214: native, nfs3hp::watcom::doclose */

/* sub_4ee2e0: native, nfs3hp::watcom::remove */

/* sub_4ee2f0: native, nfs3hp::watcom::strrchr */

/* sub_4ee310: native, nfs3hp::watcom::strcmp */

/* sub_4ee3c0: native, nfs3hp::watcom::chmod */

/* sub_4ee400: native, nfs3hp::watcom::rmdir */

/* sub_4ee420: native, nfs3hp::watcom::ftell */

/* sub_4ee480: native, nfs3hp::watcom::med3 */

/* sub_4ee4d0: native, nfs3hp::watcom::byteswap */

/* sub_4ee4f8: native, nfs3hp::watcom::qsort */

/* sub_4eeae0: native, nfs3hp::load::savefileadr */

/* sub_4eebb0: native, nfs3hp::load::savefileadrQuietly */

/* sub_4eebd0: native, nfs3hp::watcom::fwrite */

/* sub_4eede0: native, nfs3hp::watcom::toupper */

/* sub_4eedf0: native, nfs3hp::watcom::strncat */

/* sub_4eee30: native, nfs3hp::frontend::windowBitmapCreate */

/* sub_4ef190: native, nfs3hp::frontend::windowBitmapCreateDefault */

/* sub_4ef1a0: native, nfs3hp::dead::stub_0x4ef1a0 */

/* sub_4ef1b0: native, nfs3hp::frontend::drawClearClip */

/* sub_4ef1e0: native, nfs3hp::frontend::shapeFileFind */

/* sub_4ef270: native, nfs3hp::ea::locateShape */

/* sub_4ef2b0: native, nfs3hp::frontend::copyNibblesKeyed */

/* sub_4ef320: native, nfs3hp::frontend::copyBytesKeyed */

/* sub_4ef340: native, nfs3hp::frontend::copyWordsKeyed */

/* sub_4ef370: native, nfs3hp::frontend::copyTriplesKeyed */

/* sub_4ef3c0: native, nfs3hp::frontend::copyLongsKeyed */

/* sub_4ef3f0: native, nfs3hp::frontend::shapeDrawKeyed */

/* sub_4ef4a0: native, nfs3hp::dead::stub_0x4ef4a0 */

/* sub_4ef4c0: native, nfs3hp::frontend::shapeDrawKeyedAtHotspot */

/* sub_4ef4e0: native, nfs3hp::dead::stub_0x4ef4e0 */

/* sub_4ef500: native, nfs3hp::dead::stub_0x4ef500 */

/* sub_4ef520: native, nfs3hp::frontend::fontUse */

/* sub_4ef6c0: native, nfs3hp::frontend::paletteNearest */

/* sub_4ef760: native, nfs3hp::frontend::colourPack */

/* sub_4ef830: native, nfs3hp::frontend::fontSetColours */

/* sub_4efa10: native, nfs3hp::frontend::fontGlyphSearch */

/* sub_4efa70: native, nfs3hp::frontend::fontGlyphFind */

/* sub_4efb10: native, nfs3hp::frontend::fontKerning */

/* sub_4efb50: native, nfs3hp::frontend::textDrawAtPen */

/* sub_4efe30: native, nfs3hp::frontend::textDrawAt */

/* sub_4efe50: native, nfs3hp::frontend::textMeasureUpTo */

/* sub_4efef0: native, nfs3hp::frontend::textMeasureAll */

/* sub_4eff00: native, nfs3hp::frontend::windowBitmapFree */

/* sub_4eff60: native, nfs3hp::watcom::strtok */

/* sub_4f0010: native, nfs3hp::ea::keyCharacter */

/* sub_4f00d0: native, nfs3hp::frontend::keyNextLow16 */

/* sub_4f00e0: native, nfs3hp::frontend::keyNextDispatch */

/* sub_4f00f0: native, nfs3hp::frontend::keySetHookFlag */

/* sub_4f0100: native, nfs3hp::ea::keyFromHook */

/* sub_4f0120: native, nfs3hp::ea::inkey */

/* sub_4f0160: native, nfs3hp::dead::stub_0x4f0160 */

/* sub_4f01c0: native, nfs3hp::dead::stub_0x4f01c0 */

/* sub_4f01f0: native, nfs3hp::frontend::inputStateOpen */

/* sub_4f0230: native, nfs3hp::frontend::inputStateClose */

/* sub_4f0240: native, nfs3hp::ea::flushKeys */

/* sub_4f0250: native, nfs3hp::dead::stub_0x4f0250 */

/* sub_4f0260: native, nfs3hp::watcom::chdir */

/* sub_4f0280: native, nfs3hp::frontend::findNextFile */

/* sub_4f0330: native, nfs3hp::frontend::findFirstFile */

/* sub_4f0430: native, nfs3hp::watcom::atoi */

/* sub_4f0490: native, nfs3hp::watcom::atol */

/* sub_4f04f0: native, nfs3hp::watcom::cget_string */

/* sub_4f051c: native, nfs3hp::watcom::uncget_string */

/* sub_4f0520: native, nfs3hp::watcom::vsscanf */

/* sub_4f0548: native, nfs3hp::watcom::sscanf */

/* sub_4f0570: native, nfs3hp::watcom::fgets */

/* sub_4f05f0: native, nfs3hp::watcom::access */

/* sub_4f0630: native, nfs3hp::frontend::timerNow */

/* sub_4f0640: native, nfs3hp::dead::stub_0x4f0640 */

/* sub_4f0650: native, nfs3hp::frontend::timerSinceLast */

/* sub_4f0670: native, nfs3hp::dead::stub_0x4f0670 */

/* sub_4f0690: native, nfs3hp::dead::stub_0x4f0690 */

/* sub_4f06a1: native, nfs3hp::dead::stub_0x4f06a1 */

/* sub_4f06c0: native, nfs3hp::dead::stub_0x4f06c0 */

/* sub_4f06e0: native, nfs3hp::frontend::timerWait */

/* sub_4f0710: native, nfs3hp::watcom::sleep */

/* sub_4f0730: native, nfs3hp::watcom::getcwd */

/* sub_4f07d0: native, nfs3hp::watcom::getdiskfree */

/* sub_4f0840: native, nfs3hp::frontend::textDepth */

/* sub_4f0850: native, nfs3hp::frontend::textSetPen */

/* sub_4f0860: native, nfs3hp::frontend::textSetAttribute */

/* sub_4f0870: native, nfs3hp::dead::stub_0x4f0870 */

}
