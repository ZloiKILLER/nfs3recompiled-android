#ifndef NFS3HP_NATIVE_THRASH_H
#define NFS3HP_NATIVE_THRASH_H

#include <x86.h>

namespace win32
{
class WinApplication;
struct GrVertex;
}

namespace nfs3hp
{

/* One of the game's vertices as voodoo2a's THRASH functions hand it to Glide. */
void thrashVertex(win32::WinApplication* app, x86::reg32 in, float zScale, float wScale, win32::GrVertex& out);

/* Three of them to the renderer, as THRASH_drawtri hands them to grDrawTriangle. */
void thrashTriangle(win32::WinApplication* app, x86::reg32 a, x86::reg32 b, x86::reg32 c);

/* Whether voodoo2a draws its triangles straight through grDrawTriangle, as it
 * does unless the game asked for its two-pass or antialiased drawing. */
bool thrashPlainTriangles(win32::WinApplication* app);

/* Whether the game draws through ThrashRenderer, the native driver's own
 * renderer (NFS_THRASH_GL, on by default), rather than the Voodoo2 emulated
 * over an atlas. */
bool thrashRendererWanted();

/* From native_vertices.cpp: NFS_NATIVE_CHECK, NFS_NATIVES, and whether one of
 * its checks is running the generated code, when every stand-in steps aside. */
bool nativeChecking();
bool nativesEnabled();
bool nativeOriginalRunning();

}

#endif
