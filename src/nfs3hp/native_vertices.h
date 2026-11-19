#ifndef NFS3HP_NATIVE_VERTICES_H
#define NFS3HP_NATIVE_VERTICES_H

#include "native_guest.h"

#include <cstdint>
#include <cstring>

/* native_vertices.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  None of these calls
 * the game's other functions; those that take `ts` read only its x87 mode, as their thunks do:
 * floats at single precision to nearest, again through the emulated FPU should a value leave a
 * float's range. */
namespace nfs3hp
{
namespace vertices
{

// What nearestBlock found: the nearest block, -1 for none, and the blocks looked at.
struct NearestBlock
{
    nfs::u32 block;
    std::int32_t looked;
    std::int32_t count;
};

// The game's 0x41a970: `count` vertices of an object at `in` into the view `view`'s space and onto
// the screen at `out`, each given its colour from the list at `colours`.
void colouredVerticesOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 count, nfs::u32 in,
                        nfs::u32 colours, nfs::u32 out);
// The game's 0x41aab0: how far apart the points at `a` and `b` are on the ground, as the game measures it.
nfs::u32 groundDistanceAt(nfs::App* app, nfs::u32 a, nfs::u32 b);
// The game's 0x41e0f0: the track block nearest the point at `point`.
NearestBlock nearestBlock(nfs::App* app, nfs::u32 point);
// nearestBlock's block, what the game's callers take.
inline nfs::u32 nearestBlockTo(nfs::App* app, nfs::u32 point)
{
    return nearestBlock(app, point).block;
}
// The game's 0x4968a0: the height of the point p brought onto the ground plane of the frame
// `matrix` at o.
float groundHeight(nfs::App* app, nfs::u32 matrix, const float (&o)[3], const float (&p)[3]);
// groundHeight of o and p as the game's callers hand them, six floats.
inline float groundHeightAt(nfs::App* app, nfs::u32 matrix, float ox, float oy, float oz, float px,
                            float py, float pz)
{
    const float o[3] = {ox, oy, oz};
    const float p[3] = {px, py, pz};
    return groundHeight(app, matrix, o, p);
}
// The game's 0x4972f0: the vector at `vector` made of length 1 in place; its length, 0 for a zero one.
float normaliseVector(nfs::App* app, nfs::u32 vector);
// The game's 0x49d9d0: `count` vertices of an object at `in` by `matrix` and `position` into view
// space and onto the screen at `out`, each given `colour`; the nearest z of them, 65535 for none.
float colouredNearestZ(nfs::App* app, nfs::ThreadState& ts, std::int32_t count, nfs::u32 in, nfs::u32 out,
                       nfs::u32 matrix, nfs::u32 position, nfs::u32 colour);
// The game's 0x4bf4c0: `count` vertices of an object at `in` by `matrix` and `position` into view
// space and onto the screen at `out`.
void objectVerticesOf(nfs::App* app, nfs::ThreadState& ts, std::int32_t count, nfs::u32 in, nfs::u32 matrix,
                      nfs::u32 position, nfs::u32 out);
// The game's 0x4dffb0 (Add) and 0x4e0000: `count` vectors at `in`, each with the one at `by`, into `out`.
template <bool Add>
void offsetVectorList(nfs::App* app, std::int32_t count, nfs::u32 in, nfs::u32 by, nfs::u32 out);
// The game's 0x4e0050: `count` vectors at `in` times `scale` into `out`.
void scaleVectorList(nfs::App* app, std::int32_t count, nfs::u32 in, double scale, nfs::u32 out);
// scaleVectorList by a float, as the game's callers hand it.
inline void scaleVectorsBy(nfs::App* app, std::int32_t count, nfs::u32 in, float scale, nfs::u32 out)
{
    scaleVectorList(app, count, in, scale, out);
}
// The game's 0x4e01c0: the vector at `from` divided by its length into `to`.
// 0x4dffb0 (Add) and 0x4e0000 as the code leaves eax: `in` past the vectors read.
template <bool Add>
nfs::u32 offsetVectorsEax(nfs::App* app, std::int32_t count, nfs::u32 in, nfs::u32 by, nfs::u32 out);
// 0x4e0050 as the code leaves eax: `out` past the vectors written.
nfs::u32 scaleVectorsEax(nfs::App* app, std::int32_t count, nfs::u32 in, float scale, nfs::u32 out);
// 0x4e0280 as the code leaves eax: `a`, or when `c` is `a` the copy of it in the code's frame, which lies as deep
// as the stack pointer at the call (ts.sp as the callee has it, the return address's slot) puts it.
nfs::u32 multiplyMatrixEax(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c);
void makeUnitVector(nfs::App* app, nfs::u32 from, nfs::u32 to);
// The game's 0x4e01f0: the dot product of the vectors at `a` and `b`, as wide as the x87 holds it.
double dotProduct(nfs::App* app, nfs::u32 a, nfs::u32 b);
// The game's 0x4e0210: `count` vectors at `in` each dotted with the one at `d`, the floats into `out`.
void dotProductsOf(nfs::App* app, nfs::ThreadState& ts, std::int32_t count, nfs::u32 in, nfs::u32 d,
                   nfs::u32 out);
// The game's 0x4e0280: the 3x3 matrix c = a b.
void multiplyMatrix(nfs::App* app, nfs::u32 a, nfs::u32 b, nfs::u32 c);
// The game's 0x4e03b0: `count` vectors at `in` turned by `matrix` into `out`.
void turnVectors(nfs::App* app, nfs::ThreadState& ts, std::int32_t count, nfs::u32 in, nfs::u32 matrix,
                 nfs::u32 out);
// The game's 0x4e0430: `count` points at `in` turned by `matrix` and moved by `move`, into `out`.
void transformPointList(nfs::App* app, std::int32_t count, nfs::u32 in, nfs::u32 matrix,
                        nfs::u32 move, nfs::u32 out);
// The game's 0x4e06a0: the length of the vector at `vector`, as wide as the x87 holds it.
double vectorLength(nfs::App* app, nfs::u32 vector);

// The game's 0x4c11b0: the triangle a, b, c to the screen; eax what the code leaves there.
nfs::u32 triangleClipped(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c);
// The game's 0x4c12e0: the quad a, b, c, d to the screen; eax what the code leaves there.
nfs::u32 quadClipped(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c, nfs::u32 d);
// The game's 0x4c7610: the polygons of the list `list` into the buckets, `start` the stack's word (ret 4); eax.
nfs::u32 bucketsSortedInto(nfs::App* app, nfs::ThreadState& ts, nfs::u32 list, nfs::u32 start);
// The game's 0x4c7790: the frame's polygons of `list` into the depth buckets, `start` the stack's word (ret 4).
void bucketsDepthedInto(nfs::App* app, nfs::ThreadState& ts, nfs::u32 list, nfs::u32 start);
// The game's 0x492370: a light's glow for view `view`; `w0` (the lowest) to `w6` the seven stack words (ret 0x1c); eax.
nfs::u32 lightGlowDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 w0, nfs::u32 w1,
                          nfs::u32 w2, nfs::u32 w3, nfs::u32 w4, nfs::u32 w5, nfs::u32 w6);

