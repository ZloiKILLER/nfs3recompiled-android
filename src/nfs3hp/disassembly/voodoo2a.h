#ifndef VOODOO2A_H_
#define VOODOO2A_H_
#include <x86.h>
#include <lib/winapp.h>
#include <lib/library.h>

/* voodoo2a.dll is native: its exports are the port's functions
 * (disassemble_nfs3hp.py), and only its data is here, as it loads. */

namespace voodoo2a
{

extern const x86::reg8 s_AUTOSegment[];
extern const x86::reg8 s__idataSegment[];
extern const x86::reg8 s_DGROUPSegment[];
extern const x86::reg8 s__edataSegment[];
extern const x86::reg8 s__rsrcSegment[];

extern win32::Library* s_registry;

}

/* What the natives hand out to be called, where the DLL's code was. */
namespace nfs3hp
{
x86::reg32 THRASH_modemessage(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);  // 0xa84360
}

#endif /* !VOODOO2A_H_ */
