#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4e9b20: native, nfs3hp::port::SNDplaysetdef */

/* sub_4e9ba0: native, nfs3hp::port::SNDplay */

/* sub_4e9bf8: native, nfs3hp::port::playOptions */

/* sub_4e9c70: native, nfs3hp::port::SNDfxinitbus */

/* sub_4e9dfc: native, nfs3hp::port::SNDfxmasterlevel */

/* sub_4e9ec0: native, nfs3hp::watcom::stricmp */

/* sub_4e9f00: native, nfs3hp::load::typeOfBigFile */

/* sub_4e9fe0: native, nfs3hp::load::bigHeaderLength */

/* sub_4ea040: native, nfs3hp::load::findBigEntry */

/* sub_4ea290: native, nfs3hp::load::locateBigEntry */

/* sub_4ea330: native, nfs3hp::dead::stub_0x4ea330 */

/* sub_4ea360: native, nfs3hp::dead::stub_0x4ea360 */

/* sub_4ea390: native, nfs3hp::dead::stub_0x4ea390 */

/* sub_4ea3b0: native, nfs3hp::dead::stub_0x4ea3b0 */

/* sub_4ea440: native, nfs3hp::frontend::bigFileCount */

/* sub_4ea4a0: native, nfs3hp::frontend::bigFileName */

/* sub_4ea4f0: native, nfs3hp::ea::memmove */

/* sub_4ea570: native, nfs3hp::frontend::simAngle */

/* sub_4ea654: native, nfs3hp::frontend::simPackCount */

/* sub_4ea658: native, nfs3hp::frontend::simPackEntry */

/* sub_4ea665: native, nfs3hp::frontend::winaTableEntry */

/* sub_4ea680: native, nfs3hp::frontend::simTurnAboutFirstRow */

/* sub_4ea750: native, nfs3hp::dead::stub_0x4ea750 */

/* sub_4ea820: native, nfs3hp::frontend::simTurnAboutLastRow */

/* sub_4ea8f0: native, nfs3hp::dead::stub_0x4ea8f0 */

/* sub_4ea900: native, nfs3hp::dead::stub_0x4ea900 */

/* sub_4ea910: native, nfs3hp::dead::stub_0x4ea910 */

/* sub_4ea920: native, nfs3hp::frontend::simSquareRoot */

/* sub_4ea930: native, nfs3hp::frontend::simSine */

/* sub_4ea950: native, nfs3hp::frontend::simCosine */

/* sub_4ea970: native, nfs3hp::frontend::simSineCosine */

/* sub_4ea990: native, nfs3hp::dead::stub_0x4ea990 */

/* sub_4ea9c0: native, nfs3hp::frontend::simAngleOfFloats */

/* sub_4ea9e0: native, nfs3hp::frontend::simRotateX */

/* sub_4eaab0: native, nfs3hp::frontend::simRotateY */

/* sub_4eab80: native, nfs3hp::frontend::simRotateZ */

/* sub_4eac60: native, nfs3hp::frontend::angleSineCosine */

/* sub_4ead10: native, nfs3hp::dead::stub_0x4ead10 */

/* sub_4ead20: native, nfs3hp::frontend::windowApply */

/* sub_4ead80: native, nfs3hp::dead::stub_0x4ead80 */

/* sub_4ead90: native, nfs3hp::frontend::bitmapDrawToScreen */

/* sub_4eada0: native, nfs3hp::frontend::bitmapDrawInto */

/* sub_4eae90: native, nfs3hp::dead::stub_0x4eae90 */

/* sub_4eaea0: native, nfs3hp::dead::stub_0x4eaea0 */

/* sub_4eaeb0: native, nfs3hp::frontend::bitmapReadPixels */

/* sub_4eaf60: native, nfs3hp::frontend::bitmapTgaSize */

/* sub_4eafc0: native, nfs3hp::frontend::bitmapWriteTga */

/* sub_4eb160: native, nfs3hp::frontend::bitmapSaveTga */

/* sub_4eb1c0: native, nfs3hp::frontend::keyStateByte */

/* sub_4eb1d0: native, nfs3hp::watcom::mkdir */

/* sub_4eb1f0: native, nfs3hp::watcom::initrandnext */

