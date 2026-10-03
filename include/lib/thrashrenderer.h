#ifndef LIB_THRASHRENDERER_H_
#define LIB_THRASHRENDERER_H_

#include <lib/gliderenderer.h>
#include <array>
#include <unordered_map>
#include <vector>

namespace win32
{

/* 32 bytes: position and Glide's depth, the colour, s/w, t/w and 1/w, and the
 * colour and alpha combine factors (0 or 1) and whether it is fogged. */
struct ThrashVertex
{
    float           x, y, z;
    x86::reg32      color;
    float           u, v, oow;
    x86::reg8       combine[4];
};
static_assert(sizeof(ThrashVertex) == 32, "the attribute offsets in thrashrenderer.cpp assume this layout");

struct ThrashDraw;

/* The native THRASH driver's renderer: what the game draws through voodoo2a,
 * straight to OpenGL ES, with nothing of the Voodoo2 emulated under it.
 *
 * GlideRenderer keeps every texture in one atlas, as the Voodoo2 kept them in
 * its texture memory, and its shader filters, wraps and clamps each texel by
 * hand, four fetches a pixel, so neighbouring tiles never bleed into each
 * other.  Here each of the game's textures is a texture of OpenGL's, with the
 * mipmap chain the game supplies, and the GPU does all of that in one fetch.
 * Only a texture drawn with a chroma key is still filtered by hand: the
 * Voodoo2 keyed texels out before filtering, which no GPU filter does.
 *
 * Everything else -- the batching, the GL thread, the fog table, the 16-bit
 * depth steps, the clip window, the far texture coordinates brought back -- is
 * GlideRenderer's, so the picture is the same up to how a GPU rounds its
 * bilinear weights.  NFS_THRASH_GL=0 draws through GlideRenderer instead. */
class ThrashRenderer: public GlideBackend
{
public:
    explicit ThrashRenderer(Renderer* renderer);
    ~ThrashRenderer();

    void clear(x86::reg32 color) override;
    void swap() override;
    void render(x86::reg32 buffer) override;
    void atlasFreeSpace(x86::reg32& texels, x86::reg32& wholeTiles, x86::reg32& total) const override;
    x86::reg32 getTextureMemSize(x86::reg32 tmu, x86::reg32 largeMipmapSize, TextureFormat format) override;
    void setZWrite(bool enable) override;
    void setZTest(bool enable) override;
    void setDepthAlways(bool always) override;
    void setAlphaBlendDst(AlphaBlend method) override;
    void setDither(bool enable) override;
    void setCullMode(x86::reg32 mode) override;
    void setChromakeyMode(bool enable) override;
    void setTexClampMode(bool wrapS, bool wrapT) override;
    void setChromakeyValue(x86::reg32 color) override;
    void setClipWindow(x86::reg32 minX, x86::reg32 minY, x86::reg32 maxX, x86::reg32 maxY) override;
    void setGamma(float correction) override;
    void setColorFactor(float colorFactor) override;
    void setAlphaFactor(float alphaFactor) override;
    void setFogMode(x86::reg32 mode) override;
    void setFogColor(x86::reg32 color) override;
    void setFogTable(const x86::reg8* table) override;
    void setAlphaTestRef(x86::reg32 value) override;
    void setTexture(x86::reg32 tmu, x86::reg32 address) override;
    void setTextureData(x86::reg32 tmu, x86::reg32 address, const void* data,
                        x86::reg32 largeMipmap, x86::reg32 smallMimmap, TextureFormat format) override;
    void drawTriangle(const GrVertex* a, const GrVertex* b, const GrVertex* c) override;
    void resetTextures() override;

    /* A triangle's corner as the native THRASH driver hands it over, worked
     * out straight from the game's vertex (nfs3hp::thrashCorner): what
     * drawTriangle keeps of a GrVertex, the colour already as bytes,
     * r | g << 8 | b << 16 | a << 24. */
    struct Corner
    {
        float x, y, ooz, oow, sow, tow;
        x86::reg32 color;
    };
    void drawCorners(const Corner& a, const Corner& b, const Corner& c);

