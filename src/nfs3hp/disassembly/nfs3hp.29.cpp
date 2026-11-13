#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4ccbfa: native, nfs3hp::pieces::stub_0x4ccbfa */

/* sub_4ccc01: native, nfs3hp::dead::stub_0x4ccc01 */

/* sub_4ccc08: native, nfs3hp::dead::stub_0x4ccc08 */

/* sub_4ccc0f: native, nfs3hp::dead::stub_0x4ccc0f */

/* sub_4ccc16: native, nfs3hp::dead::stub_0x4ccc16 */

/* sub_4ccc1d: native, nfs3hp::dead::stub_0x4ccc1d */

/* sub_4ccc24: native, nfs3hp::dead::stub_0x4ccc24 */

/* sub_4ccc2b: native, nfs3hp::dead::stub_0x4ccc2b */

/* sub_4ccc32: native, nfs3hp::dead::stub_0x4ccc32 */

/* sub_4ccc39: native, nfs3hp::dead::stub_0x4ccc39 */

/* sub_4ccc40: native, nfs3hp::dead::stub_0x4ccc40 */

/* sub_4ccc47: native, nfs3hp::dead::stub_0x4ccc47 */

/* sub_4ccc4e: native, nfs3hp::dead::stub_0x4ccc4e */

/* sub_4ccc55: native, nfs3hp::dead::stub_0x4ccc55 */

/* sub_4ccc5c: native, nfs3hp::dead::stub_0x4ccc5c */

/* sub_4ccc63: native, nfs3hp::dead::stub_0x4ccc63 */

/* sub_4ccc70: native, nfs3hp::frontend::winaKeyboardHook */

/* sub_4ccd60: native, nfs3hp::frontend::winaOnSetFocus */

/* sub_4ccdd0: native, nfs3hp::frontend::winaOnKillFocus */

/* sub_4cce40: native, nfs3hp::frontend::winaOnClose */

/* sub_4cce60: native, nfs3hp::frontend::winaOnQuitAsked */

/* sub_4cce80: native, nfs3hp::frontend::winaOnDestroy */

/* sub_4cce90: native, nfs3hp::frontend::winaOnSysCommand */

/* sub_4cced0: native, nfs3hp::frontend::winaOnSettingChange */

/* sub_4ccf50: native, nfs3hp::frontend::winaOnMoved */

/* sub_4ccfc0: native, nfs3hp::frontend::winaOnChar */

/* sub_4cd020: native, nfs3hp::frontend::winaOnKeyDown */

/* sub_4cd0e0: native, nfs3hp::frontend::winaOnKeyUp */

/* sub_4cd130: native, nfs3hp::frontend::winaOnMouseDown */

/* sub_4cd190: native, nfs3hp::frontend::winaOnMouseUp */

/* sub_4cd1f0: native, nfs3hp::frontend::winaOnPaint */

/* sub_4cd2a0: native, nfs3hp::frontend::winaOnEraseBackground */

/* sub_4cd2d0: native, nfs3hp::frontend::winaWindowProc */

/* sub_4cd350: native, nfs3hp::frontend::winaCreateWindow */

/* sub_4cd4e0: native, nfs3hp::frontend::winaWindowThread */

/* sub_4cd6c0: native, nfs3hp::frontend::winaOpenWindow */

/* sub_4cdaa0: native, nfs3hp::frontend::winaCloseWindow */

/* sub_4cdb10: native, nfs3hp::frontend::winaPlaceWindow */

/* sub_4cdbd0: native, nfs3hp::frontend::winaCentreWindow */

/* sub_4cdc20: native, nfs3hp::dead::stub_0x4cdc20 */

/* sub_4cdc50: native, nfs3hp::frontend::winaHookOff */

/* sub_4cdc90: native, nfs3hp::frontend::winaHookOn */

/* sub_4cdcd0: native, nfs3hp::dead::stub_0x4cdcd0 */

/* sub_4cdd00: native, nfs3hp::frontend::lapsStandingsStart */

/* sub_4cdd30: native, nfs3hp::frontend::lapsCompareCars */

/* sub_4cde00: native, nfs3hp::frontend::lapsStandingsUpdate */

/* sub_4cded0: native, nfs3hp::frontend::lapsPlaceOf */

/* sub_4cdee0: native, nfs3hp::frontend::lapsRacerCount */

/* sub_4cdef0: native, nfs3hp::frontend::lapsCarInPlace */

