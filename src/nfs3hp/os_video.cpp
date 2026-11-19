#include "os_video.h"

#include <lib/palette.h>
#include <lib/renderer.h>
#include <lib/surface.h>
#include <lib/window.h>

namespace nfs3hp
{
namespace os
{
namespace video
{

namespace
{

/* A display: the renderer it shows in once it has a window. */
class Display : public nfs::GenericResource
{
public:
    nfs::Renderer* renderer = nullptr;
};

static_assert(sizeof(Colour) == 4, "a palette's colour as the game keeps it");

constexpr Format describe(nfs::u32 bits)
{
    return { 640, 480, std::int32_t(640 * (bits / 8)), bits, bits == 8 ? 0u : 0xf800u, bits == 8 ? 0u : 0x07e0u,
             bits == 8 ? 0u : 0x001fu, 60 };
}

constexpr std::array<Format, 4> kModes = { describe(16), describe(8), describe(16), describe(8) };

/* The bits a pixel of the mode the display is in. */
nfs::u32 s_bits = 16;

template <typename T>
T* object(App* app, Handle handle)
{
    return dynamic_cast<T*>(app->getResource(handle));
}

nfs::Renderer* rendererOf(App* app, Handle display)
{
    Display* const shown = object<Display>(app, display);
    return shown != nullptr ? shown->renderer : nullptr;
}

Handle surfaceOf(App* app, nfs::Renderer* renderer, nfs::u32 buffer)
{
    return app->allocateResource(new nfs::Surface(renderer, buffer));
}

}  // namespace

const std::array<Format, 4>& modes()
{
    return kModes;
}

Format mode()
{
    return describe(s_bits);
}

Handle openDisplay(App* app)
{
    return app->allocateResource(new Display());
}

void showIn(App* app, Handle display, Handle window)
{
    Display* const shown = object<Display>(app, display);
    nfs::Window* const in = object<nfs::Window>(app, window);
    if (shown == nullptr || in == nullptr)
        return;
    shown->renderer = new nfs::Renderer(app, in);
    app->allocateResource(shown->renderer);
}

void setMode(App* app, Handle display, nfs::u32 width, nfs::u32 height, nfs::u32 bits)
{
    s_bits = bits == 8 ? 8 : 16;
    if (nfs::Renderer* const renderer = rendererOf(app, display))
        renderer->setVideoMode(width, height, bits);
}

Handle makeSurface(App* app, Handle display)
{
    return surfaceOf(app, rendererOf(app, display), 0);
}

Handle backBuffer(App* app, Handle surface)
{
    nfs::Surface* const front = object<nfs::Surface>(app, surface);
    return surfaceOf(app, front != nullptr ? front->getRenderer() : nullptr, 1);
}

std::optional<Locked> lock(App* app, Handle surface)
{
    nfs::Surface* const locked = object<nfs::Surface>(app, surface);
    if (locked == nullptr)
        return std::nullopt;
    return Locked{ locked->lock(), mode() };
}

void unlock(App* app, nfs::ThreadState& ts, Handle surface)
{
    nfs::Surface* const locked = object<nfs::Surface>(app, surface);
    if (locked == nullptr)
        return;
    app->unlockContext(ts);
    locked->unlock();
    app->lockContext(ts);
}

Handle makePalette(App* app, Handle display, const Colour* colours)
{
    nfs::Palette* const palette =
        new nfs::Palette(rendererOf(app, display), colours, 256);
    return app->allocateResource(palette);
}

void setPalette(App* app, Handle palette, nfs::u32 first, nfs::u32 count, const Colour* colours)
{
    object<nfs::Palette>(app, palette)->setEntries(colours, first, count);
}

void release(App* app, Handle handle)
{
    if (handle != 0 && app->getResource(handle) != nullptr)
        app->freeResource(handle);
}

}  // namespace video
}  // namespace os
}  // namespace nfs3hp
