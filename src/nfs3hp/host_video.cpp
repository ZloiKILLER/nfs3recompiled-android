#include "host_video.h"

#include <lib/library.h>
#include <lib/memmap.h>
#include <lib/palette.h>
#include <lib/renderer.h>
#include <lib/surface.h>
#include <lib/window.h>
#include <winapi/wrapper.h>
#include <x86.h>

#include <cstring>

/* The logic is the port's Windows' (src/lib/winapi/ddraw.cpp, com.cpp and the ddraw directory) as the game
 * ran it: moved here, not changed -- the objects' layout, their memory and its free list, the
 * tables' addresses and the display modes' descriptions where they were, and every method's
 * answer, a stub's assertion kept.  The tables' entries are the functions below through
 * win32::Wrapper (stdcall: the object, then the words, taken off by the callee). */
namespace nfs3hp
{
namespace host
{
namespace ddraw
{

namespace
{

static_assert(sizeof(Guid) == 16, "GUID");
static_assert(sizeof(SurfaceDesc) == 0x6c, "DDSURFACEDESC");
static_assert(sizeof(PixelFormat) == 0x20, "DDPIXELFORMAT");
static_assert(sizeof(Caps) == 0x16c, "DDCAPS");

constexpr Guid kIdDirectDraw2 = { 0xB3A6F3E0, 0x2B43, 0x11CF, { 0xA2, 0xDE, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56 } };
constexpr Guid kIdDirectDraw3 = { 0x618f8ad4, 0x8b7a, 0x11d0, { 0x8f, 0xcc, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d } };
constexpr Guid kIdSurface = { 0x6C14DB81, 0xA733, 0x11CE, { 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60 } };
constexpr Guid kIdSurface2 = { 0x57805885, 0x6eec, 0x11cf, { 0x94, 0x41, 0xa8, 0x23, 0x03, 0xc1, 0x0e, 0x27 } };
constexpr Guid kIdSurface3 = { 0xDA044E00, 0x69B2, 0x11D0, { 0xA1, 0xD5, 0x00, 0xAA, 0x00, 0xB8, 0xDF, 0xBB } };
constexpr Guid kIdPalette = { 0x6C14DB84, 0xA733, 0x11CE, { 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60 } };

bool same(const Guid& a, const Guid& b)
{
    return a.data1 == b.data1 && a.data2 == b.data2 && a.data3 == b.data3
           && std::memcmp(a.data4, b.data4, sizeof a.data4) == 0;
}

// The palette's capabilities (DDPCAPS).
constexpr reg32 kPalette4Bit = 0x0001, kPalette8BitEntries = 0x0002, kPalette8Bit = 0x0004;
constexpr reg32 kPalettePrimary = 0x0010, kPalettePrimaryLeft = 0x0020, kPaletteAllow256 = 0x0040;
constexpr reg32 kPaletteVsync = 0x0080, kPalette1Bit = 0x0100, kPalette2Bit = 0x0200, kPaletteAlpha = 0x0400;

/* The objects' memory: their tables, then blocks of an object each on a free list. */
win32::MemMap* s_memory = nullptr;
reg32 s_firstFree = 0;
reg32 s_unknownTable = 0, s_drawTable = 0, s_paletteTable = 0, s_surfaceTable = 0;

/* The display modes the port offers, their descriptions after IDirectDraw2's table.  The two
 * 800x600 ones are written there too, but their addresses are the 640x480 ones' (as the port's
 * Windows always had them): every mode the game is told of, or reads back, is 640x480. */
reg32 s_mode8At640 = 0, s_mode8At800 = 0, s_mode16At640 = 0, s_mode16At800 = 0;
reg32 s_currentMode = 0;

reg32 allocate(App* app)
{
    const reg32 result = s_firstFree;
    const reg32* memory = &app->getMemory<reg32>(result);
    s_firstFree = *memory;
    return result;
}

void freeBlock(App* app, Object* object)
{
    reg32* memory = reinterpret_cast<reg32*>(object);
    *memory = s_firstFree;
    const reg32 block = reg32(reinterpret_cast<x86::reg8*>(memory)
                              - &app->getMemory<const x86::reg8>(s_memory->getBlockStart()));
    s_firstFree = s_memory->getBlockStart() + block;
}

reg32 addressOf(App* app, const Object* object)
{
    const reg32 offset = reg32(reinterpret_cast<const x86::reg8*>(object)
                               - &app->getMemory<const x86::reg8>(s_memory->getBlockStart()));
    return offset + s_memory->getBlockStart();
}

/* An object made in a block: its table, its interface, one reference.  What is behind it is
 * set by the kind of object (a DirectDraw object's when the window is given it). */
Object* make(App* app, reg32 address, reg32 table, const Guid& id)
{
    Object* object = &app->getMemory<Object>(address);
    object->table = table;
    object->interfaceId = id;
    object->references = 1;
    return object;
}

void makeSurface(App* app, reg32 address, win32::Renderer* renderer, reg32 surfaceIndex)
{
    Object* surface = make(app, address, s_surfaceTable, kIdSurface3);
    surface->resource = new win32::Surface(renderer, surfaceIndex);
    app->allocateResource(surface->resource);
}

void makePalette(App* app, reg32 address, win32::Renderer* renderer, reg32 flags, const win32::PALETTEENTRY* entries)
{
    Object* palette = make(app, address, s_paletteTable, kIdPalette);
    reg32 entryCount = 1;
    NFS2_ASSERT(!(flags & kPalette8BitEntries));
    NFS2_ASSERT(!(flags & kPaletteAlpha));
    NFS2_ASSERT(!(flags & kPalettePrimary));
    NFS2_ASSERT(!(flags & kPalettePrimaryLeft));
    NFS2_ASSERT(!(flags & kPaletteVsync));
    if (flags & kPalette1Bit)
        entryCount = 2;
    else if (flags & kPalette2Bit)
        entryCount = 4;
    else if (flags & kPalette4Bit)
        entryCount = 16;
    else if (flags & kPalette8Bit || flags & kPaletteAllow256)
        entryCount = 256;
    palette->resource = new win32::Palette(renderer, entries, entryCount);
    app->allocateResource(palette->resource);
}

win32::Renderer* rendererOf(Object* draw)
{
    return dynamic_cast<win32::Renderer*>(draw->resource);
}

win32::Surface* surfaceOf(Object* surface)
{
    return dynamic_cast<win32::Surface*>(surface->resource);
}

SurfaceDesc* currentMode(App* app)
{
    return &app->getMemory<SurfaceDesc>(s_currentMode);
}

// What the port never did: told to the log, the call answered as the port's Windows answered it.
#define NFS3HP_NOT_DONE(name)                                  \
    void name(App* app, CPU& cpu)                              \
    {                                                          \
        NFS2_USE(app);                                         \
        NFS2_USE(cpu);                                         \
        NFS2_ASSERT(false);                                    \
    }

NFS3HP_NOT_DONE(drawCompact)
NFS3HP_NOT_DONE(drawCreateClipper)
NFS3HP_NOT_DONE(drawDuplicateSurface)
NFS3HP_NOT_DONE(drawEnumSurfaces)
NFS3HP_NOT_DONE(drawGetFourCCCodes)
NFS3HP_NOT_DONE(drawGetGdiSurface)
NFS3HP_NOT_DONE(drawGetMonitorFrequency)
NFS3HP_NOT_DONE(drawGetScanLine)
NFS3HP_NOT_DONE(drawGetVerticalBlankStatus)
NFS3HP_NOT_DONE(drawInitialize)
NFS3HP_NOT_DONE(drawRestoreDisplayMode)
NFS3HP_NOT_DONE(drawWaitForVerticalBlank)
NFS3HP_NOT_DONE(paletteGetCaps)
NFS3HP_NOT_DONE(paletteGetEntries)
NFS3HP_NOT_DONE(paletteInitialize)
#undef NFS3HP_NOT_DONE

// A surface's method the port never did, its words taken off as the method's: 1.
#define NFS3HP_SURFACE_NOT_DONE(name, ...)                     \
    reg32 name(App* app, CPU& cpu, Object* self, __VA_ARGS__)  \
    {                                                          \
        NFS2_USE(app);                                         \
        NFS2_USE(cpu);                                         \
        NFS2_USE(self);                                        \
        NFS2_ASSERT(false);                                    \
        return 1;                                              \
    }

NFS3HP_SURFACE_NOT_DONE(surfaceAddAttachedSurface, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceAddOverlayDirtyRect, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceBltBatch, reg32, reg32, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceBltFast, reg32, reg32, reg32, reg32, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceDeleteAttachedSurface, reg32, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceEnumAttachedSurfaces, reg32, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceEnumOverlayZOrders, reg32, reg32, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceGetBltStatus, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceGetCaps, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceGetClipper, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceGetColorKey, reg32, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceGetDC, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceGetFlipStatus, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceGetOverlayPosition, reg32, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceGetPalette, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceInitialize, reg32, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceReleaseDC, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceSetOverlayPosition, reg32, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceUpdateOverlay, reg32, reg32, reg32, reg32, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceUpdateOverlayDisplay, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceUpdateOverlayZOrder, reg32, reg32)
NFS3HP_SURFACE_NOT_DONE(surfaceGetDDInterface, reg32)
NFS3HP_SURFACE_NOT_DONE(surfacePageLock, reg32)
NFS3HP_SURFACE_NOT_DONE(surfacePageUnlock, reg32)
#undef NFS3HP_SURFACE_NOT_DONE

template <auto Function>
void put(App* app, reg32 address, const char* name)
{
    app->registerMethod(address, { name, &win32::Wrapper<decltype(Function), Function>::stdcall });
}

reg32 initUnknownTable(App* app, reg32 at, x86::reg8* block)
{
    reg32* table = reinterpret_cast<reg32*>(block);
    put<&queryInterface>(app, 0x400000, "IUnknown::QueryInterface");
    put<&addRef>(app, 0x400001, "IUnknown::AddRef");
    put<&release>(app, 0x400002, "IUnknown::Release");
    s_unknownTable = at;
    table[0] = 0x400000;
    table[1] = 0x400001;
    table[2] = 0x400002;
    return 3 * sizeof(reg32);
}

void describeMode(SurfaceDesc* mode, reg32 width, reg32 height, reg32 bits)
{
    mode->size = sizeof(SurfaceDesc);
    mode->flags = 0x04100e;
    mode->height = height;
    mode->width = width;
    mode->pitch = std::int32_t(width * (bits / 8));
    mode->backBufferCount = 0;
    mode->refreshRate = 60;
    mode->alphaBitDepth = 0;
    mode->reserved = 0;
    mode->surface = 0;
    mode->destOverlay = { 0, 0 };
    mode->destBlt = { 0, 0 };
    mode->sourceOverlay = { 0, 0 };
    mode->sourceBlt = { 0, 0 };
    mode->format.size = sizeof(PixelFormat);
    mode->format.fourCC = 0;
    mode->format.alphaMask = 0;
    if (bits == 8)
    {
        mode->format.flags = 0x60;
        mode->format.bitCount = 8;
        mode->format.redMask = 0;
        mode->format.greenMask = 0;
        mode->format.blueMask = 0;
    }
    else
    {
        mode->format.flags = 0x40;
        mode->format.bitCount = 16;
        mode->format.redMask = 0xf800;
        mode->format.greenMask = 0x07e0;
        mode->format.blueMask = 0x001f;
    }
    mode->caps = 0;
}

reg32 initDrawTable(App* app, reg32 at, x86::reg8* block)
{
    reg32* table = reinterpret_cast<reg32*>(block);
    put<&drawQueryInterface>(app, 0x400003, "IDirectDraw2::QueryInterface");
    put<&drawCompact>(app, 0x400004, "IDirectDraw2::Compact");
    put<&drawCreateClipper>(app, 0x400005, "IDirectDraw2::CreateClipper");
    put<&drawCreatePalette>(app, 0x400006, "IDirectDraw2::CreatePalette");
    put<&drawCreateSurface>(app, 0x400007, "IDirectDraw2::CreateSurface");
    put<&drawDuplicateSurface>(app, 0x400008, "IDirectDraw2::DuplicateSurface");
    put<&drawEnumDisplayModes>(app, 0x400009, "IDirectDraw2::EnumDisplayModes");
    put<&drawEnumSurfaces>(app, 0x40000a, "IDirectDraw2::EnumSurfaces");
    put<&drawFlipToGdiSurface>(app, 0x40000b, "IDirectDraw2::FlipToGDISurface");
    put<&drawGetCaps>(app, 0x40000c, "IDirectDraw2::GetCaps");
    put<&drawGetDisplayMode>(app, 0x40000d, "IDirectDraw2::GetDisplayMode");
    put<&drawGetFourCCCodes>(app, 0x40000e, "IDirectDraw2::GetFourCCCodes");
    put<&drawGetGdiSurface>(app, 0x40000f, "IDirectDraw2::GetGDISurface");
    put<&drawGetMonitorFrequency>(app, 0x400010, "IDirectDraw2::GetMonitorFrequency");
    put<&drawGetScanLine>(app, 0x400011, "IDirectDraw2::GetScanLine");
    put<&drawGetVerticalBlankStatus>(app, 0x400012, "IDirectDraw2::GetVerticalBlankStatus");
    put<&drawInitialize>(app, 0x400013, "IDirectDraw2::Initialize");
    put<&drawRestoreDisplayMode>(app, 0x400014, "IDirectDraw2::RestoreDisplayMode");
    put<&drawSetCooperativeLevel>(app, 0x400015, "IDirectDraw2::SetCooperativeLevel");
    put<&drawSetDisplayMode>(app, 0x400016, "IDirectDraw2::SetDisplayMode");
    put<&drawWaitForVerticalBlank>(app, 0x400017, "IDirectDraw2::WaitForVerticalBlank");
    put<&drawGetAvailableVidMem>(app, 0x400018, "IDirectDraw2::GetAvailableVidMem");
    s_drawTable = at;
    const reg32 methods[24] = { 0x400003, 0x400001, 0x400002, 0x400004, 0x400005, 0x400006, 0x400007, 0x400008,
                                0x400009, 0x40000a, 0x40000b, 0x40000c, 0x40000d, 0x40000e, 0x40000f, 0x400010,
                                0x400011, 0x400012, 0x400013, 0x400014, 0x400015, 0x400016, 0x400017, 0x400018 };
    for (int i = 0; i < 24; ++i)
        table[i] = methods[i];

    constexpr reg32 kDescWords = sizeof(SurfaceDesc) / 4;
    describeMode(reinterpret_cast<SurfaceDesc*>(table + 24), 640, 480, 8);
    s_mode8At640 = at + 24 * sizeof(reg32);
    describeMode(reinterpret_cast<SurfaceDesc*>(table + 24 + kDescWords), 640, 480, 16);
    s_mode16At640 = at + 24 * sizeof(reg32) + sizeof(SurfaceDesc);
    describeMode(reinterpret_cast<SurfaceDesc*>(table + 24 + 2 * kDescWords), 800, 600, 8);
    s_mode8At800 = at + 24 * sizeof(reg32);
    describeMode(reinterpret_cast<SurfaceDesc*>(table + 24 + 3 * kDescWords), 800, 600, 16);
    s_mode16At800 = at + 24 * sizeof(reg32) + sizeof(SurfaceDesc);

    s_currentMode = s_mode16At800;
    return 24 * sizeof(reg32) + 4 * sizeof(SurfaceDesc);
}

reg32 initPaletteTable(App* app, reg32 at, x86::reg8* block)
{
    reg32* table = reinterpret_cast<reg32*>(block);
    put<&paletteGetCaps>(app, 0x400019, "IDirectDrawPalette::GetCaps");
    put<&paletteGetEntries>(app, 0x40001a, "IDirectDrawPalette::GetEntries");
    put<&paletteInitialize>(app, 0x40001b, "IDirectDrawPalette::Initialize");
    put<&paletteSetEntries>(app, 0x40001c, "IDirectDrawPalette::SetEntries");
    s_paletteTable = at;
    const reg32 methods[7] = { 0x400000, 0x400001, 0x400002, 0x400019, 0x40001a, 0x40001b, 0x40001c };
    for (int i = 0; i < 7; ++i)
        table[i] = methods[i];
    return 7 * sizeof(reg32);
}

reg32 initSurfaceTable(App* app, reg32 at, x86::reg8* block)
{
    reg32* table = reinterpret_cast<reg32*>(block);
    put<&surfaceQueryInterface>(app, 0x40001d, "IDirectDrawSurface::QueryInterface");
    put<&surfaceAddAttachedSurface>(app, 0x40001e, "IDirectDrawSurface::AddAttachedSurface");
    put<&surfaceAddOverlayDirtyRect>(app, 0x40001f, "IDirectDrawSurface::AddOverlayDirtyRect");
    put<&surfaceBlt>(app, 0x400020, "IDirectDrawSurface::Blt");
    put<&surfaceBltBatch>(app, 0x400021, "IDirectDrawSurface::BltBatch");
    put<&surfaceBltFast>(app, 0x400022, "IDirectDrawSurface::BltFast");
    put<&surfaceDeleteAttachedSurface>(app, 0x400023, "IDirectDrawSurface::DeleteAttachedSurface");
    put<&surfaceEnumAttachedSurfaces>(app, 0x400024, "IDirectDrawSurface::EnumAttachedSurfaces");
    put<&surfaceEnumOverlayZOrders>(app, 0x400025, "IDirectDrawSurface::EnumOverlayZOrders");
    put<&surfaceFlip>(app, 0x400026, "IDirectDrawSurface::Flip");
    put<&surfaceGetAttachedSurface>(app, 0x400027, "IDirectDrawSurface::GetAttachedSurface");
    put<&surfaceGetBltStatus>(app, 0x400028, "IDirectDrawSurface::GetBltStatus");
    put<&surfaceGetCaps>(app, 0x400029, "IDirectDrawSurface::GetCaps");
    put<&surfaceGetClipper>(app, 0x40002a, "IDirectDrawSurface::GetClipper");
    put<&surfaceGetColorKey>(app, 0x40002b, "IDirectDrawSurface::GetColorKey");
    put<&surfaceGetDC>(app, 0x40002c, "IDirectDrawSurface::GetDC");
    put<&surfaceGetFlipStatus>(app, 0x40002d, "IDirectDrawSurface::GetFlipStatus");
    put<&surfaceGetOverlayPosition>(app, 0x40002e, "IDirectDrawSurface::GetOverlayPosition");
    put<&surfaceGetPalette>(app, 0x40002f, "IDirectDrawSurface::GetPalette");
    put<&surfaceGetPixelFormat>(app, 0x400030, "IDirectDrawSurface::GetPixelFormat");
    put<&surfaceGetSurfaceDesc>(app, 0x400031, "IDirectDrawSurface::GetSurfaceDesc");
    put<&surfaceInitialize>(app, 0x400032, "IDirectDrawSurface::Initialize");
    put<&surfaceIsLost>(app, 0x400033, "IDirectDrawSurface::IsLost");
    put<&surfaceLock>(app, 0x400034, "IDirectDrawSurface::Lock");
    put<&surfaceReleaseDC>(app, 0x400035, "IDirectDrawSurface::ReleaseDC");
    put<&surfaceRestore>(app, 0x400036, "IDirectDrawSurface::Restore");
    put<&surfaceSetClipper>(app, 0x400037, "IDirectDrawSurface::SetClipper");
    put<&surfaceSetColorKey>(app, 0x400038, "IDirectDrawSurface::SetColorKey");
    put<&surfaceSetOverlayPosition>(app, 0x400039, "IDirectDrawSurface::SetOverlayPosition");
    put<&surfaceSetPalette>(app, 0x40003a, "IDirectDrawSurface::SetPalette");
    put<&surfaceUnlock>(app, 0x40003b, "IDirectDrawSurface::Unlock");
    put<&surfaceUpdateOverlay>(app, 0x40003c, "IDirectDrawSurface::UpdateOverlay");
    put<&surfaceUpdateOverlayDisplay>(app, 0x40003d, "IDirectDrawSurface::UpdateOverlayDisplay");
    put<&surfaceUpdateOverlayZOrder>(app, 0x40003e, "IDirectDrawSurface::UpdateOverlayZOrder");
    put<&surfaceGetDDInterface>(app, 0x40003f, "IDirectDrawSurface::GetDDInterface");
    put<&surfacePageLock>(app, 0x400040, "IDirectDrawSurface::PageLock");
    put<&surfacePageUnlock>(app, 0x400041, "IDirectDrawSurface::PageUnlock");
    s_surfaceTable = at;
    table[0] = 0x40001d;
    table[1] = 0x400001;
    table[2] = 0x400002;
    for (reg32 i = 3; i < 39; ++i)
        table[i] = 0x40001e + (i - 3);
    return 39 * sizeof(reg32);
}

/* The room the port's DirectSound tables had after these (IDirectSound's 11 words,
 * IDirectSoundBuffer's 21, IDirectSound3DListener's 19): kept, so the objects are where they were. */
constexpr reg32 kSoundTables = (11 + 21 + 19) * sizeof(reg32);

bool initialise(App* app)
{
    s_memory = new win32::MemMap(1024 * 1024 * 1);
    const reg32 base = s_memory->getBlockStart();
    reg32 offset = 0;
    x86::reg8* blockStart = &app->getMemory<x86::reg8>(base);
    x86::reg8* blockEnd = &app->getMemory<x86::reg8>(base + s_memory->getBlockSize() - 1);
    offset += initUnknownTable(app, base + offset, blockStart + offset);
    offset += initDrawTable(app, base + offset, blockStart + offset);
    offset += initPaletteTable(app, base + offset, blockStart + offset);
    offset += initSurfaceTable(app, base + offset, blockStart + offset);
    offset += kSoundTables;
    s_firstFree = base + offset;
    const reg32 blockSize = sizeof(Object);
    for (x86::reg8* block = blockStart + offset; block < blockEnd; block += blockSize, offset += blockSize)
        *reinterpret_cast<reg32*>(block) = base + offset + blockSize;
    return true;
}

/* ddraw.dll, as the game finds it by name. */
win32::Library s_library = win32::Library("ddraw.dll");
win32::Library* s_registry =
    &(s_library.registerSymbol("DirectDrawCreate", &win32::Wrapper<decltype(&directDrawCreate), &directDrawCreate>::stdcall));

}  // namespace

reg32 directDrawCreate(App* app, CPU& cpu, reg32 guid, reg32* made, reg32 outer)
{
    NFS2_USE(cpu);
    NFS2_USE(guid);
    static bool s_initialised = initialise(app);
    NFS2_USE(s_initialised);
    NFS2_ASSERT(outer == 0);
    *made = allocate(app);
    make(app, *made, s_drawTable, kIdDirectDraw3);
    return 0;
}

reg32 queryInterface(App* app, CPU& cpu, Object* self, const Guid* id, reg32* object)
{
    NFS2_USE(cpu);
    if (same(*id, self->interfaceId))
    {
        *object = addressOf(app, self);
        self->references++;
        return 0;
    }
    *object = 0;
    return 0x80004002;
}

reg32 addRef(App* app, CPU& cpu, Object* self)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return ++self->references;
}

reg32 release(App* app, CPU& cpu, Object* self)
{
    NFS2_USE(cpu);
    const reg32 result = --self->references;
    if (!result)
        freeBlock(app, self);
    return result;
}

reg32 drawQueryInterface(App* app, CPU& cpu, Object* self, const Guid* id, reg32* object)
{
    NFS2_USE(cpu);
    if (same(*id, kIdDirectDraw2) || same(*id, kIdDirectDraw3))
    {
        *object = addressOf(app, self);
        self->references++;
        return 0;
    }
    *object = 0;
    return 0x80004002;
}

reg32 drawCreatePalette(App* app, CPU& cpu, Object* self, reg32 flags, win32::PALETTEENTRY* entries, reg32* made,
                        reg32 outer)
{
    NFS2_USE(cpu);
    NFS2_ASSERT(outer == 0);
    *made = allocate(app);
    makePalette(app, *made, rendererOf(self), flags, entries);
    return 0;
}

reg32 drawCreateSurface(App* app, CPU& cpu, Object* self, SurfaceDesc* description, reg32* made, reg32 outer)
{
    NFS2_USE(cpu);
    NFS2_USE(description);
    NFS2_ASSERT(outer == 0);
    *made = allocate(app);
    makeSurface(app, *made, rendererOf(self), 0);
    return 0;
}

reg32 drawEnumDisplayModes(App* app, CPU& cpu, Object* self, reg32 flags, SurfaceDesc* description, reg32 context,
                           reg32 callback)
{
    NFS2_USE(self);
    NFS2_USE(flags);
    if (description)
    {
        NFS2_ASSERT(description->size == sizeof(SurfaceDesc));
    }
    for (const reg32 mode : { s_mode16At800, s_mode8At800, s_mode16At640, s_mode8At640 })
    {
        // The call's return address's slot holds the callback (as the code left it); stdcall (mode, context).
        app->getMemory<reg32>(cpu.esp - 12) = callback;
        x86::Args registers;
        guest::pointerCall(app, cpu, callback, registers, {mode, context});
    }
    return 0;
}

reg32 drawFlipToGdiSurface(App* app, CPU& cpu, Object* self)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(self);
    return 0;
}

reg32 drawGetCaps(App* app, CPU& cpu, Object* self, Caps* card, Caps* emulation)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(self);
    NFS2_ASSERT(card->size == sizeof(Caps));
    NFS2_ASSERT(emulation->size == sizeof(Caps));
    std::memset(card, 0, card->size);
    std::memset(emulation, 0, emulation->size);
    card->size = sizeof(Caps);
    return 0;
}

