#ifndef LIB_GLIDETMU_H_
#define LIB_GLIDETMU_H_

#include <lib/winapp.h>
#include <winapi/types.h>

namespace win32
{

struct GlTextureSlot
{
    GlTextureSlot*  next;
    x86::reg32      lod;
    x86::reg32      x;
    x86::reg32      y;
    /* How many mipmap levels the game actually supplied for the texture
     * currently in this slot.  Tiles are power-of-two aligned to their
     * own size, so level L of a tile at (x, y) sits at (x >> L, y >> L)
     * in atlas level L and can never overlap a neighbour. */
    x86::reg32      mipLevels;
    /* Render pass this slot was last selected for.  Tile coordinates are
     * baked into the vertices, but the vertices are not drawn until the
     * end of the frame, so overwriting a slot that pending geometry still
     * points at makes that geometry sample whatever arrived instead. */
    x86::reg32      useStamp;
};

class GlideTMU
{
public:
    GlideTMU(x86::reg32 atlasSize);
    ~GlideTMU();

    x86::reg32 getTextureMemSize() const { return m_videoMemorySize; }

    /* Gives a slot back.  A slot whose three quadrant neighbours are all free
     * again is merged with them into the slot they were split from, and so on
     * up, so a 256x256 tile broken up for small textures is whole again once
     * they are gone -- otherwise every race that loads different textures
     * would leave fewer whole tiles for the next. */
    void returnTextureSlot(GlTextureSlot* slot);
    /* A free slot for a texture of 256 >> lod texels, or nullptr when neither
     * that size nor anything larger to split is left. */
    GlTextureSlot* reserveTextureSlot(x86::reg32 lod);

    x86::reg32 textureMemStart() const;
    x86::reg32 textureMemEnd() const;
    GlTextureSlot** getTextureInfo(x86::reg32 address);

    /* Free space, for diagnostics: texels on the free lists and how many
     * whole 256x256 tiles are among them -- the second number is what
     * decides whether one more full-size texture still fits. */
    void freeSpace(x86::reg32& texels, x86::reg32& wholeTiles) const;
    x86::reg32 atlasTexels() const { return m_atlasSize * m_atlasSize; }

private:
    struct SlotChunk;
    GlTextureSlot* allocateTextureSlot();
    void freeTextureSlot(GlTextureSlot* slot);
    void breakdownTextureSlot(GlTextureSlot* slot);
    /* Unlinks the free slot of this level at (x, y), if there is one. */
    GlTextureSlot* takeFreeSlot(x86::reg32 lod, x86::reg32 x, x86::reg32 y);

private:
    x86::reg32      m_videoMemorySize;
    GlTextureSlot** m_textureSlots;
    SlotChunk*      m_slotChunks;
    GlTextureSlot*  m_firstFreeTextureSlot;
    GlTextureSlot** m_textureMem;
    x86::reg32      m_atlasSize;
};

}

#endif
