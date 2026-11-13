#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_523465: native, nfs3hp::graphics::dead_0x523465 */

/* sub_523488: native, nfs3hp::dead::stub_0x523488 */

/* sub_5234a0: native, nfs3hp::dead::stub_0x5234a0 */

/* sub_5234d0: native, nfs3hp::watcom::GetNTAccessAttr */

/* sub_523504: native, nfs3hp::watcom::GetNTShareAttr */

/* sub_523560: native, nfs3hp::watcom::isatty */

/* sub_5235d0: native, nfs3hp::dead::stub_0x5235d0 */

/* sub_5235dc: native, nfs3hp::dead::stub_0x5235dc */

/* sub_5235f0: native, nfs3hp::dead::stub_0x5235f0 */

/* sub_523610: native, nfs3hp::watcom::fullpath */

/* sub_5236c0: native, nfs3hp::frontend::watcShapeRowBytes */

/* sub_523760: native, nfs3hp::frontend::watcPutPixel */

/* sub_5239ec: native, nfs3hp::frontend::watcBlend4To8 */

/* sub_523b00: native, nfs3hp::frontend::watcWritePixel */

/* sub_523c00: native, nfs3hp::watcom::mbtowc */

/* sub_523ca0: native, nfs3hp::watcom::FDFS */

/* sub_523cec: native, nfs3hp::watcom::U8M */

/* sub_523d10: native, nfs3hp::watcom::mbsnicmp */

/* sub_523d90: native, nfs3hp::port::platformPacketSize */

/* sub_523d94: native, nfs3hp::port::iSNDplatformpacketplay */

/* sub_523f90: native, nfs3hp::port::iSNDcheckplayopts */

/* sub_524190: native, nfs3hp::watcom::EnterWVIDEO */

/* sub_5241c0: native, nfs3hp::watcom::nheapshrink */

/* sub_5241fc: native, nfs3hp::watcom::ReturnMemToSystem */

/* sub_52425c: native, nfs3hp::watcom::ReleaseNHeap */

/* sub_524290: native, nfs3hp::watcom::callMain */

/* sub_5242f0: native, nfs3hp::watcom::wcstombs */

/* sub_5243a7: native, nfs3hp::watcom::reporterror */

/* sub_5243b1: native, nfs3hp::watcom::reportError */

/* sub_52448c: native, nfs3hp::frontend::longDoubleKind */

/* sub_5244e0: native, nfs3hp::graphics::driverIdle */

/* sub_5244f0: native, nfs3hp::frontend::winaGraphicsIdle */

/* sub_524500: native, nfs3hp::frontend::winaSetDirectDraw */

/* sub_524550: native, nfs3hp::frontend::winaGraphicsLock */

/* sub_5245a0: native, nfs3hp::frontend::winaDriverMessage */

/* sub_5245f0: native, nfs3hp::frontend::winaDriverEvent */

/* sub_524620: native, nfs3hp::frontend::winaCloseDriverOnly */

/* sub_524640: native, nfs3hp::graphics::closeDriverAtExit */

/* sub_52468f: native, nfs3hp::pieces::stub_0x52468f */

/* sub_524650: native, nfs3hp::graphics::closeDriver */

/* sub_5246a0: native, nfs3hp::graphics::openDriver */

/* sub_5247ab: native, nfs3hp::pieces::stub_0x5247ab */

/* sub_5247c0: native, nfs3hp::frontend::winaSetScreenSize */

/* sub_524820: native, nfs3hp::graphics::dead_0x524820 */

/* sub_5248f0: native, nfs3hp::graphics::unlockFrameBuffer */

/* sub_524930: native, nfs3hp::frontend::debugBoxCopyScreen */

/* sub_524950: native, nfs3hp::frontend::debugBoxPutBack */

/* sub_524970: native, nfs3hp::frontend::debugBoxAsk */

/* sub_524bb0: native, nfs3hp::frontend::exitConfirm */

/* sub_524c30: native, nfs3hp::frontend::debugBanner */

