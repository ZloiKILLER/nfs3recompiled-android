#ifndef LIB_GLIDERENDERER_H_
#define LIB_GLIDERENDERER_H_

#include <lib/winapp.h>
#include <winapi/types.h>
#include <array>
#include <vector>

namespace win32
{

class Renderer;
class GlideTMU;

enum TextureFormat
{
    TF_RGB_565,
    TF_ARGB_1555,
    TF_ARGB_4444,
    TF_ARGB_8888,   // four bytes a texel; the rest are two
};

enum AlphaBlend
{
    AB_1mSrcAlpha,
    AB_1
};

static const x86::reg32 RENDERER_NUM_TMU    = 2;

struct GrTmuVertex
{
    float  sow;
    float  tow;
    float  oow;
};

struct GrVertex
{
    float x, y, z;
    float r, g, b;
    float ooz;
    float a;
    float oow;
    GrTmuVertex  tmuvtx[RENDERER_NUM_TMU];
};

struct GlVertex;
struct PendingDraw;

struct DrawCall
{
    x86::reg32 lastTriangle;
    bool zTest;
    /* GR_CMP_ALWAYS (grDepthBufferFunction) vs the default GR_CMP_LEQUAL --
     * only meaningful while zTest is true.  Real hardware still writes depth
     * on an "always passes" comparison (per grDepthMask); collapsing this
     * into a full depth-test disable would also suppress the write, unlike
     * real Glide. */
    bool zTestAlways;
    bool zBuffer;
    bool alphaBlend;
    bool dither;
    bool cull;
    bool chromaKey;
    x86::reg32 cullMode;
    x86::reg32 clipMinX;
    x86::reg32 clipMinY;
    x86::reg32 clipMaxX;
    x86::reg32 clipMaxY;
    float gamma;
    /* Which of GlideRenderer::m_fogTables the vertex shader fogs with. */
    int fogTable;
};

/* What the renderer was handed since the last look (NFS_TICK_TRACE). */
struct GlideCounters
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

/* Whether the shader writes depth in the Voodoo's 16-bit steps for a depth
 * buffer of this many bits on this GPU (gliderenderer.cpp). */
bool needsDepthSteps(int depthBits, const char* renderer);

/* What glide2x.cpp draws through: the Voodoo2 emulated over a texture atlas
 * (GlideRenderer), or the native THRASH driver's own renderer, with a texture
 * of OpenGL's for each of the game's (ThrashRenderer, thrashrenderer.h). */
class GlideBackend: public GenericResource
{
public:
    virtual void clear(x86::reg32 color) = 0;
    virtual void swap() = 0;
    virtual void render(x86::reg32 buffer) = 0;
    virtual void atlasFreeSpace(x86::reg32& texels, x86::reg32& wholeTiles, x86::reg32& total) const = 0;
    virtual x86::reg32 getTextureMemSize(x86::reg32 tmu, x86::reg32 largeMipmapSize, TextureFormat format) = 0;
    virtual void setZWrite(bool enable) = 0;
    virtual void setZTest(bool enable) = 0;
    virtual void setDepthAlways(bool always) = 0;
    virtual void setAlphaBlendDst(AlphaBlend method) = 0;
    virtual void setDither(bool enable) = 0;
    virtual void setCullMode(x86::reg32 mode) = 0;
    virtual void setChromakeyMode(bool enable) = 0;
    virtual void setTexClampMode(bool wrapS, bool wrapT) = 0;
    virtual void setChromakeyValue(x86::reg32 color) = 0;
    virtual void setClipWindow(x86::reg32 minX, x86::reg32 minY, x86::reg32 maxX, x86::reg32 maxY) = 0;
    virtual void setGamma(float correction) = 0;
    virtual void setColorFactor(float colorFactor) = 0;
    virtual void setAlphaFactor(float alphaFactor) = 0;
    virtual void setFogMode(x86::reg32 mode) = 0;
    virtual void setFogColor(x86::reg32 color) = 0;
    virtual void setFogTable(const x86::reg8* table) = 0;
    virtual void setAlphaTestRef(x86::reg32 value) = 0;
    virtual void setTexture(x86::reg32 tmu, x86::reg32 address) = 0;
    virtual void setTextureData(x86::reg32 tmu, x86::reg32 address, const void* data,
                                x86::reg32 largeMipmap, x86::reg32 smallMimmap, TextureFormat format) = 0;
    virtual void drawTriangle(const GrVertex* a, const GrVertex* b, const GrVertex* c) = 0;
    /* THRASH_treset: every texture given up.  Glide has no such call; the
     * atlas takes a slot back when its address is downloaded to again. */
    virtual void resetTextures() {}
};

class GlideRenderer: public GlideBackend
{
public:
    static GlideCounters takeCounters();
    /* The same counters, for ThrashRenderer to count into. */
    static GlideCounters& counters() { return s_counters; }

