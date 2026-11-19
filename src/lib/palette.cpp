#include <lib/palette.h>
#include <lib/renderer.h>

namespace nfs
{

Palette::Palette(Renderer* renderer, const Colour* entries, nfs::u32 entryCount)
    :   m_paletteSize(entryCount)
    ,   m_renderer(renderer)
{
    setEntries(entries, 0, entryCount);
}

Palette::~Palette()
{
}

void Palette::setEntries(const Colour *entries, nfs::u32 firstEntry, nfs::u32 entryCount)
{
    NFS2_ASSERT(firstEntry + entryCount <= m_paletteSize);
    for (nfs::u32 i = 0; i < entryCount; ++i)
    {
        m_palette[firstEntry + i] = entries[i].red << 24| entries[i].green << 16 | entries[i].blue << 8 | 0xff ;
    }
    m_renderer->updatePalette(m_paletteSize, m_palette);
}

}