/* sub_4cdf10: native, nfs3hp::frontend::lapsPerformanceTimes */

/* sub_4ce280: native, nfs3hp::frontend::lapsCarTask */

/* sub_4ce5a0: native, nfs3hp::frontend::lapsEstimateRest */

/* sub_4ce750: native, nfs3hp::frontend::lapsCloseRace */

/* sub_4ce800: native, nfs3hp::frontend::lapsSetStates */

/* sub_4ce840: native, nfs3hp::frontend::lapsAllFinishedKnockout */

/* sub_4ce8e0: native, nfs3hp::frontend::lapsAllFinishedTimed */

/* sub_4ce960: native, nfs3hp::frontend::lapsAllFinishedSides */

/* sub_4ceae0: native, nfs3hp::frontend::lapsCheckRaceEnd */

/* sub_4ceb50: native, nfs3hp::frontend::lapsClearShortTimes */

/* sub_4cebf0: native, nfs3hp::dead::stub_0x4cebf0 */

/* sub_4cecc0: native, nfs3hp::dead::stub_0x4cecc0 */

/* sub_4cedd0: native, nfs3hp::frontend::resultsMetricText */

/* sub_4cee40: native, nfs3hp::frontend::resultsTimeText */

/* sub_4cef00: native, nfs3hp::frontend::recordsHolder */

/* sub_4cef80: native, nfs3hp::frontend::postgameFinishers */

/* sub_4cefc0: native, nfs3hp::dead::stub_0x4cefc0 */

/* sub_4cf000: native, nfs3hp::frontend::postgameFinishersCount */

/* sub_4cf040: native, nfs3hp::frontend::postgameReplayExcluded */

/* sub_4cf090: native, nfs3hp::frontend::postgameRaceAgain */

/* sub_4cf120: native, nfs3hp::dead::stub_0x4cf120 */

/* sub_4cf160: native, nfs3hp::frontend::lapsToOption */

/* sub_4cf190: native, nfs3hp::frontend::optionToLaps */

/* sub_4cf1c0: native, nfs3hp::frontend::winaStreamCheck */

/* sub_4cf1f0: native, nfs3hp::frontend::winaRingBetween */

/* sub_4cf220: native, nfs3hp::frontend::winaStreamConsume */

/* sub_4cf280: native, nfs3hp::frontend::winaStreamTakeFree */

/* sub_4cf2f0: native, nfs3hp::frontend::winaStreamQueueFile */

/* sub_4cf340: native, nfs3hp::frontend::winaStreamFindFile */

/* sub_4cf380: native, nfs3hp::frontend::winaStreamUnlinkFile */

/* sub_4cf3f0: native, nfs3hp::frontend::winaStreamClassify */

/* sub_4cf440: native, nfs3hp::frontend::winaStreamParse */

/* sub_4cf570: native, nfs3hp::frontend::winaStreamFileOpened */

/* sub_4cf5a0: native, nfs3hp::frontend::winaStreamFileSwitched */

/* sub_4cf5e0: native, nfs3hp::frontend::winaStreamFilled */

/* sub_4cf6b0: native, nfs3hp::frontend::winaStreamNextFile */

/* sub_4cf810: native, nfs3hp::frontend::winaStreamReadMore */

/* sub_4cfa40: native, nfs3hp::frontend::winaStreamHeaderSize */

/* sub_4cfa80: native, nfs3hp::frontend::winaStreamCreate */

/* sub_4cfca0: native, nfs3hp::frontend::winaStreamSetRule */

/* sub_4cfd30: native, nfs3hp::frontend::winaStreamClose */

/* sub_4cfda0: native, nfs3hp::dead::stub_0x4cfda0 */

/* sub_4cfdd0: native, nfs3hp::frontend::winaStreamSetLowWater */

/* sub_4cfe30: native, nfs3hp::frontend::winaStreamSetBelow */

/* sub_4cfe70: native, nfs3hp::frontend::winaStreamChannel */

/* sub_4cfec0: native, nfs3hp::dead::stub_0x4cfec0 */

/* sub_4cfef0: native, nfs3hp::frontend::winaStreamOpenFile */

/* sub_4cffb0: native, nfs3hp::frontend::winaStreamOpenMemory */

/* sub_4d0060: native, nfs3hp::frontend::winaStreamCloseFile */

/* sub_4d0220: native, nfs3hp::frontend::winaStreamFlush */

/* sub_4d0300: native, nfs3hp::frontend::winaStreamNextChunk */

}