    /* preferredAtlasSize: 2048 or 4096, see glide2x::setPreferredAtlasSize. */
    GlideRenderer(Renderer* renderer, x86::reg32 preferredAtlasSize);
    ~GlideRenderer();

    void clear(x86::reg32 color) override;
    void swap() override;
    void render(x86::reg32 buffer) override;
    x86::reg32 textureMemStart(x86::reg32 tmu);
    x86::reg32 textureMemEnd(x86::reg32 tmu);
    /* Free texels and whole free 256x256 tiles in the texture atlas, and its
     * size in texels -- see GlideTMU::freeSpace. */
    void atlasFreeSpace(x86::reg32& texels, x86::reg32& wholeTiles, x86::reg32& total) const override;
    x86::reg32 getTextureMemSize(x86::reg32 tmu, x86::reg32 largeMipmapSize, TextureFormat format) override;
    void setZWrite(bool enable) override;
    void setZTest(bool enable) override;
    /* grDepthBufferFunction(GR_CMP_ALWAYS/GR_CMP_LEQUAL).  Orthogonal to
     * setZTest(): it only changes the comparison used while the depth test
     * is enabled, not whether it runs at all. */
    void setDepthAlways(bool always) override;
    void setAlphaBlendDst(AlphaBlend method) override;
    void setDither(bool enable) override;
    void setCullMode(x86::reg32 mode) override;
    void setChromakeyMode(bool enable) override;
    /* Glide clamps or wraps each texture axis independently
     * (grTexClampMode).  Wrapping a texture the game asked to clamp
     * folds the opposite edge back into the picture, which is what a
     * headlight cone or a glow sprite looks like when it frays at the
     * borders. */
    void setTexClampMode(bool wrapS, bool wrapT) override;
    /* Draws everything queued so far into the framebuffer.  Normally only
     * swap() needs this; a texture upload that lands on a slot pending
     * geometry depends on has to force it early. */
    void renderPending();
    void setChromakeyValue(x86::reg32 color) override;
    void setClipWindow(x86::reg32 minX, x86::reg32 minY, x86::reg32 maxX, x86::reg32 maxY) override;
    void setGamma(float correction) override;
    void setColorFactor(float colorFactor) override;
    void setAlphaFactor(float alphaFactor) override;
    /* Glide fog.  The game configures all three and the port used to drop them
     * on the floor, which is why distant geometry stayed at full texture
     * brightness instead of dissolving into the horizon. */
    void setFogMode(x86::reg32 mode) override;
    void setFogColor(x86::reg32 color) override;
    void setFogTable(const x86::reg8* table) override;
    /* Glide alpha test.  The comparison is always GR_CMP_GREATER here (the
     * only function the game ever selects), so this is the reference the
     * fragment shader discards at or below.  It used to be a hardcoded
     * 1/16, which clipped the soft edge off headlight pools. */
    void setAlphaTestRef(x86::reg32 value) override;
    void setTexture(x86::reg32 tmu, x86::reg32 address) override;
    void setTextureData(x86::reg32 tmu, x86::reg32 address, const void* data,
                        x86::reg32 largeMipmap, x86::reg32 smallMimmap, TextureFormat format) override;
    void drawTriangle(const GrVertex* a, const GrVertex* b, const GrVertex* c) override;
    
private:
    static GlideCounters s_counters;
    /* The GL work of the constructor, clear(), renderPending(), on the GL
     * thread (lib/glthread.h).  What the game's thread keeps -- the queue of
     * vertices, the tile allocator, the settings -- is never touched there;
     * what they need of it goes with them. */
    void initGl(x86::reg32 preferredAtlasSize);
    void clearFrame(x86::reg32 color, x86::reg32 clipMinX, x86::reg32 clipMinY,
                    x86::reg32 clipMaxX, x86::reg32 clipMaxY);
    void drawPending(const PendingDraw& draw);
    void compileShaders();
    unsigned int linkProgram(unsigned int vertexShader, bool depth16);
    /* The attribute and uniform locations of m_shaderProgram. */
    void locateShaderInputs();
    /* NFS_DEPTH16=ab: the program for the next frame, with or without the
     * 16-bit depth steps, in turns of a few seconds. */
    void takeDepthTurn();
    void flush();
    /* NFS_TEXCOORD_TRACE: a triangle whose texture coordinates are far from
     * the origin, counted and sampled with the state it is drawn in. */
    void traceFarTriangle(const GrVertex* const corners[3], bool farS, bool farT);

private:
    Renderer*               m_renderer;
    unsigned int            m_depthBuffer;
    unsigned int            m_framebuffer;
    GlVertex*               m_vertices;
    x86::reg32              m_vertexCount;
    std::vector<DrawCall>   m_drawCalls;
    unsigned int            m_vertexBuffer;
    unsigned int            m_vertexArray;
    unsigned int            m_shaderProgram;
    /* Without and with the 16-bit depth steps; only the one in use is built
     * unless NFS_DEPTH16=ab takes turns with both. */
    unsigned int            m_shaderPrograms[2];
    bool                    m_depth16;
    bool                    m_depthTurns;
    /* The GPU's bilinear filter for texels inside their tile (NFS_GPU_FILTER),
     * through this linear sampler on texture unit 1. */
    bool                    m_gpuFilter;
    unsigned int            m_linearSampler;
    unsigned int            m_atlas;
    x86::reg32              m_atlasSize;
    int                     m_attributes[6];
    int                     m_transform;
    //int                     m_textureBind;
    x86::reg32              m_textureOffsetX;
    x86::reg32              m_textureOffsetY;
    x86::reg32              m_textureOffsetW;
    float                   m_colorFactor;
    float                   m_alphaFactor;
    x86::reg32              m_fogMode;
    float                   m_fogColor[3];
    /* The fog tables the queued draw calls use, GR_FOG_TABLE_SIZE entries each,
     * normalised to 0..1 (the vertex shader interpolates in them), and the one
     * in force.  Back to that one alone after every renderPending(). */
    std::vector<std::array<float, 64>> m_fogTables;
    int                     m_fogTableIndex;
    int                     m_fogTableUniform;
    /* Raw copy of the last table passed to setFogTable(), so a repeat can be
     * detected with memcmp instead of always paying the flush() below -- see
     * the comment on setFogTable() in the .cpp for why that matters. */
    x86::reg8               m_fogTableRaw[64];
    bool                    m_fogTableValid;
    int                     m_fogColorUniform;
    float                   m_alphaTestRef;
    int                     m_alphaRefUniform;
    int                     m_ditherUniform;
    int                     m_chromaKeyUniform;
    int                     m_chromaKeyColorUniform;
    int                     m_gammaUniform;
    bool                    m_zBuffer;
    bool                    m_zTest;
    bool                    m_depthAlways;
    bool                    m_alphaBlend;
    bool                    m_dither;
    bool                    m_cull;
    bool                    m_chromaKey;
    x86::reg32              m_textureMipLevels;
    x86::reg32              m_renderStamp;
    bool                    m_texWrapS;
    bool                    m_texWrapT;
    x86::reg32              m_cullMode;
    x86::reg32              m_chromaKeyColor;
    x86::reg32              m_clipMinX;
    x86::reg32              m_clipMinY;
    x86::reg32              m_clipMaxX;
    x86::reg32              m_clipMaxY;
    float                   m_gamma;
    GlideTMU*               m_tmus[2];
    /* Scratch for the packed-pixel conversion in setTextureData(), sized once
     * in the constructor to the largest mipmap level (256x256x4 bytes) and
     * reused for every upload -- this used to malloc/free that buffer on
     * every single texture upload. */
    x86::reg8*              m_textureUploadScratch;
};

}

#endif
