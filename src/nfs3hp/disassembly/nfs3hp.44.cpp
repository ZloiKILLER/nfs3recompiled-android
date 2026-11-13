#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_509b10: native, nfs3hp::frontend::watbRemoveRow */

/* sub_509bd0: native, nfs3hp::dead::stub_0x509bd0 */

/* sub_509c00: native, nfs3hp::frontend::watbFindBlockHeader */

/* sub_509c50: native, nfs3hp::frontend::watbCopyNibbles */

/* sub_509c60: native, nfs3hp::frontend::watbCopyBytes */

/* sub_509c70: native, nfs3hp::frontend::watbBytesToNibbles */

/* sub_509cd0: native, nfs3hp::frontend::watbNibblesToBytes */

/* sub_509d20: native, nfs3hp::frontend::watb565ToIndex */

/* sub_509df0: native, nfs3hp::frontend::watb8888To1555Keyed */

/* sub_509e50: native, nfs3hp::frontend::watb8888To1555 */

/* sub_509eb0: native, nfs3hp::frontend::watbCopyWords */

/* sub_509ec0: native, nfs3hp::dead::stub_0x509ec0 */

/* sub_509ef0: native, nfs3hp::frontend::watb8888To565Keyed */

/* sub_509f50: native, nfs3hp::frontend::watb8888To565 */

/* sub_509fb0: native, nfs3hp::frontend::watb8888To4444 */

/* sub_50a020: native, nfs3hp::frontend::watb1555To4444 */

/* sub_50a0a0: native, nfs3hp::frontend::watb8888To8565 */

/* sub_50a100: native, nfs3hp::frontend::watbIndexTo888 */

/* sub_50a150: native, nfs3hp::frontend::watb565To888 */

/* sub_50a1c0: native, nfs3hp::frontend::watbCopyTriples */

/* sub_50a1e0: native, nfs3hp::frontend::watb8888To888 */

/* sub_50a240: native, nfs3hp::frontend::watb1555To8888 */

/* sub_50a2b0: native, nfs3hp::frontend::watb565To8888 */

/* sub_50a330: native, nfs3hp::frontend::watb4444To8888 */

/* sub_50a3c0: native, nfs3hp::frontend::watb8555To8888 */

/* sub_50a440: native, nfs3hp::frontend::watb8565To8888 */

/* sub_50a4c0: native, nfs3hp::frontend::watb888To8888 */

/* sub_50a510: native, nfs3hp::frontend::watbCopyDwords */

/* sub_50a520: native, nfs3hp::dead::stub_0x50a520 */

/* sub_50a559: native, nfs3hp::frontend::watbDepthIndex15 */

/* sub_50a550: native, nfs3hp::frontend::watbDepthIndex32 */

/* sub_50a543: native, nfs3hp::frontend::watbDepthIndex16 */

/* sub_50a570: native, nfs3hp::frontend::watbDepthIndex */

/* sub_50a5d0: native, nfs3hp::frontend::watbClutPointer */

/* sub_50a700: native, nfs3hp::dead::stub_0x50a700 */

/* sub_50a830: native, nfs3hp::frontend::watbClut16aPointer */

/* sub_50a960: native, nfs3hp::frontend::watbConverterFor */

/* sub_50aad0: native, nfs3hp::frontend::watbConvertShape */

/* sub_50af80: native, nfs3hp::dead::stub_0x50af80 */

/* sub_50afa0: native, nfs3hp::dead::stub_0x50afa0 */

/* sub_50afc0: native, nfs3hp::frontend::watbSpanTo16Keyed */

/* sub_50b080: native, nfs3hp::frontend::watbSpanTwoSteps */

/* sub_50b0b0: native, nfs3hp::frontend::watbMoveShape */

/* sub_50b540: native, nfs3hp::frontend::watbNextByte */

/* sub_50b560: native, nfs3hp::frontend::watbCharWidthEntry */

/* sub_50b580: native, nfs3hp::frontend::watbNextChar */

/* sub_50b5c0: native, nfs3hp::frontend::watbTextAlpha */

/* sub_50ba90: native, nfs3hp::frontend::watbTextPlotted */

