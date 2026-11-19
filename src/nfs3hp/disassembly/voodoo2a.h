#ifndef VOODOO2A_H_
#define VOODOO2A_H_
#include <game_types.h>
#include <lib/app.h>
#include <lib/library.h>

/* voodoo2a.dll is native: its exports are the port's functions
 * (disassemble_nfs3hp.py), and only its data is here, as it loads. */

namespace voodoo2a
{

extern const nfs::u8 s_AUTOSegment[];
extern const nfs::u8 s__idataSegment[];
extern const nfs::u8 s_DGROUPSegment[];
extern const nfs::u8 s__edataSegment[];
extern const nfs::u8 s__rsrcSegment[];

extern nfs::Library* s_registry;

}

/* What the natives hand out to be called, where the DLL's code was. */
namespace nfs3hp
{
nfs::u32 THRASH_modemessage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);  // 0xa84360
}

#endif /* !VOODOO2A_H_ */
