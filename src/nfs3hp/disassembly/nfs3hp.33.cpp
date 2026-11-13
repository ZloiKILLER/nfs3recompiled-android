#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4e10e0: native, nfs3hp::ea::unlinkFreeBlock */

/* sub_4e1100: native, nfs3hp::ea::clearMemClasses */

/* sub_4e1120: native, nfs3hp::ea::markMemBlock */

/* sub_4e11e0: native, nfs3hp::ea::memClassOverhead */

/* sub_4e11f0: native, nfs3hp::ea::memBlockOverhead */

/* sub_4e1230: native, nfs3hp::ea::creatememclass */

/* sub_4e14b0: native, nfs3hp::ea::releasememclass */

/* sub_4e1500: native, nfs3hp::ea::getfreememclass */

/* sub_4e1580: native, nfs3hp::ea::largestmem */

/* sub_4e15b0: native, nfs3hp::ea::largestmemclass */

/* sub_4e15d0: native, nfs3hp::ea::memBlockSize */

/* sub_4e15e0: native, nfs3hp::ea::memBlockName */

/* sub_4e1610: native, nfs3hp::ea::memBlockFlags */

/* sub_4e1620: native, nfs3hp::ea::reservememadr */

/* sub_4e1890: native, nfs3hp::ea::purgememadr */

/* sub_4e1990: native, nfs3hp::ea::purgememclass */

/* sub_4e19d0: native, nfs3hp::ea::reservememStdcall */

/* sub_4e19f0: native, nfs3hp::ea::purgememStdcall */

/* sub_4e1a00: native, nfs3hp::StopSoundBufferLambda */

/* sub_4e1a0c: native, nfs3hp::AcquireSoundBufferPositionLambda */

/* sub_4e1a24: native, nfs3hp::AcquireSoundDataLambda */

/* sub_4e1a34: native, nfs3hp::PrintSoundLambda */

/* sub_4e1a64: native, nfs3hp::TraceSoundLambda */

/* sub_4e1a88: native, nfs3hp::abortmessagewrapper */

/* sub_4e1adc: native, nfs3hp::TickSoundLambda */

/* sub_4e1ae4: native, nfs3hp::iSNDconvertsample */

/* sub_4e1d1c: native, nfs3hp::port::leaveAudioLocks */

/* sub_4e1d48: native, nfs3hp::audiofocushandler */

/* sub_4e1dd4: native, nfs3hp::iSNDloaddll */

/* sub_4e217c: native, nfs3hp::port::iSNDunloaddll */

/* sub_4e21b0: native, nfs3hp::SNDcaps */

/* sub_4e2224: native, nfs3hp::dead::stub_0x4e2224 */

/* sub_4e2228: native, nfs3hp::port::SNDsetrate */

/* sub_4e2234: native, nfs3hp::SoundServeHandler */

/* sub_4e225c: native, nfs3hp::port::iSNDmessageinit */

/* sub_4e2450: native, nfs3hp::port::iSNDinit */

/* sub_4e253c: native, nfs3hp::port::iSNDrestore */

/* sub_4e257c: native, nfs3hp::port::soundIdle */

/* sub_4e25c0: native, nfs3hp::port::iSNDstop */

/* sub_4e2630: native, nfs3hp::port::SNDinit */

/* sub_4e26b4: native, nfs3hp::port::SNDrestore */

/* sub_4e2754: native, nfs3hp::port::iSNDverifymode */

/* sub_4e28a0: native, nfs3hp::watcom::unlink */

/* sub_4e28c0: native, nfs3hp::ea::initHandlesDefault */

/* sub_4e28d0: native, nfs3hp::ea::openHandle */

/* sub_4e2ab0: native, nfs3hp::ea::openHandleErrorsAs */

/* sub_4e2ae0: native, nfs3hp::ea::openHandleToWrite */

/* sub_4e2ba0: native, nfs3hp::ea::openMapHandle */

/* sub_4e2da0: native, nfs3hp::ea::closeHandle */

/* sub_4e2de0: native, nfs3hp::ea::readHandle */

