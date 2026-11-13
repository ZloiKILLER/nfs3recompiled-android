#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_514650: native, nfs3hp::frontend::netcGatherNewGame */

/* sub_514820: native, nfs3hp::frontend::netcGatherSetGameInfo */

/* sub_514850: native, nfs3hp::frontend::netcGatherCloseGame */

/* sub_5148b0: native, nfs3hp::frontend::netcGatherReopenGame */

/* sub_514970: native, nfs3hp::frontend::netcGatherLeave */

/* sub_514af0: native, nfs3hp::frontend::netcGatherEnumPlayers */

/* sub_514dc0: native, nfs3hp::frontend::netcGatherKillPlayer */

/* sub_514ec0: native, nfs3hp::frontend::netcGatherOpenConnection */

/* sub_514f70: native, nfs3hp::frontend::netcGatherReceive */

/* sub_515050: native, nfs3hp::frontend::netcLoadRasApi */

/* sub_5150f0: native, nfs3hp::frontend::netLocalAddress */

/* sub_515300: native, nfs3hp::watcom::MemAllocator */

/* sub_5153b0: native, nfs3hp::watcom::MemFree */

/* sub_5154e0: native, nfs3hp::watcom::LinkUpNewNHeap */

/* sub_515558: native, nfs3hp::watcom::CreateNewNHeap */

/* sub_515614: native, nfs3hp::watcom::ExpandDGROUP */

/* sub_515628: native, nfs3hp::watcom::AdjustAmount */

/* sub_515680: native, nfs3hp::watcom::nmemneed */

/* sub_515690: native, nfs3hp::watcom::NTRealKey */

/* sub_5156b8: native, nfs3hp::watcom::openConsoles */

/* sub_515728: native, nfs3hp::watcom::NTConsoleInput */

/* sub_515734: native, nfs3hp::watcom::NTConsoleOutput */

/* sub_515740: native, nfs3hp::frontend::watbInitStreams */

/* sub_5157e0: native, nfs3hp::frontend::watbCloseStreams */

/* sub_5157ec: native, nfs3hp::pieces::stub_0x5157ec */

/* sub_5157f4: native, nfs3hp::frontend::watbCloseStreamsFrom */

/* sub_515850: native, nfs3hp::watcom::flushall */

/* sub_515858: native, nfs3hp::watcom::flushall_ */

/* sub_5158a0: native, nfs3hp::watcom::mainStart */

/* sub_515900: native, nfs3hp::frontend::watbBuildArgv */

/* sub_5159e4: native, nfs3hp::frontend::watbSplitCommandLine */

/* sub_515af0: native, nfs3hp::dead::stub_0x515af0 */

/* sub_515b0c: native, nfs3hp::dead::stub_0x515b0c */

/* sub_515b34: native, nfs3hp::watcom::tzWindows */

/* sub_515c2c: native, nfs3hp::watcom::tzset */

/* sub_515c90: native, nfs3hp::watcom::tzNumber */

/* sub_515cbc: native, nfs3hp::watcom::tzName */

/* sub_515dd4: native, nfs3hp::watcom::tzRule */

/* sub_515ec2: native, nfs3hp::watcom::tzDaylight */

/* sub_515fa0: native, nfs3hp::watcom::brktime */

/* sub_5160b8: native, nfs3hp::dead::stub_0x5160b8 */

/* sub_5160d8: native, nfs3hp::dead::stub_0x5160d8 */

/* sub_5160f0: native, nfs3hp::watcom::leapyear */

/* sub_51612c: native, nfs3hp::watcom::calc_yday */

/* sub_516224: native, nfs3hp::watcom::check_order */

/* sub_516270: native, nfs3hp::watcom::isindst */

/* sub_51652c: native, nfs3hp::watcom::time_less */

/* sub_51655c: native, nfs3hp::watcom::math87_err */

/* sub_5165d8: native, nfs3hp::watcom::ZBuf2LD */

/* sub_51663a: native, nfs3hp::watcom::normalize */

