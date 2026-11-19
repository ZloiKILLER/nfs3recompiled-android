#include "eacsnd.h"
#include <game_types.h>
#include <lib/app.h>

namespace nfs3hp
{
nfs::u32 iSNDdirectcaps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectcreate3dbuf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectplay3d(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectpos3d(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectrate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectrecordpacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectrecordstart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectrecordstop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectremovebuf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectserve(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectsetfunctions(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectstart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectstop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectstopbuf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectvol(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdllversion(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}

namespace eacsnd
{

nfs::Library s_library = nfs::Library("eacsnd.dll");
nfs::Library* s_registry = &(s_library
        .registerSymbol("_iSNDdirectcaps@4", &nfs3hp::iSNDdirectcaps)
        .registerSymbol("_iSNDdirectcreate3dbuf@28", &nfs3hp::iSNDdirectcreate3dbuf)
        .registerSymbol("_iSNDdirectplay3d@28", &nfs3hp::iSNDdirectplay3d)
        .registerSymbol("_iSNDdirectpos3d@12", &nfs3hp::iSNDdirectpos3d)
        .registerSymbol("_iSNDdirectrate@8", &nfs3hp::iSNDdirectrate)
        .registerSymbol("_iSNDdirectrecordpacket@4", &nfs3hp::iSNDdirectrecordpacket)
        .registerSymbol("_iSNDdirectrecordstart@16", &nfs3hp::iSNDdirectrecordstart)
        .registerSymbol("_iSNDdirectrecordstop@0", &nfs3hp::iSNDdirectrecordstop)
        .registerSymbol("_iSNDdirectremovebuf@4", &nfs3hp::iSNDdirectremovebuf)
        .registerSymbol("_iSNDdirectserve@0", &nfs3hp::iSNDdirectserve)
        .registerSymbol("_iSNDdirectsetfunctions@32", &nfs3hp::iSNDdirectsetfunctions)
        .registerSymbol("_iSNDdirectstart@8", &nfs3hp::iSNDdirectstart)
        .registerSymbol("_iSNDdirectstop@0", &nfs3hp::iSNDdirectstop)
        .registerSymbol("_iSNDdirectstopbuf@4", &nfs3hp::iSNDdirectstopbuf)
        .registerSymbol("_iSNDdirectvol@8", &nfs3hp::iSNDdirectvol)
        .registerSymbol("_iSNDdllversion@0", &nfs3hp::iSNDdllversion)

    );
}
