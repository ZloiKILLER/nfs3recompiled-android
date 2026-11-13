#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_5304ae: native, nfs3hp::dead::stub_0x5304ae */

/* sub_530503: native, nfs3hp::dead::stub_0x530503 */

/* sub_53054a: native, nfs3hp::dead::stub_0x53054a */

/* sub_530591: native, nfs3hp::dead::stub_0x530591 */

/* sub_5305d8: native, nfs3hp::dead::stub_0x5305d8 */

/* sub_53061f: native, nfs3hp::dead::stub_0x53061f */

/* sub_530666: native, nfs3hp::dead::stub_0x530666 */

/* sub_5306b1: native, nfs3hp::dead::stub_0x5306b1 */

/* sub_5306fc: native, nfs3hp::dead::stub_0x5306fc */

/* sub_530747: native, nfs3hp::frontend::watcLineGetDevCaps */

/* sub_530799: native, nfs3hp::dead::stub_0x530799 */

/* sub_5307eb: native, nfs3hp::dead::stub_0x5307eb */

/* sub_53083d: native, nfs3hp::frontend::watcLineGetDevConfig */

/* sub_530888: native, nfs3hp::dead::stub_0x530888 */

/* sub_5308d3: native, nfs3hp::dead::stub_0x5308d3 */

/* sub_53091e: native, nfs3hp::dead::stub_0x53091e */

/* sub_53096d: native, nfs3hp::dead::stub_0x53096d */

/* sub_5309b8: native, nfs3hp::dead::stub_0x5309b8 */

/* sub_530a03: native, nfs3hp::dead::stub_0x530a03 */

/* sub_530a4e: native, nfs3hp::frontend::watcLineGetID */

/* sub_530aa3: native, nfs3hp::dead::stub_0x530aa3 */

/* sub_530af8: native, nfs3hp::dead::stub_0x530af8 */

/* sub_530b4d: native, nfs3hp::frontend::watcLineGetLineDevStatus */

/* sub_530b94: native, nfs3hp::dead::stub_0x530b94 */

/* sub_530bdb: native, nfs3hp::dead::stub_0x530bdb */

/* sub_530c22: native, nfs3hp::dead::stub_0x530c22 */

/* sub_530c6d: native, nfs3hp::dead::stub_0x530c6d */

/* sub_530cb8: native, nfs3hp::dead::stub_0x530cb8 */

/* sub_530cff: native, nfs3hp::dead::stub_0x530cff */

/* sub_530d46: native, nfs3hp::dead::stub_0x530d46 */

/* sub_530d8d: native, nfs3hp::dead::stub_0x530d8d */

/* sub_530dd8: native, nfs3hp::dead::stub_0x530dd8 */

/* sub_530e23: native, nfs3hp::dead::stub_0x530e23 */

/* sub_530e6e: native, nfs3hp::dead::stub_0x530e6e */

/* sub_530eb9: native, nfs3hp::dead::stub_0x530eb9 */

/* sub_530f04: native, nfs3hp::dead::stub_0x530f04 */

/* sub_530f4f: native, nfs3hp::dead::stub_0x530f4f */

/* sub_530f9a: native, nfs3hp::dead::stub_0x530f9a */

/* sub_530fe5: native, nfs3hp::dead::stub_0x530fe5 */

/* sub_531030: native, nfs3hp::dead::stub_0x531030 */

/* sub_53107b: native, nfs3hp::dead::stub_0x53107b */

/* sub_5310be: native, nfs3hp::frontend::watcLineInitialize */

/* sub_531110: native, nfs3hp::dead::stub_0x531110 */

/* sub_531168: native, nfs3hp::dead::stub_0x531168 */

/* sub_5311c0: native, nfs3hp::frontend::watcLineMakeCall */

/* sub_531212: native, nfs3hp::dead::stub_0x531212 */

/* sub_531264: native, nfs3hp::dead::stub_0x531264 */

/* sub_5312b6: native, nfs3hp::dead::stub_0x5312b6 */

/* sub_5312fd: native, nfs3hp::dead::stub_0x5312fd */

/* sub_531344: native, nfs3hp::dead::stub_0x531344 */

