#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4df310: native, nfs3hp::frontend::movieDisplayClose */

/* sub_4df340: native, nfs3hp::frontend::movieDisplayMode */

/* sub_4df350: native, nfs3hp::dead::stub_0x4df350 */

/* sub_4df380: native, nfs3hp::dead::stub_0x4df380 */

/* sub_4df390: native, nfs3hp::dead::stub_0x4df390 */

/* sub_4df3a0: native, nfs3hp::dead::stub_0x4df3a0 */

/* sub_4df420: native, nfs3hp::dead::stub_0x4df420 */

/* sub_4df530: native, nfs3hp::dead::stub_0x4df530 */

/* sub_4df570: native, nfs3hp::dead::stub_0x4df570 */

/* sub_4df5c0: native, nfs3hp::dead::stub_0x4df5c0 */

/* sub_4df660: native, nfs3hp::watcom::mem_putc */

/* sub_4df674: native, nfs3hp::watcom::vsprintf */

/* sub_4df690: native, nfs3hp::watcom::sprintf */

/* sub_4df6c0: native, nfs3hp::Message */

/* sub_4df6f0: native, nfs3hp::ea::waitForObjects */

/* sub_4df730: native, nfs3hp::ea::threadStart */

/* sub_4df740: native, nfs3hp::ea::threadsInit */

/* sub_4df7a0: native, nfs3hp::ea::createThread */

/* sub_4df870: native, nfs3hp::ea::removeThread */

/* sub_4df8e0: native, nfs3hp::ea::sleepMs */

/* sub_4df8f0: native, nfs3hp::ea::sleepTicks */

/* sub_4df930: native, nfs3hp::ea::isThread */

/* sub_4df980: native, nfs3hp::ea::threadPriority */

/* sub_4dfa30: native, nfs3hp::ea::setThreadPriority */

/* sub_4dfaf0: native, nfs3hp::ea::exitThread */

/* sub_4dfb00: native, nfs3hp::ea::createEvent */

/* sub_4dfb20: native, nfs3hp::ea::postEvent */

/* sub_4dfb30: native, nfs3hp::ea::signalEvent */

/* sub_4dfb40: native, nfs3hp::ea::pollEvent */

/* sub_4dfb70: native, nfs3hp::ea::waitEvent */

/* sub_4dfb90: native, nfs3hp::ea::waitEventTicks */

/* sub_4dfbf0: native, nfs3hp::ea::waitForAny */

/* sub_4dfc00: native, nfs3hp::ea::closeEvent */

/* sub_4dfc10: native, nfs3hp::ea::createManualEvent */

/* sub_4dfc30: native, nfs3hp::ea::setEvent */

/* sub_4dfc40: native, nfs3hp::ea::resetEvent */

/* sub_4dfc50: native, nfs3hp::ea::pulseEvent */

/* sub_4dfc60: native, nfs3hp::ea::waitManualEvent */

/* sub_4dfc80: native, nfs3hp::ea::setAndCloseEvent */

/* sub_4dfca0: native, nfs3hp::ea::lastErrorText */

/* sub_4dfce0: native, nfs3hp::ea::setLastError */

/* sub_4dfcf0: native, nfs3hp::ea::lastError */

/* sub_4dfd00: native, nfs3hp::ea::doNothing */

/* sub_4dfd04: native, nfs3hp::watcom::exit */

/* sub_4dfd30: native, nfs3hp::watcom::_exit */

/* sub_4dfd56: native, nfs3hp::watcom::CHP */

/* sub_4dfd80: native, nfs3hp::ea::xRotation */

/* sub_4dfde0: native, nfs3hp::ea::yRotation */

/* sub_4dfe40: native, nfs3hp::ea::zRotation */

/* sub_4dfea0: native, nfs3hp::ea::axisRotation */

/* sub_4dffb0: native, nfs3hp::vertices::addVectors */

/* sub_4e0000: native, nfs3hp::vertices::subtractVectors */

/* sub_4e0050: native, nfs3hp::vertices::scaleVectors */

/* sub_4e0090: native, nfs3hp::frontend::simVectorDistance */

/* sub_4e00c0: native, nfs3hp::dead::stub_0x4e00c0 */

/* sub_4e0110: native, nfs3hp::frontend::simTranspose */

/* sub_4e0160: native, nfs3hp::frontend::simCross */

/* sub_4e01a0: native, nfs3hp::frontend::simUnitCross */

/* sub_4e01c0: native, nfs3hp::vertices::unitVector */

/* sub_4e01f0: native, nfs3hp::vertices::dotPush */

/* sub_4e0210: native, nfs3hp::vertices::dotProducts */

/* sub_4e0250: native, nfs3hp::dead::stub_0x4e0250 */

/* sub_4e0280: native, nfs3hp::vertices::multiplyMatrices */

/* sub_4e03b0: native, nfs3hp::vertices::rotateVectors */

/* sub_4e0430: native, nfs3hp::vertices::transformPoints */

/* sub_4e04e0: native, nfs3hp::dead::stub_0x4e04e0 */

/* sub_4e05b0: native, nfs3hp::dead::stub_0x4e05b0 */

/* sub_4e0620: native, nfs3hp::dead::stub_0x4e0620 */

/* sub_4e0640: native, nfs3hp::watcom::memset */

/* sub_4e0658: native, nfs3hp::frontend::nothingA */

/* sub_4e0659: native, nfs3hp::frontend::nothingB */

/* sub_4e0660: native, nfs3hp::dead::stub_0x4e0660 */

/* sub_4e06a0: native, nfs3hp::vertices::vectorLengthOf */

/* sub_4e06c4: native, nfs3hp::dead::stub_0x4e06c4 */

/* sub_4e06d9: native, nfs3hp::watcom::IF_DATAN2 */

/* sub_4e06ee: native, nfs3hp::dead::stub_0x4e06ee */

/* sub_4e06fa: native, nfs3hp::dead::stub_0x4e06fa */

/* sub_4e070c: native, nfs3hp::fillZero */

/* sub_4e0716: native, nfs3hp::fillByte */

/* sub_4e0721: native, nfs3hp::fillWord */

/* sub_4e0820: native, nfs3hp::watcom::strstr */

/* sub_4e08f0: native, nfs3hp::ea::monoPrintf */

/* sub_4e0980: native, nfs3hp::ea::monoPrintfAt */

/* sub_4e0a60: native, nfs3hp::ea::monoWrite */

/* sub_4e0c10: native, nfs3hp::ea::monoScroll */

/* sub_4e0ca0: native, nfs3hp::ea::monoClear */

/* sub_4e0d00: native, nfs3hp::ea::monoSetTop */

/* sub_4e0d50: native, nfs3hp::ea::monoSetAttribute */

/* sub_4e0d60: native, nfs3hp::ea::monoSetTab */

/* sub_4e0d70: native, nfs3hp::ea::monoPage */

/* sub_4e0d80: native, nfs3hp::ea::monoNoCursor */

/* sub_4e0d88: native, nfs3hp::port::eaRandom */

/* sub_4e0dee: native, nfs3hp::ea::seedRandom */

/* sub_4e0e30: native, nfs3hp::watcom::strncpy */

/* sub_4e0e60: native, nfs3hp::ea::fileExists */

/* sub_4e0ec0: native, nfs3hp::ea::loadFile */

/* sub_4e0fe0: native, nfs3hp::ea::loadFileQuietly */

/* sub_4e1000: native, nfs3hp::ea::findFreeBlock */

/* sub_4e1030: native, nfs3hp::ea::findLargestFreeBlock */

/* sub_4e1080: native, nfs3hp::ea::linkFreeBlock */

}
