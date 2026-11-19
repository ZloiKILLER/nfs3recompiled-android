#include "voodoo2a.h"
#include <game_types.h>
#include <lib/app.h>

namespace nfs3hp
{
nfs::u32 THRASH_about(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_clearwindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_clip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawline(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawlinemesh(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawlinestrip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawpoint(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawpointmesh(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawpointstrip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawquad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawquadmesh(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawtri(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawtrifan(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawtrimesh(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_drawtristrip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_flushwindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_idle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_init(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_is(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_lockwindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_pageflip(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_readrect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_restore(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_selectdisplay(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_setstate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_settexture(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_setvideomode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_sync(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_talloc(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_treset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_tupdate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_unlockwindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_window(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 THRASH_writerect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}

namespace voodoo2a
{

nfs::Library s_library = nfs::Library("voodoo2a.dll");
nfs::Library* s_registry = &(s_library
        .registerSymbol("_THRASH_about@0", &nfs3hp::THRASH_about)
        .registerSymbol("_THRASH_clearwindow@0", &nfs3hp::THRASH_clearwindow)
        .registerSymbol("_THRASH_clip@16", &nfs3hp::THRASH_clip)
        .registerSymbol("_THRASH_drawline@8", &nfs3hp::THRASH_drawline)
        .registerSymbol("_THRASH_drawlinemesh@12", &nfs3hp::THRASH_drawlinemesh)
        .registerSymbol("_THRASH_drawlinestrip@8", &nfs3hp::THRASH_drawlinestrip)
        .registerSymbol("_THRASH_drawpoint@4", &nfs3hp::THRASH_drawpoint)
        .registerSymbol("_THRASH_drawpointmesh@12", &nfs3hp::THRASH_drawpointmesh)
        .registerSymbol("_THRASH_drawpointstrip@8", &nfs3hp::THRASH_drawpointstrip)
        .registerSymbol("_THRASH_drawquad@16", &nfs3hp::THRASH_drawquad)
        .registerSymbol("_THRASH_drawquadmesh@12", &nfs3hp::THRASH_drawquadmesh)
        .registerSymbol("_THRASH_drawtri@12", &nfs3hp::THRASH_drawtri)
        .registerSymbol("_THRASH_drawtrifan@8", &nfs3hp::THRASH_drawtrifan)
        .registerSymbol("_THRASH_drawtrimesh@12", &nfs3hp::THRASH_drawtrimesh)
        .registerSymbol("_THRASH_drawtristrip@8", &nfs3hp::THRASH_drawtristrip)
        .registerSymbol("_THRASH_flushwindow@0", &nfs3hp::THRASH_flushwindow)
        .registerSymbol("_THRASH_idle@0", &nfs3hp::THRASH_idle)
        .registerSymbol("_THRASH_init@0", &nfs3hp::THRASH_init)
        .registerSymbol("_THRASH_is@0", &nfs3hp::THRASH_is)
        .registerSymbol("_THRASH_lockwindow@0", &nfs3hp::THRASH_lockwindow)
        .registerSymbol("_THRASH_pageflip@0", &nfs3hp::THRASH_pageflip)
        .registerSymbol("_THRASH_readrect@20", &nfs3hp::THRASH_readrect)
        .registerSymbol("_THRASH_restore@0", &nfs3hp::THRASH_restore)
        .registerSymbol("_THRASH_selectdisplay@4", &nfs3hp::THRASH_selectdisplay)
        .registerSymbol("_THRASH_setstate@8", &nfs3hp::THRASH_setstate)
        .registerSymbol("_THRASH_settexture@4", &nfs3hp::THRASH_settexture)
        .registerSymbol("_THRASH_setvideomode@12", &nfs3hp::THRASH_setvideomode)
        .registerSymbol("_THRASH_sync@4", &nfs3hp::THRASH_sync)
        .registerSymbol("_THRASH_talloc@20", &nfs3hp::THRASH_talloc)
        .registerSymbol("_THRASH_treset@0", &nfs3hp::THRASH_treset)
        .registerSymbol("_THRASH_tupdate@12", &nfs3hp::THRASH_tupdate)
        .registerSymbol("_THRASH_unlockwindow@4", &nfs3hp::THRASH_unlockwindow)
        .registerSymbol("_THRASH_window@4", &nfs3hp::THRASH_window)
        .registerSymbol("_THRASH_writerect@20", &nfs3hp::THRASH_writerect)

    );
}