reg32 drawGetDisplayMode(App* app, CPU& cpu, Object* self, SurfaceDesc* description)
{
    NFS2_USE(cpu);
    NFS2_USE(self);
    const SurfaceDesc* mode = currentMode(app);
    std::memcpy(description, mode, mode->size);
    return 0;
}

reg32 drawSetCooperativeLevel(App* app, CPU& cpu, Object* self, reg32 window, reg32 flags)
{
    NFS2_USE(cpu);
    NFS2_USE(flags);
    win32::Window* shown = dynamic_cast<win32::Window*>(app->getResource(window));
    if (shown)
    {
        self->resource = new win32::Renderer(app, shown);
        app->allocateResource(self->resource);
    }
    return 0;
}

reg32 drawSetDisplayMode(App* app, CPU& cpu, Object* self, reg32 width, reg32 height, reg32 bits, reg32 refresh,
                         reg32 flags)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(refresh);
    NFS2_USE(flags);
    s_currentMode = bits == 8 ? (width == 640 ? s_mode8At640 : s_mode8At800)
                              : (width == 640 ? s_mode16At640 : s_mode16At800);
    rendererOf(self)->setVideoMode(width, height, bits);
    return 0;
}

reg32 drawGetAvailableVidMem(App* app, CPU& cpu, Object* self, reg32 caps, reg32* total, reg32* left)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(caps);
    *total = rendererOf(self)->getVideoMemorySize();
    *left = rendererOf(self)->getVideoMemorySize();
    return 0;
}

