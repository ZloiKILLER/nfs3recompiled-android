#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_5261cc: native, nfs3hp::dead::stub_0x5261cc */

/* sub_5261fc: native, nfs3hp::frontend::watcConsoleUnhook */

/* sub_526238: native, nfs3hp::dead::stub_0x526238 */

/* sub_526244: native, nfs3hp::frontend::watcRaiseFp */

/* sub_526288: native, nfs3hp::dead::stub_0x526288 */

/* sub_526338: native, nfs3hp::frontend::watcRaise */

/* sub_5263a8: native, nfs3hp::frontend::watcInitThreadSignals */

/* sub_5263f0: native, nfs3hp::frontend::watcFiniThreadSignals */

/* sub_526420: native, nfs3hp::frontend::watcHookThreadSignals */

/* sub_52643c: native, nfs3hp::frontend::doubleDivide */

/* sub_526642: native, nfs3hp::frontend::doubleSubtract */

/* sub_526674: native, nfs3hp::frontend::doubleProductFromX87 */

/* sub_5267ff: native, nfs3hp::frontend::doubleMultiply */

/* sub_52699c: native, nfs3hp::frontend::doubleCompare */

/* sub_5269d4: native, nfs3hp::pieces::stub_0x5269d4 */

/* sub_526aeb: native, nfs3hp::pieces::stub_0x526aeb */

/* sub_526f87: native, nfs3hp::frontend::safeDivideCall */

/* sub_526f9a: native, nfs3hp::dead::stub_0x526f9a */

/* sub_526fad: native, nfs3hp::dead::stub_0x526fad */

/* sub_526fc0: native, nfs3hp::dead::stub_0x526fc0 */

/* sub_52700c: native, nfs3hp::frontend::safeDivide */

/* sub_527058: native, nfs3hp::dead::stub_0x527058 */

/* sub_5270a4: native, nfs3hp::dead::stub_0x5270a4 */

/* sub_5270f0: native, nfs3hp::mix::interpolateMono8 */

/* sub_527170: native, nfs3hp::mix::interpolateStereo8 */

/* sub_527210: native, nfs3hp::mix::interpolateMono16 */

/* sub_527290: native, nfs3hp::mix::interpolateStereo16 */

/* sub_527330: native, nfs3hp::snd::dead_0x527330 */

/* sub_5273c0: native, nfs3hp::snd::dead_0x5273c0 */

/* sub_5274b0: native, nfs3hp::port::pcmOnceMono */

/* sub_5274d0: native, nfs3hp::port::pcmLoopedMono */

/* sub_527500: native, nfs3hp::port::pcmStreamedMono */

/* sub_527520: native, nfs3hp::port::pcmFileMono */

/* sub_527540: native, nfs3hp::port::pcmOnceStereo */

/* sub_527560: native, nfs3hp::port::pcmLoopedStereo */

/* sub_527590: native, nfs3hp::port::pcmStreamedStereo */

/* sub_5275b0: native, nfs3hp::port::pcmFileStereo */

/* sub_5275d0: native, nfs3hp::snd::dead_0x5275d0 */

/* sub_5275f0: native, nfs3hp::snd::dead_0x5275f0 */

/* sub_527620: native, nfs3hp::snd::dead_0x527620 */

/* sub_527640: native, nfs3hp::snd::dead_0x527640 */

/* sub_527660: native, nfs3hp::snd::dead_0x527660 */

/* sub_527680: native, nfs3hp::snd::dead_0x527680 */

/* sub_5276b0: native, nfs3hp::snd::dead_0x5276b0 */

/* sub_5276d0: native, nfs3hp::snd::dead_0x5276d0 */

/* sub_5276f0: native, nfs3hp::port::pcmOnceStereo2 */

/* sub_527710: native, nfs3hp::port::pcmLoopedStereo2 */

/* sub_527740: native, nfs3hp::port::pcmStreamedStereo2 */

/* sub_527760: native, nfs3hp::port::pcmFileStereo2 */

