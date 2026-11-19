#include <nfs3hp.h>

/* The executable's jumps to its imports: each goes on to what the import's slot holds. */
namespace nfs3hp
{
namespace imports
{

#define IMPORT_THUNK(address, slot) \
    nfs::u32 thunk_##address(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args) \
    { \
        return app->callAddress(app->getMemory<nfs::u32>(slot), ts, args); \
    }
#include "native_import_thunks.inc"
#undef IMPORT_THUNK

}  // namespace imports
}  // namespace nfs3hp