    /* The native driver's way in, without a Corner between: room for one
     * triangle in the batch, which the caller fills -- x, y, z (Glide's ooz),
     * color, u and v (s/w, t/w) and oow -- and then hands to finishTriangle,
     * which brings far texture coordinates back and marks the three as
     * drawCorners does.  Nothing else may be drawn in between. */
    ThrashVertex* reserveTriangle()
    {
        if (m_vertexCount + 3 > s_maxVertices)
            renderPending();
        return m_vertices + m_vertexCount;
    }
    void finishTriangle(ThrashVertex* corners);

    static const x86::reg32 s_maxVertices = 128000;

private:
    /* One batch: the state it is drawn with, and the triangles up to `end`. */
    struct Batch
    {
        x86::reg32 end;
        x86::reg32 texture;     // index into m_glTextures on the GL thread
        x86::reg32 levels;      // its mipmap levels, for the hand-filtered chroma key
        int format;             // 0 RGBA8, 1 565, 2 1555, 3 4444: how the chroma key compares
        bool wrapS, wrapT;
        bool zTest, zTestAlways, zBuffer, alphaBlend, cull, chromaKey;
        x86::reg32 cullMode;
        x86::reg32 clipMinX, clipMinY, clipMaxX, clipMaxY;
        float gamma;
        int fogTable;
    };
    friend struct ThrashDraw;

    /* A texture of the game's, as the game thread knows it: the index of its
     * GL texture, its size and levels, and the render pass it was last
     * selected in. */
    struct Texture
    {
        x86::reg32 index;
        x86::reg32 size;
        x86::reg32 levels;
        x86::reg32 useStamp;
        int format;
    };

    void initGl();
    void compileShaders(int depthBits, const char* renderer);
    void clearFrame(x86::reg32 color, x86::reg32 clipMinX, x86::reg32 clipMinY,
                    x86::reg32 clipMaxX, x86::reg32 clipMaxY);
    void flush();
    void renderPending();
    void drawPending(const ThrashDraw& draw);
    void upload(x86::reg32 index, x86::reg32 size, x86::reg32 levels, TextureFormat format,
                std::vector<x86::reg8>& texels);

private:
    Renderer*               m_renderer;
    // The GL thread's.
    unsigned int            m_depthBuffer;
    unsigned int            m_framebuffer;
    unsigned int            m_vertexBuffer;
    unsigned int            m_vertexArray;
    unsigned int            m_program;
    unsigned int            m_samplers[4];  // [wrapS + 2 * wrapT]
    std::vector<unsigned int> m_glTextures;
    struct
    {
        int position, color, texCoord, combine;
        int transform, fogW, fogTable, fogColor, alphaRef, chromaKey, chromaKeyColor, gamma, wrap, levels, texture,
            format;
    } m_locations;
    bool                    m_depth16;

    // The game thread's.
    ThrashVertex*           m_vertices;
    x86::reg32              m_vertexCount;
    std::vector<Batch>      m_batches;
    std::unordered_map<x86::reg32, Texture> m_textures;  // by the address the game downloaded to
    x86::reg32              m_nextTexture;
    x86::reg32              m_texture;
    x86::reg32              m_textureLevels;
    int                     m_textureFormat;
    x86::reg32              m_renderStamp;
    std::vector<std::array<float, 64>> m_fogTables;
    int                     m_fogTableIndex;
    x86::reg8               m_fogTableRaw[64];
    bool                    m_fogTableValid;
    x86::reg32              m_fogMode;
    float                   m_fogColor[3];
    float                   m_alphaTestRef;
    float                   m_colorFactor;
    float                   m_alphaFactor;
    bool                    m_zBuffer;
    bool                    m_zTest;
    bool                    m_depthAlways;
    bool                    m_alphaBlend;
    bool                    m_cull;
    bool                    m_chromaKey;
    bool                    m_wrapS;
    bool                    m_wrapT;
    x86::reg32              m_cullMode;
    x86::reg32              m_chromaKeyColor;
    x86::reg32              m_clipMinX;
    x86::reg32              m_clipMinY;
    x86::reg32              m_clipMaxX;
    x86::reg32              m_clipMaxY;
    float                   m_gamma;
};

}

#endif