/* sub_527780: native, nfs3hp::port::pcmOnceQuad */

/* sub_5277a0: native, nfs3hp::port::pcmLoopedQuad */

/* sub_5277d0: native, nfs3hp::port::pcmStreamedQuad */

/* sub_5277f0: native, nfs3hp::port::pcmFileQuad */

/* sub_527810: native, nfs3hp::snd::dead_0x527810 */

/* sub_527840: native, nfs3hp::snd::dead_0x527840 */

/* sub_527870: native, nfs3hp::snd::dead_0x527870 */

/* sub_527890: native, nfs3hp::snd::dead_0x527890 */

/* sub_5278b0: native, nfs3hp::snd::dead_0x5278b0 */

/* sub_5278e0: native, nfs3hp::snd::dead_0x5278e0 */

/* sub_527910: native, nfs3hp::snd::dead_0x527910 */

/* sub_527930: native, nfs3hp::snd::dead_0x527930 */

/* sub_527950: native, nfs3hp::snd::dead_0x527950 */

/* sub_527954: native, nfs3hp::snd::dead_0x527954 */

/* sub_527ae8: native, nfs3hp::snd::dead_0x527ae8 */

/* sub_527b30: native, nfs3hp::snd::dead_0x527b30 */

/* sub_527b8c: native, nfs3hp::snd::dead_0x527b8c */

/* sub_527d54: native, nfs3hp::snd::dead_0x527d54 */

/* sub_527f20: native, nfs3hp::snd::dead_0x527f20 */

/* sub_527f80: native, nfs3hp::snd::dead_0x527f80 */

/* sub_52813c: native, nfs3hp::snd::dead_0x52813c */

/* sub_528180: native, nfs3hp::snd::dead_0x528180 */

/* sub_5283a4: native, nfs3hp::snd::dead_0x5283a4 */

/* sub_528400: native, nfs3hp::snd::dead_0x528400 */

/* sub_528404: native, nfs3hp::snd::dead_0x528404 */

/* sub_5285b8: native, nfs3hp::snd::dead_0x5285b8 */

/* sub_528610: native, nfs3hp::snd::dead_0x528610 */

/* sub_528680: native, nfs3hp::snd::dead_0x528680 */

/* sub_5288b0: native, nfs3hp::snd::dead_0x5288b0 */

/* sub_528a98: native, nfs3hp::snd::dead_0x528a98 */

/* sub_528b00: native, nfs3hp::snd::dead_0x528b00 */

/* sub_528cac: native, nfs3hp::snd::dead_0x528cac */

/* sub_528d00: native, nfs3hp::snd::dead_0x528d00 */

/* sub_528f0c: native, nfs3hp::snd::dead_0x528f0c */

/* sub_528f70: native, nfs3hp::port::xaPlayed */

/* sub_528f74: native, nfs3hp::port::xaOnceDecode */

/* sub_529104: native, nfs3hp::port::xaOnceSetup */

/* sub_529140: native, nfs3hp::port::xaKeptCopy */

/* sub_5291a8: native, nfs3hp::port::xaLoopDecode */

/* sub_529384: native, nfs3hp::port::xaIntroDecode */

/* sub_529534: native, nfs3hp::port::xaIntroSetup */

/* sub_529590: native, nfs3hp::port::xaStreamDecode */

/* sub_529778: native, nfs3hp::port::xaStreamSetup */

/* sub_5297c0: native, nfs3hp::port::xaPacketDecode */

/* sub_5299f4: native, nfs3hp::port::xaPacketSetup */

/* sub_529a50: native, nfs3hp::port::xaStereoPlayed */

/* sub_529a54: native, nfs3hp::port::xaStereoOnceDecode */

/* sub_529c2c: native, nfs3hp::port::xaStereoOnceSetup */

/* sub_529c80: native, nfs3hp::port::xaStereoKeptCopy */

/* sub_529cfc: native, nfs3hp::port::xaStereoLoopDecode */

/* sub_529f24: native, nfs3hp::port::xaStereoIntroDecode */

}
