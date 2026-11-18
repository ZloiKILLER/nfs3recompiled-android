#ifndef NFS3HP_NATIVE_VERTICES_H
#define NFS3HP_NATIVE_VERTICES_H

#include "native_guest.h"

#include <cstdint>
#include <cstring>

/* native_vertices.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  None of these calls
 * the game's other functions; those that take `cpu` read only its x87 mode, as their thunks do:
 * floats at single precision to nearest, again through the emulated FPU should a value leave a
 * float's range. */
namespace nfs3hp
{
namespace vertices
{

// What nearestBlock found: the nearest block, -1 for none, and the blocks looked at.
struct NearestBlock
{
    x86::reg32 block;
    std::int32_t looked;
    std::int32_t count;
};

// The game's 0x41a970: `count` vertices of an object at `in` into the view `view`'s space and onto
// the screen at `out`, each given its colour from the list at `colours`.
void colouredVerticesOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 count, x86::reg32 in,
                        x86::reg32 colours, x86::reg32 out);
// The game's 0x41aab0: how far apart the points at `a` and `b` are on the ground, as the game measures it.
x86::reg32 groundDistanceAt(win32::WinApplication* app, x86::reg32 a, x86::reg32 b);
// The game's 0x41e0f0: the track block nearest the point at `point`.
NearestBlock nearestBlock(win32::WinApplication* app, x86::reg32 point);
// nearestBlock's block, what the game's callers take.
inline x86::reg32 nearestBlockTo(win32::WinApplication* app, x86::reg32 point)
{
    return nearestBlock(app, point).block;
}
// The game's 0x4968a0: the height of the point p brought onto the ground plane of the frame
// `matrix` at o.
float groundHeight(win32::WinApplication* app, x86::reg32 matrix, const float (&o)[3], const float (&p)[3]);
// groundHeight of o and p as the game's callers hand them, six floats.
inline float groundHeightAt(win32::WinApplication* app, x86::reg32 matrix, float ox, float oy, float oz, float px,
                            float py, float pz)
{
    const float o[3] = {ox, oy, oz};
    const float p[3] = {px, py, pz};
    return groundHeight(app, matrix, o, p);
}
// The game's 0x4972f0: the vector at `vector` made of length 1 in place; its length, 0 for a zero one.
float normaliseVector(win32::WinApplication* app, x86::reg32 vector);
// The game's 0x49d9d0: `count` vertices of an object at `in` by `matrix` and `position` into view
// space and onto the screen at `out`, each given `colour`; the nearest z of them, 65535 for none.
float colouredNearestZ(win32::WinApplication* app, x86::CPU& cpu, std::int32_t count, x86::reg32 in, x86::reg32 out,
                       x86::reg32 matrix, x86::reg32 position, x86::reg32 colour);
// The game's 0x4bf4c0: `count` vertices of an object at `in` by `matrix` and `position` into view
// space and onto the screen at `out`.
void objectVerticesOf(win32::WinApplication* app, x86::CPU& cpu, std::int32_t count, x86::reg32 in, x86::reg32 matrix,
                      x86::reg32 position, x86::reg32 out);
// The game's 0x4dffb0 (Add) and 0x4e0000: `count` vectors at `in`, each with the one at `by`, into `out`.
template <bool Add>
void offsetVectorList(win32::WinApplication* app, std::int32_t count, x86::reg32 in, x86::reg32 by, x86::reg32 out);
// The game's 0x4e0050: `count` vectors at `in` times `scale` into `out`.
void scaleVectorList(win32::WinApplication* app, std::int32_t count, x86::reg32 in, double scale, x86::reg32 out);
// scaleVectorList by a float, as the game's callers hand it.
inline void scaleVectorsBy(win32::WinApplication* app, std::int32_t count, x86::reg32 in, float scale, x86::reg32 out)
{
    scaleVectorList(app, count, in, scale, out);
}
// The game's 0x4e01c0: the vector at `from` divided by its length into `to`.
// 0x4dffb0 (Add) and 0x4e0000 as the code leaves eax: `in` past the vectors read.
template <bool Add>
x86::reg32 offsetVectorsEax(win32::WinApplication* app, std::int32_t count, x86::reg32 in, x86::reg32 by, x86::reg32 out);
// 0x4e0050 as the code leaves eax: `out` past the vectors written.
x86::reg32 scaleVectorsEax(win32::WinApplication* app, std::int32_t count, x86::reg32 in, float scale, x86::reg32 out);
// 0x4e0280 as the code leaves eax: `a`, or when `c` is `a` the copy of it in the code's frame, which lies as deep
// as the stack pointer at the call (cpu.esp as the callee has it, the return address's slot) puts it.
x86::reg32 multiplyMatrixEax(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 c);
void makeUnitVector(win32::WinApplication* app, x86::reg32 from, x86::reg32 to);
// The game's 0x4e01f0: the dot product of the vectors at `a` and `b`, as wide as the x87 holds it.
double dotProduct(win32::WinApplication* app, x86::reg32 a, x86::reg32 b);
// The game's 0x4e0210: `count` vectors at `in` each dotted with the one at `d`, the floats into `out`.
void dotProductsOf(win32::WinApplication* app, x86::CPU& cpu, std::int32_t count, x86::reg32 in, x86::reg32 d,
                   x86::reg32 out);
// The game's 0x4e0280: the 3x3 matrix c = a b.
void multiplyMatrix(win32::WinApplication* app, x86::reg32 a, x86::reg32 b, x86::reg32 c);
// The game's 0x4e03b0: `count` vectors at `in` turned by `matrix` into `out`.
void turnVectors(win32::WinApplication* app, x86::CPU& cpu, std::int32_t count, x86::reg32 in, x86::reg32 matrix,
                 x86::reg32 out);
// The game's 0x4e0430: `count` points at `in` turned by `matrix` and moved by `move`, into `out`.
void transformPointList(win32::WinApplication* app, std::int32_t count, x86::reg32 in, x86::reg32 matrix,
                        x86::reg32 move, x86::reg32 out);
// The game's 0x4e06a0: the length of the vector at `vector`, as wide as the x87 holds it.
double vectorLength(win32::WinApplication* app, x86::reg32 vector);

// The game's 0x4c11b0: the triangle a, b, c to the screen; eax what the code leaves there.
x86::reg32 triangleClipped(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 c);
// The game's 0x4c12e0: the quad a, b, c, d to the screen; eax what the code leaves there.
x86::reg32 quadClipped(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 c, x86::reg32 d);
// The game's 0x4c7610: the polygons of the list `list` into the buckets, `start` the stack's word (ret 4); eax.
x86::reg32 bucketsSortedInto(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list, x86::reg32 start);
// The game's 0x4c7790: the frame's polygons of `list` into the depth buckets, `start` the stack's word (ret 4).
void bucketsDepthedInto(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list, x86::reg32 start);
// The game's 0x492370: a light's glow for view `view`; `w0` (the lowest) to `w6` the seven stack words (ret 0x1c); eax.
x86::reg32 lightGlowDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 w0, x86::reg32 w1,
                          x86::reg32 w2, x86::reg32 w3, x86::reg32 w4, x86::reg32 w5, x86::reg32 w6);

