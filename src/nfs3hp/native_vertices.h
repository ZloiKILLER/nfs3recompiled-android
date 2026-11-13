#ifndef NFS3HP_NATIVE_VERTICES_H
#define NFS3HP_NATIVE_VERTICES_H

#include "native_guest.h"

#include <cstdint>
#include <cstring>

/* native_vertices.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  None of these calls
 * the game's other functions; those that take `cpu` read only its x87 mode, as their adapters do:
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

/* For the ByAddress below: a call of the game's by address with `registers` in eax, edx, ebx and
 * ecx in turn and `words` on the stack (the callee's ret takes them off), every register but eax
 * kept; false when the game stopped in it.  And a float's bits, as the stack holds it. */
inline bool stackCallKeeping(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address,
                             std::initializer_list<x86::reg32> registers, std::initializer_list<x86::reg32> words)
{
    const guest::KeptRegisters kept(cpu);
    x86::reg32* const into[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    unsigned i = 0;
    for (const x86::reg32 value : registers)
        *into[i++] = value;
    return guest::call(app, cpu, address, words);
}

inline x86::reg32 floatWord(float value)
{
    x86::reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

}  // namespace vertices

namespace guest
{

// The view in eax, the count in edx, the vertices in ebx, the colours in ecx; out on the stack (ret 4).
template <>
struct ByAddress<vertices::colouredVerticesOf>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 count, x86::reg32 in,
                     x86::reg32 colours, x86::reg32 out)
    {
        vertices::stackCallKeeping(app, cpu, Address, {view, count, in, colours}, {out});
    }
};

// The points in eax and edx; the distance in eax.
template <>
struct ByAddress<vertices::groundDistanceAt>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        watcom(app, cpu, Address, {a, b});
        return cpu.eax;
    }
};

// The point in eax; the block in eax.
template <>
struct ByAddress<vertices::nearestBlockTo>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 point)
    {
        watcom(app, cpu, Address, {point});
        return cpu.eax;
    }
};

// The matrix in eax, o and p on the stack (ret 0x18); the height left on the x87's stack.
template <>
struct ByAddress<vertices::groundHeightAt>
{
    template <x86::reg32 Address>
    static float call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix, float ox, float oy, float oz,
                      float px, float py, float pz)
    {
        using vertices::floatWord;
        if (!vertices::stackCallKeeping(app, cpu, Address, {matrix},
                                        {floatWord(ox), floatWord(oy), floatWord(oz), floatWord(px), floatWord(py),
                                         floatWord(pz)}))
            return 0.0f;
        return float(cpu.fpu.take());
    }
};

// The vector in eax; the length left on the x87's stack.
template <>
struct ByAddress<vertices::normaliseVector>
{
    template <x86::reg32 Address>
    static float call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 vector)
    {
        const std::optional<x86::Float> length = watcomFloat(app, cpu, Address, {vector});
        return length ? float(*length) : 0.0f;
    }
};

// The count in eax, the vertices in edx, the matrix in ebx, the position in ecx; out on the stack (ret 4).
template <>
struct ByAddress<vertices::objectVerticesOf>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, std::int32_t count, x86::reg32 in, x86::reg32 matrix,
                     x86::reg32 position, x86::reg32 out)
    {
        vertices::stackCallKeeping(app, cpu, Address, {x86::reg32(count), in, matrix, position}, {out});
    }
};

// The count in eax, the vectors in edx, the one in ebx, out in ecx.
template <>
struct ByAddress<vertices::offsetVectorList<true>>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, std::int32_t count, x86::reg32 in, x86::reg32 by,
                     x86::reg32 out)
    {
        watcom(app, cpu, Address, {x86::reg32(count), in, by, out});
    }
};

template <>
struct ByAddress<vertices::offsetVectorList<false>>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, std::int32_t count, x86::reg32 in, x86::reg32 by,
                     x86::reg32 out)
    {
        watcom(app, cpu, Address, {x86::reg32(count), in, by, out});
    }
};

// The count in eax, the vectors in edx; the scale and out on the stack (ret 8).
template <>
struct ByAddress<vertices::scaleVectorsBy>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, std::int32_t count, x86::reg32 in, float scale,
                     x86::reg32 out)
    {
        vertices::stackCallKeeping(app, cpu, Address, {x86::reg32(count), in}, {vertices::floatWord(scale), out});
    }
};

// The vector in eax, to in edx.
template <>
struct ByAddress<vertices::makeUnitVector>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from, x86::reg32 to)
    {
        watcom(app, cpu, Address, {from, to});
    }
};

// The vectors in eax and edx; the product left on the x87's stack.
template <>
struct ByAddress<vertices::dotProduct>
{
    template <x86::reg32 Address>
    static double call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        const std::optional<x86::Float> product = watcomFloat(app, cpu, Address, {a, b});
        return product ? double(*product) : 0.0;
    }
};

// The count in eax, the vectors in edx, the one in ebx, out in ecx.
template <>
struct ByAddress<vertices::dotProductsOf>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, std::int32_t count, x86::reg32 in, x86::reg32 d,
                     x86::reg32 out)
    {
        watcom(app, cpu, Address, {x86::reg32(count), in, d, out});
    }
};

// a in eax, b in edx, c in ebx.
template <>
struct ByAddress<vertices::multiplyMatrix>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 c)
    {
        watcom(app, cpu, Address, {a, b, c});
    }
};

// The count in eax, the vectors in edx, the matrix in ebx, out in ecx.
template <>
struct ByAddress<vertices::turnVectors>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, std::int32_t count, x86::reg32 in, x86::reg32 matrix,
                     x86::reg32 out)
    {
        watcom(app, cpu, Address, {x86::reg32(count), in, matrix, out});
    }
};

// The count in eax, the points in edx, the matrix in ebx, the move in ecx; out on the stack (ret 4).
template <>
struct ByAddress<vertices::transformPointList>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, std::int32_t count, x86::reg32 in, x86::reg32 matrix,
                     x86::reg32 move, x86::reg32 out)
    {
        vertices::stackCallKeeping(app, cpu, Address, {x86::reg32(count), in, matrix, move}, {out});
    }
};

// The vector in eax; the length left on the x87's stack.
template <>
struct ByAddress<vertices::vectorLength>
{
    template <x86::reg32 Address>
    static double call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 vector)
    {
        const std::optional<x86::Float> length = watcomFloat(app, cpu, Address, {vector});
        return length ? double(*length) : 0.0;
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