/* sub_50bb80: native, nfs3hp::frontend::watbTextMono */

/* sub_50bdf0: native, nfs3hp::frontend::watbBlendTable */

/* sub_50bfd0: native, nfs3hp::frontend::watbFadeTable */

/* sub_50c0d0: native, nfs3hp::frontend::watbInitTextDraw */

/* sub_50c180: native, nfs3hp::frontend::watbTextPalette */

/* sub_50c1e0: native, nfs3hp::frontend::watbFreeTextDraw */

/* sub_50c220: native, nfs3hp::frontend::watbPlotMask8 */

/* sub_50c270: native, nfs3hp::frontend::watbPlotMask16 */

/* sub_50c2c0: native, nfs3hp::frontend::watbPlotMask24 */

/* sub_50c350: native, nfs3hp::frontend::watbPlotMask32 */

/* sub_50c3a0: native, nfs3hp::frontend::watbColourToArgb */

/* sub_50c540: native, nfs3hp::watcom::setbits */

/* sub_50c584: native, nfs3hp::frontend::watbDispatchKey */

/* sub_50c5d4: native, nfs3hp::frontend::watbRegisterKey */

/* sub_50c640: native, nfs3hp::watcom::cgetc */

/* sub_50c648: native, nfs3hp::watcom::uncget */

/* sub_50c650: native, nfs3hp::watcom::scnf */

/* sub_50c904: native, nfs3hp::watcom::get_opt */

/* sub_50ca08: native, nfs3hp::watcom::scan_white */

/* sub_50ca48: native, nfs3hp::watcom::scan_char */

/* sub_50cb30: native, nfs3hp::watcom::scan_string */

/* sub_50cc88: native, nfs3hp::watcom::report_scan */

/* sub_50cce8: native, nfs3hp::watcom::makelist */

/* sub_50cd2c: native, nfs3hp::watcom::scan_arb */

/* sub_50ce18: native, nfs3hp::watcom::scan_float */

/* sub_50d0f0: native, nfs3hp::watcom::scan_int */

/* sub_50d510: native, nfs3hp::watcom::radix_value */

/* sub_50d538: native, nfs3hp::watcom::chktrm */

/* sub_50d560: native, nfs3hp::frontend::watbCopyBlock */

/* sub_50d6d0: native, nfs3hp::frontend::watbNothing50d6d0 */

/* sub_50d6e0: native, nfs3hp::frontend::watbBlendSpanIndexed */

/* sub_50d7e0: native, nfs3hp::frontend::watbBlendSpan1555 */

/* sub_50d880: native, nfs3hp::frontend::watbBlendSpan565 */

/* sub_50d910: native, nfs3hp::frontend::watbBlendSpan888 */

/* sub_50d9c0: native, nfs3hp::frontend::watbBlendSpan8888 */

/* sub_50daa0: native, nfs3hp::frontend::watbBlendSpanPalette565 */

/* sub_50db30: native, nfs3hp::frontend::watbDrawAlphaShape */

/* sub_50dbb0: native, nfs3hp::dead::stub_0x50dbb0 */

/* sub_50dbd0: native, nfs3hp::dead::stub_0x50dbd0 */

/* sub_50dbf0: native, nfs3hp::frontend::watbNothing50dbf0 */

/* sub_50dc00: native, nfs3hp::ea::qnth */

/* sub_50dc20: native, nfs3hp::ea::qunlink */

/* sub_50dcd0: native, nfs3hp::frontend::qinit */

/* sub_50dd60: native, nfs3hp::frontend::qinitpeer */

/* sub_50dde0: native, nfs3hp::ea::qfree */

/* sub_50de50: native, nfs3hp::frontend::qpush */

/* sub_50ded0: native, nfs3hp::frontend::qbottom */

/* sub_50df60: native, nfs3hp::frontend::qpop */

/* sub_50dff0: native, nfs3hp::frontend::qpoplast */

/* sub_50e0a0: native, nfs3hp::frontend::qinsert */

/* sub_50e150: native, nfs3hp::ea::qremove */

/* sub_50e1d0: native, nfs3hp::ea::qlength */

}
