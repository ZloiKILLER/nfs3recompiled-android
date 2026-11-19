#ifndef LIB_PALETTE_H_
#define LIB_PALETTE_H_

#include <lib/app.h>

namespace nfs
{

class Renderer;

/* A palette's colour, as the game keeps it. */
struct Colour
{
    nfs::u8 red;
    nfs::u8 green;
    nfs::u8 blue;
    nfs::u8 flags;
};

class Palette : public GenericResource
{
public:
    Palette(Renderer* renderer, const Colour* entries, nfs::u32 entryCount);
    ~Palette();

    void setEntries(const Colour* entries, nfs::u32 firstEntry, nfs::u32 entryCount);
private:
    nfs::u32      m_paletteSize;
    nfs::u32      m_palette[256];
    Renderer*       m_renderer;
};

}

#endif