reg32 paletteSetEntries(App* app, CPU& cpu, Object* self, reg32 flags, reg32 first, reg32 count,
                        win32::PALETTEENTRY* entries)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(flags == 0);
    win32::Palette* palette = dynamic_cast<win32::Palette*>(self->resource);
    palette->setEntries(entries, first, count);
    return 0;
}

reg32 surfaceQueryInterface(App* app, CPU& cpu, Object* self, const Guid* id, reg32* object)
{
    NFS2_USE(cpu);
    if (same(*id, kIdSurface) || same(*id, kIdSurface2) || same(*id, kIdSurface3))
    {
        *object = addressOf(app, self);
        self->references++;
        return 0;
    }
    *object = 0;
    return 0x80004002;
}

reg32 surfaceBlt(App* app, CPU& cpu, Object* self, Rect* destination, Object* source, Rect* sourceRect, reg32 flags,
                 reg32 effects)
{
    NFS2_USE(flags);
    NFS2_USE(effects);
    NFS2_ASSERT(destination->top == 0);
    NFS2_ASSERT(destination->bottom == 480);
    NFS2_ASSERT(destination->left == 0);
    NFS2_ASSERT(destination->right == 640);
    NFS2_ASSERT(sourceRect->top == 0);
    NFS2_ASSERT(sourceRect->bottom == 480);
    NFS2_ASSERT(sourceRect->left == 0);
    NFS2_ASSERT(sourceRect->right == 640);
    SurfaceDesc destinationDesc;
    destinationDesc.size = sizeof(destinationDesc);
    const x86::reg8* sourceData = &app->getMemory<const x86::reg8>(surfaceOf(source)->lock());
    surfaceLock(app, cpu, self, nullptr, &destinationDesc, 0, 0);
    std::memcpy(&app->getMemory<x86::reg8>(destinationDesc.surface), sourceData,
                destinationDesc.width * destinationDesc.height * destinationDesc.format.bitCount / 8);
    surfaceUnlock(app, cpu, source, 0);
    surfaceUnlock(app, cpu, self, 0);
    return 1;
}