/* sub_53138f: native, nfs3hp::frontend::watcLineNegotiateAPIVersion */

/* sub_5313e4: native, nfs3hp::dead::stub_0x5313e4 */

/* sub_531439: native, nfs3hp::frontend::watcLineOpen */

/* sub_531497: native, nfs3hp::dead::stub_0x531497 */

/* sub_5314f5: native, nfs3hp::dead::stub_0x5314f5 */

/* sub_531553: native, nfs3hp::dead::stub_0x531553 */

/* sub_5315a2: native, nfs3hp::dead::stub_0x5315a2 */

/* sub_5315f1: native, nfs3hp::dead::stub_0x5315f1 */

/* sub_531640: native, nfs3hp::dead::stub_0x531640 */

/* sub_531692: native, nfs3hp::dead::stub_0x531692 */

/* sub_5316e4: native, nfs3hp::dead::stub_0x5316e4 */

/* sub_531736: native, nfs3hp::dead::stub_0x531736 */

/* sub_531781: native, nfs3hp::dead::stub_0x531781 */

/* sub_5317cc: native, nfs3hp::dead::stub_0x5317cc */

/* sub_531817: native, nfs3hp::dead::stub_0x531817 */

/* sub_53186c: native, nfs3hp::dead::stub_0x53186c */

/* sub_5318b7: native, nfs3hp::dead::stub_0x5318b7 */

/* sub_531902: native, nfs3hp::dead::stub_0x531902 */

/* sub_53194d: native, nfs3hp::dead::stub_0x53194d */

/* sub_531998: native, nfs3hp::dead::stub_0x531998 */

/* sub_5319e7: native, nfs3hp::dead::stub_0x5319e7 */

/* sub_531a2a: native, nfs3hp::dead::stub_0x531a2a */

/* sub_531a6d: native, nfs3hp::dead::stub_0x531a6d */

/* sub_531ab4: native, nfs3hp::dead::stub_0x531ab4 */

/* sub_531af7: native, nfs3hp::dead::stub_0x531af7 */

/* sub_531b42: native, nfs3hp::dead::stub_0x531b42 */

/* sub_531b8d: native, nfs3hp::dead::stub_0x531b8d */

/* sub_531bd8: native, nfs3hp::dead::stub_0x531bd8 */

/* sub_531c27: native, nfs3hp::dead::stub_0x531c27 */

/* sub_531c7c: native, nfs3hp::dead::stub_0x531c7c */

/* sub_531cd1: native, nfs3hp::dead::stub_0x531cd1 */

/* sub_531d26: native, nfs3hp::dead::stub_0x531d26 */

/* sub_531d6d: native, nfs3hp::dead::stub_0x531d6d */

/* sub_531db8: native, nfs3hp::dead::stub_0x531db8 */

/* sub_531e0a: native, nfs3hp::dead::stub_0x531e0a */

/* sub_531e51: native, nfs3hp::dead::stub_0x531e51 */

/* sub_531ea3: native, nfs3hp::dead::stub_0x531ea3 */

/* sub_531eea: native, nfs3hp::dead::stub_0x531eea */

/* sub_531f31: native, nfs3hp::frontend::watcLineSetDevConfig */

/* sub_531f80: native, nfs3hp::dead::stub_0x531f80 */

/* sub_531fcf: native, nfs3hp::dead::stub_0x531fcf */

/* sub_53201e: native, nfs3hp::dead::stub_0x53201e */

/* sub_532069: native, nfs3hp::dead::stub_0x532069 */

/* sub_5320d0: native, nfs3hp::dead::stub_0x5320d0 */

/* sub_532117: native, nfs3hp::dead::stub_0x532117 */

/* sub_532162: native, nfs3hp::frontend::watcLineSetStatusMessages */

/* sub_5321ad: native, nfs3hp::dead::stub_0x5321ad */

/* sub_532205: native, nfs3hp::dead::stub_0x532205 */

/* sub_532254: native, nfs3hp::dead::stub_0x532254 */

/* sub_5322a3: native, nfs3hp::dead::stub_0x5322a3 */

}
