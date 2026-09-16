#include <lib/glidetmu.h>

namespace win32
{

static const x86::reg32 s_slotChunkSize = 1024;
static const x86::reg32 s_atlasLodCount = 9;

/* Slot records, a chunk at a time.  They used to come from one pool of 2048
 * with nothing checking for its end: enough for a 2048 atlas split the way the
 * game's own textures split it, not for a 4096 one full of small textures. */
struct GlideTMU::SlotChunk
{
    SlotChunk*    next;
    GlTextureSlot slots[s_slotChunkSize];
};

GlideTMU::GlideTMU(x86::reg32 atlasSize)
    :   m_textureSlots(new GlTextureSlot*[s_atlasLodCount])
    ,   m_slotChunks(nullptr)
    ,   m_firstFreeTextureSlot(nullptr)
    ,   m_textureMem(new GlTextureSlot*[atlasSize])
    ,   m_atlasSize(atlasSize)
{
    for (x86::reg32 i = 0; i < s_atlasLodCount; ++i)
    {
        m_textureSlots[i] = nullptr;
    }
    for (x86::reg32 i = 0;i < (atlasSize/256) * (atlasSize/256); ++i)
    {
        GlTextureSlot* slot = allocateTextureSlot();
        slot->lod = 0;
        slot->x = (256 * i) % atlasSize;
        slot->y = 256 * (256 * i / atlasSize);
        slot->next = m_textureSlots[0];
        m_textureSlots[0] = slot;
    }
    memset(m_textureMem, 0, atlasSize*sizeof(GlTextureSlot*));
}

GlideTMU::~GlideTMU()
{
    delete[] m_textureSlots;
    while (m_slotChunks)
    {
        SlotChunk* next = m_slotChunks->next;
        delete m_slotChunks;
        m_slotChunks = next;
    }
    delete[] m_textureMem;
}

x86::reg32 GlideTMU::textureMemStart() const
{
    return 0;
}

x86::reg32 GlideTMU::textureMemEnd() const
{
    return 4096*1024;
}

GlTextureSlot** GlideTMU::getTextureInfo(x86::reg32 address)
{
    return m_textureMem + address / sizeof(GlTextureSlot*);
}

void GlideTMU::freeSpace(x86::reg32& texels, x86::reg32& wholeTiles) const
{
    texels = 0;
    wholeTiles = 0;
    for (x86::reg32 lod = 0; lod < s_atlasLodCount; ++lod)
    {
        const x86::reg32 size = 256 >> lod;
        for (const GlTextureSlot* slot = m_textureSlots[lod]; slot; slot = slot->next)
        {
            texels += size * size;
            if (lod == 0)
                ++wholeTiles;
        }
    }
}

GlTextureSlot* GlideTMU::allocateTextureSlot()
{
    if (!m_firstFreeTextureSlot)
    {
        SlotChunk* chunk = new SlotChunk;
        chunk->next = m_slotChunks;
        m_slotChunks = chunk;
        for (x86::reg32 i = 0; i < s_slotChunkSize; ++i)
        {
            chunk->slots[i].next = (i + 1 < s_slotChunkSize) ? &chunk->slots[i + 1] : nullptr;
        }
        m_firstFreeTextureSlot = chunk->slots;
    }
    GlTextureSlot* result = m_firstFreeTextureSlot;
    m_firstFreeTextureSlot = result->next;
    result->lod = 0;
    result->x = 0;
    result->y = 0;
    result->mipLevels = 1;
    result->useStamp = 0;
    return result;
}

void GlideTMU::freeTextureSlot(GlTextureSlot* slot)
{
    slot->next = m_firstFreeTextureSlot;
    m_firstFreeTextureSlot = slot;
}

void GlideTMU::breakdownTextureSlot(GlTextureSlot* slot)
{
    slot->lod++;
    x86::reg32 size = 256 >> slot->lod;
    slot->next = m_textureSlots[slot->lod];
    m_textureSlots[slot->lod] = slot;
    GlTextureSlot* s = allocateTextureSlot();
    s->lod = slot->lod;
    s->x = slot->x + size;
    s->y = slot->y;
    s->next = m_textureSlots[slot->lod];
    m_textureSlots[slot->lod] = s;
    s = allocateTextureSlot();
    s->lod = slot->lod;
    s->x = slot->x;
    s->y = slot->y + size;
    s->next = m_textureSlots[slot->lod];
    m_textureSlots[slot->lod] = s;
    s = allocateTextureSlot();
    s->lod = slot->lod;
    s->x = slot->x + size;
    s->y = slot->y + size;
    s->next = m_textureSlots[slot->lod];
    m_textureSlots[slot->lod] = s;
}

GlTextureSlot* GlideTMU::reserveTextureSlot(x86::reg32 lod)
{
    if (lod >= s_atlasLodCount)
        return nullptr;
    GlTextureSlot* result = m_textureSlots[lod];
    if (!result)
    {
        for (x86::sreg32 parentLod = lod-1; parentLod >= 0; --parentLod)
        {
            GlTextureSlot* parent = m_textureSlots[parentLod];
            if (parent)
            {
                m_textureSlots[parentLod] = parent->next;
                for (x86::reg32 childLod = parentLod; childLod < lod-1; ++childLod)
                {
                    breakdownTextureSlot(parent);
                    parent = m_textureSlots[childLod + 1];
                    m_textureSlots[childLod + 1] = parent->next;
                }
                breakdownTextureSlot(parent);
                break;
            }
        }
        result = m_textureSlots[lod];
        /* Nothing this size and nothing larger to split: the atlas is full.
         * This used to go on and follow the null pointer. */
        if (!result)
            return nullptr;
    }
    m_textureSlots[lod] = result->next;
    return result;
}

GlTextureSlot* GlideTMU::takeFreeSlot(x86::reg32 lod, x86::reg32 x, x86::reg32 y)
{
    for (GlTextureSlot** link = &m_textureSlots[lod]; *link; link = &(*link)->next)
    {
        if ((*link)->x == x && (*link)->y == y)
        {
            GlTextureSlot* slot = *link;
            *link = slot->next;
            return slot;
        }
    }
    return nullptr;
}

void GlideTMU::returnTextureSlot(GlTextureSlot* slot)
{
    /* Merge back up while all four quadrants of the slot this one was split
     * from are free.  Slots only ever split into aligned quadrants of 256x256
     * tiles laid out from the origin, so the parent's corner is this slot's
     * position rounded down to twice its size. */
    while (slot->lod > 0)
    {
        const x86::reg32 size = 256 >> slot->lod;
        const x86::reg32 parentX = slot->x - slot->x % (size * 2);
        const x86::reg32 parentY = slot->y - slot->y % (size * 2);
        GlTextureSlot* buddies[3];
        x86::reg32 found = 0;
        for (x86::reg32 quadrant = 0; quadrant < 4; ++quadrant)
        {
            const x86::reg32 x = parentX + (quadrant & 1) * size;
            const x86::reg32 y = parentY + (quadrant >> 1) * size;
            if (x == slot->x && y == slot->y)
                continue;
            GlTextureSlot* buddy = takeFreeSlot(slot->lod, x, y);
            if (!buddy)
                break;
            buddies[found++] = buddy;
        }
        if (found < 3)
        {
            for (x86::reg32 i = 0; i < found; ++i)
            {
                buddies[i]->next = m_textureSlots[slot->lod];
                m_textureSlots[slot->lod] = buddies[i];
            }
            break;
        }
        for (x86::reg32 i = 0; i < 3; ++i)
        {
            freeTextureSlot(buddies[i]);
        }
        slot->lod--;
        slot->x = parentX;
        slot->y = parentY;
    }
    slot->next = m_textureSlots[slot->lod];
    m_textureSlots[slot->lod] = slot;
}

}
