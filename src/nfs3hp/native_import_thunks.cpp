#include <nfs3hp.h>

/* The executable's jumps to its imports: each goes on to what the import's slot holds. */
namespace nfs3hp
{
namespace imports
{

#define IMPORT_THUNK(address, slot) \
    x86::reg32 thunk_##address(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args) \
    { \
        return app->callAddress(app->getMemory<x86::reg32>(slot), cpu, args); \
    }
#include "native_import_thunks.inc"
#undef IMPORT_THUNK

}  // namespace imports
}  // namespace nfs3hp
