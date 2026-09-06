#ifndef LIB_GLIDERENDERER_H_
#define LIB_GLIDERENDERER_H_

#include <lib/winapp.h>
#include <winapi/types.h>

namespace win32
{

class Renderer;
class GlideTMU;

enum TextureFormat
{
    TF_RGB_565,
    TF_ARGB_1555,
    TF_ARGB_4444,
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
};

class GlideRenderer: public GenericResource
{
public:
    GlideRenderer(Renderer* renderer);
    ~GlideRenderer();

    void clear(x86::reg32 color);
    void swap();
    void render(x86::reg32 buffer);
    x86::reg32 textureMemStart(x86::reg32 tmu);
    x86::reg32 textureMemEnd(x86::reg32 tmu);
    x86::reg32 getTextureMemSize(x86::reg32 tmu, x86::reg32 largeMipmapSize, TextureFormat format);
    void setZWrite(bool enable);
    void setZTest(bool enable);
    /* grDepthBufferFunction(GR_CMP_ALWAYS/GR_CMP_LEQUAL).  Orthogonal to
     * setZTest(): it only changes the comparison used while the depth test
     * is enabled, not whether it runs at all. */
    void setDepthAlways(bool always);
    void setAlphaBlendDst(AlphaBlend method);
    void setDither(bool enable);
    void setCullMode(x86::reg32 mode);
    void setChromakeyMode(bool enable);
    /* Glide clamps or wraps each texture axis independently
     * (grTexClampMode).  Wrapping a texture the game asked to clamp
     * folds the opposite edge back into the picture, which is what a
     * headlight cone or a glow sprite looks like when it frays at the
     * borders. */
    void setTexClampMode(bool wrapS, bool wrapT);
    /* Draws everything queued so far into the framebuffer.  Normally only
     * swap() needs this; a texture upload that lands on a slot pending
     * geometry depends on has to force it early. */
    void renderPending();
    float texWrapAttribute() const;
    void setChromakeyValue(x86::reg32 color);
    void setClipWindow(x86::reg32 minX, x86::reg32 minY, x86::reg32 maxX, x86::reg32 maxY);
    void setGamma(float correction);
    void setColorFactor(float colorFactor);
    void setAlphaFactor(float alphaFactor);
    /* Glide fog.  The game configures all three and the port used to drop them
     * on the floor, which is why distant geometry stayed at full texture
     * brightness instead of dissolving into the horizon. */
    void setFogMode(x86::reg32 mode);
    void setFogColor(x86::reg32 color);
    void setFogTable(const x86::reg8* table);
    /* Glide alpha test.  The comparison is always GR_CMP_GREATER here (the
     * only function the game ever selects), so this is the reference the
     * fragment shader discards at or below.  It used to be a hardcoded
     * 1/16, which clipped the soft edge off headlight pools. */
    void setAlphaTestRef(x86::reg32 value);
    void setTexture(x86::reg32 tmu, x86::reg32 address);
    void setTextureData(x86::reg32 tmu, x86::reg32 address, const void* data,
                        x86::reg32 largeMipmap, x86::reg32 smallMimmap, TextureFormat format);
    void drawTriangle(const GrVertex* a, const GrVertex* b, const GrVertex* c);
    
private:
    void compileShaders();
    void flush();

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
    unsigned int            m_atlas;
    int                     m_attributes[6];
    int                     m_transform;
    //int                     m_textureBind;
    x86::reg32              m_textureOffsetX;
    x86::reg32              m_textureOffsetY;
    x86::reg32              m_textureOffsetW;
    float                   m_colorFactor;
    float                   m_alphaFactor;
    /* Fog factor for one vertex, from its 1/w.  Zero whenever fog is off, so
     * the shader's mix() becomes a no-op without a second code path. */
    float fogFactor(float oow) const;
    x86::reg32              m_fogMode;
    float                   m_fogColor[3];
    /* GR_FOG_TABLE_SIZE entries, already normalised to 0..1. */
    float                   m_fogTable[64];
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
