#ifndef EACSND_H_
#define EACSND_H_
#include <game_types.h>
#include <lib/app.h>
#include <lib/library.h>

/* eacsnd.dll is native: its exports are the port's functions
 * (disassemble_nfs3hp.py), and only its data is here, as it loads. */

namespace eacsnd
{

extern const nfs::u8 s_AUTOSegment[];
extern const nfs::u8 s__idataSegment[];
extern const nfs::u8 s_DGROUPSegment[];
extern const nfs::u8 s__edataSegment[];

extern nfs::Library* s_registry;

}

#endif /* !EACSND_H_ */