// The game's 0x4c12e0: the quad of the four vertices at a, b, c and d clipped and drawn; its answer; esp the frame base.
nfs::u32 quadClippedAt(nfs::App* app, nfs::ThreadState& ts, nfs::u32 esp, nfs::u32 a, nfs::u32 b,
                       nfs::u32 c, nfs::u32 d);
// The game's 0x4c11b0: the triangle of the vertices at a, b and c clipped and drawn; its answer; esp the frame base.
nfs::u32 triangleClippedAt(nfs::App* app, nfs::ThreadState& ts, nfs::u32 esp, nfs::u32 a, nfs::u32 b,
                           nfs::u32 c);
// The game's 0x4c1e20: `count` quads of the mesh (vertices, quad numbers) drawn; its answer; esp the frame base.
nfs::u32 quadMeshDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 esp, nfs::s32 count,
                         nfs::u32 vertices, nfs::u32 quads);

// 0x41a550 (stdcall, 1 word), 0x4dbef0 (2), 0x41b640 (1), 0x41b300 (2), 0x41ae90 (3), 0x41bf30, 0x4c7790 (1), 0x4c79a0
// as typed cores (their thunks' registers and stack words as parameters); the eax each leaves is the result.
void trackVerticesCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 count, nfs::u32 in,
                       nfs::u32 out, nfs::u32 record);
nfs::u32 objectVerticesListCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 matrix, nfs::u32 move,
                                  nfs::u32 count, nfs::u32 vertices, nfs::u32 colours, nfs::u32 out);
nfs::u32 objectRecordsCore(nfs::App* app, nfs::ThreadState& ts, nfs::s32 count, nfs::u32 polygon,
                             nfs::u32 record, nfs::u32 base);
nfs::u32 trackRecordsCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 records, nfs::u32 pieceRow,
                            nfs::u32 pieceColumn, nfs::u32 tail, nfs::u32 head);
nfs::u32 trackPolygonsCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 pieceRow,
                             nfs::u32 pieceColumn, nfs::u32 records, nfs::u32 tail, nfs::u32 head);
nfs::u32 boxOutsideCore(nfs::App* app, nfs::u32 view, nfs::u32 corners, nfs::u32 matrix,
                          nfs::u32 move);
void depthBucketsCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 list, nfs::u32 start);
void gatherBucketsCore(nfs::App* app, nfs::u32 deep, nfs::u32 out);
// 0x4c11b0, 0x4c1aa0, 0x4c12e0, 0x434380: a triangle, a triangle by z, a quad to the screen; the polygon records
// from `first` on drawn (the record it stopped at).
nfs::u32 clipTriangleCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c);
void clipTriangleByZCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c);
nfs::u32 clipQuadCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c,
                        nfs::u32 d);
nfs::u32 submitPolygonsCore(nfs::App* app, nfs::ThreadState& ts, nfs::u32 first);
}  // namespace vertices

}  // namespace nfs3hp

#endif
