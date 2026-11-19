#ifndef LIB_SURFACE_H_
#define LIB_SURFACE_H_

#include <lib/app.h>
#include <lib/memmap.h>

struct SDL_Texture;

namespace nfs
{

class Renderer;

class Surface : public GenericResource
{
public:
    Surface(Renderer* renderer, nfs::u32 bufferIndex);
    ~Surface();

    Renderer* getRenderer();
    nfs::u32 lock();
    void unlock();

private:
    Renderer*   m_renderer;
    nfs::u32  m_surfaceIndex;
};

}

#endif

