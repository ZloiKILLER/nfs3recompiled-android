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

    void returnTextureSlot(GlTextureSlot* slot);
    GlTextureSlot* reserveTextureSlot(x86::reg32 lod);

    x86::reg32 textureMemStart() const;
    x86::reg32 textureMemEnd() const;
    GlTextureSlot** getTextureInfo(x86::reg32 address);

private:
    GlTextureSlot* allocateTextureSlot();
    void freeTextureSlot(GlTextureSlot* slot);
    void breakdownTextureSlot(GlTextureSlot* slot);

private:
    x86::reg32      m_videoMemorySize;
    GlTextureSlot** m_textureSlots;
    GlTextureSlot*  m_textureSlotPool;
    GlTextureSlot*  m_firstFreeTextureSlot;
    GlTextureSlot** m_textureMem;
};

}

#endif