reg32 surfaceFlip(App* app, CPU& cpu, Object* self, reg32 target, reg32 flags)
{
    NFS2_USE(flags);
    NFS2_ASSERT(target == 0);
    app->unlockContext(cpu);
    surfaceOf(self)->getRenderer()->swap();
    app->lockContext(cpu);
    return 0;
}

reg32 surfaceGetAttachedSurface(App* app, CPU& cpu, Object* self, reg32 caps, reg32* attached)
{
    NFS2_USE(cpu);
    NFS2_USE(caps);
    *attached = allocate(app);
    makeSurface(app, *attached, surfaceOf(self)->getRenderer(), 1);
    return 0;
}

reg32 surfaceGetPixelFormat(App* app, CPU& cpu, Object* self, PixelFormat* format)
{
    NFS2_USE(cpu);
    NFS2_USE(self);
    std::memcpy(format, &currentMode(app)->format, sizeof(PixelFormat));
    return 0;
}

reg32 surfaceGetSurfaceDesc(App* app, CPU& cpu, Object* self, SurfaceDesc* description)
{
    NFS2_USE(cpu);
    NFS2_USE(self);
    const SurfaceDesc* mode = currentMode(app);
    std::memcpy(description, mode, mode->size);
    return 0;
}

reg32 surfaceIsLost(App* app, CPU& cpu, Object* self)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(self);
    return 0;
}