/* sub_524d90: native, nfs3hp::frontend::debugKeyPause */

/* sub_524dd0: native, nfs3hp::frontend::debugKeyViewerFirst */

/* sub_524de0: native, nfs3hp::frontend::debugKeyViewerSecond */

/* sub_524df0: native, nfs3hp::frontend::blockResize */

/* sub_524f80: native, nfs3hp::dead::stub_0x524f80 */

/* sub_524f88: native, nfs3hp::frontend::watcInitEnviron */

/* sub_52506c: native, nfs3hp::frontend::watcFreeEnviron */

/* sub_5250c0: native, nfs3hp::watcom::mbterm */

/* sub_525100: native, nfs3hp::watcom::mbsinc */

/* sub_525130: native, nfs3hp::dead::stub_0x525130 */

/* sub_5251a0: native, nfs3hp::pieces::stub_0x5251a0 */

/* sub_5252a0: native, nfs3hp::watcom::create_wide_environment */

/* sub_525300: native, nfs3hp::watcom::wcslen */

/* sub_525320: native, nfs3hp::watcom::memcpy */

/* sub_525350: native, nfs3hp::watcom::wcscpy */

/* sub_525370: native, nfs3hp::watcom::towupper */

/* sub_525380: native, nfs3hp::watcom::msize */

/* sub_525390: native, nfs3hp::watcom::nexpand */

/* sub_5253cc: native, nfs3hp::watcom::HeapManager_expand */

/* sub_525578: native, nfs3hp::watcom::nexpandFailed */

/* sub_525590: native, nfs3hp::dead::stub_0x525590 */

/* sub_525600: native, nfs3hp::frontend::winaFreeSurfaceCache */

/* sub_525630: native, nfs3hp::frontend::winaNextByte */

/* sub_525650: native, nfs3hp::frontend::winaMapCharacter */

/* sub_525670: native, nfs3hp::frontend::winaNextCharacter */

/* sub_5256b0: native, nfs3hp::dead::stub_0x5256b0 */

/* sub_5257c0: native, nfs3hp::frontend::fontSetProperty */

/* sub_5259d0: native, nfs3hp::frontend::fontGetProperty */

/* sub_525ac0: native, nfs3hp::dead::stub_0x525ac0 */

/* sub_525b80: native, nfs3hp::frontend::rendFontSetOrigin */

/* sub_525b90: native, nfs3hp::dead::stub_0x525b90 */

/* sub_525ba0: native, nfs3hp::frontend::watcSetMbcp */

/* sub_525d04: native, nfs3hp::dead::stub_0x525d04 */

/* sub_525d59: native, nfs3hp::watcom::U8D */

/* sub_525df0: native, nfs3hp::watcom::calloc */

/* sub_525e10: native, nfs3hp::frontend::watcAttachThisThread */

/* sub_525e4c: native, nfs3hp::frontend::watcResizeThreadData */

/* sub_525f20: native, nfs3hp::frontend::watcAddThread */

/* sub_525f84: native, nfs3hp::frontend::watcRemoveThread */

/* sub_525fd0: native, nfs3hp::dead::stub_0x525fd0 */

/* sub_525ff8: native, nfs3hp::frontend::watcFreeThreadList */

/* sub_526030: native, nfs3hp::watcom::initThread */

/* sub_526060: native, nfs3hp::frontend::watcFreeThreadListJump */

/* sub_526070: native, nfs3hp::frontend::watcClearFpExceptions */

/* sub_526080: native, nfs3hp::frontend::watcSetSignalHandler */

/* sub_5260c8: native, nfs3hp::frontend::watcSignalHandler */

/* sub_5260ec: native, nfs3hp::frontend::watcSignalMask */

/* sub_526110: native, nfs3hp::frontend::watcSignalForCode */

/* sub_52612c: native, nfs3hp::frontend::watcConsoleControl */

/* sub_526194: native, nfs3hp::frontend::watcConsoleWanted */

}
