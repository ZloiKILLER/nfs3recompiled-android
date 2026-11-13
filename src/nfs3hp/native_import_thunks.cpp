#include <nfs3hp.h>

/* The executable's jumps to its imports: each goes on to what the import's slot holds. */
namespace nfs3hp
{
namespace imports
{

#define IMPORT_THUNK(address, slot) \
    void thunk_##address(win32::WinApplication* app, x86::CPU& cpu) \
    { \
        app->dynamic_call(app->getMemory<x86::reg32>(slot), cpu); \
    }
#include "native_import_thunks.inc"
#undef IMPORT_THUNK

}  // namespace imports
}  // namespace nfs3hp