/* sub_4eb1fc: native, nfs3hp::watcom::rand */

/* sub_4eb220: native, nfs3hp::watcom::srand */

/* sub_4eb230: native, nfs3hp::ea::randomFraction */

/* sub_4eb260: native, nfs3hp::ea::growLocks */

/* sub_4eb2e0: native, nfs3hp::ea::makeLock */

/* sub_4eb340: native, nfs3hp::ea::enterLock */

/* sub_4eb350: native, nfs3hp::ea::leaveLock */

/* sub_4eb360: native, nfs3hp::ea::freeLock */

/* sub_4eb390: native, nfs3hp::watcom::strncmp */

/* sub_4eb3c0: native, nfs3hp::graphics::makeReciprocals */

/* sub_4eb420: native, nfs3hp::pieces::stub_0x4eb420 */

/* sub_4eb430: native, nfs3hp::frontend::ddrawCreate */

/* sub_4eb490: native, nfs3hp::frontend::ddrawMakePrimary */

/* sub_4eb5e0: native, nfs3hp::frontend::ddrawLockSurface */

/* sub_4eb810: native, nfs3hp::dead::stub_0x4eb810 */

/* sub_4eb830: native, nfs3hp::dead::stub_0x4eb830 */

/* sub_4eb880: native, nfs3hp::frontend::countBits */

/* sub_4eb8a0: native, nfs3hp::frontend::ddrawNoteMode */

/* sub_4eba30: native, nfs3hp::dead::stub_0x4eba30 */

/* sub_4eba70: native, nfs3hp::AcquireWindow */

/* sub_4eba80: native, nfs3hp::frontend::movieScreenDevice */

/* sub_4eba90: native, nfs3hp::frontend::movieDirectDraw */

/* sub_4ebaa0: native, nfs3hp::dead::stub_0x4ebaa0 */

/* sub_4ebae0: native, nfs3hp::dead::stub_0x4ebae0 */

/* sub_4ebaf0: native, nfs3hp::frontend::ddrawInit */

/* sub_4ebd50: native, nfs3hp::dead::stub_0x4ebd50 */

/* sub_4ebe50: native, nfs3hp::frontend::ddrawAccessDisplay */

/* sub_4ec0d0: native, nfs3hp::frontend::ddrawRemoveSurfaces */

/* sub_4ec1a0: native, nfs3hp::frontend::ddrawConfigDisplay */

/* sub_4ec870: native, nfs3hp::frontend::ddrawSetDisplayMode */

/* sub_4ecd50: native, nfs3hp::frontend::ddrawDisposeDisplay */

/* sub_4ece00: native, nfs3hp::dead::stub_0x4ece00 */

/* sub_4ece20: native, nfs3hp::dead::stub_0x4ece20 */

/* sub_4ece60: native, nfs3hp::dead::stub_0x4ece60 */

/* sub_4eceb0: native, nfs3hp::dead::stub_0x4eceb0 */

/* sub_4ecef0: native, nfs3hp::pieces::stub_0x4ecef0 */

/* sub_4ecf20: native, nfs3hp::dead::stub_0x4ecf20 */

/* sub_4ecf60: native, nfs3hp::frontend::ddrawApplyPalette */

/* sub_4ed060: native, nfs3hp::dead::stub_0x4ed060 */

/* sub_4ed0f0: native, nfs3hp::frontend::ddrawLockBitmap */

/* sub_4ed2a0: native, nfs3hp::frontend::ddrawUnlockBitmap */

/* sub_4ed330: native, nfs3hp::frontend::ddrawRelockSurface */

/* sub_4ed440: native, nfs3hp::dead::stub_0x4ed440 */

/* sub_4ed480: native, nfs3hp::frontend::ddrawMakeSurface */

/* sub_4ed730: native, nfs3hp::frontend::ddrawFreeSurface */

/* sub_4ed760: native, nfs3hp::dead::stub_0x4ed760 */

/* sub_4ed770: native, nfs3hp::frontend::ddrawBackSurface */

/* sub_4ed7a0: native, nfs3hp::dead::stub_0x4ed7a0 */

/* sub_4ed7d0: native, nfs3hp::dead::stub_0x4ed7d0 */

}
