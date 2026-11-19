#include <lib/surface.h>
#include <lib/renderer.h>

namespace nfs
{

Surface::Surface(Renderer* renderer, nfs::u32 bufferIndex)
    :   m_renderer(renderer)
    ,   m_surfaceIndex(bufferIndex)
{
}

Surface::~Surface()
{
}

Renderer* Surface::getRenderer()
{
    return m_renderer;
}

nfs::u32 Surface::lock()
{
    return m_renderer->lock(m_surfaceIndex);
}

void Surface::unlock()
{
    m_renderer->unlock(m_surfaceIndex);
}

}