/* sub_51667b: native, nfs3hp::watcom::scalePowers */

/* sub_5166cf: native, nfs3hp::watcom::scaleEnd */

/* sub_5166d8: native, nfs3hp::watcom::scaleBy */

/* sub_516759: native, nfs3hp::watcom::LDScale10x */

/* sub_5167b3: native, nfs3hp::frontend::watbLongDoubleToDigits */

/* sub_5169ef: native, nfs3hp::frontend::watbDigitsLoop */

/* sub_516a87: native, nfs3hp::frontend::watbEightDigits */

/* sub_516ab4: native, nfs3hp::frontend::watbTwoDigits */

/* sub_516a9d: native, nfs3hp::frontend::watbFourDigits */

/* sub_516acc: native, nfs3hp::frontend::watbAfterEightDigits */

/* sub_516ae2: native, nfs3hp::frontend::watbRoundDigits */

/* sub_516be2: native, nfs3hp::frontend::watbEndLongDouble */

/* sub_516bf5: native, nfs3hp::frontend::watbFormatF */

/* sub_516dfa: native, nfs3hp::frontend::watbEndFormat */

/* sub_516e13: native, nfs3hp::frontend::watbFormatE */

/* sub_517170: native, nfs3hp::graphics::useDriverSlot */

/* sub_517370: native, nfs3hp::graphics::keepDriverSlot */

/* sub_517560: native, nfs3hp::dead::stub_0x517560 */

/* sub_517570: native, nfs3hp::dead::stub_0x517570 */

/* sub_517580: native, nfs3hp::frontend::winaGraphicsClose */

/* sub_5175b0: native, nfs3hp::frontend::winaGraphicsAskClose */

/* sub_517610: native, nfs3hp::graphics::setVideoMode */

/* sub_517890: native, nfs3hp::graphics::lockFrameBuffer */

/* sub_5178d0: native, nfs3hp::graphics::releaseFrameLock */

/* sub_517920: native, nfs3hp::frontend::winaGraphicsPalette */

/* sub_517990: native, nfs3hp::graphics::releaseAndFlip */

/* sub_5179d0: native, nfs3hp::graphics::setUpScreen */

/* sub_517b30: native, nfs3hp::frontend::eaTextStart */

/* sub_517b70: native, nfs3hp::frontend::eaTextEnd */

/* sub_517ba0: native, nfs3hp::frontend::keyboardIsPc98 */

/* sub_517bf0: native, nfs3hp::frontend::memPoolDump */

/* sub_517d60: native, nfs3hp::frontend::memPoolDumpDefault */

/* sub_517d70: native, nfs3hp::frontend::debugKeyDisplayFlag */

/* sub_517da0: native, nfs3hp::frontend::debugKeyScreenshot */

/* sub_517de0: native, nfs3hp::frontend::debugKeysBind */

/* sub_517e50: native, nfs3hp::frontend::blockHasCrcTag */

/* sub_517eb0: native, nfs3hp::frontend::blockCrcGood */

/* sub_517f10: native, nfs3hp::frontend::blockCheckCrc */

/* sub_518040: native, nfs3hp::frontend::eaSetLoadHook */

/* sub_518050: native, nfs3hp::watcom::mbslen */

/* sub_518080: native, nfs3hp::watcom::mbstowcs */

/* sub_518100: native, nfs3hp::dead::stub_0x518100 */

/* sub_518290: native, nfs3hp::watcom::wsetenv */

/* sub_518498: native, nfs3hp::watcom::wfindenv */

/* sub_5185c0: native, nfs3hp::watcom::nrealloc */

/* sub_518640: native, nfs3hp::watcom::memmove */

/* sub_518690: native, nfs3hp::graphics::dead_0x518690 */

/* sub_518870: native, nfs3hp::graphics::dead_0x518870 */

/* sub_518d50: native, nfs3hp::graphics::dead_0x518d50 */

/* sub_518df0: native, nfs3hp::graphics::dead_0x518df0 */

}