/* sub_4e2e20: native, nfs3hp::ea::writeHandle */

/* sub_4e2e60: native, nfs3hp::ea::seekHandle */

/* sub_4e2eb0: native, nfs3hp::ea::isSlash */

/* sub_4e2ebe: native, nfs3hp::ea::notSlash */

/* sub_4e2ed0: native, nfs3hp::ea::volumeRoot */

/* sub_4e2fd0: native, nfs3hp::ea::fullPathName */

/* sub_4e3110: native, nfs3hp::ea::logicalDrives */

/* sub_4e3120: native, nfs3hp::ea::driveOfPath */

/* sub_4e3250: native, nfs3hp::ea::driveLetter */

/* sub_4e32e0: native, nfs3hp::ea::diskFreeBytes */

/* sub_4e3330: native, nfs3hp::ea::driveNumber */

/* sub_4e3340: native, nfs3hp::ea::setDirectory */

/* sub_4e33a0: native, nfs3hp::ea::getDirectory */

/* sub_4e3410: native, nfs3hp::ea::handlesReady */

/* sub_4e3430: native, nfs3hp::ea::initHandles */

/* sub_4e34c0: native, nfs3hp::ea::closeAllHandles */

/* sub_4e3570: native, nfs3hp::ea::takeHandleSlot */

/* sub_4e3600: native, nfs3hp::ea::deleteFile */

/* sub_4e3620: native, nfs3hp::ea::openFileHandle */

/* sub_4e3980: native, nfs3hp::ea::handleInfo */

/* sub_4e3ae0: native, nfs3hp::ea::readHandleBytes */

/* sub_4e3cc0: native, nfs3hp::ea::writeHandleBytes */

/* sub_4e3e60: native, nfs3hp::ea::seekHandleTo */

/* sub_4e3f30: native, nfs3hp::ea::handlePosition */

/* sub_4e3fe0: native, nfs3hp::ea::closeHandleSlot */

/* sub_4e40a0: native, nfs3hp::ea::lockHandleDrive */

/* sub_4e40f0: native, nfs3hp::ea::unlockHandleDrive */

/* sub_4e4140: native, nfs3hp::ea::openMemoryHandle */

/* sub_4e41f0: native, nfs3hp::ea::handleMemory */

/* sub_4e4230: native, nfs3hp::frontend::winaFileSizeAndSeek */

/* sub_4e42e0: native, nfs3hp::dead::stub_0x4e42e0 */

/* sub_4e4320: native, nfs3hp::frontend::winaFileLockOf */

/* sub_4e4360: native, nfs3hp::frontend::fileSizeOpened */

/* sub_4e43a0: native, nfs3hp::frontend::fileSize */

/* sub_4e43b0: native, nfs3hp::dead::stub_0x4e43b0 */

/* sub_4e43c0: native, nfs3hp::dead::stub_0x4e43c0 */

/* sub_4e43d0: native, nfs3hp::dead::stub_0x4e43d0 */

/* sub_4e4420: native, nfs3hp::port::initStreamStruct */

/* sub_4e45d0: native, nfs3hp::port::initStreamStructSecond */

/* sub_4e45e0: native, nfs3hp::port::initStreamStructFirst */

/* sub_4e45f0: native, nfs3hp::port::initStream */

/* sub_4e4780: native, nfs3hp::port::initStreamSecond */

/* sub_4e4790: native, nfs3hp::port::initStreamFirst */

/* sub_4e47a0: native, nfs3hp::pieces::stub_0x4e47a0 */

/* sub_4e4a10: native, nfs3hp::dead::stub_0x4e4a10 */

/* sub_4e4a30: native, nfs3hp::port::deleteStreamStruct */

/* sub_4e4b50: native, nfs3hp::port::deleteStream */

/* sub_4e4c60: native, nfs3hp::port::streamCommand */

/* sub_4e4d70: native, nfs3hp::port::purgeStreamCommand */

/* sub_4e4ea0: native, nfs3hp::port::streamOpen */

}