reg32 surfaceLock(App* app, CPU& cpu, Object* self, Rect* rect, SurfaceDesc* description, reg32 flags, reg32 event)
{
    NFS2_USE(cpu);
    NFS2_USE(flags);
    NFS2_ASSERT(rect == nullptr);
    NFS2_ASSERT(event == 0);
    std::memcpy(description, currentMode(app), description->size);
    description->surface = surfaceOf(self)->lock();
    return 0;
}

reg32 surfaceRestore(App* app, CPU& cpu, Object* self)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(self);
    NFS2_ASSERT(false);
    return 1;
}

reg32 surfaceSetClipper(App* app, CPU& cpu, Object* self, reg32 clipper)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(self);
    NFS2_USE(clipper);
    NFS2_ASSERT(false);
    return 1;
}

reg32 surfaceSetColorKey(App* app, CPU& cpu, Object* self, reg32 flags, ColourKey* key)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(self);
    NFS2_USE(flags);
    NFS2_USE(key);
    return 0;
}

reg32 surfaceSetPalette(App* app, CPU& cpu, Object* self, reg32 palette)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(self);
    NFS2_USE(palette);
    return 0;
}

reg32 surfaceUnlock(App* app, CPU& cpu, Object* self, reg32 bits)
{
    NFS2_USE(bits);
    app->unlockContext(cpu);
    surfaceOf(self)->unlock();
    app->lockContext(cpu);
    return 0;
}

}  // namespace ddraw

void installVideo(win32::WinApplication* app)
{
    ddraw::s_registry->registerSymbols(app);
}

}  // namespace host
}  // namespace nfs3hp
