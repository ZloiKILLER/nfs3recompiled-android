#ifndef NFS3HP_HOST_VIDEO_H
#define NFS3HP_HOST_VIDEO_H

#include "host_window.h"
#include "native_windows.h"

#include <lib/palette.h>

#include <cstdint>
#include <optional>

/* DirectDraw, the port's (plan step 6): host code on src/lib's Renderer, Surface and Palette.
 *
 * The objects live in the game's memory as COM objects -- a table of methods, the interface, a
 * reference count and the host object behind them -- because the game keeps them and calls some
 * of their methods through their tables (a movie's GetCaps and Release, the clipper).  Their
 * tables hold the addresses the port's Windows had (0x400000..0x400041), each registered to the
 * host function below that answers it.  A call site of the game's code that calls a method by its
 * offset in the table goes through guest::ddraw (below): the host function as C++ where the
 * table has that very method there, else through the table.
 *
 * Each method takes the object, then the words the game pushes for it, as the import's words
 * (native_windows.h, WindowsWord), and gives back the HRESULT (0, or 1 for what the port never
 * did).  The ddraw.dll library the game loads by name (its DirectDrawCreate) is registered by
 * installVideo. */
namespace nfs3hp
{
namespace host
{
namespace ddraw
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using x86::reg32;

/* An interface's id (GUID). */
struct Guid
{
    std::uint32_t data1;
    std::uint16_t data2;
    std::uint16_t data3;
    std::uint8_t data4[8];
};

/* A COM object in the game's memory: what the game sees of it is its table, at 0. */
struct Object
{
    reg32 table;
    Guid interfaceId;
    reg32 references;
    win32::GenericResource* resource;  // the Renderer, Surface or Palette behind it
};

struct ColourKey
{
    reg32 low;
    reg32 high;
};

struct PixelFormat  // DDPIXELFORMAT
{
    reg32 size;
    reg32 flags;
    reg32 fourCC;
    reg32 bitCount;
    reg32 redMask;
    reg32 greenMask;
    reg32 blueMask;
    reg32 alphaMask;
};

struct SurfaceDesc  // DDSURFACEDESC
{
    reg32 size;
    reg32 flags;
    reg32 height;
    reg32 width;
    std::int32_t pitch;
    reg32 backBufferCount;
    reg32 refreshRate;
    reg32 alphaBitDepth;
    reg32 reserved;
    reg32 surface;  // the bits, in the game's memory
    ColourKey destOverlay;
    ColourKey destBlt;
    ColourKey sourceOverlay;
    ColourKey sourceBlt;
    PixelFormat format;
    reg32 caps;
};

/* DDCAPS: its size, the rest as the game reads it. */
struct Caps
{
    reg32 size;
    reg32 words[90];
};

/* sub_4eb430's import and the name the game looks up in ddraw.dll. */
reg32 directDrawCreate(App* app, CPU& cpu, reg32 guid, reg32* made, reg32 outer);

// IUnknown's, in every table.
reg32 queryInterface(App* app, CPU& cpu, Object* self, const Guid* id, reg32* object);
reg32 addRef(App* app, CPU& cpu, Object* self);
reg32 release(App* app, CPU& cpu, Object* self);

// IDirectDraw2's.
reg32 drawQueryInterface(App* app, CPU& cpu, Object* self, const Guid* id, reg32* object);
reg32 drawCreatePalette(App* app, CPU& cpu, Object* self, reg32 flags, win32::PALETTEENTRY* entries, reg32* made,
                        reg32 outer);
reg32 drawCreateSurface(App* app, CPU& cpu, Object* self, SurfaceDesc* description, reg32* made, reg32 outer);
reg32 drawEnumDisplayModes(App* app, CPU& cpu, Object* self, reg32 flags, SurfaceDesc* description, reg32 context,
                           reg32 callback);
reg32 drawFlipToGdiSurface(App* app, CPU& cpu, Object* self);
reg32 drawGetCaps(App* app, CPU& cpu, Object* self, Caps* card, Caps* emulation);
reg32 drawGetDisplayMode(App* app, CPU& cpu, Object* self, SurfaceDesc* description);
reg32 drawSetCooperativeLevel(App* app, CPU& cpu, Object* self, reg32 window, reg32 flags);
reg32 drawSetDisplayMode(App* app, CPU& cpu, Object* self, reg32 width, reg32 height, reg32 bits, reg32 refresh,
                         reg32 flags);
reg32 drawGetAvailableVidMem(App* app, CPU& cpu, Object* self, reg32 caps, reg32* total, reg32* left);

// IDirectDrawPalette's.
reg32 paletteSetEntries(App* app, CPU& cpu, Object* self, reg32 flags, reg32 first, reg32 count,
                        win32::PALETTEENTRY* entries);

// IDirectDrawSurface's.
reg32 surfaceQueryInterface(App* app, CPU& cpu, Object* self, const Guid* id, reg32* object);
reg32 surfaceBlt(App* app, CPU& cpu, Object* self, Rect* destination, Object* source, Rect* sourceRect, reg32 flags,
                 reg32 effects);
reg32 surfaceFlip(App* app, CPU& cpu, Object* self, reg32 target, reg32 flags);
reg32 surfaceGetAttachedSurface(App* app, CPU& cpu, Object* self, reg32 caps, reg32* attached);
reg32 surfaceGetPixelFormat(App* app, CPU& cpu, Object* self, PixelFormat* format);
reg32 surfaceGetSurfaceDesc(App* app, CPU& cpu, Object* self, SurfaceDesc* description);
reg32 surfaceIsLost(App* app, CPU& cpu, Object* self);
reg32 surfaceLock(App* app, CPU& cpu, Object* self, Rect* rect, SurfaceDesc* description, reg32 flags, reg32 event);
reg32 surfaceRestore(App* app, CPU& cpu, Object* self);
reg32 surfaceSetClipper(App* app, CPU& cpu, Object* self, reg32 clipper);
reg32 surfaceSetColorKey(App* app, CPU& cpu, Object* self, reg32 flags, ColourKey* key);
reg32 surfaceSetPalette(App* app, CPU& cpu, Object* self, reg32 palette);
reg32 surfaceUnlock(App* app, CPU& cpu, Object* self, reg32 bits);

/* The address in an object's table that stands for a method above (0 for none). */
template <auto Method>
inline constexpr reg32 kTableAddress = 0;
template <> inline constexpr reg32 kTableAddress<&queryInterface> = 0x400000;
template <> inline constexpr reg32 kTableAddress<&addRef> = 0x400001;
template <> inline constexpr reg32 kTableAddress<&release> = 0x400002;
template <> inline constexpr reg32 kTableAddress<&drawQueryInterface> = 0x400003;
template <> inline constexpr reg32 kTableAddress<&drawCreatePalette> = 0x400006;
template <> inline constexpr reg32 kTableAddress<&drawCreateSurface> = 0x400007;
template <> inline constexpr reg32 kTableAddress<&drawEnumDisplayModes> = 0x400009;
template <> inline constexpr reg32 kTableAddress<&drawFlipToGdiSurface> = 0x40000b;
template <> inline constexpr reg32 kTableAddress<&drawGetCaps> = 0x40000c;
template <> inline constexpr reg32 kTableAddress<&drawGetDisplayMode> = 0x40000d;
template <> inline constexpr reg32 kTableAddress<&drawSetCooperativeLevel> = 0x400015;
template <> inline constexpr reg32 kTableAddress<&drawSetDisplayMode> = 0x400016;
template <> inline constexpr reg32 kTableAddress<&drawGetAvailableVidMem> = 0x400018;
template <> inline constexpr reg32 kTableAddress<&paletteSetEntries> = 0x40001c;
template <> inline constexpr reg32 kTableAddress<&surfaceQueryInterface> = 0x40001d;
template <> inline constexpr reg32 kTableAddress<&surfaceBlt> = 0x400020;
template <> inline constexpr reg32 kTableAddress<&surfaceFlip> = 0x400026;
template <> inline constexpr reg32 kTableAddress<&surfaceGetAttachedSurface> = 0x400027;
template <> inline constexpr reg32 kTableAddress<&surfaceGetPixelFormat> = 0x400030;
template <> inline constexpr reg32 kTableAddress<&surfaceGetSurfaceDesc> = 0x400031;
template <> inline constexpr reg32 kTableAddress<&surfaceIsLost> = 0x400033;
template <> inline constexpr reg32 kTableAddress<&surfaceLock> = 0x400034;
template <> inline constexpr reg32 kTableAddress<&surfaceRestore> = 0x400036;
template <> inline constexpr reg32 kTableAddress<&surfaceSetClipper> = 0x400037;
template <> inline constexpr reg32 kTableAddress<&surfaceSetColorKey> = 0x400038;
template <> inline constexpr reg32 kTableAddress<&surfaceSetPalette> = 0x40003a;
template <> inline constexpr reg32 kTableAddress<&surfaceUnlock> = 0x40003b;

}  // namespace ddraw

/* ddraw.dll registered with the game, for LoadLibrary and GetProcAddress (its DirectDrawCreate):
 * once, after the application is made. */
void installVideo(win32::WinApplication* app);

}  // namespace host

namespace guest
{

/* A method of a DirectDraw object, `object` in the game's memory and the method at `Offset` in its
 * table, `Method` the host function that answers it:
 *
 *     guest::ddraw<0x64, &host::ddraw::surfaceLock>(app, cpu, surface, 0, description, 1, 0)
 *
 * In the game, where the object's table has that very method there, the host function is called
 * as C++ on the object; anywhere else -- another table -- the call goes through the
 * table as the game's code makes it (stdcall: the object, then the words). */
template <x86::reg32 Offset, auto Method, typename... W>
inline std::optional<x86::reg32> ddraw(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 object, W... words)
{
    const x86::reg32 target = at(app, at(app, object) + Offset);
    static_assert(host::ddraw::kTableAddress<Method> != 0, "a method the host answers");
    if (object != 0 && target == host::ddraw::kTableAddress<Method>)
    {
        const x86::reg32 eax = WindowsFunction<decltype(Method)>::template call<Method>(app, cpu, object, words...);
        if (cpu.terminate)
            return std::nullopt;
        return eax;
    }
    x86::Args registers;
    return pointerCall(app, cpu, target, registers, {object, x86::reg32(words)...});
}

}  // namespace guest
}  // namespace nfs3hp

#endif