// The game's 0x4c12e0: the quad of the four vertices at a, b, c and d clipped and drawn; its answer; esp the frame base.
x86::reg32 quadClippedAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 esp, x86::reg32 a, x86::reg32 b,
                       x86::reg32 c, x86::reg32 d);
// The game's 0x4c11b0: the triangle of the vertices at a, b and c clipped and drawn; its answer; esp the frame base.
x86::reg32 triangleClippedAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 esp, x86::reg32 a, x86::reg32 b,
                           x86::reg32 c);
// The game's 0x4c1e20: `count` quads of the mesh (vertices, quad numbers) drawn; its answer; esp the frame base.
x86::reg32 quadMeshDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 esp, x86::sreg32 count,
                         x86::reg32 vertices, x86::reg32 quads);

// 0x41a550 (stdcall, 1 word), 0x4dbef0 (2), 0x41b640 (1), 0x41b300 (2), 0x41ae90 (3), 0x41bf30, 0x4c7790 (1), 0x4c79a0
// as typed cores (their thunks' registers and stack words as parameters); the eax each leaves is the result.
void trackVerticesCore(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 count, x86::reg32 in,
                       x86::reg32 out, x86::reg32 record);
x86::reg32 objectVerticesListCore(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix, x86::reg32 move,
                                  x86::reg32 count, x86::reg32 vertices, x86::reg32 colours, x86::reg32 out);
x86::reg32 objectRecordsCore(win32::WinApplication* app, x86::CPU& cpu, x86::sreg32 count, x86::reg32 polygon,
                             x86::reg32 record, x86::reg32 base);
x86::reg32 trackRecordsCore(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 records, x86::reg32 pieceRow,
                            x86::reg32 pieceColumn, x86::reg32 tail, x86::reg32 head);
x86::reg32 trackPolygonsCore(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 pieceRow,
                             x86::reg32 pieceColumn, x86::reg32 records, x86::reg32 tail, x86::reg32 head);
x86::reg32 boxOutsideCore(win32::WinApplication* app, x86::reg32 view, x86::reg32 corners, x86::reg32 matrix,
                          x86::reg32 move);
void depthBucketsCore(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list, x86::reg32 start);
void gatherBucketsCore(win32::WinApplication* app, x86::reg32 deep, x86::reg32 out);
// 0x4c11b0, 0x4c1aa0, 0x4c12e0, 0x434380: a triangle, a triangle by z, a quad to the screen; the polygon records
// from `first` on drawn (the record it stopped at).
x86::reg32 clipTriangleCore(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 c);
void clipTriangleByZCore(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 c);
x86::reg32 clipQuadCore(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 c,
                        x86::reg32 d);
x86::reg32 submitPolygonsCore(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first);
}  // namespace vertices

}  // namespace nfs3hp

#endif
