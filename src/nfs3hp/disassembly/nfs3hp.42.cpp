#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4ffce0: native, nfs3hp::port::bufferPosition */

/* sub_4ffd80: native, nfs3hp::snd::dead_0x4ffd80 */

/* sub_4ffd84: native, nfs3hp::port::mixFunctions */

/* sub_4fff18: native, nfs3hp::port::iSNDmixinit */

/* sub_500008: native, nfs3hp::port::iSNDmixreinit */

/* sub_5000b8: native, nfs3hp::port::iSNDmixstart */

/* sub_5004c8: native, nfs3hp::port::mixerRamp */

/* sub_500604: native, nfs3hp::port::iSNDmixstop */

/* sub_500654: native, nfs3hp::port::setSendVolume */

/* sub_5006a4: native, nfs3hp::port::setMixVolume */

/* sub_5006f8: native, nfs3hp::snd::dead_0x5006f8 */

/* sub_500864: native, nfs3hp::port::mixerServe */

/* sub_5008a0: native, nfs3hp::port::convert16 */

/* sub_500910: native, nfs3hp::port::convertToWords */

/* sub_500990: native, nfs3hp::port::convertXaStereo */

/* sub_500b10: native, nfs3hp::port::convert8 */

/* sub_500b90: native, nfs3hp::port::convertXaMono */

/* sub_500c80: native, nfs3hp::port::soundTick */

/* sub_500e20: native, nfs3hp::port::iSNDenteraudio */

/* sub_500e7c: native, nfs3hp::port::iSNDleaveaudio */

/* sub_500e94: native, nfs3hp::port::iSNDserveradd100hzclient */

/* sub_500ef0: native, nfs3hp::port::iSNDserverremove100hzclient */

/* sub_500f80: native, nfs3hp::snd::dead_0x500f80 */

/* sub_500f8c: native, nfs3hp::watcom::set_errno_dos */

/* sub_500ff4: native, nfs3hp::watcom::set_errno_nt */

/* sub_501010: native, nfs3hp::ea::checkSurfaceUnlocked */

/* sub_501050: native, nfs3hp::ea::fileReport */

/* sub_501110: native, nfs3hp::ea::virtualAllocPages */

/* sub_501180: native, nfs3hp::ea::virtualFreePages */

/* sub_5011a0: native, nfs3hp::ea::touchMemoryPages */

/* sub_501270: native, nfs3hp::ea::openBlockHandle */

/* sub_501360: native, nfs3hp::ea::blockHandleFile */

/* sub_5013a0: native, nfs3hp::ea::closeBlockHandle */

/* sub_5013b0: native, nfs3hp::ea::readBlockHandle */

/* sub_501450: native, nfs3hp::ea::seekBlockHandle */

/* sub_5014a0: native, nfs3hp::port::updateVoiceVolume */

/* sub_501540: native, nfs3hp::port::iSNDvol */

/* sub_501670: native, nfs3hp::port::soundVolume */

/* sub_5016c0: native, nfs3hp::port::iSNDclearchunk */

/* sub_5017b4: native, nfs3hp::port::platformChunks */

/* sub_5017c0: native, nfs3hp::port::timbreDefaults */

/* sub_5017d8: native, nfs3hp::port::resetTimbre */

/* sub_5018dc: native, nfs3hp::port::addTimbreDeltas */

/* sub_501938: native, nfs3hp::port::newGroup */

/* sub_501978: native, nfs3hp::port::iSNDplaytaggedtimbre */

/* sub_501de8: native, nfs3hp::port::iSNDplaytaggedpatch */

/* sub_5021ac: native, nfs3hp::port::iSNDresolvetaggedpatch */

/* sub_5022d0: native, nfs3hp::port::unresolveTaggedPatch */

/* sub_502330: native, nfs3hp::port::iSNDextracthdrinfo */

/* sub_5024a0: native, nfs3hp::port::iSNDbankremovepat */

/* sub_502570: native, nfs3hp::port::bankUsable */

/* sub_5025b0: native, nfs3hp::port::iSNDbankalloc */

/* sub_502620: native, nfs3hp::port::iSNDdownloadbank */

/* sub_502760: native, nfs3hp::watcom::flush */

/* sub_502880: native, nfs3hp::watcom::set_errno */

/* sub_502890: native, nfs3hp::watcom::set_EDOM */

/* sub_5028a4: native, nfs3hp::watcom::set_ERANGE */

/* sub_5028b8: native, nfs3hp::dead::stub_0x5028b8 */

/* sub_5028c8: native, nfs3hp::watcom::set_doserrno */

/* sub_5028e0: native, nfs3hp::watcom::lseek */

/* sub_502960: native, nfs3hp::watcom::tell */

/* sub_5029c0: native, nfs3hp::watcom::ioalloc */

/* sub_502a50: native, nfs3hp::watcom::qread */

/* sub_502af0: native, nfs3hp::watcom::fgetc */

/* sub_502bb8: native, nfs3hp::watcom::filbuf */

/* sub_502be8: native, nfs3hp::watcom::fill_buffer */

/* sub_502cb0: native, nfs3hp::port::updateVoicePitch */

/* sub_502d30: native, nfs3hp::port::iSNDplatformdetune */

/* sub_502df0: native, nfs3hp::port::iSNDplatform3dpos */

/* sub_502ef0: native, nfs3hp::port::iSNDpan */

/* sub_502ff0: native, nfs3hp::port::iSNDwinsetfx */

/* sub_5030ac: native, nfs3hp::port::setBusFx */

/* sub_5030c4: native, nfs3hp::port::busLevelSet */

/* sub_5030d0: native, nfs3hp::port::SNDfxlevel */

/* sub_503280: native, nfs3hp::pieces::stub_0x503280 */

/* sub_503281: native, nfs3hp::pieces::stub_0x503281 */

/* sub_5033e4: native, nfs3hp::mix::copySwapped */

/* sub_5033e5: native, nfs3hp::mix::copy */

/* sub_50356c: native, nfs3hp::pieces::stub_0x50356c */

/* sub_50356d: native, nfs3hp::pieces::stub_0x50356d */

/* sub_503670: native, nfs3hp::port::mul64 */

/* sub_503700: native, nfs3hp::dead::stub_0x503700 */

/* sub_503760: native, nfs3hp::port::div64 */

/* sub_5037a0: native, nfs3hp::dead::stub_0x5037a0 */

/* sub_503800: native, nfs3hp::dead::stub_0x503800 */

/* sub_503860: native, nfs3hp::dead::stub_0x503860 */

/* sub_5038c0: native, nfs3hp::dead::stub_0x5038c0 */

/* sub_503910: native, nfs3hp::dead::stub_0x503910 */

/* sub_503990: native, nfs3hp::dead::stub_0x503990 */

/* sub_503a10: native, nfs3hp::dead::stub_0x503a10 */

/* sub_503aa0: native, nfs3hp::dead::stub_0x503aa0 */

/* sub_503b00: native, nfs3hp::frontend::winaShiftTo64 */

/* sub_503b60: native, nfs3hp::dead::stub_0x503b60 */

/* sub_503bc0: native, nfs3hp::frontend::watbReadPixel */

/* sub_503cd0: native, nfs3hp::frontend::winaPaletteToSurface */

/* sub_503e50: native, nfs3hp::frontend::winaPaletteSet */

/* sub_503e80: native, nfs3hp::dead::stub_0x503e80 */

/* sub_503ec0: native, nfs3hp::frontend::winaPaletteGet */

/* sub_503f10: native, nfs3hp::dead::stub_0x503f10 */

/* sub_503f50: native, nfs3hp::dead::stub_0x503f50 */

}
