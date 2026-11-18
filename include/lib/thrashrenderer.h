#ifndef LIB_THRASHRENDERER_H_
#define LIB_THRASHRENDERER_H_

#include <lib/winapp.h>
#include <array>
#include <unordered_map>
#include <vector>

namespace win32
{

class Renderer;

/* The texture formats the game's textures come in. */
enum TextureFormat
{
    TF_RGB_565,
    TF_ARGB_1555,
    TF_ARGB_4444,
    TF_ARGB_8888,   // four bytes a texel; the rest are two
};

/* Where blending takes the destination by: one less the source's alpha, or
 * whole. */
enum AlphaBlend
{
    AB_1mSrcAlpha,
    AB_1
};

/* What the renderer was handed since the last look (NFS_TICK_TRACE). */
struct DrawCounters
{
    unsigned triangles = 0;
    unsigned fogged = 0;
    unsigned fogTables = 0;   // a table that differs from the one before
    unsigned drawCalls = 0;   // batches with triangles in them
    unsigned emptyCalls = 0;  // state changes with no triangle since the last
    unsigned cut = 0;         // clamped far out, cut along the texture's edges
    unsigned frames = 0;
    unsigned depth16Frames = 0;  // drawn with the depth in the Voodoo's 16-bit steps
};

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

/* The THRASH driver's renderer: what the game draws through it, straight to
 * OpenGL ES, with nothing of the Voodoo2 emulated under it.
 *
 * Each of the game's textures is a texture of OpenGL's, with the mipmap chain
 * the game supplies, and the GPU filters, wraps and clamps it in one fetch.
 * Only a texture drawn with a chroma key is still filtered by hand: the
 * Voodoo2 keyed texels out before filtering, which no GPU filter does.  The
 * triangles are batched by their state and drawn on the GL thread
 * (lib/glthread.h), with the Voodoo's fog table, its 16-bit depth steps where
 * the GPU's depth is finer, its clip window, and far texture coordinates
 * brought back. */
class ThrashRenderer: public GenericResource
{
public:
    explicit ThrashRenderer(Renderer* renderer);
    ~ThrashRenderer();

    void clear(x86::reg32 color);
    void swap();
    void render(x86::reg32 buffer);
    void setZWrite(bool enable);
    void setZTest(bool enable);
    /* The depth compared ALWAYS rather than LEQUAL while the test is on: it
     * still writes, as the Voodoo did (the rear-view mirror resets its
     * rectangle's depth so). */
    void setDepthAlways(bool always);
    void setAlphaBlendDst(AlphaBlend method);
    void setDither(bool enable);
    void setCullMode(x86::reg32 mode);
    void setChromakeyMode(bool enable);
    /* Each texture axis wrapped or clamped. */
    void setTexClampMode(bool wrapS, bool wrapT);
    void setChromakeyValue(x86::reg32 color);
    void setClipWindow(x86::reg32 minX, x86::reg32 minY, x86::reg32 maxX, x86::reg32 maxY);
    void setGamma(float correction);
    void setColorFactor(float colorFactor);
    void setAlphaFactor(float alphaFactor);
    void setFogMode(x86::reg32 mode);
    void setFogColor(x86::reg32 color);
    void setFogTable(const x86::reg8* table);
    /* The alpha the fragment must pass (the game compares GREATER). */
    void setAlphaTestRef(x86::reg32 value);
    /* The texture downloaded to `address`. */
    void setTexture(x86::reg32 tmu, x86::reg32 address);
    void setTextureData(x86::reg32 tmu, x86::reg32 address, const void* data,
                        x86::reg32 largeMipmap, x86::reg32 smallMimmap, TextureFormat format);
    /* THRASH_treset: every texture given up. */
    void resetTextures();

    /* A triangle: room for it in the batch, which the driver fills -- x, y, z
     * (Glide's 1/z), the colour as bytes, r | g << 8 | b << 16 | a << 24, u
     * and v (s/w, t/w) and 1/w -- and then hands to finishTriangle, which
     * brings far texture coordinates back and marks the three with the
     * combine and the fog they are drawn with.  Nothing else may be drawn in
     * between. */
    ThrashVertex* reserveTriangle()
    {
        if (m_vertexCount + 3 > s_maxVertices)
            renderPending();
        return m_vertices + m_vertexCount;
    }
    void finishTriangle(ThrashVertex* corners);

    static const x86::reg32 s_maxVertices = 128000;

    /* The counters since the last look, and those counted into. */
    static DrawCounters takeCounters();
    static DrawCounters& counters() { return s_counters; }

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

    /* What a watcher of the renderer sees: all it hands the GL thread, as it hands it.  None
     * in the game. */
    struct Watcher
    {
        virtual ~Watcher() = default;
        virtual void drawn(const ThrashDraw& draw) = 0;
        virtual void uploaded(x86::reg32 index, x86::reg32 size, x86::reg32 levels, TextureFormat format,
                              bool fresh, const std::vector<x86::reg8>& texels) = 0;
        virtual void cleared(x86::reg32 color, x86::reg32 clipMinX, x86::reg32 clipMinY, x86::reg32 clipMaxX,
                             x86::reg32 clipMaxY) = 0;
        virtual void swapped() = 0;
        virtual void rendered(x86::reg32 buffer) = 0;
        virtual void released(const std::vector<x86::reg32>& indices) = 0;
    };
    static void watch(Watcher* watcher) { s_watcher = watcher; }
    /* The game thread's side of the renderer, every word of it: what is
     * pending, the textures as it keeps them, the state it draws with -- for a
     * check to compare after each call of the driver. */
    void snapshot(std::vector<x86::reg32>& out) const;

private:
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

    static DrawCounters s_counters;
    static Watcher* s_watcher;

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

/* What renderPending() hands the GL thread: the vertices, the batches and the
 * state the whole draw shares. */
struct ThrashDraw
{
    ThrashVertex*                       vertices;
    x86::reg32                          vertexCount;
    std::vector<ThrashRenderer::Batch>  batches;
    std::vector<std::array<float, 64>>  fogTables;
    float                               fogColor[3];
    float                               alphaTestRef;
    x86::reg32                          chromaKeyColor;
    x86::reg32                          width;
    x86::reg32                          height;
};

}

#endif
